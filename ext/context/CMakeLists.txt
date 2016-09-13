project( switch_stack C CXX ASM )
cmake_minimum_required(VERSION 2.8)

#add_definitions(-std=c++14 -Wall -pedantic -mtune=native -march=native )

set( SWITCH_STACK_SRC 
    ./src/asm/jump_x86_64_sysv_elf_gas.S  
    ./src/asm/make_x86_64_sysv_elf_gas.S  
    ./src/asm/ontop_x86_64_sysv_elf_gas.S
)
add_library( switch_stack STATIC ${SWITCH_STACK_SRC} )
