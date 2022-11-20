add_executable(zap_binary)

file(GLOB_RECURSE ZAP_SOURCES *.h *.c *.hpp *.cpp)

target_sources(
    zap_binary
    PRIVATE
        ${ZAP_SOURCES}
)

target_include_directories(
    zap_binary
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    zap_binary
    PRIVATE
        zap_library
        docopt
)

set_target_properties(
    zap_binary
    PROPERTIES
    OUTPUT_NAME zap
)
