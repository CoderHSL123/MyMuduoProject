cmake_minimum_required(VERSION 2.5)#cmake的最低版本

project(hslMuduo)#工程名称

#hslMuduo最终编译成so动态库，设置动态库的路径，放在项目目录的lib下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#设置调试信息 以及C++标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++14 -fPIC")

#定义参与编译的源文件 将当前目录下的所有源文件名 加入到SRC_LIST中
aux_source_directory(. SRC_LIST)

#编译生成动态库 hslMuduo
add_library(hslMuduo SHARED ${SRC_LIST})