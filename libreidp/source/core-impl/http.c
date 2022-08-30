///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the HTTP core.
//
// CREATED:         08/22/2022
//
// LAST EDITED:     08/29/2022
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

static const int DEFAULT_BACKLOG = 10;
static const char* DEFAULT_LISTEN_ADDRESS = "0.0.0.0";

///////////////////////////////////////////////////////////////////////////////
// Private API
////

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void echo_write(uv_write_t* req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }

    free(req);
}

static void echo_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
    if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*)client, NULL);
        }
    } else if (nread > 0) {
        uv_write_t* req = malloc(sizeof(uv_write_t));
        uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
        uv_write(req, client, &wrbuf, 1, echo_write);
    }

    if (buf->base) {
        free(buf->base);
    }
}

static void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
    IdpHttpCore* core = server->data;
    uv_tcp_init(core->loop, client);
    if (0 == uv_accept(server, (uv_stream_t*)client)) {
        uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
    }
}

static IdpHttpCoreResult idp_http_core_result_error_from_libuv(
    IdpHttpCoreError error, int status)
{
    return (IdpHttpCoreResult){
        .ok = false,
        .error = {
            .owned = true,
            .error = error,
            .message = {
                .owned = strdup(uv_strerror(status)),
            },
        },
    };
}

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

void idp_http_core_add_port(IdpHttpCore* core, int port) {
    core->port = port;
}

void idp_http_core_shutdown(IdpHttpCore* core) {
    free(core);
}

IdpHttpCoreResult idp_http_core_register(IdpHttpCore* core, uv_loop_t* loop) {
    uv_tcp_init(loop, &core->server);
    // Bind pointers to objects for callbacks
    core->server.data = core;
    core->loop = loop;

    uv_ip4_addr(DEFAULT_LISTEN_ADDRESS, core->port, &core->address);
    uv_tcp_bind(&core->server, (const struct sockaddr*)&core->address, 0);

    int result = uv_listen((uv_stream_t*)&core->server, DEFAULT_BACKLOG,
        on_new_connection);
    if (0 != result) {
        return idp_http_core_result_error_from_libuv(
            IDP_HTTP_CORE_LISTEN_ERROR, result);
    }

    return (IdpHttpCoreResult){.ok = true};
}

///////////////////////////////////////////////////////////////////////////////
