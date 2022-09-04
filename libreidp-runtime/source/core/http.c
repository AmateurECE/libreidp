///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the public HTTP core API.
//
// CREATED:         08/22/2022
//
// LAST EDITED:     09/03/2022
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

#include <libreidp/priv/core/http.h>

///////////////////////////////////////////////////////////////////////////////
// Request
////

const char* idp_http_request_get_path(IdpHttpRequest* request)
{ return request->path; }

const char* idp_http_request_get_header(IdpHttpRequest* request,
    const char* name)
{
    for (size_t i = 0; NULL != request->headers[i].name; ++i) {
        if (name == request->headers[i].name) {
            return request->headers[i].value;
        }
    }

    return NULL;
}

const char* idp_http_request_get_body(IdpHttpRequest* request)
{ return request->body; }

size_t idp_http_request_get_body_length(IdpHttpRequest* request)
{ return request->body_length; }

///////////////////////////////////////////////////////////////////////////////
