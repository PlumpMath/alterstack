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

#include "alterstack/Api.hpp"

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
    Task task;
    task.run( ctx_function );
    Task task2;
    int x = 11;
    task2.run([x]{std::cout << "Hello from lambda: x = " << x << "\n";});
    Task con_task2;
    con_task2.run(ctx_function);
    Task con_task3;
    con_task3.run(ctx_function);
    Task con_task4;
    con_task4.run(ctx_function);

    Task con_task10;
    con_task10.run(ctx_function2);
    std::cout << "Returned to fcm after run\n";
    con_task10.wait();
    Task con_task11;
    con_task11.run(::std::bind(ctx_arg1, 0));
    Task con_task12;
    con_task12.run(::std::bind(ctx_arg1, 11));

    Task::yield();
    std::cout << "Returned to main\n";
    con_task2.wait();
    return 0;
}

