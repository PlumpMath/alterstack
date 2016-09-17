project(alterstack)
cmake_minimum_required(VERSION 3.1)

include(ExternalProject)
link_directories(${CMAKE_BINARY_DIR}/InstalledExternals/lib)
include_directories(${CMAKE_BINARY_DIR}/InstalledExternals/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# this works with only cmake 3.1.0+
#target_link_libraries(my_app Threads::Threads)

add_subdirectory( ext/scontext )
include_directories( ext/scontext/include )

include_directories( ext/crash_log/include )

include(cmake/AddExternalCpuUtils.cmake)

include_directories(include)

set(alterstack_SRCS
    src/StaticsInit.cpp
    src/Awaitable.cpp
    src/Scheduler.cpp
    src/BgRunner.cpp
    src/CpuCore.cpp
    src/Stack.cpp
    src/Task.cpp
)

add_definitions(-std=c++11 -Wall -pedantic -mtune=native -march=native -pthread)
#add_definitions(-std=c++11 -Wall -pedantic -mtune=native -march=native -pthread -g -fsanitize=address -fno-omit-frame-pointer)

add_library(alterstack STATIC ${alterstack_SRCS})
target_link_libraries(alterstack scontext)

enable_testing()
add_subdirectory(test)
