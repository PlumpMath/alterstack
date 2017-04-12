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
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/bound_buffer.hpp"
#include "alterstack/intrusive_list.hpp"

namespace alterstack
{
class Item : public IntrusiveList<Item>
{
};

using alterstack::BoundBuffer;

TEST_CASE("API check")
{
    BoundBuffer<Item> buffer;
    bool have_more_items = false;
    SECTION( "empty BoundBuffer returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( buffer.get_item( have_more_items ) == nullptr );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        Item item;
        buffer.put_items_list( &item );
        REQUIRE( buffer.get_item( have_more_items ) == &item );
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( buffer.get_item(have_more_items) == nullptr );
            }
        }
    }
    SECTION( "get_items_list with single item returns single Item* (not list)" )
    {
        Item item;
        buffer.put_items_list( &item );
        Item* got_task = buffer.get_item( have_more_items );
        REQUIRE( got_task->next() == nullptr );
    }
    constexpr int ITEMS_COUNT = 100;
    std::vector<Item> items( ITEMS_COUNT );
    SECTION( "many get_item() return the same set of tasks, that put_items_list() store" )
    {
        for(auto& item: items)
        {
            buffer.put_items_list( &item );
        }
        Item* task;
        std::set<Item*> task_set;
        while( (task = buffer.get_item( have_more_items )) != nullptr )
        {
            REQUIRE( task->next() == nullptr );
            task_set.insert(task);
        }
        REQUIRE( task_set.size() == ITEMS_COUNT );
        for(auto& task: items)
        {
            REQUIRE( task_set.find(&task) != task_set.end() );
        }
    }
    SECTION( "get_item() will not set have_more flag with one Task*" )
    {
        Item item;
        buffer.put_items_list( &item );
        have_more_items = false;
        Item* got_item = buffer.get_item( have_more_items );
        REQUIRE( have_more_items == false );
    }
    SECTION( "get_item() will set have_more flag with two Item*s" )
    {
        Item item1, item2;;
        item1.set_next( &item2 );
        buffer.put_items_list( &item1 );
        have_more_items = false;
        buffer.get_item( have_more_items );
        REQUIRE( have_more_items == true );
        have_more_items = false;
        buffer.get_item( have_more_items );
        REQUIRE( have_more_items == false );
    }
}

}
