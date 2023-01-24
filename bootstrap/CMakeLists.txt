cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(zap VERSION 1.0.0 LANGUAGES C CXX)

include(FindPkgConfig)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(httplib CONFIG REQUIRED)
find_package(reproc++ CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(docopt CONFIG REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(tabulate CONFIG REQUIRED)

add_subdirectory(lib)
add_subdirectory(bin)

install(
    TARGETS
        zap_library
        zap_binary
    EXPORT zap-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    EXPORT zap-config
    FILE zap-config.cmake
    NAMESPACE zap::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/zap
)
