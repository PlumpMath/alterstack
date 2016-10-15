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

#include <netdb.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <cstdlib>


void notify_function (union sigval data)
{
    ::std::cout << "notify_function tid = " << ::syscall(SYS_gettid)
                << " sigval = " << data.sival_int << "\n";

}

int main()
{
    gaicb* cb = new gaicb[4];
    cb[0].ar_name = "www.ru";
    cb[1].ar_name = "www.linux.org.ru";
    cb[2].ar_name = "www.ya.ru";
    cb[3].ar_name = "www.mail.ru";
    gaicb** cb_pptr = &cb;
    sigevent event;
    event.sigev_notify = SIGEV_THREAD;
    event.sigev_signo = 0;
    event.sigev_value.sival_int = 0;
    event.sigev_notify_function = notify_function;
    event.sigev_notify_attributes = 0;
    for( int i = 0; i < 4; ++i )
    {
        event.sigev_value.sival_int = i;
        //cb_ptr = cb+i;
        int ret = ::getaddrinfo_a(GAI_NOWAIT, cb_pptr, 1, &event);
        if (ret) {
            std::cout << "getaddrinfo_a() failed: " << gai_strerror(ret) << "\n";
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
    sleep(1);
    return 0;
}
