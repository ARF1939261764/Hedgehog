
#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "malloc.h"

#include "fc.h"
#include "fc_cpu.h"

/*
* buffer loader info
*/
typedef struct
{
  const uint8_t *rom;
  uint32_t offset;
} fc_buffer_loader_info_t;

/**
 *  description:default memory allocation function
 *  @param ptr    : old ptr
 *  @param nsize  : size
 *  
 *  @return void*
 */
void *fc_malloc(void *ptr, uint32_t nsize)
{
  void *newptr;
  if (nsize == 0)
  {
    free(ptr);
    newptr = NULL;
  }
  else
  {
    newptr = malloc(nsize);
  }
  return newptr;
}

/**
 *  description:file loader
 *  @param stream : file stream
 *  @param buff   : target buffer
 *  @param len    : length
 *  
 *  @return void
 */
void fc_file_loader(FILE *stream, uint8_t *buff, uint32_t len)
{
  if (buff != NULL)
  {
    fread(buff, sizeof(uint8_t), len, stream);
  }
  else
  {
    fseek(stream, 512, SEEK_CUR);
  }
}

/**
 *  description:buffer loader
 *  @param stream : buffer info
 *  @param buff   : target buffer
 *  @param len    : length
 *  
 *  @return void
 */
void fc_buffer_loader(fc_buffer_loader_info_t *stream, uint8_t *buff, uint32_t len)
{
  if (buff != NULL)
  {
    memcpy(buff, stream->rom + stream->offset, len);
  }
  stream->offset += len;
}
/**
 *  description:create a new state machine
 *  @param  f:memory allocation function
 *  
 *  @return fc_state_t*
 */
fc_state_t* fc_alloc_new(fc_alloc f)
{
  fc_state_t *fs;
  if (f == NULL)
  {
    return NULL;
  }
  fs = f(NULL, sizeof(fc_state_t));
  if (fs == NULL)
  {
    return NULL;
  }
  memset(fs, (int)NULL, sizeof(fc_state_t));
  fs->frealloc = f;
  fs->cpu.ram = f(NULL, CPU_6502_RAM_SIZE);
  fs->cpu.reg.sp = 0xFD;
  if (fs->cpu.ram == NULL)
  {
    fs = f(fs, 0);
  }
  return fs;
}

/**
 *  description:create a new state machine
 *  @param  void
 *  
 *  @return fc_state_t*
 */
fc_state_t *fc_new(void)
{
  fc_state_t *fs;
  fs = fc_alloc_new(fc_malloc);
  return fs;
}

fc_res_t fc_del(fc_state_t **fs)
{
  fc_alloc f;
  if (fs == NULL)
  {
    return FC_RES_INPUT_PTR_NULL;
  }
  f = (*fs)->frealloc;
  f((*fs)->cpu.ram, 0);
  f((*fs)->cpu.prg, 0);
  f((*fs)->cpu.chr, 0);
  f((*fs), 0);
  *fs = NULL;
  return FC_RES_OK;
}

/**
 *  description:execute a ROM file
 *  @param fc_state_t : emulator state sachine
 *  @param path     : ROM file path
 *  
 *  @return fc_res_t
 */
fc_res_t fc_dofile(fc_state_t *fs, const char *path)
{
  fc_res_t status;
  FILE *stream;
  stream = fopen(path, "rb");
  if (stream == NULL)
  {
    return FC_RES_CALL_FAIL;
  }
  status = fc_do(fs, fc_file_loader, stream);
  fclose(stream);
  return status;
}

/**
 *  description:execute ROM buffer
 *  @param fc_state_t : emulator state sachine
 *  @param rom      : ROM buffer
 *  
 *  @return fc_res_t
 */
fc_res_t fc_dobuffer(fc_state_t *fs, const uint8_t *rom)
{
  fc_res_t status;
  fc_buffer_loader_info_t loader_info;
  /*check arg*/
  if (fs == NULL || rom == NULL)
  {
    return FC_RES_INPUT_PTR_NULL;
  }
  loader_info.offset = 0;
  loader_info.rom = rom;
  status = fc_do(fs, fc_buffer_loader, &loader_info);
  return status;
}

void fc_load_ines_header(fc_state_t *fs, fc_reader reader, void *arg, uint8_t *header)
{
  fc_header_info_t *h = &fs->hinfo;
  h->prg_rom_size = header[4] * 16384;
  h->chr_rom_size = header[5] * 8192;
  h->flag6_mirror = (header[6] >> 0) & 0x01;
  h->flag6_bkram = (header[6] >> 1) & 0x01;
  h->flag6_trainer = (header[6] >> 2) & 0x01;
  h->flag6_hwm = (header[6] >> 3) & 0x01;
  h->mapper = ((header[7] >> 0) & 0xF0) | ((header[6] >> 4) & 0x0F);
  h->flag7_ctype = (header[7] >> 0) & 0x03;
  h->flag7_nes20 = (header[7] >> 2) & 0x03;
}
fc_res_t fc_load_ines_rom(fc_state_t *fs, fc_reader reader, void *arg, uint8_t *header)
{
  uint8_t *memory;
  fc_res_t status;
  fc_alloc f;
  fc_cpu_t *cpu;
  fc_header_info_t *hinfo;
  status = FC_RES_OK;
  f = fs->frealloc;
  cpu = &fs->cpu;
  hinfo = &fs->hinfo;
  fc_load_ines_header(fs, reader, arg, header);
  memory = f(NULL, hinfo->prg_rom_size);
  if (memory == NULL)
  {
    return FC_RES_OUT_OF_MEMORY;
  }
  cpu->prg = memory;
  memory = f(NULL, hinfo->chr_rom_size);
  if (memory == NULL)
  {
    return FC_RES_OUT_OF_MEMORY;
  }
  cpu->chr = memory;
  if (hinfo->flag6_trainer)
  {
    /*jump*/
    reader(arg, NULL, 512);
  }
  cpu->bank[4] = cpu->prg;
  cpu->bank[5] = cpu->prg + CPU_6502_BANK_SIZE;
  cpu->bank[6] = cpu->prg + (hinfo->prg_rom_size == 16*1024 ? 0 : 16*1024);
  cpu->bank[7] = cpu->prg + CPU_6502_BANK_SIZE + (hinfo->prg_rom_size == 16*1024 ? 0 : 16*1024);
  reader(arg, cpu->prg, hinfo->prg_rom_size);
  reader(arg, cpu->chr, hinfo->chr_rom_size);
  return status;
}
/**
 *  description:execute ROM in INES format
 *  @param fc_state_t : emulator state sachine
 *  @param reader   : read ROM file interface
 *  @param arg      : interface arg
 *  @param header   : file header
 *  @return fc_res_t
 */
fc_res_t fc_do_ines(fc_state_t *fs, fc_reader reader, void *arg, uint8_t *header)
{
  fc_res_t status;
  status = fc_load_ines_rom(fs, reader, arg, header);
  if (status != FC_RES_OK)
  {
    return status;
  }
  status = fc_execute(fs);
  return status;
}
/**
 *  description:execute ROM in NES20 format
 *  @param fc_state_t : emulator state sachine
 *  @param reader   : read ROM file interface
 *  @param arg      : interface arg
 *  @param header   : file header
 *  @return fc_res_t
 */
fc_res_t fc_do_nes20(fc_state_t *fs, fc_reader reader, void *arg, uint8_t *header)
{
  /*nonsupport*/
  return FC_RES_CALL_FAIL;
}

/**
 *  description:execute ROM
 *  @param fc_state_t : emulator state sachine
 *  @param reader   : read ROM file interface
 *  @param arg      : interface arg
 *  @return fc_res_t
 */
fc_res_t fc_do(fc_state_t *fs, fc_reader reader, void *arg)
{
  fc_res_t status;
  uint8_t header[16];
  fc_format_t format = fc_format_null;
  /*check arg*/
  if (fs == NULL || reader == NULL || arg == NULL)
  {
    return FC_RES_INPUT_PTR_NULL;
  }
  /*ger header*/
  reader(arg, header, 16);
  /*identifying file types*/
  /*ines*/
  if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)
  {
    format = fc_format_ines;
  }
  /*nes2.0*/
  if (format == fc_format_ines && (header[7] & 0x0C) == 0x08)
  {
    format = fc_format_nes20;
  }

  fs->format = format;

  /*run*/
  switch (format)
  {
  case fc_format_ines:
    status = fc_do_ines(fs, reader, arg, header);
    break;
  case fc_format_nes20:
    status = fc_do_nes20(fs, reader, arg, header);
    break;
  default:
    status = FC_RES_CALL_FAIL;
    break;
  }
  return status;
}
