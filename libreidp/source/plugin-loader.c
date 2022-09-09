///////////////////////////////////////////////////////////////////////////////
// NAME:            plugin-loader.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Plugin loader for LibreIdP proper.
//
// CREATED:         08/20/2022
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

#include "plugin-loader.h"
#include <dlfcn.h>
#include <libreidp/libreidp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct IdpPlugin {
    IdpPluginDefinition* definition;
    char* file_path;
    void* dl_handle;
} IdpPlugin;

static const char* PLUGIN_DEFINITION_SYMBOL_NAME = "idp_plugin_definition";

///////////////////////////////////////////////////////////////////////////////
// Public API
////

IdpPlugin* idp_plugin_load(char* filename) {
    IdpPlugin* plugin = malloc(sizeof(IdpPlugin));
    if (NULL == plugin) {
        perror("couldn't allocate memory for plugin");
        exit(1);
    }

    memset(plugin, 0, sizeof(*plugin));
    plugin->file_path = filename;

    plugin->dl_handle = dlopen(plugin->file_path, RTLD_NOW | RTLD_LOCAL);
    if (NULL == plugin->dl_handle) {
        perror(dlerror());
        free(plugin->file_path);
        free(plugin);
        return NULL;
    }

    plugin->definition =
        dlsym(plugin->dl_handle, PLUGIN_DEFINITION_SYMBOL_NAME);
    if (NULL == plugin->definition) {
        perror(dlerror());
        dlclose(plugin->dl_handle);
        free(plugin->file_path);
        free(plugin);
        return NULL;
    }
    return plugin;
}

void idp_plugin_remove(IdpPlugin* plugin) {
    dlclose(plugin->dl_handle);
    free(plugin->file_path);
    free(plugin);
}

IdpPluginInterface idp_plugin_get_interface(const IdpPlugin* plugin) {
    return plugin->definition->interface;
}

IdpHttpInterface* idp_plugin_get_http_interface(IdpPlugin* plugin) {
    if (IDP_PLUGIN_HTTP != plugin->definition->interface) {
        return NULL;
    }

    return &plugin->definition->http;
}

///////////////////////////////////////////////////////////////////////////////
