///////////////////////////////////////////////////////////////////////////////
// NAME:            plugin-resolver.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the plugin resolver.
//
// CREATED:         08/17/2022
//
// LAST EDITED:     08/17/2022
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

#include <stdlib.h>

#include "plugin-resolver.h"

typedef struct IdpPluginResolver {
    char** plugin_load_directories;
} IdpPluginResolver;

///////////////////////////////////////////////////////////////////////////////
// Public API
////

IdpPluginResolver* idp_plugin_resolver_new(char** plugin_load_directories) {
    return NULL;
}

void idp_plugin_resolver_free(IdpPluginResolver* resolver) {}

void idp_plugin_resolver_add_directory(IdpPluginResolver* resolver,
    char* load_directory)
{
}

char* idp_plugin_resolver_get_plugin_path(IdpPluginResolver* resolver,
    const char* plugin_name)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
