cmake_minimum_required(VERSION 3.10)
project(MemoryPoolTest)

set(CMAKE_CXX_STANDARD 17)
# set(MEMORY_POOL_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
# set(MEMORY_POOL_TEST_DIR "${CMAKE_SOURCE_DIR}/test")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# # 设置编译器选项
# if(MSVC)
#     add_compile_options("/utf-8")
# else()
#     # 对于 MinGW 和其他非 MSVC 编译器，设置 UTF-8 编码选项
#     add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
# endif()

# 添加调试标志
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

include_directories(
    "${CMAKE_SOURCE_DIR}/com"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
)

file(GLOB MY_SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/test/*.cpp"
)
add_executable(memory_pool_test 
    ${MY_SOURCES}
    # ${MEMORY_POOL_SOURCE_DIR}/ThreadCache.cpp
)

# 设置pthread链接
target_link_libraries(memory_pool_test pthread)

# set_target_properties(memory_pool_test PROPERTIES LINK_FLAGS "-pthread")
