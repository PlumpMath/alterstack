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
#include <deque>
#include <memory>
#include <thread>

#include "BgThread.hpp"

namespace alterstack
{
class Scheduler;
/**
 * @brief Creates pool of threads which runs Task in background
 */
class BgRunner
{
public:
    BgRunner() = delete;
    /**
     * @brief thread pool constructor
     * @param scheduler Scheduler reference
     * @param min_spare number of threads to start
     * @param max_running max BgThread to use
     */
    explicit BgRunner(
            Scheduler* scheduler
            ,uint32_t min_spare = 1
            ,uint32_t max_running = std::thread::hardware_concurrency());
    ~BgRunner();

    /**
     * @brief set spare BgThread count
     * @param cores count
     */
    void set_min_cores(uint32_t cores); // not implemented
    /**
     * @brief set max BgThread limit
     * @param cores max limit
     */
    void set_max_cores(uint32_t cores); // not implemented

private:
    friend class BgThread;
    friend class Scheduler;
    /**
     * @brief wake up all sleeping BgThread's
     */
    void notify_all();
    /**
     * @brief notify BgRunner, that there is more Task s in RunningQueue
     *
     * If some BgThread is sleeping, one will be woked up
     */
    void notify();

    Scheduler* scheduler_;   //!< reference to Scheduler
    uint32_t   min_spare_;   //!< min spare threads
    uint32_t   max_running_; //!< max BgThread threads limit
    ::std::deque<std::unique_ptr<BgThread>> m_cpu_core_list;
};

}

