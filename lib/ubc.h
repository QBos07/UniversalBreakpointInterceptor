#pragma once

#include "helper.h"

#ifdef __cplusplus
extern "C" {
#define DEFAULT(val) = val
#else
#define DEFAULT(val)
#endif

#include <stdint.h>

typedef regstruct {
  unsigned : 31;
  // use handler from DBR when true else normal interupt handler
  bool ubde : 1;
}
ubc_cbcr_t;

// is written like bitwise and
typedef regstruct {
  unsigned : 30;
  // channel 1 match condition satified
  bool mf1 : 1;
  // channel 0 match condition satified
  bool mf0 : 1;
}
ubc_ccmfr_t;

typedef regstruct {
  unsigned : 20;
  // decremented each time
  // break when after 1 aka when setting to 0
  uint16_t cet : 12;
}
ubc_cetr_t;

// specifies the mask for the data
typedef uint32_t ubc_cdmr_t;

// specifies the data
typedef uint32_t ubc_cdr_t;

// specifies the mask for the address
typedef uint32_t ubc_camr_t;

// specifies the address
typedef volatile void *ubc_car_t;

typedef regstruct {
  unsigned : 18;
  enum {
    UBC_CCR_RESERVED_FIX_ONE = 1
  } reserved : 1 DEFAULT(UBC_CCR_RESERVED_FIX_ONE);
  unsigned : 11;
  enum {
    UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION = 0,
    UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM = 1
  } pcb : 1 DEFAULT(UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION);
  // if true break when match condition is satisfied
  bool bie : 1;
}
ubc_crr_t;

typedef regstruct {
  // match flag enable
  bool mfe : 1 DEFAULT(false);
  // ASID check enable
  bool aie : 1 DEFAULT(false);
  // match flag specify
  enum {
    UBC_CBR1_MFI_USE_MF0 = 0,
    UBC_CBR1_MFI_USE_MF1 = 1
  } mfi : 6 DEFAULT(UBC_CBR1_MFI_USE_MF1);
  // ASID specify
  uint8_t aiv DEFAULT(0);
  // data value select; needs an operand size selected
  bool dbe : 1 DEFAULT(false);
  // operand size select
  enum {
    // ignore
    UBC_CBR1_SZ_OPERAND_ANY_ACCESS = 0,
    // 8 bit / 1 byte
    UBC_CBR1_SZ_OPERAND_BYTE_ACCESS = 1,
    // 16 bit / 2 bytes
    UBC_CBR1_SZ_OPERAND_WORD_ACCESS = 2,
    // 32 bit / 4 bytes
    UBC_CBR1_SZ_OPERAND_LONGWORD_ACCESS = 3,
    // 64 bit / 8 bytes (two 32 bit matches are tryed)
    UBC_CBR1_SZ_OPERAND_QUADWORD_ACCESS = 4
  } sz : 3 DEFAULT(UBC_CBR1_SZ_OPERAND_ANY_ACCESS);
  // execution count select
  bool etbe : 1 DEFAULT(false);
  unsigned : 3;
  // bus select
  enum { UBC_CBR1_CD_OPERAND_BUS = 0 } cd : 2 DEFAULT(UBC_CBR1_CD_OPERAND_BUS);
  enum {
    UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE_OPERAND_ACCESS_CYCLE = 0,
    UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE = 1,
    UBC_CBR1_ID_OPERAND_ACCESS_CYCLE = 2,
    // same as the other
    UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE_OPERAND_ACCESS_CYCLE2 = 3
  } id : 2;
  unsigned : 1;
  // bus command select
  enum {
    UBC_CBR1_RW_BUS_READ_WRITE_CYCLE = 0,
    UBC_CBR1_RW_BUS_READ_CYCLE = 1,
    UBC_CBR1_RW_BUS_WRITE_CYCLE = 2,
    // same as the other
    UBC_CBR1_RW_BUS_READ_WRITE_CYCLE2 = 3
  } rw : 2 DEFAULT(UBC_CBR1_RW_BUS_READ_WRITE_CYCLE);
  // enable
  bool ce : 1;
}
ubc_cbr1_t;

// lacks some options of cbr1
typedef regstruct {
  // match flag enable
  bool mfe : 1 DEFAULT(false);
  // ASID check enable
  bool aie : 1 DEFAULT(false);
  // match flag specify
  enum {
    UBC_CBR0_MFI_USE_MF0 = 0,
    UBC_CBR0_MFI_USE_MF1 = 1
  } mfi : 6 DEFAULT(UBC_CBR0_MFI_USE_MF0);
  // ASID specify
  uint8_t aiv DEFAULT(0);
  unsigned : 1;
  // operand size select
  enum {
    // ignore
    UBC_CBR0_SZ_OPERAND_ANY_ACCESS = 0,
    // 8 bit / 1 byte
    UBC_CBR0_SZ_OPERAND_BYTE_ACCESS = 1,
    // 16 bit / 2 bytes
    UBC_CBR0_SZ_OPERAND_WORD_ACCESS = 2,
    // 32 bit / 4 bytes
    UBC_CBR0_SZ_OPERAND_LONGWORD_ACCESS = 3,
    // 64 bit / 8 bytes (two 32 bit matches are tryed)
    UBC_CBR0_SZ_OPERAND_QUADWORD_ACCESS = 4
  } sz : 3 DEFAULT(UBC_CBR0_SZ_OPERAND_ANY_ACCESS);
  unsigned : 4;
  // bus select
  enum { UBC_CBR0_CD_OPERAND_BUS = 0 } cd : 2 DEFAULT(UBC_CBR0_CD_OPERAND_BUS);
  enum {
    UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE_OPERAND_ACCESS_CYCLE = 0,
    UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE = 1,
    UBC_CBR0_ID_OPERAND_ACCESS_CYCLE = 2,
    // same as the other
    UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE_OPERAND_ACCESS_CYCLE2 = 3
  } id : 2;
  unsigned : 1;
  // bus command select
  enum {
    UBC_CBR0_RW_BUS_READ_WRITE_CYCLE = 0,
    UBC_CBR0_RW_BUS_READ_CYCLE = 1,
    UBC_CBR0_RW_BUS_WRITE_CYCLE = 2,
    // same as the other
    UBC_CBR0_RW_BUS_READ_WRITE_CYCLE2 = 3
  } rw : 2 DEFAULT(UBC_CBR0_RW_BUS_READ_WRITE_CYCLE);
  // enable
  bool ce : 1;
}
ubc_cbr0_t;

__attribute__((weak)) void
ubc_setDBR(void *fun) {
  __asm__ volatile("ldc %0,dbr" : : "r"(fun));
}

HW_REG32(UBC_CBR0, 0xFF200000);
#define AS_STRUCT_TYPE_UBC_CBR0 ubc_cbr0_t
HW_REG32(UBC_CRR0, 0xFF200004);
#define AS_STRUCT_TYPE_UBC_CRR0 ubc_crr_t
HW_REG_T(UBC_CAR0, 0xFF200008, ubc_car_t);
HW_REG_T(UBC_CAMR0, 0xFF20000C, ubc_camr_t);

HW_REG32(UBC_CBR1, 0xFF200020);
#define AS_STRUCT_TYPE_UBC_CBR1 ubc_cbr1_t
HW_REG32(UBC_CRR1, 0xFF200024);
#define AS_STRUCT_TYPE_UBC_CRR1 ubc_crr_t
HW_REG_T(UBC_CAR1, 0xFF200028, ubc_car_t);
HW_REG_T(UBC_CAMR1, 0xFF20002C, ubc_camr_t);

HW_REG_T(UBC_CDR1, 0xFF200030, ubc_cdr_t);
HW_REG_T(UBC_CDMR1, 0xFF200034, ubc_cdmr_t);
HW_REG32(UBC_CETR1, 0xFF200038);
#define AS_STRUCT_TYPE_UBC_CETR1 ubc_cetr_t

HW_REG32(UBC_CCMFR, 0xFF200600);
#define AS_STRUCT_TYPE_UBC_CCMFR ubc_ccmfr_t
HW_REG32(UBC_CBCR, 0xFF200620);
#define AS_STRUCT_TYPE_UBC_CBCR ubc_cbcr_t

#undef DEFAULT

#ifdef __cplusplus
}
#endif