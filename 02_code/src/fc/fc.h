#ifndef __FC_H
#define __FC_H

#include "stdint.h"

#include "fc_type.h"


extern fc_state_t* fc_new(void);

extern fc_res_t fc_dofile(fc_state_t *fs,const char *path);

extern fc_res_t fc_dobuffer(fc_state_t *fs,const uint8_t *rom);

extern fc_res_t fc_do(fc_state_t *fs,fc_reader reader,void *arg);

extern fc_res_t fc_del(fc_state_t** fs);

#endif
