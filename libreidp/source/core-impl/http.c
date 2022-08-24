///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libreidp/priv/core/http.h>

///////////////////////////////////////////////////////////////////////////////
// Public API
////

IdpHttpCore* idp_http_core_new() {
    IdpHttpCore* core = malloc(sizeof(IdpHttpCore));
    if (NULL == core) {
        perror("couldn't allocate memory for HTTP core");
        exit(1);
    }

    memset(core, 0, sizeof(IdpHttpCore));
    return core;
}

void idp_http_core_shutdown(IdpHttpCore* core) {
    free(core);
}

void idp_http_core_register(IdpHttpCore* core, uv_loop_t* loop) {}

///////////////////////////////////////////////////////////////////////////////
