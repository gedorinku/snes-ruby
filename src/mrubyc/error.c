/*! @file
  @brief
  exception classes

  <pre>
  Copyright (C) 2015-2023 Kyushu Institute of Technology.
  Copyright (C) 2015-2023 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <string.h>
#include <stdarg.h>
//@endcond


/***** Local headers ********************************************************/
#include "alloc.h"
#include "value.h"
#include "symbol.h"
#include "error.h"
#include "class.h"
#include "c_string.h"
#include "vm.h"
#include "console.h"


/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Functions ************************************************************/
//================================================================
/*! constructor

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class.
  @param  message	message.
  @param  len		message length or zero.
  @return		exception object.
*/
mrbc_value mrbc_exception_new(struct VM *vm, struct RClass *exc_cls, const void *message, int len )
{
  // allocate memory.
  mrbc_exception *ex = mrbc_alloc( vm, sizeof(mrbc_exception) );
  if( !ex ) {		// ENOMEM
    return mrbc_nil_value();
  }

  MRBC_INIT_OBJECT_HEADER( ex, "EX" );
  ex->cls = exc_cls;

  // in case of no message.
  if( !message ) {
    ex->message = 0;
    ex->message_size = 0;
    goto RETURN;
  }

  // in case of message is ""
  if( *(const char *)message == 0 ) {
    ex->message = (const uint8_t *)"";
    ex->message_size = 0;
    goto RETURN;
  }

  // in case of message in ROM.
  if( len == 0 ) {
    ex->message = message;
    ex->message_size = 0;
    goto RETURN;
  }

  // else, copy the message.
  uint8_t *buf = mrbc_alloc( vm, len+1 );
  if( buf ) {
    memcpy( buf, message, len );
    buf[len] = 0;
    ex->message_size = len;
  } else {
    ex->message_size = 0;
  }
  ex->message = buf;

 RETURN:
  return (mrbc_value){.tt = MRBC_TT_EXCEPTION, .exception = ex};
}


//================================================================
/*! constructor with allocated message buffer.

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class.
  @param  message	message buffer.
  @param  len		message length.
  @return		exception object.
*/
mrbc_value mrbc_exception_new_alloc(struct VM *vm, struct RClass *exc_cls, const void *message, int len )
{
  // allocate memory.
  mrbc_exception *ex = mrbc_alloc( vm, sizeof(mrbc_exception) );
  if( !ex ) {		// ENOMEM
    return mrbc_nil_value();
  }

  MRBC_INIT_OBJECT_HEADER( ex, "EX" );
  ex->cls = exc_cls;
  ex->message = message;
  ex->message_size = len;

  return (mrbc_value){.tt = MRBC_TT_EXCEPTION, .exception = ex};
}


//================================================================
/*! destructor

  @param  value		target.
*/
void mrbc_exception_delete(mrbc_value *value)
{
  if( value->exception->message_size ) {
    mrbc_raw_free( (void *)value->exception->message );
  }
  mrbc_raw_free( value->exception );
}


//================================================================
/*! raise exception

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class or NULL.
  @param  msg		message or NULL.
  @note	(usage) mrbc_raise(vm, MRBC_CLASS(TypeError), "message here.");
*/
void mrbc_raise( struct VM *vm, struct RClass *exc_cls, const char *msg )
{
  if( vm ) {
    struct RClass *cls = exc_cls ? exc_cls : MRBC_CLASS(RuntimeError);
    const char msg_len = msg ? strlen(msg) : 0;

    mrbc_decref(&vm->exception);
    vm->exception = mrbc_exception_new( vm, cls, msg, msg_len );
    vm->flag_preemption = 2;

  } else {
    mrbc_printf("Exception: %s (%s)\n", msg ? msg : mrbc_symid_to_str(exc_cls->sym_id), mrbc_symid_to_str(exc_cls->sym_id));
  }
}


//================================================================
/*! raise exception like printf

  @param  vm		pointer to VM.
  @param  exc_cls	pointer to Exception class.
  @param  fstr		format string.
*/
void mrbc_raisef( struct VM *vm, struct RClass *exc_cls, const char *fstr, ... )
{
  static const int MESSAGE_INI_LEN = 32;
  va_list ap;
  va_start( ap, fstr );

  char *buf = 0;
  if( vm ) buf = mrbc_alloc( vm, MESSAGE_INI_LEN );

  if( buf ) {
    mrbc_vasprintf( &buf, MESSAGE_INI_LEN, fstr, ap );
    mrbc_decref(&vm->exception);
    vm->exception = mrbc_exception_new_alloc( vm,
			exc_cls ? exc_cls : MRBC_CLASS(RuntimeError),
			buf, strlen(buf) );
    vm->flag_preemption = 2;

  } else {
    // VM == NULL or ENOMEM
    mrbc_print("Exception: ");
    mrbc_vprintf( fstr, ap );
    mrbc_printf(" (%s)\n", exc_cls ? mrbc_symid_to_str(exc_cls->sym_id) : "RuntimeError");
  }

  va_end( ap );
}


//================================================================
/*! display exception

  @param  v	pointer to Exception object.
*/
void mrbc_print_exception( const mrbc_value *v )
{
  if( mrbc_type(*v) != MRBC_TT_EXCEPTION ) return;

  const mrbc_exception *exc = v->exception;
  const char *clsname = mrbc_symid_to_str(exc->cls->sym_id);

  mrbc_printf("Exception: %s (%s)\n",
	      exc->message ? (const char *)exc->message : clsname, clsname );
}


//================================================================
/*! display exception in vm.

  @param  vm	pointer to VM
*/
void mrbc_print_vm_exception( const struct VM *vm )
{
  if( mrbc_type(vm->exception) != MRBC_TT_EXCEPTION ) return;

  const mrbc_exception *exc = vm->exception.exception;
  const char *clsname = mrbc_symid_to_str(exc->cls->sym_id);

  mrbc_printf("Exception(vm_id=%d): %s (%s)\n", vm->vm_id,
	      exc->message ? (const char *)exc->message : clsname, clsname );
}



/***** Exception class ******************************************************/
//================================================================
/*! (method) new
 */
static void c_exception_new(struct VM *vm, mrbc_value v[], int argc)
{
  assert( mrbc_type(v[0]) == MRBC_TT_CLASS );

  mrbc_value value;
  if( argc == 1 && mrbc_type(v[1]) == MRBC_TT_STRING ) {
    value = mrbc_exception_new(vm, v[0].cls, mrbc_string_cstr(&v[1]), mrbc_string_size(&v[1]));
  } else {
    value = mrbc_exception_new(vm, v[0].cls, NULL, 0);
  }

  SET_RETURN(value);
}


//================================================================
/*! (method) message
 */
static void c_exception_message(struct VM *vm, mrbc_value v[], int argc)
{
  mrbc_value value;

  if( v[0].exception->message ) {
    value = mrbc_string_new( vm, v[0].exception->message, v[0].exception->message_size );
  } else {
    value = mrbc_string_new_cstr(vm, mrbc_symid_to_str(v->exception->cls->sym_id));
  }

  mrbc_decref( &v[0] );
  v[0] = value;
}


/* mruby/c Exception class hierarchy.

    Exception
      NoMemoryError
      NotImplementedError
      StandardError
        ArgumentError
        IndexError
        NameError
          NoMethodError
        RangeError
        RuntimeError
        TypeError
        ZeroDivisionError
*/

/* MRBC_AUTOGEN_METHOD_TABLE
  FILE("_autogen_class_exception.h")

  CLASS("Exception")
  METHOD("new", c_exception_new )
  METHOD("message", c_exception_message )

  CLASS("NoMemoryError")
  SUPER("Exception")

  CLASS("NotImplementedError")
  SUPER("Exception")

  CLASS("StandardError")
  SUPER("Exception")

  CLASS("ArgumentError")
  SUPER("StandardError")

  CLASS("IndexError")
  SUPER("StandardError")

  CLASS("NameError")
  SUPER("StandardError")

  CLASS("NoMethodError")
  SUPER("NameError")

  CLASS("RangeError")
  SUPER("StandardError")

  CLASS("RuntimeError")
  SUPER("StandardError")

  CLASS("TypeError")
  SUPER("StandardError")

  CLASS("ZeroDivisionError")
  SUPER("StandardError")
*/
#include "_autogen_class_exception.h"
