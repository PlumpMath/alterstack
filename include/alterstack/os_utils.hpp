/*
 * Copyright 2016 Alexey Syrnikov <san@masterspline.net>
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

#ifdef __linux__
#include <sys/prctl.h>

namespace alterstack
{
namespace os
{

void set_thread_name()
{
    static const char* name = "BgThread";
    ::prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));
}
#else
void set_thread_name()
{}
#endif

} // namespace os
} // namespace alterstack
