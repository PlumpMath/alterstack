#include <cstddef>
#include <iostream>

#include "scontext.hpp"

void context_function(scontext::transfer_t tr)
{
    scontext::fcontext_t from = tr.fctx;
    std::cout << "Hello from context_function from " << from
              << " with data " << tr.data << "\n";
    scontext::jump_fcontext( from, (void*)0xaa5577EE );
    return;
}

int main()
{
    constexpr size_t stack_size = 1024*1024;
    char* stack = new char[stack_size];
    char* stack_top = stack + stack_size;
    scontext::fcontext_t f_ctx =
            scontext::make_fcontext( stack_top, stack_size, context_function);
    scontext::transfer_t tr = scontext::jump_fcontext( f_ctx, (void*)0xaa55f00f );
    std::cout << "Hello from main function, from " << tr.fctx
              << " with data " << tr.data << "\n";

    delete [] stack;
    return 0;
}
