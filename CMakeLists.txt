project( alterstack )
cmake_minimum_required(VERSION 3.1)

option( ALTERSTACK_USE_JEMALLOC "Link with jemalloc" OFF )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# this works with only cmake 3.1.0+
#target_link_libraries(my_app Threads::Threads)

add_subdirectory( ext/scontext )
include_directories( ext/scontext/include )
include_directories( ext/crash_log/include )
include_directories( ext/cpu_utils/include )

include_directories(include)

set(alterstack_SRCS
    src/awaitable.cpp
    src/scheduler.cpp
    src/bg_runner.cpp
    src/bg_thread.cpp
    src/stack.cpp
    src/task.cpp
)

add_definitions( -std=c++14 -Wall -pedantic -mtune=native -march=native )
#add_definitions(-std=c++11 -Wall -pedantic -mtune=native -march=native -pthread -g -fsanitize=address -fno-omit-frame-pointer)

add_library(alterstack STATIC ${alterstack_SRCS})
target_link_libraries(alterstack scontext)

enable_testing()
add_subdirectory(test)
