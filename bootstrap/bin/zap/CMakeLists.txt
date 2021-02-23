add_executable(zap)

file(GLOB_RECURSE ZAP_SOURCES *.h *.c *.hpp *.cpp)

target_sources(
    zap
    PRIVATE
        ${ZAP_SOURCES}
)

target_include_directories(
    zap
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    zap
    PRIVATE
        lib_zap
        structopt::structopt
)

set_target_properties(
    zap
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ../../../bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ../../../bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ../../../bin
    OUTPUT_NAME zap
)
