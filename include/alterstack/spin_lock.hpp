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

#include <atomic>

inline void cpu_relax(void)
{
    asm volatile("rep; nop" ::: "memory");
}

class SpinLock
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    void lock()
    {
        while( flag.test_and_set(std::memory_order_acquire) )
        {
            cpu_relax();
        }
    }
    bool try_lock()
    {
        return !flag.test_and_set(std::memory_order_acquire);
    }
    void unlock()
    {
        flag.clear(std::memory_order_release);
    }
    void clear()
    {
        flag.clear(std::memory_order_release);
    }
};
