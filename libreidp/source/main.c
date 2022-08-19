///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for libreidp.
//
// CREATED:         08/16/2022
//
// LAST EDITED:     08/18/2022
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

#include "idp-config.h"
#include "plugin-resolver.h"

int main() {
    char* plugins[] = {
        "dummy",
        NULL,
    };

    char* plugin_load_directories[] = {
        "plugins/dummy",
        NULL,
    };

    const IdpConfig config = {
        .auth_form_path = "/auth",
        .auth_form_uri = "", // Authentication form resides on our server
        .plugins = plugins,
        .plugin_load_directories = plugin_load_directories,
    };

    IdpPluginResolver* resolver = idp_plugin_resolver_new();
    for (size_t i = 0; NULL != config.plugin_load_directories[i]; ++i) {
        printf("Plugin directory: %s\n", config.plugin_load_directories[i]);
        idp_plugin_resolver_add_directory(resolver,
            strdup(config.plugin_load_directories[i]));
    }

    for (size_t i = 0; NULL != config.plugins[i]; ++i) {
        char* plugin_path = idp_plugin_resolver_get_plugin_path(resolver,
            config.plugins[i]);
        if (NULL == plugin_path) {
            fprintf(stderr, "Unable to locate plugin \"%s\"\n",
                config.plugins[i]);
        } else {
            printf("Discovered plugin %s\n", plugin_path);
        }
        free(plugin_path);
    }

    idp_plugin_resolver_free(resolver);
}

///////////////////////////////////////////////////////////////////////////////
