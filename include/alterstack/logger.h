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

#include "alterstack/scheduler.h"

#include <CrashLog.hpp>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

//#define LOG ::std::cout << "  tid,task:" << ::syscall(SYS_gettid) << "," << Scheduler::get_current_task() << "  "

#ifdef DISABLE_LOG
class EmptyLog
{
public:
    template<typename T>
    EmptyLog& operator <<( T )
    { return *this; }
};
#endif

inline LogStream& trace_log()
{
    static LogStream file_plog("trace.log");
    return file_plog;
}

#ifndef DISABLE_TRACE_LOG
    #define TRACE_LOG trace_log()
#else
    #define TRACE_LOG EmptyLog()
#endif

#define LOG_TASK_INFO "  tid,task:" << ::syscall(SYS_gettid) << "," << Scheduler::get_current_task() << "  "
#define LOG TRACE_LOG << LOG_TASK_INFO
