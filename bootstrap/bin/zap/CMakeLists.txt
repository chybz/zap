add_executable(binary_zap)

file(GLOB_RECURSE ZAP_SOURCES *.h *.c *.hpp *.cpp)

target_sources(
    binary_zap
    PRIVATE
        ${ZAP_SOURCES}
)

target_include_directories(
    binary_zap
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    binary_zap
    PRIVATE
        zap
        docopt
)

set_target_properties(
    binary_zap
    PROPERTIES
    OUTPUT_NAME zap
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
