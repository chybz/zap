cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(zap VERSION 1.0.0 LANGUAGES C CXX)

include(FindPkgConfig)
include(GNUInstallDirs)

pkg_check_modules(libarchive REQUIRED IMPORTED_TARGET GLOBAL libarchive)
find_package(CURL CONFIG REQUIRED)
find_package(reproc++ CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(docopt CONFIG REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(lib/zap)
add_subdirectory(bin/zap)
