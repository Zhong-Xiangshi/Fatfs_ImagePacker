#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdint.h>

extern char *disk_image_path;
extern uint64_t disk_image_size;
/* 扇区大小（字节），必须是 512/1024/2048/4096 之一 */
extern uint32_t disk_sector_size;

#endif