#ifndef __FC_PPU_H
#define __FC_PPU_H

#include "stdint.h"

extern uint8_t fc_ppu_r_reg(uint8_t addr);
extern void fc_ppu_w_reg(uint8_t addr,uint8_t data);

#endif
