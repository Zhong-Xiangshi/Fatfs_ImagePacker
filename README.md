# FatFs Image Packer
一个简单的exe程序：新建一个指定大小的文件，并将文件格式化为FatFs文件系统，然后拷贝指定文件夹的内容到里面。
``` PowerShell
Fatfs_ImagePacker.exe -h
Usage: Fatfs_ImagePacker.exe [options] [output_image.img] [size_in_bytes] [source_folder]
Options:
  -h, --help        Show this help message.
  -f <format>       Specify the filesystem format. Options are:
                    'FAT', 'FAT32', 'EXFAT' (default: EXFAT).

Arguments default to:
  - output_image.img: fatfs.img
  - size_in_bytes:    33554432
  - source_folder:    assets_to_pack
```