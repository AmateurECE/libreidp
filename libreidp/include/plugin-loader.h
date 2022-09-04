///////////////////////////////////////////////////////////////////////////////
// NAME:            plugin-loader.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Load a plugin given a path to its file.
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

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <libreidp/libreidp.h>

typedef struct IdpPlugin IdpPlugin;

IdpPlugin* idp_plugin_load(char* filename);
void idp_plugin_remove(IdpPlugin* plugin);

IdpPluginInterface idp_plugin_get_interface(const IdpPlugin* plugin);
IdpHttpInterface* idp_plugin_get_http_interface(IdpPlugin* plugin);

#endif // PLUGIN_LOADER_H

///////////////////////////////////////////////////////////////////////////////
