/*! @file
  @brief
  Hardware abstraction layer
        for POSIX

  <pre>
  Copyright (C) 2016-2021 Kyushu Institute of Technology.
  Copyright (C) 2016-2021 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.
  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>


/***** Local headers ********************************************************/
#include "hal.h"


/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
#if !defined(MRBC_NO_TIMER)
static sigset_t sigset_, sigset2_;
#endif


/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
#if !defined(MRBC_NO_TIMER)
//================================================================
/*!@brief
  alarm signal handler

*/
static void sig_alarm(int dummy)
{
  mrbc_tick();
}

#endif


/***** Local functions ******************************************************/
/***** Global functions *****************************************************/
#if !defined(MRBC_NO_TIMER)

//================================================================
/*!@brief
  initialize

*/
void hal_init(void)
{
  sigemptyset(&sigset_);
  sigaddset(&sigset_, SIGALRM);

  // タイマー用シグナル準備
  struct sigaction sa;
  sa.sa_handler = sig_alarm;
  sa.sa_flags   = SA_RESTART;
  sa.sa_mask    = sigset_;
  sigaction(SIGALRM, &sa, 0);

  // タイマー設定
  struct itimerval tval;
  int sec  = 0;
  int usec = MRBC_TICK_UNIT * 1000;
  tval.it_interval.tv_sec  = sec;
  tval.it_interval.tv_usec = usec;
  tval.it_value.tv_sec     = sec;
  tval.it_value.tv_usec    = usec;
  setitimer(ITIMER_REAL, &tval, 0);
}


//================================================================
/*!@brief
  enable interrupt

*/
void hal_enable_irq(void)
{
  sigprocmask(SIG_SETMASK, &sigset2_, 0);
}


//================================================================
/*!@brief
  disable interrupt

*/
void hal_disable_irq(void)
{
  sigprocmask(SIG_BLOCK, &sigset_, &sigset2_);
}

#endif /* if !defined(MRBC_NO_TIMER) */



//================================================================
/*!@brief
  abort program

*/
void hal_abort( const char *s )
{
  if( s ) {
    hal_write(1, s, strlen(s));
  }
  exit( 1 );
}
