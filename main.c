#include <stdio.h>
#include <string.h>
#include <stdlib.h>     // 用于 strtoull
#include "tools.h"
#include "ff.h"         // FatFs库
#include "main.h"

/* 默认的镜像文件名 */
char *disk_image_path="fatfs.img";
/* 默认的镜像文件大小 (32MiB) */
uint64_t disk_image_size=(32 * 1024 * 1024);
/* 默认要打包的文件夹名 */
char* source_folder = "assets_to_pack";


/*
=================================================================================
 主函数
=================================================================================
*/

int main(int argc, char *argv[]) {
    FATFS fs;
    FRESULT res;
    BYTE work[FF_MAX_SS];

    // --- 从命令行参数解析配置 ---
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: %s [output_image.img] [size_in_bytes] [source_folder]\n", argv[0]);
        printf("  - If no arguments are provided, default values will be used.\n");
        return 0;
    }
    
    if (argc > 4) {
        fprintf(stderr, "Error: Too many arguments.\n");
        fprintf(stderr, "Usage: %s [output_image.img] [size_in_bytes] [source_folder]\n", argv[0]);
        return 1;
    }

    if (argc >= 2) {
        disk_image_path = argv[1];
    }
    if (argc >= 3) {
        char* endptr;
        unsigned long long size_bytes = strtoull(argv[2], &endptr, 10);
        if (*endptr != '\0' || argv[2][0] == '\0' || size_bytes == 0) {
            fprintf(stderr, "Error: Invalid size '%s'. Please provide a positive integer for bytes.\n", argv[2]);
            return 1;
        }
        disk_image_size = size_bytes;
    }
    if (argc >= 4) {
        source_folder = argv[3];
    }

    printf("----------------------------------------\n");
    printf("FatFs Image Packer Configuration:\n");
    printf("  - Image Path:    %s\n", disk_image_path);
    printf("  - Image Size:    %llu bytes (%.2f MiB)\n", 
           (unsigned long long)disk_image_size, 
           (double)disk_image_size / (1024.0 * 1024.0));
    printf("  - Source Folder: %s\n", source_folder);
    printf("----------------------------------------\n\n");

    // --- 准备工作：格式化和挂载 ---
    printf("Formatting the disk image...\n");
    res = f_mkfs("0:", NULL, work, sizeof(work));
    if (res != FR_OK) {
        fprintf(stderr, "ERROR: f_mkfs failed. FRESULT: %d\n", res);
        return -1;
    }
    printf("Format successful.\n");

    res = f_mount(&fs, "0:", 1);
    if (res != FR_OK) {
        fprintf(stderr, "ERROR: f_mount failed. FRESULT: %d\n", res);
        return -1;
    }
    printf("Mount successful.\n");

    // --- 核心操作：拷贝整个文件夹 ---
    const char* dest_root = "0:";                // <-- FatFs的根目录

    // 在运行程序前，请确保源文件夹存在
    printf("\nStarting to copy directory '%s' to the root of the image...\n", source_folder);

    // 首先在PC上创建这个目录，以便程序能找到它
    CreateDirectory(source_folder, NULL); 

    if (copy_directory_to_fatfs(source_folder, dest_root) == 0) {
        printf("\nSuccessfully copied all contents from '%s'!\n", source_folder);
    } else {
        fprintf(stderr, "\nERROR: Directory copy failed.\n");
    }

    // --- 清理工作：卸载 ---
    f_mount(NULL, "0:", 0);
    printf("Unmounted the disk image.\n");

    return 0;
}