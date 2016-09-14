project( scontext C CXX ASM )
cmake_minimum_required(VERSION 2.8)
option( SCONTEXT_BUILD_TESTS "Build stack context tests" ON )

add_definitions(-std=c++11 -Wall -pedantic -mtune=native -march=native )

include_directories( include )
set( SCONTEXT_SRC
    ./src/asm/jump_x86_64_sysv_elf_gas.S  
    ./src/asm/make_x86_64_sysv_elf_gas.S  
    ./src/asm/ontop_x86_64_sysv_elf_gas.S
)
add_library( scontext STATIC ${SCONTEXT_SRC} )

if( ${SCONTEXT_BUILD_TESTS} )
    add_executable( test_scontext ./test/test_scontext.cpp )
    target_link_libraries( test_scontext scontext )
    add_test( NAME TEST_SCONTEXT
              COMMAND test_scontext )
endif()
