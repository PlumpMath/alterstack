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

#pragma once

#include <algorithm>
#include <cstdint>
#include <atomic>

namespace alterstack
{
/**
 * @brief Almost fifo lockfree buffer to hold Task* in RunningTaskQueue
 *
 * TaskBuffer is fifo-like bound lockfree buffer which hold Task* in RunningTaskQueue.
 * Task* order can change especially if stored Task* count exceeds buffer slots.
 * TaskBuffer uses intrusive Task* list and does not need any memory except
 * already allocated and function stack for local variables.
 *
 * Task* get_task(bool &have_more) noexcept; return single Task* from buffer if
 * exists or return nullptr
 *
 * void put_task(Task* task) noexcept; always put Task* (single or list)
 * in buffer
 */
template<typename T>
class alignas(64) BoundBuffer
{
public:
    BoundBuffer() noexcept;

    BoundBuffer( const BoundBuffer& ) = delete;
    BoundBuffer( BoundBuffer&& )      = delete;
    BoundBuffer& operator=( const BoundBuffer& ) = delete;
    BoundBuffer& operator=( BoundBuffer&& )      = delete;
    /**
     * @brief get T* or nullptr from BoundBuffer
     *
     * get_item(bool &have_more) also set have_more flag to true if there is more
     * items in buffer (rare false negative possible, when there are more items,
     * but have_more will be false).
     * @param have_more will be set to true, if there is more T* items (not always)
     * @return single T* (not list) or nullptr if no items in buffer
     */
    T* get_item( bool& have_more ) noexcept;
    /**
     * @brief store item in buffer slot (empty or nonempty)
     *
     * put_item_list(T* task) will store item in single slot, it will not
     * distribute it in different buffer slots even if task is list (this
     * will done in get_item() stage).
     * @param item T* (single or list) to store
     */
    void put_items_list( T* item ) noexcept;
    /**
     * @brief find last item in intrusive list (i.e. next() == nullptr)
     * @param item_list T* intrusive list
     * @return item T* whose next() == nullptr
     */
    static T* find_last_item_in_list(T* items_list);

    private:
    /**
     * @brief store T* (single or list) in any empty slot in buffer
     * @param item T* to store
     * @return true if empty slot found and T* stored there
     */
    bool store_in_empty_slot( T *item ) noexcept;
    /**
     * @brief store item T* (single or list) in slot poined by m_put_position
     * @param item_list T* list to store
     */
    void store_in_occupied_slot( T* items_list ) noexcept;
    /**
     * @brief distribute item list T* in empty slots first, and rest as list in occupied
     * @param item_list list to store
     */
    void store_tail( T* item_list) noexcept;

    static constexpr uint32_t BUFFER_SIZE = 7; // to fit in 64 bytes cache line
    std::atomic<uint32_t> m_get_position;
    std::atomic<uint32_t> m_put_position;
    std::atomic<T*>       m_buffer[BUFFER_SIZE];
};
template<typename T>
BoundBuffer<T>::BoundBuffer() noexcept
{
    std::fill( m_buffer, m_buffer + BUFFER_SIZE, nullptr);
    m_get_position.store(0, std::memory_order_relaxed);
    m_put_position.store(0, std::memory_order_relaxed);
}
template<typename T>
T* BoundBuffer<T>::get_item( bool& have_more ) noexcept
{
    T* item;
    uint32_t index = m_get_position.load(std::memory_order_relaxed);
    uint32_t i = 0;
    while( i < BUFFER_SIZE )
    {
        item = m_buffer[(index + i) % BUFFER_SIZE].exchange(
                    nullptr, std::memory_order_acquire );
        if( item != nullptr )
        {
            m_get_position.fetch_add( i + 1, std::memory_order_relaxed );
            if( item->next() != nullptr )
            {
                T* items_tail = item->next();
                store_tail( items_tail );
                item->set_next( nullptr );
                have_more = true;
            }
            break;
        }
        uint32_t next_index = m_get_position.load(std::memory_order_relaxed);
        if( index != next_index )
        {
            index = next_index;
            i = 0;
        }
        else
        {
            ++i;
        }
    }
    return item;
}

template<typename T>
bool BoundBuffer<T>::store_in_empty_slot(T* item) noexcept
{
    uint32_t index = m_put_position.load(std::memory_order_relaxed);
    uint32_t i = 0;
    while( i < BUFFER_SIZE )
    {
        T* expected = nullptr;
        bool change_done = m_buffer[(index + i) % BUFFER_SIZE].compare_exchange_strong(
                    expected, item
                    ,std::memory_order_release, std::memory_order_relaxed);
        if( change_done )
        {
            m_put_position.fetch_add( i + 1, std::memory_order_relaxed);
            return true;
        }
        uint32_t next_index = m_put_position.load(std::memory_order_relaxed);
        if( index != next_index )
        {
            index = next_index;
            i = 0;
        }
        else
        {
            ++i;
        }
    }
    return false;
}

template<typename T>
void BoundBuffer<T>::store_tail(T* items_list) noexcept
{
    for( uint32_t i = 0; i < BUFFER_SIZE-1; ++i)
    {
        if( items_list == nullptr)
        {
            return;
        }
        T* item = items_list;
        items_list = items_list->next();
        item->set_next( nullptr );
        if( !store_in_empty_slot( item ) )
        {
            item->set_next( items_list );
            store_in_occupied_slot( item );
            return;
        }
    }
    store_in_occupied_slot( items_list );
}

template<typename T>
T* BoundBuffer<T>::find_last_item_in_list( T* items_list )
{
    T* last_item = items_list;
    while( last_item->next() != nullptr )
    {
        last_item = last_item->next();
    }
    return last_item;
}

template<typename T>
void BoundBuffer<T>::store_in_occupied_slot( T* items_list ) noexcept
{
    uint32_t index = m_put_position.fetch_add(1, std::memory_order_relaxed);
    T* old_list = m_buffer[index % BUFFER_SIZE].exchange(
                items_list, std::memory_order_acq_rel);
    if( old_list == nullptr )
    {
        return;
    }

    T* last_task_in_list = find_last_item_in_list( old_list );

    last_task_in_list->set_next( items_list );
    while( !m_buffer[index % BUFFER_SIZE].compare_exchange_weak(
               items_list, old_list
               ,std::memory_order_release, std::memory_order_relaxed) )
    {
        last_task_in_list->set_next( items_list );
    }
}

template<typename T>
void BoundBuffer<T>::put_items_list( T* items_list ) noexcept
{
    if( store_in_empty_slot(items_list) )
    {
        return;
    }
    store_in_occupied_slot(items_list);
}
}
