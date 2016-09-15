project(alterstack)
cmake_minimum_required(VERSION 2.8)
#OPTION(DEBUG_MODE               "Enable debug output [default: OFF]"             OFF)
#OPTION(ENABLE_OPTIMIZATION      "Enable optimization [default: OFF]"             ON)

include(ExternalProject)
link_directories(${CMAKE_BINARY_DIR}/InstalledExternals/lib)
include_directories(${CMAKE_BINARY_DIR}/InstalledExternals/include)

#if (NOT CMAKE_BUILD_TYPE)
#set(CMAKE_BUILD_TYPE Debug)
#endif (NOT CMAKE_BUILD_TYPE)

#set(BOOST_COMPONENTS
#        system
#        thread
#        filesystem)
#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#SET(BOOST_MIN_VERSION "1.54")

#find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${Boost_LIBRARIES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# this works with only cmake 3.1.0+
#target_link_libraries(my_app Threads::Threads)

#set(BOOST_ROOT "${CMAKE_CURRENT_LIST_DIR}/ext/boost/")
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.58.0 COMPONENTS context)
if(NOT Boost_FOUND)
  include(cmake/AddExternalBoost.cmake)
endif()
#include_directories(${Boost_INCLUDE_DIRS})

#find_package(PkgConfig)
#pkg_check_modules(TCMALLOC REQUIRED libtcmalloc_minimal)
#include_directories(${TCMALLOC_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${TCMALLOC_LIBRARIES})
set(COMMON_LIBS ${COMMON_LIBS} -ljemalloc -latomic)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
#set(CMAKE_AUTOMOC ON)
#set(Qt5Core_DIR "H:\\Qt\\5.0.0beta1\\Desktop\\Qt\\5.0.0-beta1\\msvc2010_64\\lib\\cmake\\Qt5Core")
#find_package(Qt5Core REQUIRED)

#set(LIBS ${LIBS} -lcrypt -lpthread -luuid -lctpp2 -lssl -lcrypto)
#include_directories(${CMAKE_BINARY_DIR}/InstalledExternals/include)
include(cmake/AddExternalCpuUtils.cmake)
include(cmake/AddExternalCrashLog.cmake)

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

add_definitions(-std=c++14 -Wall -pedantic -mtune=native -march=native -pthread)
#add_definitions(-std=c++14 -Wall -pedantic -mtune=native -march=native -pthread -g -fsanitize=address -fno-omit-frame-pointer)

add_library(alterstack STATIC ${alterstack_SRCS})
target_link_libraries(alterstack boost_context)
#target_link_libraries(context ${COMMON_LIBS})
#target_link_libraries(alterstack ${COMMON_LIBS} ${Boost_LIBRARIES} -pthread)
#target_link_libraries(cot ${COMMON_LIBS} ${Boost_LIBRARIES} -pthread -g -fsanitize=address)

#add_library(ws_config SHARED src/core/WsConfig.cpp)
#add_executable(ws_update ${ws_update_SRCS})
#target_link_libraries(ws_update ${TCMALLOC_LIBRARIES} ${MongoDB_LIBRARIES})

include(./cmake/AddExternalCatch.cmake)
enable_testing()
add_subdirectory(test)
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIR})
#
#add_subdirectory( /usr/src/gtest ${PROJECT_BINARY_DIR}/gtest )
#include_directories( "/usr/include/gtest" )
#add_executable(test_common_variable
#    tests/CommonVariableTest.cpp
#    src/core/CommonVariable.cpp
#)
#target_link_libraries(test_common_variable gtest gtest_main pthread ${LIBS} )

#add_test( CommonVarialbeTest test_common_variable )
#
#add_executable(test_ip_info
#    tests/IPInfoTest.cpp
#    src/core/IPInfo.cpp
#)
#target_link_libraries(test_ip_info gtest gtest_main ${LIBS} )
#add_test( IPInfoTest test_ip_info )
