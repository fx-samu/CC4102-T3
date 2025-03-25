cmake_minimum_required(VERSION 3.10)
project(CC4102T2)
set(CMAKE_BINARY_DIR "/bin")

# set(CMAKE_CXX_COMPILER_LAUNCHER env LD_PRELOAD=/usr/lib/libasan.so)
set(CMAKE_CXX_COMPILER_LAUNCHER env)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-O3)
# add_compile_options(-g)
# add_compile_options(-fsanitize=address)
# add_compile_options(-static-libasan)
# add_link_options(-fsanitize=address)
# add_link_options(-static-libasan)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HDRS "include/*.h")
file(GLOB TPPS "src/*.tpp")

add_executable(T3.bin ${SOURCES}
        ${HDRS} ${TPPS})

