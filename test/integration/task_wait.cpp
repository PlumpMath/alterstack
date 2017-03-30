/*
 * Copyright 2015-2017 Alexey Syrnikov <san@masterspline.net>
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

#include "alterstack/api.hpp"

#include <iostream>

using alterstack::Task;

alterstack::Awaitable aw;

void ctx_function2()
{
    std::cout << "Context function2, first part\n";
    Task::yield();
    std::cout << "Context function2, second part\n";
    aw.release();
}

int main()
{
    Task task{ ctx_function2 };
    aw.wait();
    return 0;
}

