#include "tools.h"
#include <windows.h>    // 用于Windows文件和目录遍历
#include "ff.h"         // FatFs库
#include <stdlib.h>     // 用于 strtoull
#include <stdio.h>
#include <string.h>

// 定义一个足够大的缓冲区用于文件读写
#define COPY_BUFFER_SIZE (8 * 1024)

/*
=================================================================================
 1. 辅助函数：将单个PC文件复制到FatFs镜像中
=================================================================================
*/

/**
 * @brief Copies a single file from the local PC filesystem to the FatFs virtual disk.
 * @param pc_path Full path to the source file on the PC.
 * @param fatfs_path Full path to the destination file within the FatFs image (e.g., "0:/images/pic.png").
 * @return 0 on success, -1 on failure.
 */
static int copy_file_to_fatfs(const char* pc_path, const char* fatfs_path) {
    FILE* f_src = NULL;
    FIL f_dst;
    FRESULT res;
    BYTE buffer[COPY_BUFFER_SIZE];
    size_t bytes_read;
    UINT bytes_written;
    int ret = -1; // 默认返回失败

    // 1. 以二进制读模式打开PC上的源文件
    f_src = fopen(pc_path, "rb");
    if (!f_src) {
        fprintf(stderr, "Error: Cannot open PC file '%s'.\n", pc_path);
        return -1;
    }

    // 2. 在FatFs中创建并打开目标文件
    res = f_open(&f_dst, fatfs_path, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        fprintf(stderr, "Error: Cannot create FatFs file '%s'. FRESULT: %d\n", fatfs_path, res);
        fclose(f_src);
        return -1;
    }

    printf("Copying file: '%s' -> '%s'\n", pc_path, fatfs_path);

    // 3. 循环读写，直到源文件结束
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_src)) > 0) {
        res = f_write(&f_dst, buffer, bytes_read, &bytes_written);
        if (res != FR_OK || bytes_written < bytes_read) {
            fprintf(stderr, "Error: Failed writing to FatFs file. Disk may be full. FRESULT: %d\n", res);
            goto cleanup; // 使用goto进行清理
        }
    }

    // 检查fread是否出错
    if (ferror(f_src)) {
        fprintf(stderr, "Error: Failed reading from PC file '%s'.\n", pc_path);
    } else {
        ret = 0; // 成功
    }

cleanup:
    // 4. 关闭两个文件句柄
    f_close(&f_dst);
    fclose(f_src);
    return ret;
}

/*
=================================================================================
 2. 核心函数：递归地将PC文件夹内容复制到FatFs镜像
=================================================================================
*/

/**
 * @brief Recursively copies the contents of a PC directory to a directory in the FatFs image.
 * @param pc_dir_path Path to the source directory on the PC (e.g., "C:/my_assets").
 * @param fatfs_dir_path Path to the destination directory in FatFs (e.g., "0:/").
 * @return 0 on success, -1 on failure.
 */
int copy_directory_to_fatfs(const char* pc_dir_path, const char* fatfs_dir_path) {
    WIN32_FIND_DATA find_data;
    HANDLE h_find = INVALID_HANDLE_VALUE;
    char search_path[MAX_PATH];
    char src_path_full[MAX_PATH];
    char dst_path_full[MAX_PATH];

    // 构造Windows API的搜索路径，例如 "C:/my_assets/*"
    snprintf(search_path, sizeof(search_path), "%s\\*", pc_dir_path);

    h_find = FindFirstFile(search_path, &find_data);
    if (h_find == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Cannot find first file in directory '%s'. Error code: %lu\n", pc_dir_path, GetLastError());
        return -1;
    }

    do {
        // 忽略特殊的 "." 和 ".." 目录
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        // 构造当前项的完整源路径和目标路径
        snprintf(src_path_full, sizeof(src_path_full), "%s/%s", pc_dir_path, find_data.cFileName);
        snprintf(dst_path_full, sizeof(dst_path_full), "%s/%s", fatfs_dir_path, find_data.cFileName);

        // 判断当前项是目录还是文件
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("Creating directory: '%s'\n", dst_path_full);
            
            // 在FatFs中创建对应的目录
            FRESULT res = f_mkdir(dst_path_full);
            if (res != FR_OK && res != FR_EXIST) {
                fprintf(stderr, "Error: Failed to create FatFs directory '%s'. FRESULT: %d\n", dst_path_full, res);
                FindClose(h_find);
                return -1;
            }

            // 递归进入子目录
            if (copy_directory_to_fatfs(src_path_full, dst_path_full) != 0) {
                FindClose(h_find);
                return -1; // 如果子目录拷贝失败，则中止
            }

        } else {
            // 如果是文件，则调用文件拷贝函数
            if (copy_file_to_fatfs(src_path_full, dst_path_full) != 0) {
                FindClose(h_find);
                return -1; // 如果文件拷贝失败，则中止
            }
        }

    } while (FindNextFile(h_find, &find_data) != 0);

    FindClose(h_find);
    return 0; // 整个目录成功复制
}
