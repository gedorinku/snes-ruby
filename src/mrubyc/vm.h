/*! @file
  @brief
  mruby bytecode executor.

  <pre>
  Copyright (C) 2015-2022 Kyushu Institute of Technology.
  Copyright (C) 2015-2022 Shimane IT Open-Innovation Center.

  This file is distributed under BSD 3-Clause License.

  Fetch mruby VM bytecodes, decode and execute.

  </pre>
*/

#ifndef MRBC_SRC_VM_H_
#define MRBC_SRC_VM_H_

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
//@cond
#include "vm_config.h"
#include <stdint.h>
//@endcond


/***** Local headers ********************************************************/
#include "value.h"
#include "class.h"

#ifdef __cplusplus
extern "C" {
#endif
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  IREP Internal REPresentation
*/
typedef struct IREP {
#if defined(MRBC_DEBUG)
  uint8_t type[2];		//!< set "RP" for debug.
#endif

  uint16_t nlocals;		//!< num of local variables
  uint16_t nregs;		//!< num of register variables
  uint16_t rlen;		//!< num of child IREP blocks
  uint16_t clen;		//!< num of catch handlers
  uint32_t ilen;		//!< num of bytes in OpCode
  uint16_t plen;		//!< num of pools
  uint16_t slen;		//!< num of symbols
  uint16_t ofs_ireps;		//!< offset of data->tbl_ireps. (32bit aligned)

  const uint8_t *inst;		//!< pointer to instruction in RITE binary
  const uint8_t *pool;		//!< pointer to pool in RITE binary

  uint8_t data[];		//!< variable data. (see load.c)
				//!<  mrbc_sym   tbl_syms[slen]
				//!<  uint16_t   tbl_pools[plen]
				//!<  mrbc_irep *tbl_ireps[rlen]
} mrbc_irep;
typedef struct IREP mrb_irep;

// mrbc_irep manipulate macro.
//! get a symbol id table pointer.
#define mrbc_irep_tbl_syms(irep)	((mrbc_sym *)(irep)->data)

//! get a n'th symbol id in irep
#define mrbc_irep_symbol_id(irep, n)	mrbc_irep_tbl_syms(irep)[(n)]

//! get a n'th symbol string in irep
#define mrbc_irep_symbol_cstr(irep, n)	mrbc_symid_to_str( mrbc_irep_symbol_id(irep, n) )


//! get a pool data offset table pointer.
#define mrbc_irep_tbl_pools(irep) \
  ( (uint16_t *) ((irep)->data + (irep)->slen * sizeof(mrbc_sym)) )

//! get a pointer to n'th pool data.
#define mrbc_irep_pool_ptr(irep, n) \
  ( (irep)->pool + mrbc_irep_tbl_pools(irep)[(n)] )


//! get a child irep table pointer.
#define mrbc_irep_tbl_ireps(irep) \
  ( (mrbc_irep **) ((irep)->data + (irep)->ofs_ireps * 4) )

//! get a n'th child irep
#define mrbc_irep_child_irep(irep, n) \
  ( mrbc_irep_tbl_ireps(irep)[(n)] )



//================================================================
/*!@brief
  IREP Catch Handler
*/
typedef struct IREP_CATCH_HANDLER {
  uint8_t type;		//!< enum mrb_catch_type, 1 byte. 0=rescue, 1=ensure
  uint8_t begin[4];	//!< The starting address to match the handler. Includes this.
  uint8_t end[4];	//!< The endpoint address that matches the handler. Not Includes this.
  uint8_t target[4];	//!< The address to jump to if a match is made.
} mrbc_irep_catch_handler;


//================================================================
/*!@brief
  Call information
*/
typedef struct CALLINFO {
  struct CALLINFO *prev;	//!< previous linked list.
  const mrbc_irep *cur_irep;	//!< copy from mrbc_vm.
  const uint8_t *inst;		//!< copy from mrbc_vm.
  mrbc_value *cur_regs;		//!< copy from mrbc_vm.
  mrbc_class *target_class;	//!< copy from mrbc_vm.

  mrbc_class *own_class;	//!< class that owns method.
  mrbc_sym method_id;		//!< called method ID.
  uint8_t reg_offset;		//!< register offset after call.
  uint8_t n_args;		//!< num of arguments.
  uint8_t kd_reg_offset;	//!< keyword or dictionary register offset.
  uint8_t is_called_super;	//!< this is called by op_super.

} mrbc_callinfo;
typedef struct CALLINFO mrb_callinfo;


//================================================================
/*!@brief
  Virtual Machine
*/
typedef struct VM {
#if defined(MRBC_DEBUG)
  char type[2];				// set "VM" for debug
#endif
  uint8_t vm_id;			//!< vm_id : 1..MAX_VM_COUNT
  volatile int8_t flag_preemption;
  unsigned int flag_need_memfree : 1;
  unsigned int flag_stop : 1;
  unsigned int flag_permanence : 1;

  uint16_t	  regs_size;		//!< size of regs[]

  mrbc_irep       *top_irep;		//!< IREP tree top.
  const mrbc_irep *cur_irep;		//!< IREP currently running.
  const uint8_t   *inst;		//!< Instruction pointer.
  mrbc_value	  *cur_regs;		//!< Current register top.
  mrbc_class      *target_class;	//!< Target class.
  mrbc_callinfo	  *callinfo_tail;	//!< Last point of CALLINFO link.
  mrbc_proc	  *ret_blk;		//!< Return block.

  mrbc_value	  exception;		//!< Raised exception or nil.
  mrbc_value      regs[];
} mrbc_vm;
typedef struct VM mrb_vm;


/***** Global variables *****************************************************/
/***** Function prototypes **************************************************/
void mrbc_cleanup_vm(void);
mrbc_sym mrbc_get_callee_symid(struct VM *vm);
const char *mrbc_get_callee_name(struct VM *vm);
mrbc_callinfo *mrbc_push_callinfo(struct VM *vm, mrbc_sym method_id, int reg_offset, int n_args);
void mrbc_pop_callinfo(struct VM *vm);
mrbc_vm *mrbc_vm_new(int regs_size);
mrbc_vm *mrbc_vm_open(struct VM *vm);
void mrbc_vm_close(struct VM *vm);
void mrbc_vm_begin(struct VM *vm);
void mrbc_vm_end(struct VM *vm);
int mrbc_vm_run(struct VM *vm);


/***** Inline functions *****************************************************/
/*
  (note)
  Conversion functions from binary (byte array) to each data type.

  Use the MRBC_BIG_ENDIAN, MRBC_LITTLE_ENDIAN and MRBC_REQUIRE_32BIT_ALIGNMENT
  macros.

  (each cases)
  Little endian, no alignment.
   MRBC_LITTLE_ENDIAN && !MRBC_REQUIRE_32BIT_ALIGNMENT
   (e.g.) ARM Cortex-M4, Intel x86

  Big endian, no alignment.
   MRBC_BIG_ENDIAN && !MRBC_REQUIRE_32BIT_ALIGNMENT
   (e.g.) IBM PPC405

  Little endian, 32bit alignment required.
   MRBC_LITTLE_ENDIAN && MRBC_REQUIRE_32BIT_ALIGNMENT
   (e.g.) ARM Cortex-M0

  Big endian, 32bit alignment required.
   MRBC_BIG_ENDIAN) && MRBC_REQUIRE_32BIT_ALIGNMENT
   (e.g.) OpenRISC
*/
#if defined(MRBC_REQUIRE_32BIT_ALIGNMENT) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
#define MRBC_REQUIRE_64BIT_ALIGNMENT
#endif


//================================================================
/*! Get 16bit int value from memory.

  @param  s	Pointer to memory.
  @return	16bit unsigned int value.
*/
static inline uint16_t bin_to_uint16( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint16_t x = *((uint16_t *)s);
  x = (x << 8) | (x >> 8);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Big endian, no alignment.
  uint16_t x = *((uint16_t *)s);

#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // 32bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint16_t x = *p++;
  x <<= 8;
  x |= *p;

#else
  #error "Specify MRBC_BIG_ENDIAN or MRBC_LITTLE_ENDIAN"
#endif

  return x;
}


//================================================================
/*! Get 32bit int value from memory.

  @param  s	Pointer to memory.
  @return	32bit unsigned int value.
*/
static inline uint32_t bin_to_uint32( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint32_t x = *((uint32_t *)s);
  x = (x << 24) | ((x & 0xff00) << 8) | ((x >> 8) & 0xff00) | (x >> 24);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // Big endian, no alignment.
  uint32_t x = *((uint32_t *)s);

#elif defined(MRBC_REQUIRE_32BIT_ALIGNMENT)
  // 32bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint32_t x = *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p;

#endif

  return x;
}


#if defined(MRBC_INT64)
//================================================================
/*! Get 64bit int value from memory.

  @param  s	Pointer to memory.
  @return	64bit int value.
*/
static inline int64_t bin_to_int64( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, no alignment.
  uint64_t x = *((uint64_t *)s);
  uint64_t y = (x << 56);
  y |= ((uint64_t)(uint8_t)(x >>  8)) << 48;
  y |= ((uint64_t)(uint8_t)(x >> 16)) << 40;
  y |= ((uint64_t)(uint8_t)(x >> 24)) << 32;
  y |= ((uint64_t)(uint8_t)(x >> 32)) << 24;
  y |= ((uint64_t)(uint8_t)(x >> 40)) << 16;
  y |= ((uint64_t)(uint8_t)(x >> 48)) << 8;
  y |= (x >> 56);
  return y;

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, no alignment.
  return *((uint64_t *)s);

#elif defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // 64bit alignment required.
  uint8_t *p = (uint8_t *)s;
  uint64_t x = *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p++;
  x <<= 8;
  x |= *p;
  return x;

#endif

}
#endif


//================================================================
/*! Get double (64bit) value from memory.

  @param  s	Pointer to memory.
  @return	double value.
*/
static inline double bin_to_double64( const void *s )
{
#if defined(MRBC_LITTLE_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, no alignment.
  return *((double *)s);

#elif defined(MRBC_BIG_ENDIAN) && !defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, no alignment.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x + 7;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2-- = *p1++;
  }
  return x;

#elif defined(MRBC_LITTLE_ENDIAN) && defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Little endian, 64bit alignment required.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2++ = *p1++;
  }
  return x;

#elif defined(MRBC_BIG_ENDIAN) && defined(MRBC_REQUIRE_64BIT_ALIGNMENT)
  // Big endian, 64bit alignment required.
  double x;
  uint8_t *p1 = (uint8_t*)s;
  uint8_t *p2 = (uint8_t*)&x + 7;
  int i;
  for( i = 7; i >= 0; i-- ) {
    *p2-- = *p1++;
  }
  return x;

#endif
}


//================================================================
/*! Set 32bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint32_to_bin( uint32_t v, void *d )
{
  uint8_t *p = (uint8_t *)d + 3;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p-- = 0xff & v;
  v >>= 8;
  *p = 0xff & v;
}


//================================================================
/*! Set 16bit value to memory.

  @param  v	Source value.
  @param  d	Pointer to memory.
*/
static inline void uint16_to_bin( uint16_t v, void *d )
{
  uint8_t *p = (uint8_t *)d;
  *p++ = (v >> 8);
  *p = 0xff & v;
}


#ifdef __cplusplus
}
#endif
#endif
