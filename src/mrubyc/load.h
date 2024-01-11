/*! @file
  @brief
  mruby bytecode loader.

  <pre>
  Copyright (C) 2015-2021 Kyushu Institute of Technology.
  Copyright (C) 2015-2021 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  </pre>
*/

#ifndef MRBC_SRC_LOAD_H_
#define MRBC_SRC_LOAD_H_

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
/***** Typedefs *************************************************************/
/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
int mrbc_load_mrb(struct VM *vm, const void *bytecode);
int mrbc_load_irep(struct VM *vm, const void *bytecode);
void mrbc_irep_free(struct IREP *irep);
mrbc_value mrbc_irep_pool_value(struct VM *vm, int n);

/***** Inline functions *****************************************************/

#ifdef __cplusplus
}
#endif
#endif
