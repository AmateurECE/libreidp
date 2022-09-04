///////////////////////////////////////////////////////////////////////////////
// NAME:            http-handler.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     An interface for a plugin that responds to HTTP requests.
//
// CREATED:         08/19/2022
//
// LAST EDITED:     09/04/2022
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

#ifndef IDP_HTTP_HANDLER_H
#define IDP_HTTP_HANDLER_H

#include <libreidp/core/http.h>

typedef enum IdpHttpHandlerVersion {
    IDP_HTTP_HANDLER_VERSION_UNSTABLE,
} IdpHttpHandlerVersion;

typedef struct IdpHttpHandlerPlugin {
    IdpHttpCoreResult (*register_endpoints)(IdpHttpCore* core);
} IdpHttpHandlerPlugin;

#endif // IDP_HTTP_HANDLER_H

///////////////////////////////////////////////////////////////////////////////
