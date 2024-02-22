/*! @file
  @brief
  exception classes

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_ERROR_H_
#define MRBC_SRC_ERROR_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include <stdint.h>
//@endcond

/***** Local headers ********************************************************/
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
#define mrbc_israised(vm) (mrbc_type((vm)->exception) == MRBC_TT_EXCEPTION)


/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  Exception object.

  @extends RBasic
*/
typedef struct RException {
  MRBC_OBJECT_HEADER;

  struct RClass *cls;		//!< exception class.
  uint16_t message_size;	//!< message length.
  const uint8_t *message;	//!< to heap or ROM.

} mrbc_exception;


/***** Global variables *****************************************************/
mrbc_value mrbc_exception_new(struct VM *vm, struct RClass *exc_cls, const void *message, int len);
mrbc_value mrbc_exception_new_alloc(struct VM *vm, struct RClass *exc_cls, const void *message, int len);
void mrbc_exception_delete(mrbc_value *value);
void mrbc_raise(struct VM *vm, struct RClass *exc_cls, const char *msg);
void mrbc_raisef(struct VM *vm, struct RClass *exc_cls, const char *fstr, ...);
void mrbc_print_exception(const mrbc_value *v);
void mrbc_print_vm_exception(const struct VM *vm);


/***** Function prototypes **************************************************/
/***** Inline functions *****************************************************/


#ifdef __cplusplus
}
#endif
#endif
