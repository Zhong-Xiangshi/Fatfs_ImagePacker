cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Fatfs_ImagePacker VERSION 0.1.0 LANGUAGES C)

file(GLOB_RECURSE FATFS_SOURCES "lib/ff16/source/*.c")

add_executable(Fatfs_ImagePacker ${FATFS_SOURCES} tools.c main.c)

target_include_directories(Fatfs_ImagePacker PUBLIC "lib/ff16/source" ".")

# 源文件为 UTF-8 编码，强制 MSVC 按 UTF-8 解析（否则中文注释会破坏语法）
if(MSVC)
    target_compile_options(Fatfs_ImagePacker PRIVATE /utf-8)
endif()