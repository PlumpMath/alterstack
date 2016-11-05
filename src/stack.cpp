/*
 * Copyright 2015-2016 Alexey Syrnikov <san@masterspline.net>
 * 
 * This file is part of Alterstack.
 *
 * Alterstack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alterstack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Alterstack.  If not, see <http://www.gnu.org/licenses/>
 */

#include "alterstack/stack.hpp"

#if defined(WITH_VALGRIND)
#include <valgrind/valgrind.h>
#endif

#include <stdexcept>
#include <cassert>

#include <sys/mman.h>

namespace alterstack
{
/**
 * @brief allocates memory for stack, protect last page to prevent overflow
 */
Stack::Stack()
    :m_size(1024*1024)
{
    m_base = ::mmap( 0, m_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ( m_base == MAP_FAILED ) throw std::bad_alloc();

    int result = ::mprotect( m_base, 1, PROT_NONE);
    assert( result == 0 );
#if defined(WITH_VALGRIND)
    m_valgrind_stack_id = VALGRIND_STACK_REGISTER( _stack_top(), m_stack_base);
#endif
}
/**
 * @brief deallocates memory used by stack
 */
Stack::~Stack()
{
#if defined(WITH_VALGRIND)
    VALGRIND_STACK_DEREGISTER( m_valgrind_stack_id );
#endif
    auto result = ::munmap( m_base, m_size);
    assert( result == 0 );
}

}
