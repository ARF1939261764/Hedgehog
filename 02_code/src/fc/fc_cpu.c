#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"

#include "fc_type.h"
#include "fc_ppu.h"

extern const fc_op_info_t fc_op_info[256];

#define FC_ADDR_BANK(addr) (((addr) >> 13) & 0x07)
#define FC_ADDR_OFFSET(addr) ((addr)&0x1FFF)

#define fc_reg (fs->cpu.reg)
#define fc_pc (fc_reg.pc)
#define fc_x (fc_reg.x)
#define fc_y (fc_reg.y)
#define fc_p (fc_reg.p)
#define fc_sp (fc_reg.sp)
#define fc_acc (fc_reg.acc)
#define fc_read(addr) (fc_cpu_r_memory(fs, addr))
#define fc_write(addr, d) (fc_cpu_w_memory(fs, addr, d))
/**
 *  push
 */
#define fc_push(d)              \
  do                            \
  {                             \
    fc_write(0x100 | fc_sp, d); \
    fc_sp--;                    \
  } while (0);
/**
 *  pop
 */
#define fc_pop() (fc_read((++fc_sp) | 0x100))
/**
 *  fetch
 */
#define fc_fetch(o)     \
  do                    \
  {                     \
    o = fc_read(fc_pc); \
    fc_pc++;            \
  } while (0)
/**
 *  write bit
 */
#define fc_w_bit(t, b, v) \
  do                      \
  {                       \
    if (v)                \
    {                     \
      (t) |= 1 << (b);    \
    }                     \
    else                  \
    {                     \
      (t) &= ~(1 << (b)); \
    }                     \
  } while (0)
/**
 *  read bit
 */
#define fc_r_bit(t, b) (((t) >> (b)) & 0x01)
/**
 *  Set/Res/R/W the carry flag bit
 */
#define fc_cpu_op_set_flag_c() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 0, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_c() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 0, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_c(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 0, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_c() fc_r_bit(fc_p, 0)
/**
 *  Set/Res/R/W the zero flag bit
 */
#define fc_cpu_op_set_flag_z() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 1, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_z() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 1, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_z(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 1, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_z() fc_r_bit(fc_p, 1)
#define fc_cpu_op_check_z(v) fc_cpu_op_w_flag_z((v) == 0 ? 1 : 0)
/**
 *  Set/Res/R/W the Interrupt Disable bit
 */
#define fc_cpu_op_set_flag_i() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 2, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_i() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 2, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_i(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 2, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_i() fc_r_bit(fc_p, 2)
/**
 *  Set/Res/R/W the Decimal bit
 */
#define fc_cpu_op_set_flag_d() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 3, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_d() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 3, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_d(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 3, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_d() fc_r_bit(fc_p, 3)
/**
  *  Set/Res/R/W the b flag bit
  */
#define fc_cpu_op_set_flag_b() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 4, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_b() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 4, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_b(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 4, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_b() fc_r_bit(fc_p, 4)
/**
   *  Set/Res/R/W the r flag bit
   */
#define fc_cpu_op_set_flag_r() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 5, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_r() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 5, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_r(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 5, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_r() fc_r_bit(fc_p, 5)
/**
 *  Set/Res/R/W the Overflow bit
 */
#define fc_cpu_op_set_flag_v() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 6, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_v() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 6, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_v(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 6, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_v() fc_r_bit(fc_p, 6)
/**
 *  Set/Res/R/W the Negative bit
 */
#define fc_cpu_op_set_flag_n() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 7, 1);      \
  } while (0)
#define fc_cpu_op_res_flag_n() \
  do                           \
  {                            \
    fc_w_bit(fc_p, 7, 0);      \
  } while (0)
#define fc_cpu_op_w_flag_n(v) \
  do                          \
  {                           \
    fc_w_bit(fc_p, 7, (v));   \
  } while (0)
#define fc_cpu_op_r_flag_n() fc_r_bit(fc_p, 7)
#define fc_cpu_op_check_n(v) fc_cpu_op_w_flag_n(((v)&0x80) != 0 ? 1 : 0)
/**
 *  none addressing
 */
#define fc_cpu_addresing_none() \
  do                            \
  {                             \
    addr = 0x00;                \
  } while (0)
/**
 *  Accumulator addressing
 */
#define fc_cpu_addresing_acc() \
  do                           \
  {                            \
    addr = 0x00;               \
  } while (0)

/**
 *  Implicit addressing
 */
#define fc_cpu_addresing_imp() \
  do                           \
  {                            \
    addr = 0x00;               \
  } while (0)

/**
 *  Immediate addressing
 */
#define fc_cpu_addresing_imm() \
  do                           \
  {                            \
    addr = fc_pc;              \
    fc_pc += 1;                \
  } while (0)

/**
 *  Absolute addressing
 */
#define fc_cpu_addresing_abs()                   \
  do                                             \
  {                                              \
    addr = ((uint16_t)fc_read(fc_pc + 0) << 0) | \
           ((uint16_t)fc_read(fc_pc + 1) << 8);  \
    fc_pc += 2;                                  \
  } while (0)

/**
 *  Absolute,X addressing
 */
#define fc_cpu_addresing_abs_x() \
  do                             \
  {                              \
    fc_cpu_addresing_abs();      \
    addr += fc_x;                \
  } while (0)

/**
 *  Absolute,Y addressing
 */
#define fc_cpu_addresing_abs_y() \
  do                             \
  {                              \
    fc_cpu_addresing_abs();      \
    addr += fc_y;                \
  } while (0)

/**
 *  Zero Page addressing
 */
#define fc_cpu_addresing_zero_page() \
  do                                 \
  {                                  \
    addr = fc_read(fc_pc);           \
    fc_pc += 1;                      \
  } while (0)

/**
 *  Zero Page,X addressing
 */
#define fc_cpu_addresing_zero_page_x() \
  do                                   \
  {                                    \
    addr = fc_read(fc_pc);             \
    addr += fc_x;                      \
    addr &= 0xff;                      \
    fc_pc += 1;                        \
  } while (0)

/**
 *  Zero Page,Y addressing
 */
#define fc_cpu_addresing_zero_page_y() \
  do                                   \
  {                                    \
    addr = fc_read(fc_pc);             \
    addr += fc_y;                      \
    addr &= 0xff;                      \
    fc_pc += 1;                        \
  } while (0)

/**
 *  Indirect addressing
 */
#define fc_cpu_addresing_ind()                 \
  do                                           \
  {                                            \
    uint16_t t0, t1;                           \
    t0 = ((uint16_t)fc_read(fc_pc + 0) << 0) | \
         ((uint16_t)fc_read(fc_pc + 1) << 8);  \
    t1 = (t0 & 0xff00) | ((t0 + 1) & 0x00ff);  \
    addr = ((uint16_t)fc_read(t0) << 0) |      \
           ((uint16_t)fc_read(t1) << 8);       \
    fc_pc += 2;                                \
  } while (0)

/**
 *  Indirect,X addressing
 */
#define fc_cpu_addresing_ind_x() \
  do                             \
  {                              \
    uint8_t t0, t1, t2;          \
    t0 = fc_read(fc_pc) + fc_x;  \
    t1 = (t0 + 0) & 0xff;        \
    t2 = (t0 + 1) & 0xff;        \
    addr = (fc_read(t1) << 0) |  \
           (fc_read(t2) << 8);   \
    fc_pc += 1;                  \
  } while (0)

/**
 *  Indirect,Y addressing
 */
#define fc_cpu_addresing_ind_y() \
  do                             \
  {                              \
    uint8_t t0, t1, t2;          \
    t0 = fc_read(fc_pc);         \
    t1 = (t0 + 0) & 0xff;        \
    t2 = (t0 + 1) & 0xff;        \
    addr = (fc_read(t1) << 0) |  \
           (fc_read(t2) << 8);   \
    addr += fc_y;                \
    fc_pc += 1;                  \
  } while (0)

/**
 *  Relative addressing
 */
#define fc_cpu_addresing_rel()   \
  do                             \
  {                              \
    int8_t t0;                   \
    t0 = (int8_t)fc_read(fc_pc); \
    fc_pc += 1;                  \
    addr = fc_pc + t0;           \
  } while (0)
/**
 *  instruct
 */
#define fc_cpu_op(n, o, a)   \
  case n:                    \
  {                          \
    uint16_t addr;           \
    fc_cpu_addresing_##a();  \
    fc_cpu_op_##o(fs, addr); \
    break;                   \
  }

/**
 *  description:read memory
 *  @param fs : emulator state sachine
 *  @param addr : address
 *  
 *  @return data
 */
static uint8_t apu_reg[32];
uint8_t fc_cpu_r_memory(fc_state_t *fs, uint16_t addr)
{
  uint8_t data;
  switch (FC_ADDR_BANK(addr))
  {
  case 0:
    data = fs->cpu.ram[FC_ADDR_OFFSET(addr)];
    break;
  case 1:
    /*ppu寄存器,8字节镜像*/
    data = fc_ppu_r_reg(addr & 0x07);
    break;
  case 2:
    /*apu寄存器,20字节镜像*/
    data = apu_reg[addr & 0x1f];
    break;
  case 3:
    /*备份ram*/
    data = 0xcc;
    abort();
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    data = fs->cpu.bank[FC_ADDR_BANK(addr)][FC_ADDR_OFFSET(addr)];
    break;
  default:
    data = 0xcc;
    break;
  }
  return data;
}
/**
 *  description:write memory
 *  @param fs : emulator state sachine
 *  @param addr : address
 *  @param data : data
 *  
 *  @return void
 */

void fc_cpu_w_memory(fc_state_t *fs, uint16_t addr, uint8_t data)
{
  switch (FC_ADDR_BANK(addr))
  {
  case 0:
    fs->cpu.ram[FC_ADDR_OFFSET(addr)] = data;
    break;
  case 1:
    /*ppu寄存器,8字节镜像*/
    fc_ppu_w_reg(addr & 0x07, data);
    break;
  case 2:
    /*apu寄存器,20字节镜像*/
    apu_reg[addr & 0x1f] = data;
    break;
  case 3:
    /*备份ram*/
    abort();
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    fs->cpu.bank[FC_ADDR_BANK(addr)][FC_ADDR_OFFSET(addr)] = data;
    break;
  default:
    break;
  }
}
/**
 *  undefined
 */
fc_res_t fc_cpu_op_undefined(fc_state_t *fs, uint16_t addr)
{
  printf("Error:Undefined Instruction");
  while (1)
  {
    /* code */
  }
  return FC_RES_OK;
}
/**
 *  brk
 */
fc_res_t fc_cpu_op_brk(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ora(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_acc |= t0;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_stp(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_slo(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_cpu_op_w_flag_c(((t0 >> 7) & 0x01) ? 1 : 0);
  t0 <<= 1;
  fc_write(addr, t0);
  fc_acc |= t0;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_nop(fc_state_t *fs, uint16_t addr)
{
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_asl(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_cpu_op_w_flag_c((t0 & 0x80) != 0 ? 1 : 0);
  t0 <<= 1;
  fc_write(addr, t0);
  fc_cpu_op_check_z(t0);
  fc_cpu_op_check_n(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_asl_acc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_acc;
  fc_cpu_op_w_flag_c((t0 & 0x80) != 0 ? 1 : 0);
  t0 <<= 1;
  fc_acc = t0;
  fc_cpu_op_check_z(t0);
  fc_cpu_op_check_n(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_php(fc_state_t *fs, uint16_t addr)
{
  fc_push(fc_p);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_anc(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bpl(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_n() == 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_clc(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_res_flag_c();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_jsr(fc_state_t *fs, uint16_t addr)
{
  uint16_t ret;
  ret = fc_pc - 1;
  fc_push((ret >> 8) & 0xFF);
  fc_push((ret >> 0) & 0xFF);
  fc_pc = addr;
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_and(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_acc &= t0;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rla(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0, t1;
  t0 = fc_read(addr);
  t1 = (t0 & 0x80) ? 1 : 0;
  t0 = (t0 << 1) | (fc_cpu_op_r_flag_c() ? 1 : 0);
  fc_write(addr, t0);
  fc_cpu_op_w_flag_c(t1);
  fc_acc &= t0;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bit(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_cpu_op_w_flag_z((t0 & fc_acc) == 0 ? 1 : 0);
  fc_cpu_op_w_flag_v((t0 & 0x40) != 0 ? 1 : 0);
  fc_cpu_op_w_flag_n((t0 & 0x80) != 0 ? 1 : 0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rol_acc(fc_state_t *fs, uint16_t addr)
{
  uint16_t t0;
  t0 = fc_acc;
  t0 <<= 1;
  if (fc_cpu_op_r_flag_c() != 0)
  {
    t0 |= 0x01;
  }
  fc_cpu_op_w_flag_c((t0 & 0x0100) != 0x0000 ? 1 : 0);
  fc_acc = (uint8_t)t0;
  fc_cpu_op_check_n(fc_acc);
  fc_cpu_op_check_z(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rol(fc_state_t *fs, uint16_t addr)
{
  uint16_t t0;
  t0 = fc_read(addr);
  t0 <<= 1;
  if (fc_cpu_op_r_flag_c() != 0)
  {
    t0 |= 0x01;
  }
  fc_cpu_op_w_flag_c((t0 & 0x0100) != 0x0000 ? 1 : 0);
  t0 &= 0xff;
  fc_write(addr, (uint8_t)t0);
  fc_cpu_op_check_n((uint8_t)t0);
  fc_cpu_op_check_z((uint8_t)t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_plp(fc_state_t *fs, uint16_t addr)
{
  fc_p = fc_pop();
  fc_cpu_op_res_flag_b();
  fc_cpu_op_set_flag_r();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bmi(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_n() != 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sec(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_set_flag_c();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rti(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0, t1;
  fc_p = fc_pop();
  t0 = fc_pop();
  t1 = fc_pop();
  fc_pc = (t1 << 8) | (t0 << 0);
  fc_cpu_op_res_flag_b();
  fc_cpu_op_set_flag_r();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_eor(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_acc ^= t0;
  fc_cpu_op_check_n(fc_acc);
  fc_cpu_op_check_z(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sre(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_cpu_op_w_flag_c((t0 & 0x01) ? 1 : 0);
  t0 >>= 1;
  fc_write(addr, t0);
  fc_acc ^= t0;
  fc_cpu_op_check_n(fc_acc);
  fc_cpu_op_check_z(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_lsr(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  fc_cpu_op_w_flag_c(t0 & 0x01);
  t0 >>= 1;
  fc_write(addr, t0);
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_lsr_acc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_acc;
  fc_cpu_op_w_flag_c(t0 & 0x01);
  t0 >>= 1;
  fc_acc = t0;
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_alr(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_jmp(fc_state_t *fs, uint16_t addr)
{
  fc_pc = addr;
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bvc(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_v() == 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_cli(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rts(fc_state_t *fs, uint16_t addr)
{
  uint16_t ret;
  ret = fc_pop();
  ret |= fc_pop() << 8;
  fc_pc = ++ret;
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_adc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  uint16_t t1;
  uint8_t t2;
  t0 = fc_read(addr);
  t1 = t0 + fc_acc + (fc_cpu_op_r_flag_c() ? 1 : 0);
  t2 = (uint8_t)t1;
  fc_cpu_op_w_flag_c(((t1 & 0xFF00) != 0) ? 1 : 0);
  fc_cpu_op_w_flag_v((!((fc_acc ^ t0) & 0x80) && ((fc_acc ^ t2) & 0x80)) ? 1 : 0);
  fc_acc = t2;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_rra(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0, t1;
  uint16_t t2;
  t0 = fc_read(addr);
  t1 = t0 & 0x01;
  t0 = (t0 >> 1) | (fc_cpu_op_r_flag_c() ? 0x80 : 0x00);
  fc_write(addr, t0);
  fc_cpu_op_w_flag_c(t1);
  t2 = fc_acc + t0 + (fc_cpu_op_r_flag_c() ? 0x01 : 0x00);
  fc_cpu_op_w_flag_v((!((fc_acc ^ t0) & 0x80) && ((fc_acc ^ t2) & 0x80)) ? 1 : 0);
  fc_acc = (uint8_t)t2;
  fc_cpu_op_w_flag_c(t2 > 0xff ? 1 : 0);
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ror_acc(fc_state_t *fs, uint16_t addr)
{
  uint16_t t0;
  t0 = fc_acc;
  if (fc_cpu_op_r_flag_c() != 0)
  {
    t0 |= 0x0100;
  }
  fc_cpu_op_w_flag_c(t0 & 0x01);
  t0 >>= 1;
  fc_acc = (uint8_t)t0;
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ror(fc_state_t *fs, uint16_t addr)
{
  uint16_t t0;
  t0 = fc_read(addr);
  if (fc_cpu_op_r_flag_c() != 0)
  {
    t0 |= 0x0100;
  }
  fc_cpu_op_w_flag_c(t0 & 0x01);
  t0 >>= 1;
  fc_write(addr, (uint8_t)t0);
  fc_cpu_op_check_n((uint8_t)t0);
  fc_cpu_op_check_z((uint8_t)t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_pla(fc_state_t *fs, uint16_t addr)
{
  fc_acc = fc_pop();
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_arr(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bvs(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_v() != 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sei(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_set_flag_i();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sta(fc_state_t *fs, uint16_t addr)
{
  fc_write(addr, fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sax(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_acc & fc_x;
  fc_write(addr, t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sty(fc_state_t *fs, uint16_t addr)
{
  fc_write(addr, fc_y);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_stx(fc_state_t *fs, uint16_t addr)
{
  fc_write(addr, fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_dey(fc_state_t *fs, uint16_t addr)
{
  fc_y--;
  fc_cpu_op_check_z(fc_y);
  fc_cpu_op_check_n(fc_y);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_txa(fc_state_t *fs, uint16_t addr)
{
  fc_acc = fc_x;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_xaa(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bcc(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_c() == 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ahx(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_tya(fc_state_t *fs, uint16_t addr)
{
  fc_acc = fc_y;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_txs(fc_state_t *fs, uint16_t addr)
{
  fc_sp = fc_x;
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_tas(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_shy(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_shx(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ldy(fc_state_t *fs, uint16_t addr)
{
  fc_y = fc_read(addr);
  fc_cpu_op_check_z(fc_y);
  fc_cpu_op_check_n(fc_y);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_lda(fc_state_t *fs, uint16_t addr)
{
  fc_acc = fc_read(addr);
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_ldx(fc_state_t *fs, uint16_t addr)
{
  fc_x = fc_read(addr);
  fc_cpu_op_check_z(fc_x);
  fc_cpu_op_check_n(fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_lax(fc_state_t *fs, uint16_t addr)
{
  fc_x = fc_acc = fc_read(addr);
  fc_cpu_op_check_z(fc_x);
  fc_cpu_op_check_n(fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_tay(fc_state_t *fs, uint16_t addr)
{
  fc_y = fc_acc;
  fc_cpu_op_check_z(fc_y);
  fc_cpu_op_check_n(fc_y);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_tax(fc_state_t *fs, uint16_t addr)
{
  fc_x = fc_acc;
  fc_cpu_op_check_z(fc_x);
  fc_cpu_op_check_n(fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_lxa(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bcs(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_c() != 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_clv(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_res_flag_v();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_tsx(fc_state_t *fs, uint16_t addr)
{
  fc_x = fc_sp;
  fc_cpu_op_check_n(fc_x);
  fc_cpu_op_check_z(fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_las(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_cpy(fc_state_t *fs, uint16_t addr)
{
  int16_t t0;
  t0 = fc_read(addr);
  t0 = (int16_t)fc_y - t0;
  fc_cpu_op_w_flag_c(t0 >= 0 ? 1 : 0);
  t0 &= 0xff;
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_cmp(fc_state_t *fs, uint16_t addr)
{
  int16_t t0;
  t0 = fc_read(addr);
  t0 = (int16_t)fc_acc - t0;
  fc_cpu_op_w_flag_c((t0 >= 0) ? 1 : 0);
  t0 &= 0xff;
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_dcp(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  int16_t t1;
  t0 = fc_read(addr);
  t0--;
  fc_write(addr, t0);
  t1 = fc_acc - t0;
  fc_cpu_op_check_z(t1);
  fc_cpu_op_check_n(t1);
  fc_cpu_op_w_flag_c((t1 >= 0) ? 1 : 0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_dec(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  t0--;
  fc_write(addr, t0);
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_bne(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_z() == 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_cld(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_res_flag_d();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_cpx(fc_state_t *fs, uint16_t addr)
{
  int16_t t0;
  t0 = fc_read(addr);
  t0 = (int16_t)fc_x - t0;
  fc_cpu_op_w_flag_c(t0 >= 0 ? 1 : 0);
  t0 &= 0xff;
  fc_cpu_op_check_n(t0);
  fc_cpu_op_check_z(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_beq(fc_state_t *fs, uint16_t addr)
{
  if (fc_cpu_op_r_flag_z() != 0)
  {
    fc_pc = addr;
  }
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sed(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_set_flag_d();
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_pha(fc_state_t *fs, uint16_t addr)
{
  fc_push(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_iny(fc_state_t *fs, uint16_t addr)
{
  fc_y++;
  fc_cpu_op_check_z(fc_y);
  fc_cpu_op_check_n(fc_y);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_dex(fc_state_t *fs, uint16_t addr)
{
  fc_x--;
  fc_cpu_op_check_z(fc_x);
  fc_cpu_op_check_n(fc_x);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_axs(fc_state_t *fs, uint16_t addr)
{
  fc_cpu_op_undefined(fs, addr);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_sbc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  uint16_t t1;
  uint8_t t2;
  t0 = fc_read(addr);
  t1 = fc_acc - t0 - (fc_cpu_op_r_flag_c() ? 0 : 1);
  t2 = (uint8_t)t1;
  fc_cpu_op_w_flag_c(((t1 & 0xFF00) != 0) ? 0 : 1);
  fc_cpu_op_w_flag_v((((fc_acc ^ t0) & 0x80) && ((fc_acc ^ t2) & 0x80)) ? 1 : 0);
  fc_acc = t2;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_isc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  uint16_t t1;
  t0 = fc_read(addr);
  t0++;
  fc_write(addr, t0);
  t1 = fc_acc - t0 - (fc_cpu_op_r_flag_c() ? 0 : 1);
  fc_cpu_op_w_flag_c(((t1 & 0xFF00) != 0) ? 0 : 1);
  fc_cpu_op_w_flag_v((((fc_acc ^ t0) & 0x80) && ((fc_acc ^ t1) & 0x80)) ? 1 : 0);
  fc_acc = (uint8_t)t1;
  fc_cpu_op_check_z(fc_acc);
  fc_cpu_op_check_n(fc_acc);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_inc(fc_state_t *fs, uint16_t addr)
{
  uint8_t t0;
  t0 = fc_read(addr);
  t0++;
  fc_write(addr, t0);
  fc_cpu_op_check_z(t0);
  fc_cpu_op_check_n(t0);
  return FC_RES_OK;
}
fc_res_t fc_cpu_op_inx(fc_state_t *fs, uint16_t addr)
{
  fc_x++;
  fc_cpu_op_check_z(fc_x);
  fc_cpu_op_check_n(fc_x);
  return FC_RES_OK;
}

/**
 *  execute one
 */
fc_res_t fc_cpu_execute_one(fc_state_t *fs)
{
  static int i = 0;
  uint8_t opcode;
  uint16_t addr;
  addr = fc_pc;
  fc_fetch(opcode);
  printf("%04d\t%04X %02X %s\tA:%02X\tX:%02X\tY:%02X\tP:%02X\tSP:%02X\r\n", ++i, addr, opcode, fc_op_info[opcode].name, fc_acc, fc_x, fc_y, fc_p, fc_sp);
  switch (opcode)
  {
    fc_cpu_op(0x00, brk, imm);
    fc_cpu_op(0x01, ora, ind_x);
    fc_cpu_op(0x02, stp, none);
    fc_cpu_op(0x03, slo, ind_x);
    fc_cpu_op(0x04, nop, zero_page);
    fc_cpu_op(0x05, ora, zero_page);
    fc_cpu_op(0x06, asl, zero_page);
    fc_cpu_op(0x07, slo, zero_page);
    fc_cpu_op(0x08, php, imp);
    fc_cpu_op(0x09, ora, imm);
    fc_cpu_op(0x0A, asl_acc, acc);
    fc_cpu_op(0x0B, anc, imm);
    fc_cpu_op(0x0C, nop, abs);
    fc_cpu_op(0x0D, ora, abs);
    fc_cpu_op(0x0E, asl, abs);
    fc_cpu_op(0x0F, slo, abs);
    fc_cpu_op(0x10, bpl, rel);
    fc_cpu_op(0x11, ora, ind_y);
    fc_cpu_op(0x12, stp, none);
    fc_cpu_op(0x13, slo, ind_y);
    fc_cpu_op(0x14, nop, zero_page_x);
    fc_cpu_op(0x15, ora, zero_page_x);
    fc_cpu_op(0x16, asl, zero_page_x);
    fc_cpu_op(0x17, slo, zero_page_x);
    fc_cpu_op(0x18, clc, imp);
    fc_cpu_op(0x19, ora, abs_y);
    fc_cpu_op(0x1A, nop, imp);
    fc_cpu_op(0x1B, slo, abs_y);
    fc_cpu_op(0x1C, nop, abs_x);
    fc_cpu_op(0x1D, ora, abs_x);
    fc_cpu_op(0x1E, asl, abs_x);
    fc_cpu_op(0x1F, slo, abs_x);
    fc_cpu_op(0x20, jsr, abs);
    fc_cpu_op(0x21, and, ind_x);
    fc_cpu_op(0x22, stp, none);
    fc_cpu_op(0x23, rla, ind_x);
    fc_cpu_op(0x24, bit, zero_page);
    fc_cpu_op(0x25, and, zero_page);
    fc_cpu_op(0x26, rol, zero_page);
    fc_cpu_op(0x27, rla, zero_page);
    fc_cpu_op(0x28, plp, imp);
    fc_cpu_op(0x29, and, imm);
    fc_cpu_op(0x2A, rol_acc, acc);
    fc_cpu_op(0x2B, anc, imm);
    fc_cpu_op(0x2C, bit, abs);
    fc_cpu_op(0x2D, and, abs);
    fc_cpu_op(0x2E, rol, abs);
    fc_cpu_op(0x2F, rla, abs);
    fc_cpu_op(0x30, bmi, rel);
    fc_cpu_op(0x31, and, ind_y);
    fc_cpu_op(0x32, stp, none);
    fc_cpu_op(0x33, rla, ind_y);
    fc_cpu_op(0x34, nop, zero_page_x);
    fc_cpu_op(0x35, and, zero_page_x);
    fc_cpu_op(0x36, rol, zero_page_x);
    fc_cpu_op(0x37, rla, zero_page_x);
    fc_cpu_op(0x38, sec, imp);
    fc_cpu_op(0x39, and, abs_y);
    fc_cpu_op(0x3A, nop, imp);
    fc_cpu_op(0x3B, rla, abs_y);
    fc_cpu_op(0x3C, nop, abs_x);
    fc_cpu_op(0x3D, and, abs_x);
    fc_cpu_op(0x3E, rol, abs_x);
    fc_cpu_op(0x3F, rla, abs_x);
    fc_cpu_op(0x40, rti, imp);
    fc_cpu_op(0x41, eor, ind_x);
    fc_cpu_op(0x42, stp, none);
    fc_cpu_op(0x43, sre, ind_x);
    fc_cpu_op(0x44, nop, zero_page);
    fc_cpu_op(0x45, eor, zero_page);
    fc_cpu_op(0x46, lsr, zero_page);
    fc_cpu_op(0x47, sre, zero_page);
    fc_cpu_op(0x48, pha, imp);
    fc_cpu_op(0x49, eor, imm);
    fc_cpu_op(0x4A, lsr_acc, acc);
    fc_cpu_op(0x4B, alr, imm);
    fc_cpu_op(0x4C, jmp, abs);
    fc_cpu_op(0x4D, eor, abs);
    fc_cpu_op(0x4E, lsr, abs);
    fc_cpu_op(0x4F, sre, abs);
    fc_cpu_op(0x50, bvc, rel);
    fc_cpu_op(0x51, eor, ind_y);
    fc_cpu_op(0x52, stp, none);
    fc_cpu_op(0x53, sre, ind_y);
    fc_cpu_op(0x54, nop, zero_page_x);
    fc_cpu_op(0x55, eor, zero_page_x);
    fc_cpu_op(0x56, lsr, zero_page_x);
    fc_cpu_op(0x57, sre, zero_page_x);
    fc_cpu_op(0x58, cli, imp);
    fc_cpu_op(0x59, eor, abs_y);
    fc_cpu_op(0x5A, nop, imp);
    fc_cpu_op(0x5B, sre, abs_y);
    fc_cpu_op(0x5C, nop, abs_x);
    fc_cpu_op(0x5D, eor, abs_x);
    fc_cpu_op(0x5E, lsr, abs_x);
    fc_cpu_op(0x5F, sre, abs_x);
    fc_cpu_op(0x60, rts, imp);
    fc_cpu_op(0x61, adc, ind_x);
    fc_cpu_op(0x62, stp, none);
    fc_cpu_op(0x63, rra, ind_x);
    fc_cpu_op(0x64, nop, zero_page);
    fc_cpu_op(0x65, adc, zero_page);
    fc_cpu_op(0x66, ror, zero_page);
    fc_cpu_op(0x67, rra, zero_page);
    fc_cpu_op(0x68, pla, imp);
    fc_cpu_op(0x69, adc, imm);
    fc_cpu_op(0x6A, ror_acc, acc);
    fc_cpu_op(0x6B, arr, imm);
    fc_cpu_op(0x6C, jmp, ind);
    fc_cpu_op(0x6D, adc, abs);
    fc_cpu_op(0x6E, ror, abs);
    fc_cpu_op(0x6F, rra, abs);
    fc_cpu_op(0x70, bvs, rel);
    fc_cpu_op(0x71, adc, ind_y);
    fc_cpu_op(0x72, stp, none);
    fc_cpu_op(0x73, rra, ind_y);
    fc_cpu_op(0x74, nop, zero_page_x);
    fc_cpu_op(0x75, adc, zero_page_x);
    fc_cpu_op(0x76, ror, zero_page_x);
    fc_cpu_op(0x77, rra, zero_page_x);
    fc_cpu_op(0x78, sei, imp);
    fc_cpu_op(0x79, adc, abs_y);
    fc_cpu_op(0x7A, nop, imp);
    fc_cpu_op(0x7B, rra, abs_y);
    fc_cpu_op(0x7C, nop, abs_x);
    fc_cpu_op(0x7D, adc, abs_x);
    fc_cpu_op(0x7E, ror, abs_x);
    fc_cpu_op(0x7F, rra, abs_x);
    fc_cpu_op(0x80, nop, imm);
    fc_cpu_op(0x81, sta, ind_x);
    fc_cpu_op(0x82, nop, imm);
    fc_cpu_op(0x83, sax, ind_x);
    fc_cpu_op(0x84, sty, zero_page);
    fc_cpu_op(0x85, sta, zero_page);
    fc_cpu_op(0x86, stx, zero_page);
    fc_cpu_op(0x87, sax, zero_page);
    fc_cpu_op(0x88, dey, imp);
    fc_cpu_op(0x89, nop, imm);
    fc_cpu_op(0x8A, txa, imp);
    fc_cpu_op(0x8B, xaa, imm);
    fc_cpu_op(0x8C, sty, abs);
    fc_cpu_op(0x8D, sta, abs);
    fc_cpu_op(0x8E, stx, abs);
    fc_cpu_op(0x8F, sax, abs);
    fc_cpu_op(0x90, bcc, rel);
    fc_cpu_op(0x91, sta, ind_y);
    fc_cpu_op(0x92, stp, none);
    fc_cpu_op(0x93, ahx, none);
    fc_cpu_op(0x94, sty, zero_page_x);
    fc_cpu_op(0x95, sta, zero_page_x);
    fc_cpu_op(0x96, stx, zero_page_y);
    fc_cpu_op(0x97, sax, zero_page_y);
    fc_cpu_op(0x98, tya, imp);
    fc_cpu_op(0x99, sta, abs_y);
    fc_cpu_op(0x9A, txs, imp);
    fc_cpu_op(0x9B, tas, abs_y);
    fc_cpu_op(0x9C, shy, abs_x);
    fc_cpu_op(0x9D, sta, abs_x);
    fc_cpu_op(0x9E, shx, abs_y);
    fc_cpu_op(0x9F, ahx, none);
    fc_cpu_op(0xA0, ldy, imm);
    fc_cpu_op(0xA1, lda, ind_x);
    fc_cpu_op(0xA2, ldx, imm);
    fc_cpu_op(0xA3, lax, ind_x);
    fc_cpu_op(0xA4, ldy, zero_page);
    fc_cpu_op(0xA5, lda, zero_page);
    fc_cpu_op(0xA6, ldx, zero_page);
    fc_cpu_op(0xA7, lax, zero_page);
    fc_cpu_op(0xA8, tay, imp);
    fc_cpu_op(0xA9, lda, imm);
    fc_cpu_op(0xAA, tax, imp);
    fc_cpu_op(0xAB, lxa, imm);
    fc_cpu_op(0xAC, ldy, abs);
    fc_cpu_op(0xAD, lda, abs);
    fc_cpu_op(0xAE, ldx, abs);
    fc_cpu_op(0xAF, lax, abs);
    fc_cpu_op(0xB0, bcs, rel);
    fc_cpu_op(0xB1, lda, ind_y);
    fc_cpu_op(0xB2, stp, none);
    fc_cpu_op(0xB3, lax, ind_y);
    fc_cpu_op(0xB4, ldy, zero_page_x);
    fc_cpu_op(0xB5, lda, zero_page_x);
    fc_cpu_op(0xB6, ldx, zero_page_y);
    fc_cpu_op(0xB7, lax, zero_page_y);
    fc_cpu_op(0xB8, clv, imp);
    fc_cpu_op(0xB9, lda, abs_y);
    fc_cpu_op(0xBA, tsx, imp);
    fc_cpu_op(0xBB, las, abs);
    fc_cpu_op(0xBC, ldy, abs_x);
    fc_cpu_op(0xBD, lda, abs_x);
    fc_cpu_op(0xBE, ldx, abs_y);
    fc_cpu_op(0xBF, lax, abs_y);
    fc_cpu_op(0xC0, cpy, imm);
    fc_cpu_op(0xC1, cmp, ind_x);
    fc_cpu_op(0xC2, nop, imm);
    fc_cpu_op(0xC3, dcp, ind_x);
    fc_cpu_op(0xC4, cpy, zero_page);
    fc_cpu_op(0xC5, cmp, zero_page);
    fc_cpu_op(0xC6, dec, zero_page);
    fc_cpu_op(0xC7, dcp, zero_page);
    fc_cpu_op(0xC8, iny, imp);
    fc_cpu_op(0xC9, cmp, imm);
    fc_cpu_op(0xCA, dex, imp);
    fc_cpu_op(0xCB, axs, imm);
    fc_cpu_op(0xCC, cpy, abs);
    fc_cpu_op(0xCD, cmp, abs);
    fc_cpu_op(0xCE, dec, abs);
    fc_cpu_op(0xCF, dcp, abs);
    fc_cpu_op(0xD0, bne, rel);
    fc_cpu_op(0xD1, cmp, ind_y);
    fc_cpu_op(0xD2, stp, none);
    fc_cpu_op(0xD3, dcp, ind_y);
    fc_cpu_op(0xD4, nop, zero_page_x);
    fc_cpu_op(0xD5, cmp, zero_page_x);
    fc_cpu_op(0xD6, dec, zero_page_x);
    fc_cpu_op(0xD7, dcp, zero_page_x);
    fc_cpu_op(0xD8, cld, imp);
    fc_cpu_op(0xD9, cmp, abs_y);
    fc_cpu_op(0xDA, nop, imp);
    fc_cpu_op(0xDB, dcp, abs_y);
    fc_cpu_op(0xDC, nop, abs_x);
    fc_cpu_op(0xDD, cmp, abs_x);
    fc_cpu_op(0xDE, dec, abs_x);
    fc_cpu_op(0xDF, dcp, abs_x);
    fc_cpu_op(0xE0, cpx, imm);
    fc_cpu_op(0xE1, sbc, ind_x);
    fc_cpu_op(0xE2, nop, imm);
    fc_cpu_op(0xE3, isc, ind_x);
    fc_cpu_op(0xE4, cpx, zero_page);
    fc_cpu_op(0xE5, sbc, zero_page);
    fc_cpu_op(0xE6, inc, zero_page);
    fc_cpu_op(0xE7, isc, zero_page);
    fc_cpu_op(0xE8, inx, imp);
    fc_cpu_op(0xE9, sbc, imm);
    fc_cpu_op(0xEA, nop, imp);
    fc_cpu_op(0xEB, sbc, imm);
    fc_cpu_op(0xEC, cpx, abs);
    fc_cpu_op(0xED, sbc, abs);
    fc_cpu_op(0xEE, inc, abs);
    fc_cpu_op(0xEF, isc, abs);
    fc_cpu_op(0xF0, beq, rel);
    fc_cpu_op(0xF1, sbc, ind_y);
    fc_cpu_op(0xF2, stp, none);
    fc_cpu_op(0xF3, isc, ind_y);
    fc_cpu_op(0xF4, nop, zero_page_x);
    fc_cpu_op(0xF5, sbc, zero_page_x);
    fc_cpu_op(0xF6, inc, zero_page_x);
    fc_cpu_op(0xF7, isc, zero_page_x);
    fc_cpu_op(0xF8, sed, imp);
    fc_cpu_op(0xF9, sbc, abs_y);
    fc_cpu_op(0xFA, nop, imp);
    fc_cpu_op(0xFB, isc, abs_y);
    fc_cpu_op(0xFC, nop, abs_x);
    fc_cpu_op(0xFD, sbc, abs_x);
    fc_cpu_op(0xFE, inc, abs_x);
    fc_cpu_op(0xFF, isc, abs_x);
  default:
    break;
  }
  return 0x00;
}

fc_res_t fc_execute(fc_state_t *fs)
{
  fs->cpu.reg.pc = 0xC000;
  fc_p = 0x24;
  for (;;)
  {
    fc_cpu_execute_one(fs);
  }
  return FC_RES_OK;
}
/*
* opcode info
*/
const fc_op_info_t fc_op_info[256] = {
    {"BRK", FC_ADDR_MODE_IMM},
    {"ORA", FC_ADDR_MODE_IND_X},
    {"STP", FC_ADDR_MODE_NONE},
    {"SLO", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE},
    {"ORA", FC_ADDR_MODE_ZERO_PAGE},
    {"ASL", FC_ADDR_MODE_ZERO_PAGE},
    {"SLO", FC_ADDR_MODE_ZERO_PAGE},
    {"PHP", FC_ADDR_MODE_IMP},
    {"ORA", FC_ADDR_MODE_IMM},
    {"ASL", FC_ADDR_MODE_ACC},
    {"ANC", FC_ADDR_MODE_IMM},
    {"NOP", FC_ADDR_MODE_ABS},
    {"ORA", FC_ADDR_MODE_ABS},
    {"ASL", FC_ADDR_MODE_ABS},
    {"SLO", FC_ADDR_MODE_ABS},
    {"BPL", FC_ADDR_MODE_REL},
    {"ORA", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"SLO", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ORA", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ASL", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SLO", FC_ADDR_MODE_ZERO_PAGE_X},
    {"CLC", FC_ADDR_MODE_IMM},
    {"ORA", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMM},
    {"SLO", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"ORA", FC_ADDR_MODE_ABS_X},
    {"ASL", FC_ADDR_MODE_ABS_X},
    {"SLO", FC_ADDR_MODE_ABS_X},
    {"JSR", FC_ADDR_MODE_ABS},
    {"AND", FC_ADDR_MODE_IND_X},
    {"STP", FC_ADDR_MODE_NONE},
    {"RLA", FC_ADDR_MODE_IND_X},
    {"BIT", FC_ADDR_MODE_ZERO_PAGE},
    {"AND", FC_ADDR_MODE_ZERO_PAGE},
    {"ROL", FC_ADDR_MODE_ZERO_PAGE},
    {"RLA", FC_ADDR_MODE_ZERO_PAGE},
    {"PLP", FC_ADDR_MODE_IMP},
    {"AND", FC_ADDR_MODE_IMM},
    {"ROL", FC_ADDR_MODE_ACC},
    {"ANC", FC_ADDR_MODE_IMM},
    {"BIT", FC_ADDR_MODE_ABS},
    {"AND", FC_ADDR_MODE_ABS},
    {"ROL", FC_ADDR_MODE_ABS},
    {"RLA", FC_ADDR_MODE_ABS},
    {"BMI", FC_ADDR_MODE_REL},
    {"AND", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"RLA", FC_ADDR_MODE_IND_Y},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"AND", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ROL", FC_ADDR_MODE_ZERO_PAGE_X},
    {"RLA", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SEC", FC_ADDR_MODE_IMP},
    {"AND", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMP},
    {"RLA", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"AND", FC_ADDR_MODE_ABS_X},
    {"ROL", FC_ADDR_MODE_ABS_X},
    {"RLA", FC_ADDR_MODE_ABS_X},
    {"RTI", FC_ADDR_MODE_IMP},
    {"EOR", FC_ADDR_MODE_IND_X},
    {"STP", FC_ADDR_MODE_NONE},
    {"SRE", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE},
    {"EOR", FC_ADDR_MODE_ZERO_PAGE},
    {"LSR", FC_ADDR_MODE_ACC},
    {"SRE", FC_ADDR_MODE_ZERO_PAGE},
    {"PHA", FC_ADDR_MODE_IMP},
    {"EOR", FC_ADDR_MODE_IMM},
    {"LSR", FC_ADDR_MODE_ZERO_PAGE},
    {"ALR", FC_ADDR_MODE_IMM},
    {"JMP", FC_ADDR_MODE_ABS},
    {"EOR", FC_ADDR_MODE_ABS},
    {"LSR", FC_ADDR_MODE_ABS},
    {"SRE", FC_ADDR_MODE_ABS},
    {"BVC", FC_ADDR_MODE_REL},
    {"EOR", FC_ADDR_MODE_IND_X},
    {"STP", FC_ADDR_MODE_NONE},
    {"SRE", FC_ADDR_MODE_IND_Y},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"EOR", FC_ADDR_MODE_ZERO_PAGE_X},
    {"LSR", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SRE", FC_ADDR_MODE_ZERO_PAGE_X},
    {"CLI", FC_ADDR_MODE_IMP},
    {"EOR", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMP},
    {"SRE", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"EOR", FC_ADDR_MODE_ABS_X},
    {"LSR", FC_ADDR_MODE_ABS_X},
    {"SRE", FC_ADDR_MODE_ABS_X},
    {"RTS", FC_ADDR_MODE_IMP},
    {"ADC", FC_ADDR_MODE_IND_X},
    {"STP", FC_ADDR_MODE_NONE},
    {"RRA", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE},
    {"ADC", FC_ADDR_MODE_ZERO_PAGE},
    {"ROR", FC_ADDR_MODE_ZERO_PAGE},
    {"RRA", FC_ADDR_MODE_ZERO_PAGE},
    {"PLA", FC_ADDR_MODE_IMP},
    {"ADC", FC_ADDR_MODE_IMM},
    {"ROR", FC_ADDR_MODE_ACC},
    {"ARR", FC_ADDR_MODE_IMM},
    {"JMP", FC_ADDR_MODE_IND},
    {"ADC", FC_ADDR_MODE_ABS},
    {"ROR", FC_ADDR_MODE_ABS},
    {"RRA", FC_ADDR_MODE_ABS},
    {"BVS", FC_ADDR_MODE_REL},
    {"ADC", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"RRA", FC_ADDR_MODE_IND_Y},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ADC", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ROR", FC_ADDR_MODE_ZERO_PAGE_X},
    {"RRA", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SEI", FC_ADDR_MODE_IMP},
    {"ADC", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMP},
    {"RRA", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"ADC", FC_ADDR_MODE_ABS_X},
    {"ROR", FC_ADDR_MODE_ABS_X},
    {"RRA", FC_ADDR_MODE_ABS_X},
    {"NOP", FC_ADDR_MODE_IMM},
    {"STA", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_IMM},
    {"SAX", FC_ADDR_MODE_IND_X},
    {"STY", FC_ADDR_MODE_ZERO_PAGE},
    {"STA", FC_ADDR_MODE_ZERO_PAGE},
    {"STX", FC_ADDR_MODE_ZERO_PAGE},
    {"SAX", FC_ADDR_MODE_ZERO_PAGE},
    {"DEY", FC_ADDR_MODE_IMP},
    {"NOP", FC_ADDR_MODE_IMM},
    {"TXA", FC_ADDR_MODE_IMP},
    {"XAA", FC_ADDR_MODE_IMM},
    {"STY", FC_ADDR_MODE_ABS},
    {"STA", FC_ADDR_MODE_ABS},
    {"STX", FC_ADDR_MODE_ABS},
    {"SAX", FC_ADDR_MODE_ABS_Y},
    {"BCC", FC_ADDR_MODE_REL},
    {"STA", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"AHX", FC_ADDR_MODE_NONE},
    {"STY", FC_ADDR_MODE_ZERO_PAGE_X},
    {"STA", FC_ADDR_MODE_ZERO_PAGE_X},
    {"STX", FC_ADDR_MODE_ZERO_PAGE_Y},
    {"SAX", FC_ADDR_MODE_ZERO_PAGE_Y},
    {"TYA", FC_ADDR_MODE_IMP},
    {"STA", FC_ADDR_MODE_ABS_Y},
    {"TXS", FC_ADDR_MODE_IMP},
    {"TAS", FC_ADDR_MODE_ABS_Y},
    {"SHY", FC_ADDR_MODE_ABS_X},
    {"STA", FC_ADDR_MODE_ABS_X},
    {"SHX", FC_ADDR_MODE_ABS_Y},
    {"AHX", FC_ADDR_MODE_NONE},
    {"LDY", FC_ADDR_MODE_IMM},
    {"LDA", FC_ADDR_MODE_IND_X},
    {"LDX", FC_ADDR_MODE_IMM},
    {"LAX", FC_ADDR_MODE_IND_X},
    {"LDY", FC_ADDR_MODE_ZERO_PAGE},
    {"LDA", FC_ADDR_MODE_ZERO_PAGE},
    {"LDX", FC_ADDR_MODE_ZERO_PAGE},
    {"LAX", FC_ADDR_MODE_ZERO_PAGE},
    {"TAY", FC_ADDR_MODE_IMP},
    {"LDA", FC_ADDR_MODE_IMM},
    {"TAX", FC_ADDR_MODE_IMP},
    {"LXA", FC_ADDR_MODE_IMM},
    {"LDY", FC_ADDR_MODE_ABS},
    {"LDA", FC_ADDR_MODE_ABS},
    {"LDX", FC_ADDR_MODE_ABS},
    {"LAX", FC_ADDR_MODE_ABS},
    {"BCS", FC_ADDR_MODE_REL},
    {"LDA", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"LAX", FC_ADDR_MODE_IND_Y},
    {"LDY", FC_ADDR_MODE_ZERO_PAGE_X},
    {"LDA", FC_ADDR_MODE_ZERO_PAGE_X},
    {"LDX", FC_ADDR_MODE_ZERO_PAGE_Y},
    {"LAX", FC_ADDR_MODE_ZERO_PAGE_Y},
    {"CLV", FC_ADDR_MODE_IMP},
    {"LDA", FC_ADDR_MODE_ABS_Y},
    {"TSX", FC_ADDR_MODE_IMP},
    {"LAS", FC_ADDR_MODE_ABS},
    {"LDY", FC_ADDR_MODE_ABS_X},
    {"LDA", FC_ADDR_MODE_ABS_X},
    {"LDX", FC_ADDR_MODE_ABS_Y},
    {"LAX", FC_ADDR_MODE_ABS_Y},
    {"CPY", FC_ADDR_MODE_IMM},
    {"CMP", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_IMM},
    {"DCP", FC_ADDR_MODE_IND_X},
    {"CPY", FC_ADDR_MODE_ZERO_PAGE},
    {"CMP", FC_ADDR_MODE_ZERO_PAGE},
    {"DEC", FC_ADDR_MODE_ZERO_PAGE},
    {"DCP", FC_ADDR_MODE_ZERO_PAGE},
    {"INY", FC_ADDR_MODE_IMP},
    {"CMP", FC_ADDR_MODE_IMM},
    {"DEX", FC_ADDR_MODE_IMP},
    {"AXS", FC_ADDR_MODE_IMM},
    {"CPY", FC_ADDR_MODE_ABS},
    {"CMP", FC_ADDR_MODE_ABS},
    {"DEC", FC_ADDR_MODE_ABS},
    {"DCP", FC_ADDR_MODE_ABS},
    {"BNE", FC_ADDR_MODE_REL},
    {"CMP", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"DCP", FC_ADDR_MODE_IND_Y},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"CMP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"DEC", FC_ADDR_MODE_ZERO_PAGE_X},
    {"DCP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"CLD", FC_ADDR_MODE_IMP},
    {"CMP", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMP},
    {"DCP", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"CMP", FC_ADDR_MODE_ABS_X},
    {"DEC", FC_ADDR_MODE_ABS_X},
    {"DCP", FC_ADDR_MODE_ABS_X},
    {"CPX", FC_ADDR_MODE_IMM},
    {"SBC", FC_ADDR_MODE_IND_X},
    {"NOP", FC_ADDR_MODE_IMM},
    {"ISC", FC_ADDR_MODE_IND_X},
    {"CPX", FC_ADDR_MODE_ZERO_PAGE},
    {"SBC", FC_ADDR_MODE_ZERO_PAGE},
    {"INC", FC_ADDR_MODE_ZERO_PAGE},
    {"ISC", FC_ADDR_MODE_ZERO_PAGE},
    {"INX", FC_ADDR_MODE_IMP},
    {"SBC", FC_ADDR_MODE_IMM},
    {"NOP", FC_ADDR_MODE_IMP},
    {"SBC", FC_ADDR_MODE_IMM},
    {"CPX", FC_ADDR_MODE_ABS},
    {"SBC", FC_ADDR_MODE_ABS},
    {"INC", FC_ADDR_MODE_ABS},
    {"ISC", FC_ADDR_MODE_ABS},
    {"BEQ", FC_ADDR_MODE_REL},
    {"SBC", FC_ADDR_MODE_IND_Y},
    {"STP", FC_ADDR_MODE_NONE},
    {"ISC", FC_ADDR_MODE_IND_Y},
    {"NOP", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SBC", FC_ADDR_MODE_ZERO_PAGE_X},
    {"INC", FC_ADDR_MODE_ZERO_PAGE_X},
    {"ISC", FC_ADDR_MODE_ZERO_PAGE_X},
    {"SED", FC_ADDR_MODE_IMP},
    {"SBC", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_IMP},
    {"ISC", FC_ADDR_MODE_ABS_Y},
    {"NOP", FC_ADDR_MODE_ABS_X},
    {"SBC", FC_ADDR_MODE_ABS_X},
    {"INC", FC_ADDR_MODE_ABS_X},
    {"ISC", FC_ADDR_MODE_ABS_X}};
