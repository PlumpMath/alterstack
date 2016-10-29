/*
 * Copyright 2015 Alexey Syrnikov <san@masterspline.net>
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

#pragma once

#include <cstddef>

namespace alterstack
{
/**
 * @brief The Stack class allocates protected stack in constructor and deallocates in destructor
 */
class Stack
{
public:
    /**
     * @brief allocates memory for stack, protect last page to prevent overflow
     */
    Stack();
    /**
     * @brief deallocates memory used by stack
     */
    ~Stack();
    /**
     * @brief address for empty stack pointer
     * @return stack top - StackPointer address for empty stack (max address for x86)
     */
    void* stack_top() const noexcept;
    /**
     * @brief stack size
     * @return memory size allocated for stack
     */
    size_t size() const noexcept;

private:
    void*  m_base;
    size_t m_size;
#if defined(WITH_VALGRIND)
    unsigned m_valgrind_stack_id;
#endif
};

inline void* Stack::stack_top() const noexcept
{
    return reinterpret_cast<char*>(m_base)+m_size;
}

inline size_t Stack::size() const noexcept
{
    return m_size;
}
//#include <signal.h>
// min_stack_size    SIGSTKSZ;

}
