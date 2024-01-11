/*! @file
  @brief
  Realtime multitask monitor for mruby/c

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
//@endcond


/***** Local headers ********************************************************/
#include "alloc.h"
#include "load.h"
#include "class.h"
#include "global.h"
#include "symbol.h"
#include "vm.h"
#include "console.h"
#include "rrt0.h"
#include "hal_selector.h"


/***** Macros ***************************************************************/
#ifndef MRBC_SCHEDULER_EXIT
#define MRBC_SCHEDULER_EXIT 0
#endif

#define VM2TCB(p) ((mrbc_tcb *)((uint8_t *)p - offsetof(mrbc_tcb, vm)))
#define MRBC_MUTEX_TRACE(...) ((void)0)


/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
static mrbc_tcb *q_dormant_;
static mrbc_tcb *q_ready_;
static mrbc_tcb *q_waiting_;
static mrbc_tcb *q_suspended_;
static volatile uint32_t tick_;


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/

//================================================================
/*! Insert to task queue

  @param        Pointer of target TCB

  引数で指定されたタスク(TCB)を、状態別Queueに入れる。
  TCBはフリーの状態でなければならない。（別なQueueに入っていてはならない）
  Queueはpriority_preemption順にソート済みとなる。
  挿入するTCBとQueueに同じpriority_preemption値がある場合は、同値の最後に挿入される。

 */
static void q_insert_task(mrbc_tcb *p_tcb)
{
  mrbc_tcb **pp_q;

  switch( p_tcb->state ) {
  case TASKSTATE_DORMANT: pp_q   = &q_dormant_; break;
  case TASKSTATE_READY:
  case TASKSTATE_RUNNING: pp_q   = &q_ready_; break;
  case TASKSTATE_WAITING: pp_q   = &q_waiting_; break;
  case TASKSTATE_SUSPENDED: pp_q = &q_suspended_; break;
  default:
    assert(!"Wrong task state.");
    return;
  }

  // case insert on top.
  if((*pp_q == NULL) ||
     (p_tcb->priority_preemption < (*pp_q)->priority_preemption)) {
    p_tcb->next = *pp_q;
    *pp_q       = p_tcb;
    assert(p_tcb->next != p_tcb);
    return;
  }

  // find insert point in sorted linked list.
  mrbc_tcb *p = *pp_q;
  while( 1 ) {
    if((p->next == NULL) ||
       (p_tcb->priority_preemption < p->next->priority_preemption)) {
      p_tcb->next = p->next;
      p->next     = p_tcb;
      assert(p->next != p);
      return;
    }

    p = p->next;
  }
}


//================================================================
/*! Delete from task queue

  @param        Pointer of target TCB

  Queueからタスク(TCB)を取り除く。

 */
static void q_delete_task(mrbc_tcb *p_tcb)
{
  mrbc_tcb **pp_q;

  switch( p_tcb->state ) {
  case TASKSTATE_DORMANT: pp_q   = &q_dormant_; break;
  case TASKSTATE_READY:
  case TASKSTATE_RUNNING: pp_q   = &q_ready_; break;
  case TASKSTATE_WAITING: pp_q   = &q_waiting_; break;
  case TASKSTATE_SUSPENDED: pp_q = &q_suspended_; break;
  default:
    assert(!"Wrong task state.");
    return;
  }

  if( *pp_q == NULL ) return;
  if( *pp_q == p_tcb ) {
    *pp_q       = p_tcb->next;
    p_tcb->next = NULL;
    return;
  }

  mrbc_tcb *p = *pp_q;
  while( p ) {
    if( p->next == p_tcb ) {
      p->next     = p_tcb->next;
      p_tcb->next = NULL;
      return;
    }

    p = p->next;
  }
}


//================================================================
/*! 一定時間停止（cruby互換）

*/
static void c_sleep(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = VM2TCB(vm);

  if( argc == 0 ) {
    mrbc_suspend_task(tcb);
    return;
  }

  switch( mrbc_type(v[1]) ) {
  case MRBC_TT_INTEGER:
  {
    mrbc_int_t sec;
    sec = mrbc_integer(v[1]);
    SET_INT_RETURN(sec);
    mrbc_sleep_ms(tcb, sec * 1000);
    break;
  }

#if MRBC_USE_FLOAT
  case MRBC_TT_FLOAT:
  {
    mrbc_float_t sec;
    sec = mrbc_float(v[1]);
    SET_INT_RETURN(sec);
    mrbc_sleep_ms(tcb, (mrbc_int_t)(sec * 1000));
    break;
  }
#endif

  default:
    break;
  }
}


//================================================================
/*! 一定時間停止（ms単位）

*/
static void c_sleep_ms(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = VM2TCB(vm);

  mrbc_int_t sec = mrbc_integer(v[1]);
  SET_INT_RETURN(sec);
  mrbc_sleep_ms(tcb, sec);
}


//================================================================
/*! 実行権を手放す (BETA)

*/
static void c_relinquish(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = VM2TCB(vm);

  mrbc_relinquish(tcb);
}


//================================================================
/*! プライオリティー変更

*/
static void c_change_priority(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = VM2TCB(vm);

  mrbc_change_priority(tcb, mrbc_integer(v[1]));
}


//================================================================
/*! 実行停止 (BETA)

*/
static void c_suspend_task(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( argc == 0 ) {
    mrbc_tcb *tcb = VM2TCB(vm);
    mrbc_suspend_task(tcb);	// suspend self.
    return;
  }

  if( mrbc_type(v[1]) != MRBC_TT_HANDLE ) return;	// error.
  mrbc_suspend_task( (mrbc_tcb *)(v[1].handle) );
}


//================================================================
/*! 実行再開 (BETA)

*/
static void c_resume_task(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if( mrbc_type(v[1]) != MRBC_TT_HANDLE ) return;	// error.
  mrbc_resume_task( (mrbc_tcb *)(v[1].handle) );
}


//================================================================
/*! TCBを得る (BETA)

*/
static void c_get_tcb(mrbc_vm *vm, mrbc_value v[], int argc)
{
  mrbc_tcb *tcb = VM2TCB(vm);

  mrbc_value value = {.tt = MRBC_TT_HANDLE};
  value.handle = tcb;

  SET_RETURN( value );
}


//================================================================
/*! mutex constructor method

*/
static void c_mutex_new(mrbc_vm *vm, mrbc_value v[], int argc)
{
  *v = mrbc_instance_new(vm, v->cls, sizeof(mrbc_mutex));
  if( !v->instance ) return;

  mrbc_mutex_init( (mrbc_mutex *)(v->instance->data) );
}


//================================================================
/*! mutex lock method

*/
static void c_mutex_lock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int r = mrbc_mutex_lock( (mrbc_mutex *)v->instance->data, VM2TCB(vm) );
  if( r == 0 ) return;  // return self

  // raise ThreadError
  assert(!"Mutex recursive lock.");
}


//================================================================
/*! mutex unlock method

*/
static void c_mutex_unlock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int r = mrbc_mutex_unlock( (mrbc_mutex *)v->instance->data, VM2TCB(vm) );
  if( r == 0 ) return;  // return self

  // raise ThreadError
  assert(!"Mutex unlock error. not owner or not locked.");
}


//================================================================
/*! mutex trylock method

*/
static void c_mutex_trylock(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int r = mrbc_mutex_trylock( (mrbc_mutex *)v->instance->data, VM2TCB(vm) );
  if( r == 0 ) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}


//================================================================
/*! vm tick
*/
static void c_vm_tick(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(tick_);
}



/***** Global functions *****************************************************/

//================================================================
/*! Tick timer interrupt handler.

*/
void mrbc_tick(void)
{
  mrbc_tcb *tcb;
  int flag_preemption = 0;

  tick_++;

  // 実行中タスクのタイムスライス値を減らす
  tcb = q_ready_;
  if((tcb != NULL) &&
     (tcb->state == TASKSTATE_RUNNING) &&
     (tcb->timeslice > 0)) {
    tcb->timeslice--;
    if( tcb->timeslice == 0 ) tcb->vm.flag_preemption = 1;
  }

  // 待ちタスクキューから、ウェイクアップすべきタスクを探す
  tcb = q_waiting_;
  while( tcb != NULL ) {
    mrbc_tcb *t = tcb;
    tcb = tcb->next;

    if( t->reason == TASKREASON_SLEEP && t->wakeup_tick == tick_ ) {
      q_delete_task(t);
      t->state     = TASKSTATE_READY;
      t->timeslice = MRBC_TIMESLICE_TICK_COUNT;
      q_insert_task(t);
      flag_preemption = 1;
    }
  }

  if( flag_preemption ) {
    tcb = q_ready_;
    while( tcb != NULL ) {
      if( tcb->state == TASKSTATE_RUNNING ) tcb->vm.flag_preemption = 1;
      tcb = tcb->next;
    }
  }
}



//================================================================
/*! initialize

*/
void mrbc_init(uint8_t *ptr, unsigned int size)
{
  hal_init();
  mrbc_init_alloc(ptr, size);
  mrbc_init_global();
  mrbc_init_class();


  // TODO 関数呼び出しが、c_XXX => mrbc_XXX の daisy chain になっている。
  //      不要な複雑さかもしれない。要リファクタリング。
  mrbc_define_method(0, mrbc_class_object, "sleep",           c_sleep);
  mrbc_define_method(0, mrbc_class_object, "sleep_ms",        c_sleep_ms);
  mrbc_define_method(0, mrbc_class_object, "relinquish",      c_relinquish);
  mrbc_define_method(0, mrbc_class_object, "change_priority", c_change_priority);
  mrbc_define_method(0, mrbc_class_object, "suspend_task",    c_suspend_task);
  mrbc_define_method(0, mrbc_class_object, "resume_task",     c_resume_task);
  mrbc_define_method(0, mrbc_class_object, "get_tcb",	      c_get_tcb);


  mrbc_class *c_mutex;
  c_mutex = mrbc_define_class(0, "Mutex", mrbc_class_object);
  mrbc_define_method(0, c_mutex, "new", c_mutex_new);
  mrbc_define_method(0, c_mutex, "lock", c_mutex_lock);
  mrbc_define_method(0, c_mutex, "unlock", c_mutex_unlock);
  mrbc_define_method(0, c_mutex, "try_lock", c_mutex_trylock);

  mrbc_class *c_vm;
  c_vm = mrbc_define_class(0, "VM", mrbc_class_object);
  mrbc_define_method(0, c_vm, "tick", c_vm_tick);
}


//================================================================
/*! clenaup all resources.

*/
void mrbc_cleanup(void)
{
  mrbc_cleanup_vm();
  mrbc_cleanup_symbol();
  mrbc_cleanup_alloc();

  q_dormant_ = 0;
  q_ready_ = 0;
  q_waiting_ = 0;
  q_suspended_ = 0;
}


//================================================================
/*! create (allocate) TCB.

  @param  regs_size	num of allocated registers.
  @param  task_state	task initial state.
  @param  priority	task priority.
  @return pointer to TCB or NULL.

<b>Code example</b>
@code
  //  If you want specify default value, see below.
  //    regs_size:  MAX_REGS_SIZE (in vm_config.h)
  //    task_state: MRBC_TASK_DEFAULT_STATE
  //    priority:   MRBC_TASK_DEFAULT_PRIORITY
  mrbc_tcb *tcb;
  tcb = mrbc_tcb_new( MAX_REGS_SIZE, MRBC_TASK_DEFAULT_STATE, MRBC_TASK_DEFAULT_PRIORITY );
  mrbc_create_task( byte_code, tcb );
@endcode
*/
mrbc_tcb * mrbc_tcb_new( int regs_size, enum MrbcTaskState task_state, int priority )
{
  mrbc_tcb *tcb;

  tcb = mrbc_raw_alloc( sizeof(mrbc_tcb) + sizeof(mrbc_value) * regs_size );
  if( !tcb ) return NULL;	// ENOMEM

  memset(tcb, 0, sizeof(mrbc_tcb));
#if defined(MRBC_DEBUG)
  memcpy( tcb->type, "TCB", 4 );
#endif
  tcb->priority = priority;
  tcb->state = task_state;
  tcb->vm.regs_size = regs_size;

  return tcb;
}


//================================================================
/*! specify running VM code.

  @param  byte_code	pointer to VM byte code.
  @param  tcb		Task control block with parameter, or NULL.
  @return Pointer to mrbc_tcb or NULL.
*/
mrbc_tcb * mrbc_create_task(const void *byte_code, mrbc_tcb *tcb)
{
  if( !tcb ) tcb = mrbc_tcb_new( MAX_REGS_SIZE, MRBC_TASK_DEFAULT_STATE, MRBC_TASK_DEFAULT_PRIORITY );
  if( !tcb ) return NULL;	// ENOMEM

  tcb->timeslice = MRBC_TIMESLICE_TICK_COUNT;
  tcb->priority_preemption = tcb->priority;

  // assign VM ID
  if( mrbc_vm_open( &tcb->vm ) == NULL ) {
    mrbc_printf("Error: Can't assign VM-ID.\n");
    return NULL;
  }

  if( mrbc_load_mrb(&tcb->vm, byte_code) != 0 ) {
    mrbc_print_vm_exception( &tcb->vm );
    mrbc_vm_close( &tcb->vm );
    return NULL;
  }
  if( tcb->state != TASKSTATE_DORMANT ) {
    mrbc_vm_begin( &tcb->vm );
  }

  hal_disable_irq();
  q_insert_task(tcb);
  hal_enable_irq();

  return tcb;
}


//================================================================
/*! Start execution of dormant task.

  @param	tcb	Task control block with parameter, or NULL.
  @retval	int	zero / no error.
*/
int mrbc_start_task(mrbc_tcb *tcb)
{
  if( tcb->state != TASKSTATE_DORMANT ) return -1;
  tcb->timeslice           = MRBC_TIMESLICE_TICK_COUNT;
  tcb->priority_preemption = tcb->priority;
  mrbc_vm_begin(&tcb->vm);

  hal_disable_irq();

  mrbc_tcb *t = q_ready_;
  while( t != NULL ) {
    if( t->state == TASKSTATE_RUNNING ) t->vm.flag_preemption = 1;
    t = t->next;
  }

  q_delete_task(tcb);
  tcb->state = TASKSTATE_READY;
  q_insert_task(tcb);
  hal_enable_irq();

  return 0;
}


//================================================================
/*! execute

*/
int mrbc_run(void)
{
  int ret = 1;

#if MRBC_SCHEDULER_EXIT
  if( q_ready_ == NULL && q_waiting_ == NULL && q_suspended_ == NULL ) return 0;
#endif

  while( 1 ) {
    mrbc_tcb *tcb = q_ready_;
    if( tcb == NULL ) {		// no task to run.
      hal_idle_cpu();
      continue;
    }

    tcb->state = TASKSTATE_RUNNING;	// to execute.
    int res = 0;

#ifndef MRBC_NO_TIMER
    tcb->vm.flag_preemption = 0;
    res = mrbc_vm_run(&tcb->vm);

#else
    while( tcb->timeslice > 0 ) {
      tcb->vm.flag_preemption = 1;
      res = mrbc_vm_run(&tcb->vm);
      tcb->timeslice--;
      if( res != 0 ) break;
      if( tcb->state != TASKSTATE_RUNNING ) break;
    }
    mrbc_tick();
#endif /* ifndef MRBC_NO_TIMER */

    // did the task done?
    if( res != 0 ) {
      hal_disable_irq();
      q_delete_task(tcb);
      tcb->state = TASKSTATE_DORMANT;
      q_insert_task(tcb);
      hal_enable_irq();
      if( tcb->vm.flag_permanence == 0 ) mrbc_vm_end(&tcb->vm);
      if( res != 1 ) ret = res;

#if MRBC_SCHEDULER_EXIT
      if( q_ready_ == NULL && q_waiting_ == NULL && q_suspended_ == NULL ) break;
#endif
      continue;
    }

    // switch task.
    hal_disable_irq();
    if( tcb->state == TASKSTATE_RUNNING ) {
      tcb->state = TASKSTATE_READY;

      if( tcb->timeslice == 0 ) {
        q_delete_task(tcb);
        tcb->timeslice = MRBC_TIMESLICE_TICK_COUNT;
        q_insert_task(tcb); // insert task on queue last.
      }
    }
    hal_enable_irq();
  }

  return ret;
}


//================================================================
/*! 実行一時停止

*/
void mrbc_sleep_ms(mrbc_tcb *tcb, uint32_t ms)
{
  hal_disable_irq();
  q_delete_task(tcb);
  tcb->timeslice   = 0;
  tcb->state       = TASKSTATE_WAITING;
  tcb->reason      = TASKREASON_SLEEP;
  tcb->wakeup_tick = tick_ + (ms / MRBC_TICK_UNIT) + 1;
  if( ms % MRBC_TICK_UNIT ) tcb->wakeup_tick++;
  q_insert_task(tcb);
  hal_enable_irq();

  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! 実行権を手放す

*/
void mrbc_relinquish(mrbc_tcb *tcb)
{
  tcb->timeslice           = 0;
  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! プライオリティーの変更
  TODO: No check, yet.
*/
void mrbc_change_priority(mrbc_tcb *tcb, int priority)
{
  tcb->priority            = (uint8_t)priority;
  tcb->priority_preemption = (uint8_t)priority;
  tcb->timeslice           = 0;
  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! 実行停止

*/
void mrbc_suspend_task(mrbc_tcb *tcb)
{
  hal_disable_irq();
  q_delete_task(tcb);
  tcb->state = TASKSTATE_SUSPENDED;
  q_insert_task(tcb);
  hal_enable_irq();

  tcb->vm.flag_preemption = 1;
}


//================================================================
/*! 実行再開

*/
void mrbc_resume_task(mrbc_tcb *tcb)
{
  hal_disable_irq();

  mrbc_tcb *t = q_ready_;
  while( t != NULL ) {
    if( t->state == TASKSTATE_RUNNING ) t->vm.flag_preemption = 1;
    t = t->next;
  }

  q_delete_task(tcb);
  tcb->state = TASKSTATE_READY;
  q_insert_task(tcb);
  hal_enable_irq();
}


//================================================================
/*! mutex initialize

*/
mrbc_mutex * mrbc_mutex_init( mrbc_mutex *mutex )
{
  if( mutex == NULL ) {
    mutex = mrbc_raw_alloc( sizeof(mrbc_mutex) );
    if( mutex == NULL ) return NULL;	// ENOMEM
  }

  static const mrbc_mutex init_val = MRBC_MUTEX_INITIALIZER;
  *mutex = init_val;

  return mutex;
}


//================================================================
/*! mutex lock

*/
int mrbc_mutex_lock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex lock / MUTEX: %p TCB: %p",  mutex, tcb );

  int ret = 0;
  hal_disable_irq();

  // Try lock mutex;
  if( mutex->lock == 0 ) {      // a future does use TAS?
    mutex->lock = 1;
    mutex->tcb = tcb;
    MRBC_MUTEX_TRACE("  lock OK\n" );
    goto DONE;
  }
  MRBC_MUTEX_TRACE("  lock FAIL\n" );

  // Can't lock mutex
  // check recursive lock.
  if( mutex->tcb == tcb ) {
    ret = 1;
    goto DONE;
  }

  // To WAITING state.
  q_delete_task(tcb);
  tcb->state  = TASKSTATE_WAITING;
  tcb->reason = TASKREASON_MUTEX;
  tcb->mutex = mutex;
  q_insert_task(tcb);
  tcb->vm.flag_preemption = 1;

 DONE:
  hal_enable_irq();

  return ret;
}


//================================================================
/*! mutex unlock

*/
int mrbc_mutex_unlock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex unlock / MUTEX: %p TCB: %p\n",  mutex, tcb );

  // check some parameters.
  if( !mutex->lock ) return 1;
  if( mutex->tcb != tcb ) return 2;

  // wakeup ONE waiting task.
  int flag_preemption = 0;
  hal_disable_irq();
  tcb = q_waiting_;
  while( tcb != NULL ) {
    if( tcb->reason == TASKREASON_MUTEX && tcb->mutex == mutex ) {
      MRBC_MUTEX_TRACE("SW: TCB: %p\n", tcb );
      mutex->tcb = tcb;
      q_delete_task(tcb);
      tcb->state = TASKSTATE_READY;
      q_insert_task(tcb);
      flag_preemption = 1;
      break;
    }
    tcb = tcb->next;
  }

  if( flag_preemption ) {
    tcb = q_ready_;
    while( tcb != NULL ) {
      if( tcb->state == TASKSTATE_RUNNING ) tcb->vm.flag_preemption = 1;
      tcb = tcb->next;
    }
  }
  else {
    // unlock mutex
    MRBC_MUTEX_TRACE("mutex unlock all.\n" );
    mutex->lock = 0;
  }

  hal_enable_irq();

  return 0;
}


//================================================================
/*! mutex trylock

*/
int mrbc_mutex_trylock( mrbc_mutex *mutex, mrbc_tcb *tcb )
{
  MRBC_MUTEX_TRACE("mutex try lock / MUTEX: %p TCB: %p",  mutex, tcb );

  int ret;
  hal_disable_irq();

  if( mutex->lock == 0 ) {
    mutex->lock = 1;
    mutex->tcb = tcb;
    ret = 0;
    MRBC_MUTEX_TRACE("  trylock OK\n" );
  }
  else {
    MRBC_MUTEX_TRACE("  trylock FAIL\n" );
    ret = 1;
  }

  hal_enable_irq();
  return ret;
}




#ifdef MRBC_DEBUG

//================================================================
/*! DEBUG print queue

 */
void pq(mrbc_tcb *p_tcb)
{
  mrbc_tcb *p;

  p = p_tcb;
  while( p != NULL ) {
#if defined(UINTPTR_MAX)
    mrbc_printf("%08x  ", (uint32_t)(uintptr_t)p);
#else
    mrbc_printf("%08x  ", (uint32_t)p);
#endif
    p = p->next;
  }
  mrbc_printf("\n");

  p = p_tcb;
  while( p != NULL ) {
#if defined(UINTPTR_MAX)
    mrbc_printf(" nx:%04x  ", (uint16_t)(uintptr_t)p->next);
#else
    mrbc_printf(" nx:%04x  ", (uint16_t)p->next);
#endif
    p = p->next;
  }
  mrbc_printf("\n");

  p = p_tcb;
  while( p != NULL ) {
    mrbc_printf(" pri:%3d  ", p->priority_preemption);
    p = p->next;
  }
  mrbc_printf("\n");

  p = p_tcb;
  while( p != NULL ) {
    mrbc_printf(" st:%c%c%c%c  ",
                   (p->state & TASKSTATE_SUSPENDED)?'S':'-',
                   (p->state & TASKSTATE_WAITING)?("sm"[p->reason]):'-',
                   (p->state &(TASKSTATE_RUNNING & ~TASKSTATE_READY))?'R':'-',
                   (p->state & TASKSTATE_READY)?'r':'-' );
    p = p->next;
  }
  mrbc_printf("\n");

  p = p_tcb;
  while( p != NULL ) {
    mrbc_printf(" tmsl:%2d ", p->timeslice);
    p = p->next;
  }
  mrbc_printf("\n");
}


void pqall(void)
{
//  mrbc_printf("<<<<< DORMANT >>>>>\n");	pq(q_dormant_);
  mrbc_printf("<<<<< READY >>>>>\n");	pq(q_ready_);
  mrbc_printf("<<<<< WAITING >>>>>\n");	pq(q_waiting_);
  mrbc_printf("<<<<< SUSPENDED >>>>>\n");	pq(q_suspended_);
}
#endif
