#pragma once
// this file is based on original boost::context library
// Alex Syrnikov <san@masterspline.net>

//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdint>

namespace switch_context
{

typedef void*   fcontext_t;

struct transfer_t
{
    fcontext_t  fctx;
    void    *   data;
};

extern "C"
transfer_t jump_fcontext( fcontext_t const to, void * vp);
extern "C"
fcontext_t make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

// based on an idea of Giovanni Derreta
extern "C" transfer_t ontop_fcontext( fcontext_t const to, void * vp, transfer_t (* fn)( transfer_t) );

}
