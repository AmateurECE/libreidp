///////////////////////////////////////////////////////////////////////////////
// NAME:            plugin-resolver.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the plugin resolver.
//
// CREATED:         08/17/2022
//
// LAST EDITED:     09/05/2022
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

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "plugin-resolver.h"
#include <libreidp/vector.h>

typedef struct IdpPluginResolver {
    IdpVector* directories;
} IdpPluginResolver;

static const char* PLUGIN_NAME_FORMAT = "libreidp-%s.so";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static char* get_plugin_filename_for_short_name(const char* short_name) {
    char path_name[64] = {0}; // Ought to be long enough
    snprintf(path_name, sizeof(path_name), PLUGIN_NAME_FORMAT, short_name);
    char* owned_path_name = strdup(path_name);
    if (NULL == owned_path_name) {
        perror("Couldn't allocate memory for pathname");
        exit(1);
    }

    return owned_path_name;
}

static char* path_concatenate(const char* first, const char* second) {
    size_t first_length = strlen(first);
    size_t full_length = first_length;
    if ('/' != first[first_length - 1]) {
        full_length += 1;
    }

    full_length += strlen(second) + 1;
    char* full_path = malloc(full_length);
    memset(full_path, 0, full_length);
    strcat(full_path, first);
    if ('/' != first[first_length - 1]) {
        full_path[first_length] = '/';
    }
    strcat(full_path, second);
    return full_path;
}

static char* get_plugin_path_for_name_in_dir(const char* directory_path,
                                             const char* plugin_name) {
    DIR* directory = opendir(directory_path);

    char error_message[80];
    if (NULL == directory) {
        strerror_r(errno, error_message, sizeof(error_message));
        fprintf(stderr, "Couldn't open plugin directory '%s': %s\n",
                directory_path, plugin_name);
        return NULL;
    }

    char* file_path = get_plugin_filename_for_short_name(plugin_name);

    struct dirent* entry = NULL;
    char* result = NULL;
    while (NULL != (entry = readdir(directory))) {
        if (0 == strcmp(entry->d_name, file_path)) {
            result = path_concatenate(directory_path, file_path);
            break;
        }
    }

    free(file_path);
    closedir(directory);
    return result;
}

static void string_free(char** string) { free(*string); }

///////////////////////////////////////////////////////////////////////////////
// Public API
////

IdpPluginResolver* idp_plugin_resolver_new() {
    IdpPluginResolver* resolver = malloc(sizeof(IdpPluginResolver));
    if (NULL == resolver) {
        perror("couldn't allocate memory for plugin resolver");
        exit(1);
    }

    memset(resolver, 0, sizeof(*resolver));
    resolver->directories =
        idp_vector_new(sizeof(char*), (IdpVectorFreeFn*)string_free);
    return resolver;
}

void idp_plugin_resolver_free(IdpPluginResolver* resolver) {
    idp_vector_free(resolver->directories);
    free(resolver);
}

void idp_plugin_resolver_add_directory(IdpPluginResolver* resolver,
                                       char* load_directory) {
    char** directory = idp_vector_reserve(resolver->directories);
    *directory = load_directory;
}

char* idp_plugin_resolver_get_plugin_path(IdpPluginResolver* resolver,
                                          const char* plugin_name) {
    IdpVectorIter iterator = {0};
    idp_vector_iter_init(&iterator, resolver->directories);
    char** directory = NULL;
    while (NULL != (directory = idp_vector_iter_next(&iterator))) {
        char* plugin_path =
            get_plugin_path_for_name_in_dir(*directory, plugin_name);
        if (NULL != plugin_path) {
            return plugin_path;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
