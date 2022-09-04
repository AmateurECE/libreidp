///////////////////////////////////////////////////////////////////////////////
// NAME:            oauth2.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Plugin for exposing an OAuth2 identity provider.
//
// CREATED:         08/17/2022
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

#include <stdio.h>
#include <stdlib.h>
#include <libreidp/libreidp.h>

static IdpHttpCoreResult oauth2_register_endpoints(IdpHttpCore* core) {
    fprintf(stderr, "%s: register called with core %p\n", __FILE__,
        (void*)core);
    return (IdpHttpCoreResult){.ok = true};
}

IdpPluginDefinition idp_plugin_definition = {
    .interface = IDP_PLUGIN_HTTP,
    .http = {
        .version = IDP_HTTP_INTERFACE_UNSTABLE,
        .register_endpoints = oauth2_register_endpoints,
    },
};

///////////////////////////////////////////////////////////////////////////////
