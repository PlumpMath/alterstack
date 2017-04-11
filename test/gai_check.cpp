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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>

#include <iostream>

#include "cpu_utils.hpp"

int main()
{
    uint32_t aux;
    uint64_t ts_begin = rdtsc();
    uint32_t eax = 1, ebx, ecx, edx;
    cpuid(eax, ebx,ecx,edx);
    uint64_t ts_end = rdtsc();
    std::cout << "cpuid got " << ts_end -ts_begin << " clocks\n";
    std::cout << "apic ID = " << ((ebx >> 24) & 0xFF) << "\n";
    return 0;

    struct addrinfo hint;
    ::memset(&hint,0,sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    struct addrinfo *res;
    getaddrinfo("google.com",0,&hint,&res);
    struct addrinfo *list = res;
    char dst[1024];
    while(list != 0)
    {
        const char* name;
        if(list->ai_family == AF_INET)
        {
            name = inet_ntop(list->ai_family
                             ,&((struct sockaddr_in *)list->ai_addr)->sin_addr
                             ,dst,1024);
        }
        if(list->ai_family == AF_INET6)
        {
            name = inet_ntop(list->ai_family
                             ,&((struct sockaddr_in6 *)list->ai_addr)->sin6_addr
                             ,dst,1024);
        }
        if( name != 0 )
        {
            std::cout << name << " "
                      << "socktype " << list->ai_socktype
                      <<"\n";
        }
        else
        {
            std::cout << "error\n";
        }
        list = list->ai_next;
    }
    freeaddrinfo(res);
}
