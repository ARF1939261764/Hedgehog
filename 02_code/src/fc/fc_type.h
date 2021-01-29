#ifndef __FC_TYPE_H
#define __FC_TYPE_H

#include "stdint.h"

#define CPU_6502_BANK_NUM         (8)
#define CPU_6502_BANK_SIZE        (8*1024)
#define CPU_6502_RAM_SIZE         (2*1024)
#define CPU_6502_PRG_ROM_BASE     (0x8000)

typedef void (*fc_reader)(void* arg, uint8_t* buff, uint32_t len);
typedef  void* (*fc_alloc) (void* ptr, uint32_t nsize);

typedef enum
{
	FC_RES_OK,
	FC_RES_INPUT_PTR_NULL,
	FC_RES_CALL_FAIL,
	FC_RES_OUT_OF_MEMORY
}fc_res_t;

typedef enum
{
	fc_format_null,
	fc_format_ines,
	fc_format_nes20,
}fc_format_t;

typedef struct
{
	uint32_t prg_rom_size;     /*PRG-ROM Area*/
	uint32_t chr_rom_size;     /*CHR-ROM Area*/
	uint16_t mapper;           /*Mapper*/
	uint8_t  flag6_mirror : 1;
	uint8_t  flag6_bkram : 1;
	uint8_t  flag6_trainer : 1;
	uint8_t  flag6_hwm : 1;
	uint8_t  flag7_ctype : 2;
	uint8_t  flag7_nes20 : 2;
}fc_header_info_t;



typedef struct
{
  uint16_t pc;
  uint8_t  p;
  uint8_t  acc;
  uint8_t  x;
  uint8_t  y;
  uint8_t  sp;
}fc_cpu_reg_t;

typedef struct
{
  fc_cpu_reg_t  reg;
  uint8_t       *ram;
  uint8_t       *save_ram;
  uint8_t       *prg;
  uint8_t       *chr;
  uint8_t       *bank[CPU_6502_BANK_NUM];
}fc_cpu_t;

typedef struct
{
  fc_cpu_t         cpu;     /*cpu state*/
  fc_alloc         frealloc;/*function to reallocate memory*/
  fc_format_t      format;
  fc_header_info_t hinfo;
  uint32_t         clock;
}fc_state_t;


/*
* opcode handler
*/
typedef fc_res_t (*fc_op_t)(fc_state_t *fs);

/*
* address Mode
*/
typedef enum
{
  FC_ADDR_MODE_NONE,
  FC_ADDR_MODE_ACC,
  FC_ADDR_MODE_IMP,
  FC_ADDR_MODE_IMM,
  FC_ADDR_MODE_ABS,
  FC_ADDR_MODE_ABS_X,
  FC_ADDR_MODE_ABS_Y,
  FC_ADDR_MODE_ZERO_PAGE,
  FC_ADDR_MODE_ZERO_PAGE_X,
  FC_ADDR_MODE_ZERO_PAGE_Y,
  FC_ADDR_MODE_IND,
  FC_ADDR_MODE_IND_X,
  FC_ADDR_MODE_IND_Y,
  FC_ADDR_MODE_REL
} fc_addr_mode_t;
/*
* opcode info
*/
typedef struct
{
  char *name;
  fc_addr_mode_t mode;
} fc_op_info_t;

#endif
