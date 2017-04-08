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

#include <functional>
#include <iostream>

using alterstack::Task;

void ctx_function()
{
    std::cout << "Context function, single part\n";
}

void ctx_function2()
{
    std::cout << "Context function2, first part\n";
    Task::yield();
    std::cout << "Context function2, second part\n";
}

void ctx_arg1(int data)
{
    std::cout << "Context function arg " << data << "\n";
}

int main()
{
    Task task{ ctx_function };

    int x = 11;
    Task task2{ [x]{std::cout << "Hello from lambda: x = " << x << "\n";} };
    Task con_task2{ ctx_function };
    Task con_task3{ ctx_function };
    Task con_task4{ ctx_function };

    Task con_task10{ ctx_function2 };
    std::cout << "Returned to fcm after run\n";
    con_task10.join();
    Task con_task11{ ::std::bind(ctx_arg1, 0) };
    Task con_task12{ ::std::bind(ctx_arg1, 11) };

    Task::yield();
    std::cout << "Returned to main\n";
    con_task2.join();
    return 0;
}

