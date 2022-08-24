///////////////////////////////////////////////////////////////////////////////
// NAME:            main.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Entrypoint for libreidp.
//
// CREATED:         08/16/2022
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

#include "idp-config.h"
#include "plugin-loader.h"
#include "plugin-resolver.h"

///////////////////////////////////////////////////////////////////////////////
// Supporting Logic
////

static IdpPlugin** idp_load_plugins(const IdpConfig* config) {
    // Initialize the resolver
    IdpPluginResolver* resolver = idp_plugin_resolver_new();
    for (size_t i = 0; NULL != config->plugin_load_directories[i]; ++i) {
        printf("Plugin directory: %s\n", config->plugin_load_directories[i]);
        idp_plugin_resolver_add_directory(resolver,
            strdup(config->plugin_load_directories[i]));
    }

    // Prepare a list for the loaded plugins
    size_t number_plugins = 0;
    for (; NULL != config->plugins[number_plugins]; ++number_plugins);
    number_plugins += 1; // for NULL sentinel

    IdpPlugin** loaded_plugins = malloc(number_plugins * sizeof(IdpPlugin*));
    if (NULL == loaded_plugins) {
        perror("couldn't allocate memory for plugin registry");
        exit(1);
    }
    memset(loaded_plugins, 0, number_plugins * sizeof(IdpPlugin*));
    size_t number_loaded_plugins = 0;

    // Resolve any requested plugins to a filesystem path
    for (size_t i = 0; NULL != config->plugins[i]; ++i) {
        char* plugin_path = idp_plugin_resolver_get_plugin_path(resolver,
            config->plugins[i]);
        if (NULL == plugin_path) {
            fprintf(stderr, "Unable to locate plugin \"%s\"\n",
                config->plugins[i]);
        } else {
            printf("Discovered plugin %s...", plugin_path);
            IdpPlugin* loaded_plugin = idp_plugin_load(plugin_path);
            if (NULL == loaded_plugin) {
                printf("failed to load\n");
            } else {
                printf("interface: %s\n", idp_plugin_interface_to_str(
                        idp_plugin_get_interface(loaded_plugin)));
            }

            loaded_plugins[number_loaded_plugins++] = loaded_plugin;
        }
    }

    idp_plugin_resolver_free(resolver);
    return loaded_plugins;
}

///////////////////////////////////////////////////////////////////////////////
// Main
////

int main() {
    // Static configuration (for now).
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

    IdpPlugin** loaded_plugins = idp_load_plugins(&config);

    // Free the loaded plugins
    for (size_t i = 0; NULL != loaded_plugins[i]; ++i) {
        idp_plugin_remove(loaded_plugins[i]);
    }

    free(loaded_plugins);
}

///////////////////////////////////////////////////////////////////////////////
