/*
 * Copyright 2017 Alexey Syrnikov <san@masterspline.net>
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
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/intrusive_list.hpp"
#include "alterstack/lock_free_queue.hpp"

class Item: public alterstack::IntrusiveList<Item>
{
};

using ItemsQueue = alterstack::LockFreeQueue<Item>;

TEST_CASE("API check")
{
    ItemsQueue queue;
    bool have_more_tasks = false;
    SECTION( "LockFreeQueue size is 64 bytes" ) {
        REQUIRE( sizeof(ItemsQueue) == 64 );
    }
    SECTION("LockFreeQueue aligned 64 bytes")
    {
        REQUIRE( (unsigned long long)(&queue) % 64 == 0);
    }
    SECTION( "empty LockFreeQueue returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( queue.get_item( have_more_tasks ) == nullptr );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        Item item;
        queue.put_item( &item, 1 );
        REQUIRE( queue.get_item( have_more_tasks ) == &item );
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( queue.get_item( have_more_tasks ) == nullptr );
            }
        }
    }
    SECTION( "get_task returns single Task*" )
    {
        Item item;
        queue.put_item( &item, 1 );
        Item* got_item = queue.get_item( have_more_tasks );
        REQUIRE( got_item->next() == nullptr );
    }
    constexpr int ITEMS_COUNT = 100;
    std::vector<Item> items( ITEMS_COUNT );
    SECTION( "many get_item() return the same set of tasks, that put_item() store" )
    {
        for( auto& item: items )
        {
            queue.put_item( &item, 1 );
        }
        Item* item;
        std::set<Item*> item_set;
        while( (item = queue.get_item( have_more_tasks )) != nullptr )
        {
            REQUIRE( item->next() == nullptr );
            item_set.insert( item );
        }
        REQUIRE( item_set.size() == ITEMS_COUNT );
        for( auto& item: items )
        {
            REQUIRE( item_set.find( &item ) != item_set.end() );
        }
    }
    SECTION( "get_item() will not set have_more flag with one Item*" )
    {
        Item item;
        queue.put_item( &item, 1 );
        have_more_tasks = false;
        Item* got_item = queue.get_item( have_more_tasks );
        REQUIRE( have_more_tasks == false );
    }
    SECTION( "get_task will set have_more flag with two Item*s" )
    {
        Item task1;
        queue.put_item( &task1, 1 );
        Item task2;
        queue.put_item( &task2, 1 );
        have_more_tasks = false;

        queue.get_item( have_more_tasks );
        REQUIRE( have_more_tasks == true );
        have_more_tasks = false;
        queue.get_item( have_more_tasks );
        REQUIRE( have_more_tasks == false );
    }
}
