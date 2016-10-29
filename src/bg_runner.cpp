/*
 * Copyright 2015-2016 Alexey Syrnikov <san@masterspline.net>
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

#include "alterstack/bg_runner.hpp"

#include "alterstack/atomic_guard.hpp"
#include "alterstack/scheduler.hpp"
#include "alterstack/task.hpp"
#include "alterstack/logger.hpp"

namespace alterstack
{
/**
 * @brief thread pool constructor
 * @param scheduler Scheduler reference
 * @param min_spare number of threads to start
 * @param max_running max BgThread to use
 */
BgRunner::BgRunner(
        Scheduler* scheduler
        ,uint32_t min_spare
        ,uint32_t max_running )
{
    LOG << "BgRunner::BgRunner()\n";
    for(uint32_t i = 0; i < min_spare; ++i)
    {
        m_cpu_core_list.push_back(::std::unique_ptr<BgThread>(new BgThread(scheduler)));
    }
}

BgRunner::~BgRunner()
{
    for( auto& core: m_cpu_core_list)
    {
        core->request_stop();
    }
    LOG << "BgRunner::~BgRunner()\n";
}
/**
 * @brief wake up all sleeping BgThread's
 */
void BgRunner::notify_all()
{
    if( BgThread::sleep_count() )
    {
        for( auto& core: m_cpu_core_list)
        {
            core->wake_up();
        }
    }
}
/**
 * @brief notify BgRunner, that there is more Task s in RunningQueue
 *
 * If some BgThread is sleeping, one will be woked up
 */
void BgRunner::notify()
{
    if( BgThread::sleep_count() )
    {
        // FIXME: only single CpuCore need to by notifyed here
        for( auto& core: m_cpu_core_list)
        {
            core->wake_up();
        }
    }
}

}
