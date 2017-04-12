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

#pragma once

#include "bound_buffer.hpp"
#include "lock_free_stack.hpp"

namespace alterstack
{
/**
 * @brief Lock Free items Queue without starvation and any ordering (FIFO or LIFO)
 *
 * T item MUST have next() and set_next() methods (to make intrusive list)
 *
 * T* get_item() noexcept; dequeue single T* item from queue if exists
 * or return nullptr
 *
 * void put_item(T* item) noexcept; enqueue single T* item in queue
 */
template<typename T>
class alignas(64) LockFreeQueue
{
public:
    LockFreeQueue() = default;

    LockFreeQueue( const LockFreeQueue& ) = delete;
    LockFreeQueue( LockFreeQueue&& )      = delete;
    LockFreeQueue& operator=( const LockFreeQueue& ) = delete;
    LockFreeQueue& operator=( LockFreeQueue&& )      = delete;

    T*   get_item( bool &have_more_items ) noexcept;
    void put_item( T* item, uint32_t prio ) noexcept;

private:
    static constexpr uint32_t QUEUE_COUNT = 3;
    BoundBuffer<T>   m_item_buffer;
    LockFreeStack<T> m_prio_queue[ QUEUE_COUNT ]; // to fit LockFreeQueue in 64 bytes cache line
};

/**
 * @brief dequeue T* item or nullptr
 *
 * Dequeue T* and set flag if there is more stored items. Sometimes flag
 * can be unset even if queue has more stored T* items (false negative).
 * @param have_more_items will set this flag to true, if there is more Ts
 * @return single T* item (not list) or nullptr if queue is empty
 */
template<typename T>
T* LockFreeQueue<T>::get_item( bool& have_more_items ) noexcept
{
    T* item = m_item_buffer.get_item( have_more_items );
    if( item != nullptr )
    {
        return item;
    }
    T* items_list = nullptr;
    for( uint32_t i = 0; i < QUEUE_COUNT; ++i )
    {
        items_list = m_prio_queue[ i ].pop_list();
        if( items_list != nullptr )
            break;
    }
    if( items_list != nullptr )
    {
        item = items_list;
        items_list = items_list->next();
        item->set_next( nullptr );
        if( items_list != nullptr )
        {
            m_item_buffer.put_items_list( items_list );
            have_more_items = true;
        }
    }
    return item;
}

/**
 * @brief enqueue single T* item in queue
 *
 * @param item T* to store
 */
template<typename T>
void LockFreeQueue<T>::put_item( T* item, uint32_t prio ) noexcept
{
    if( prio >= QUEUE_COUNT )
        prio = QUEUE_COUNT - 1;
    m_prio_queue[ prio ].push( item );
}

}
