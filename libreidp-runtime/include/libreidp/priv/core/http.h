///////////////////////////////////////////////////////////////////////////////
// NAME:            http.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the HTTP core.
//
// CREATED:         08/22/2022
//
// LAST EDITED:     08/24/2022
//
// Copyright 2022, Ethan D. Twardy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
////

#ifndef IDP_CORE_IMPL_HTTP_H
#define IDP_CORE_IMPL_HTTP_H

#include <uv.h>
#include <libreidp/core/http.h>

typedef struct IdpHttpCore {
    uv_tcp_t server;
} IdpHttpCore;

// Initialize an HTTP core
IdpHttpCore* idp_http_core_new();

// Configure the core to begin listening on `port'.
void idp_http_core_add_port(IdpHttpCore* core, int port);

// Shutdown an HTTP core
void idp_http_core_shutdown(IdpHttpCore* core);

// Register all watchers with an event loop. Since handles can later be closed
// without a reference to the run loop, this is the only additional method
// necessary to integrate with the rest of the libuv plumbing.
//
// NOTE: This must be the last function that's called while setting up!
void idp_http_core_register(IdpHttpCore* core, uv_loop_t* loop);

#endif // IDP_CORE_IMPL_HTTP_H

///////////////////////////////////////////////////////////////////////////////
