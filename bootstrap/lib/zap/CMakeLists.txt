add_library(zap_library)
add_library(zap::zap ALIAS zap_library)

file(GLOB_RECURSE LIB_ZAP_SOURCES *.h *.c *.hpp *.cpp)

target_sources(
    zap_library
    PRIVATE
        ${LIB_ZAP_SOURCES}
)

target_include_directories(
    zap_library
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/zap>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/zap>
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    zap_library
    PUBLIC
        Taskflow::Taskflow
        re2::re2
        tomlplusplus::tomlplusplus
    PRIVATE
        tabulate::tabulate
        sqlite_orm::sqlite_orm
        CURL::libcurl
        reproc++
        stdc++fs
)

set_target_properties(
    zap_library
    PROPERTIES
    OUTPUT_NAME zap
)
