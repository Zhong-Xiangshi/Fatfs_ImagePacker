/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs (C)ChaN, 2019+                    */
/*-----------------------------------------------------------------------*/
/* This is a simple implementation of the disk I/O layer for             */
/* running FatFs on a PC/Windows environment. It uses a local file       */
/* as the physical disk drive.                                           */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */
#include <time.h>
#include "main.h"
/*-----------------------------------------------------------------------*/
/* Definitions                                                           */
/*-----------------------------------------------------------------------*/

#define DEV_IMAGE_FILE	0	/* We only support one drive, which is our image file */



/* For this example, we'll use a fixed sector size of 512 bytes */
#define SECTOR_SIZE 512



static FILE* fp_image = NULL;	/* File pointer for the disk image */
static DSTATUS Stat = STA_NOINIT;	/* Disk status */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	if (pdrv != DEV_IMAGE_FILE) {
		return STA_NODISK;
	}
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	if (pdrv != DEV_IMAGE_FILE) {
		return STA_NODISK;
	}

	if (Stat == 0) {
		return Stat;
	}
	remove(disk_image_path); /* 删除旧的镜像文件，确保每次都是新的开始 */
	printf("Creating a new one...\n", disk_image_path);
	fp_image = fopen(disk_image_path, "w+b"); /* 以写+更新模式创建 */
	if (!fp_image) {
		fprintf(stderr, "Error: Failed to create disk image file.\n");
		return STA_NOINIT;
	}

	/* --- 将文件扩展到预定义的大小 --- */
	// 1. 移动文件指针到目标大小的前一个字节
	if (fseek(fp_image, disk_image_size - 1, SEEK_SET) != 0) {
		fprintf(stderr, "Error: fseek failed while setting image size.\n");
		fclose(fp_image);
		fp_image = NULL;
		return STA_NOINIT;
	}
	// 2. 在该位置写入一个字节，这会强制操作系统将文件扩展到该大小
	if (fputc(0, fp_image) == EOF) {
		fprintf(stderr, "Error: fputc failed while setting image size.\n");
		fclose(fp_image);
		fp_image = NULL;
		return STA_NOINIT;
	}
	// 3. (可选但推荐) 将文件指针移回文件开头
	rewind(fp_image);

	printf("Successfully created a %.2f MB disk image.\n", (double)disk_image_size / (1024.0 * 1024.0));
	

	Stat &= ~STA_NOINIT; /* 清除未初始化标志 */
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != DEV_IMAGE_FILE || Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}

	/* Move file pointer to the correct sector */
	if (fseek(fp_image, (long)sector * SECTOR_SIZE, SEEK_SET) != 0) {
		return RES_ERROR;
	}

	/* Read data from the file */
	size_t read_count = fread(buff, SECTOR_SIZE, count, fp_image);
	if (read_count != count) {
		// This can happen if trying to read past the end of the file.
		// For a simple tool, we can treat it as an error.
		return RES_ERROR;
	}

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if (pdrv != DEV_IMAGE_FILE || Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}

	/* Move file pointer to the correct sector */
	if (fseek(fp_image, (long)sector * SECTOR_SIZE, SEEK_SET) != 0) {
		return RES_ERROR;
	}

	/* Write data to the file */
	size_t write_count = fwrite(buff, SECTOR_SIZE, count, fp_image);
	if (write_count != count) {
		return RES_ERROR;
	}

	return RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv != DEV_IMAGE_FILE || Stat & STA_NOINIT) {
		return RES_NOTRDY;
	}

	DRESULT res = RES_ERROR;
	long file_size;

	switch (cmd) {
		/* Make sure that no pending write process */
		case CTRL_SYNC:
			if (fflush(fp_image) == 0) {
				res = RES_OK;
			}
			break;

		/* Get number of sectors on the disk (LBA_t) */
		case GET_SECTOR_COUNT:
			if (disk_image_size > 0) {
                *(LBA_t*)buff = disk_image_size / SECTOR_SIZE;
                res = RES_OK;
            } else {
                res = RES_ERROR; // 如果大小为0，则报告错误
            }
			break;

		/* Get R/W sector size (WORD) */
		case GET_SECTOR_SIZE:
			*(WORD*)buff = SECTOR_SIZE;
			res = RES_OK;
			break;

		/* Get erase block size in unit of sector (DWORD) */
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1; /* For a file, the erase block concept doesn't apply. 1 is a safe default. */
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	return res;
}

DWORD get_fattime (void)
{
    time_t raw_time;
    struct tm* time_info;

    // 获取当前日历时间
    time(&raw_time);
    // 转换为本地时间
    time_info = localtime(&raw_time);

    // 将时间信息打包成FAT文件系统要求的DWORD格式
    // bit31:25=Year from 1980 (0..127), bit24:21=Month (1..12), bit20:16=Day (1..31)
    // bit15:11=Hour (0..23), bit10:5=Minute (0..59), bit4:0=Second/2 (0..29)
    return    ((DWORD)(time_info->tm_year - 80) << 25)  /* Year since 1980 */
            | ((DWORD)(time_info->tm_mon + 1) << 21)    /* Month */
            | ((DWORD)time_info->tm_mday << 16)         /* Day */
            | ((DWORD)time_info->tm_hour << 11)         /* Hour */
            | ((DWORD)time_info->tm_min << 5)           /* Minute */
            | ((DWORD)time_info->tm_sec >> 1);          /* Second / 2 */
}