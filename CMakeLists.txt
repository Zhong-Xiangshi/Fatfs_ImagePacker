cmake_minimum_required(VERSION 3.10.0)
project(Fatfs_ImagePacker VERSION 0.1.0 LANGUAGES C)

file(GLOB_RECURSE FATFS_SOURCES "lib/ff16/source/*.c")

add_executable(Fatfs_ImagePacker ${FATFS_SOURCES} tools.c main.c)

target_include_directories(Fatfs_ImagePacker PUBLIC "lib/ff16/source" ".")