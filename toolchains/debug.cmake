include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fsanitize=address")
