set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER_LAUNCHER ccache)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/root")

set(
    CMAKE_C_FLAGS
    "-g -Wall -Wextra -Wno-missing-field-initializers \
    -Wno-unused-parameter -pedantic"
)

set(
    CMAKE_CXX_FLAGS
    "-g -Wall -Wextra -Wno-missing-field-initializers \
    -Wno-unused-parameter -pedantic"
)
