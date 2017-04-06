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

#include "alterstack/intrusive_list.hpp"
#include "alterstack/lock_free_stack.hpp"

class Item : public alterstack::IntrusiveList<Item>
{
};

using ItemsStack = alterstack::LockFreeStack<Item>;

TEST_CASE("API check")
{
    ItemsStack stack;
    SECTION( "empty LockFreeStack returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( stack.pop_all() == nullptr );
        }
    }
    SECTION( "first push returns true, next false, push after pop_all returns true" )
    {
        std::vector<Item> items(100);
        bool res = stack.push(&items[0]);
        REQUIRE( res == true );
        for( unsigned i = 1; i < items.size(); ++i)
        {
            res = stack.push(&items[i]);
            REQUIRE( res == false );
        }
        stack.pop_all();
        items.clear();
        items.resize(100);
        res = stack.push(&items[0]);
        REQUIRE( res == true );
        for( unsigned i = 1; i < items.size(); ++i)
        {
            res = stack.push(&items[i]);
            REQUIRE( res == false );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        Item item;
        stack.push( &item );
        Item* got_item = stack.pop_all();
        REQUIRE( got_item == &item );
        REQUIRE( got_item->next() == nullptr );
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( stack.pop_all() == nullptr );
            }
        }
    }
    constexpr int ITEMS_COUNT = 100;
    std::vector<Item> items(ITEMS_COUNT);
    SECTION( "pop_all returns the same set of tasks, that push store" )
    {
        for( auto& item: items )
        {
            stack.push( &item );
        }
        Item* items_list = stack.pop_all();
        std::set<Item*> items_set;
        while( items_list != nullptr )
        {
            items_set.insert( items_list );
            items_list = items_list->next();
        }
        REQUIRE( items_set.size() == ITEMS_COUNT );
        for( auto& item: items)
        {
            REQUIRE( items_set.find( &item ) != items_set.end() );
            items_set.erase( &item );
        }
    }
}

