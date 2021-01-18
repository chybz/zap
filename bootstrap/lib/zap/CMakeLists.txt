add_library(lib_zap)

file(GLOB_RECURSE LIB_ZAP_SOURCES *.h *.c *.hpp *.cpp)

target_sources(
    lib_zap
    PRIVATE
        ${LIB_ZAP_SOURCES}
)

target_include_directories(
    lib_zap
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../../include/zap"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    lib_zap
    PUBLIC
        Taskflow::Taskflow
        re2::re2
        tomlplusplus::tomlplusplus
    PRIVATE
        reproc++
        PkgConfig::libarchive
        stdc++fs
)

set_target_properties(
    lib_zap
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ../../../lib
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ../../../lib
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ../../../lib
    OUTPUT_NAME zap
)
