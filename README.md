# FatFs Image Packer
此.exe程序会新建一个指定大小的文件，并将文件格式化为FatFs文件系统，然后拷贝指定文件夹的内容到里面。
``` PowerShell
./Fatfs_ImagePacker.exe [输出镜像名] [镜像大小，单位bytes] [要拷贝的文件夹]
#例子
./Fatfs_ImagePacker.exe fatfs.img 67108864 my_files
```