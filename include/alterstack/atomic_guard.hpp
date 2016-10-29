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

namespace alterstack
{

/**
 * @brief RAII guard set guarded bool to true in destructor
 */
class AtomicReturnBoolGuard
{
public:
  /**
   * @brief constructor. Stores reference to guarded data.
   * @param guarded_data std::atomic<bool> reference
   */
  AtomicReturnBoolGuard(std::atomic<bool> &guarded_data);
  AtomicReturnBoolGuard()                    = delete;
  AtomicReturnBoolGuard(const AtomicReturnBoolGuard&) = delete;
  AtomicReturnBoolGuard(AtomicReturnBoolGuard&& )     = delete;
  /**
   * @brief destructor. Set bool to false
   */
  ~AtomicReturnBoolGuard();
  AtomicReturnBoolGuard& operator=(const AtomicReturnBoolGuard&) = delete;
  AtomicReturnBoolGuard& operator=(AtomicReturnBoolGuard&&)      = delete;
private:
  std::atomic<bool>& m_guarded_data; //!< reference to guarded data
};

inline AtomicReturnBoolGuard::AtomicReturnBoolGuard(std::atomic<bool> &guarded_data)
  :m_guarded_data(guarded_data)
{}

inline AtomicReturnBoolGuard::~AtomicReturnBoolGuard()
{
    m_guarded_data.store(true, std::memory_order_release);
}

template <typename T>
/**
 * @brief RAII guard make atomic increment in constructor and decrement in destructor
 */
class AtomicGuard
{
public:
  /**
   * @brief constructor. Increments counter
   * @param guarded_data std::atomic<T> counter reference, T integral type
   */
  AtomicGuard(std::atomic<T> &guarded_data);
  AtomicGuard()                    = delete;
  AtomicGuard(const AtomicGuard&) = delete;
  AtomicGuard(AtomicGuard&& )     = delete;
  /**
   * @brief destructor. Decrements counter
   */
  ~AtomicGuard();
  AtomicGuard& operator=(const AtomicGuard&) = delete;
  AtomicGuard& operator=(AtomicGuard&&)      = delete;
private:
  std::atomic<T>& m_guarded_data; //!< reference to incremented/decremented counter
};

template <typename T>
AtomicGuard<T>::AtomicGuard(std::atomic<T> &guarded_data)
  :m_guarded_data(guarded_data)
{
    m_guarded_data.fetch_add(1, std::memory_order_acq_rel);
}
template <typename T>
AtomicGuard<T>::~AtomicGuard()
{
    m_guarded_data.fetch_sub(1, std::memory_order_acq_rel);
}

}
