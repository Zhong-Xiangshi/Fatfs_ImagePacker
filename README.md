# FatFs Image Packer
一个简单的exe程序：新建一个指定大小的文件，并将文件格式化为FatFs文件系统（固定Fat扇区大小512），然后拷贝指定文件夹的内容到里面。
``` PowerShell
Fatfs_ImagePacker.exe -h
Usage: Fatfs_ImagePacker.exe [-o output_image.img] [-s size_in_bytes] [-i source_folder] [-f format]
Options:
  -h                Show this help message.
  -o <path>         Output image path (default: fatfs.img).
  -s <bytes>        Image size in bytes (default: 33554432).
  -i <folder>       Source folder path (default: assets_to_pack).
  -f <format>       Specify the filesystem format. Options are:
                    'FAT', 'FAT32', 'EXFAT' (default: EXFAT).
```
当卷大小 > 63 扇区时，FatFs 会预留 63 个扇区(31.5KB)用于 MBR 和分区表，剩余的才分配给文件系统。
## 在Fat扇区大小512Byte条件下的最小镜像大小：
FAT最小97792字节(~0.09MB)
exFAT最小2129408字节(~2.03MB)
FAT32最小33862656字节（~32.29MB）
