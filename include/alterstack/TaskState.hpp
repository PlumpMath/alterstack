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

namespace alterstack
{
enum class TaskState
{
    Running,
    Waiting,
    Clearing, ///< Task function finished, but it's stack can still be in use, so it's temporary
              /// state to Finished
    Finished ///< this state MUST be set (atomically) after context switch
             /// because Task destructor in some thread can exit in case TaskState == Finished
             /// and Task memory can be freed so nobody can read or write Task data in parallel
             /// thread if TaskState == Finished (Task stack also MUST be not used)
};

}
