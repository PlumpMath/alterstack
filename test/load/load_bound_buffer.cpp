/*
 * Copyright 2015,2017 Alexey Syrnikov <san@masterspline.net>
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

#include <algorithm>
#include <thread>
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/bound_buffer.hpp"
#include "alterstack/intrusive_list.hpp"

class Item : public alterstack::IntrusiveList<Item>
{
};

using alterstack::BoundBuffer;

BoundBuffer<Item> buffer;
constexpr int ITEMS_COUNT = 1000;
std::vector<Item> items( ITEMS_COUNT);

void thread_function()
{
    for(int i = 0; i < 10000; ++i)
    {
        Item* item;
        std::set<Item*> item_set;
        bool have_more_tasks = false;
        while( ( item = buffer.get_item( have_more_tasks )) != nullptr )
        {
            if( item->next() != nullptr )
            {
                std::cerr << "got not single Item*\n";
                std::cerr << "item " << item << "\n";
                std::cerr << "next " << item->next() << "\n";
                exit(1);
            }
            item_set.insert( item );
        }
//        for(auto& task: tasks)
//        {
//            if( task_set.find(&task) == task_set.end() )
//            {
//                std::cerr << "Got task " << &task << " not from initial array\n";
//                exit(1);
//            }
//        }
        for( auto& item: item_set)
        {
            buffer.put_items_list( item );
        }
        item_set.clear();
    }
}

int main()
{
    if( intptr_t(&buffer) % 64 != 0 )
    {
        std::cerr << "Buffer unaligned " << &buffer << "\n";
    }
    for(auto& task: items)
    {
        buffer.put_items_list( &task );
    }

    std::thread t1(thread_function);
    std::thread t2(thread_function);
    t1.join();
    t2.join();
    Item* item;
    std::set<Item*> item_set;
    bool have_more_tasks = false;
    while( (item = buffer.get_item( have_more_tasks )) != nullptr )
    {
        if( item->next() != nullptr )
        {
            std::cerr << "got not single Item*\n";
            std::cerr << "item " << item << "\n";
            std::cerr << "next " << item->next() << "\n";
            exit(1);
        }
        item_set.insert( item );
    }
    if( item_set.size() != ITEMS_COUNT )
    {
        std::cerr << "item_set.size() != ITEMS_COUNT\n";
        exit(1);
    }
    for( auto& item: items)
    {
        if( item_set.find( &item ) == item_set.end() )
        {
            std::cerr << "Got item " << &item << " not from initial array\n";
            exit(1);
        }
    }
}
