# FatFs Image Packer
一个简单的exe程序：新建一个指定大小的文件，并将文件格式化为FatFs文件系统，然后拷贝指定文件夹的内容到里面。

支持指定扇区大小（512~4096）、SFD（无分区表）布局、镜像头部 0xFF 偏置填充。

``` PowerShell
Fatfs_ImagePacker.exe -h
Usage: Fatfs_ImagePacker.exe [-o output_image.img] [-s size_in_bytes] [-i source_folder] [-f format] [-b sector_size]
Options:
  -h                Show this help message.
  -o <path>         Output image path (default: fatfs.img).
  -s <bytes>        Image size in bytes (default: 33554432).
  -i <folder>       Source folder path (default: assets_to_pack).
  -f <format>       Specify the filesystem format. Options are:
                    'FAT', 'FAT32', 'EXFAT', 'ANY' (default: EXFAT).
                    'ANY' lets f_mkfs choose automatically by volume size.
  -b <bytes>        Sector size in bytes: 512, 1024, 2048 or 4096 (default: 512).
  -sfd              SFD format: no partition table, volume starts at sector 0.
  -p <bytes>        Pad image head with 0xFF bytes; filesystem starts at this
                    offset. Use when the flash tool writes the image at a
                    non-sector-aligned address (default: 0).
```

## 各文件系统格式的最小镜像大小

- FAT最小97792字节(~0.09MB)
- exFAT最小2129408字节(~2.03MB)
- FAT32最小33862656字节（~32.29MB）

当卷大小 > 63 扇区时，FatFs 会预留 63 个扇区(31.5KB)用于 MBR 和分区表，剩余的才分配给文件系统。

## 不同扇区大小下的最小镜像大小

扇区变大后最小卷大小（扇区数）不变，字节数随扇区大小放大：

| 格式 | 512B | 4096B |
|---|---|---|
| FAT12 | ~0.09MB | ~0.75MB（≥128扇区） |
| exFAT | ~2.03MB | ~16MB（≥4096扇区） |
| FAT32 | ~32.29MB | ~256MB（≥65525簇） |

镜像小于对应下限时 f_mkfs 返回 `FR_MKFS_ABORTED` (14)。

## 常用示例

``` PowerShell
# 默认：exFAT、512字节扇区、带分区表
Fatfs_ImagePacker.exe -i root -s 33554432

# 4096字节扇区的小镜像，自动选择格式，无分区表
Fatfs_ImagePacker.exe -i root -f ANY -sfd -b 4096 -s 1024000

# 烧录工具会把镜像写到非4096对齐地址（如偏移0x400）时，
# 头部填充3072字节0xFF，使文件系统在烧录后仍落在扇区边界上
Fatfs_ImagePacker.exe -i root -f ANY -sfd -b 3072 -p 4096 -s 2048000
```
