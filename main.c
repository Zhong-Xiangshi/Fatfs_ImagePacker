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
/* 默认的文件系统格式 */
BYTE fs_format_type = FM_EXFAT;

/*
=================================================================================
 打印使用说明
=================================================================================
*/
void print_usage(const char* prog_name) {
    printf("Usage: %s [-o output_image.img] [-s size_in_bytes] [-i source_folder] [-f format]\n", prog_name);
    printf("Options:\n");
    printf("  -h                Show this help message.\n");
    printf("  -o <path>         Output image path (default: %s).\n", disk_image_path);
    printf("  -s <bytes>        Image size in bytes (default: %llu).\n", (unsigned long long)disk_image_size);
    printf("  -i <folder>       Source folder path (default: %s).\n", source_folder);
    printf("  -f <format>       Specify the filesystem format. Options are:\n");
    printf("                    'FAT', 'FAT32', 'EXFAT' (default: EXFAT).\n");
}


/*
=================================================================================
 主函数
=================================================================================
*/
int main(int argc, char *argv[]) {
    FATFS fs;
    FRESULT res;
    BYTE work[FF_MAX_SS];
    char* format_str = "EXFAT"; // 用于打印日志

    // --- 从命令行参数解析配置 ---
    int arg_index = 1;
    while (arg_index < argc) {
        if (strcmp(argv[arg_index], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[arg_index], "-o") == 0) {
            if (arg_index + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for -o option.\n");
                print_usage(argv[0]);
                return 1;
            }
            disk_image_path = argv[++arg_index];
        }
        else if (strcmp(argv[arg_index], "-s") == 0) {
            char* endptr;
            unsigned long long size_bytes;

            if (arg_index + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for -s option.\n");
                print_usage(argv[0]);
                return 1;
            }

            size_bytes = strtoull(argv[++arg_index], &endptr, 10);
            if (*endptr != '\0' || argv[arg_index][0] == '\0' || size_bytes == 0) {
                fprintf(stderr, "Error: Invalid size '%s'. Please provide a positive integer for bytes.\n", argv[arg_index]);
                return 1;
            }
            disk_image_size = size_bytes;
        }
        else if (strcmp(argv[arg_index], "-i") == 0) {
            if (arg_index + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for -i option.\n");
                print_usage(argv[0]);
                return 1;
            }
            source_folder = argv[++arg_index];
        }
        else if (strcmp(argv[arg_index], "-f") == 0) {
            if (arg_index + 1 < argc) {
                arg_index++;
                if (stricmp(argv[arg_index], "FAT") == 0) {
                    fs_format_type = FM_FAT;
                    format_str = "FAT";
                } else if (stricmp(argv[arg_index], "FAT32") == 0) {
                    fs_format_type = FM_FAT32;
                    format_str = "FAT32";
                } else if (stricmp(argv[arg_index], "EXFAT") == 0) {
                    fs_format_type = FM_EXFAT;
                    format_str = "EXFAT";
                } else {
                    fprintf(stderr, "Error: Invalid format type '%s'. Use 'FAT', 'FAT32', or 'EXFAT'.\n", argv[arg_index]);
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: Missing value for -f option.\n");
                print_usage(argv[0]);
                return 1;
            }
        }
        else {
            fprintf(stderr, "Error: Unknown option or positional argument '%s'.\n", argv[arg_index]);
            print_usage(argv[0]);
            return 1;
        }
        arg_index++;
    }

    printf("----------------------------------------\n");
    printf("FatFs Image Packer Configuration:\n");
    printf("  - Image Path:    %s\n", disk_image_path);
    printf("  - Image Size:    %llu bytes (%.2f MiB)\n", 
           (unsigned long long)disk_image_size, 
           (double)disk_image_size / (1024.0 * 1024.0));
    printf("  - Source Folder: %s\n", source_folder);
    printf("  - FS Format:     %s\n", format_str);
    printf("----------------------------------------\n\n");

    // --- 准备工作：格式化和挂载 ---
    printf("Formatting the disk image with %s...\n", format_str);
    // 使用 MKFS_PARM 结构体来指定格式
    MKFS_PARM opt = { .fmt = fs_format_type };
    res = f_mkfs("0:", &opt, work, sizeof(work));
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