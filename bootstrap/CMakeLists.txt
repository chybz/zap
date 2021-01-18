cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(zap VERSION 1.0.0 LANGUAGES C CXX)

include(FindPkgConfig)
include(GNUInstallDirs)

pkg_check_modules(libarchive REQUIRED IMPORTED_TARGET GLOBAL libarchive)
find_package(reproc++ REQUIRED)
find_package(tomlplusplus REQUIRED)
find_package(re2 REQUIRED)
find_package(Taskflow REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(lib/zap)
add_subdirectory(bin/zap)
