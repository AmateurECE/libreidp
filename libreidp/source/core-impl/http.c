///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the HTTP core.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llhttp.h>
#include <libreidp/priv/core/http.h>

static const int DEFAULT_BACKLOG = 10;
static const char* DEFAULT_LISTEN_ADDRESS = "0.0.0.0";
static const char* NOT_FOUND_RESPONSE = "\
HTTP/1.1 404 Not Found\r\n\
Server: libreidp\r\n\
Date: Sat, 03 Sep 2022 20:05:17 GMT\r\n\
Content-Type: text/html\r\n\
Content-Length: 149\r\n\
Connection: close\r\n\
\r\n\
<html>\r\n\
<head><title>404 Not Found</title></head>\r\n\
<body>\r\n\
<center><h1>404 Not Found</h1></center>\r\n\
<hr><center>libreidp</center>\r\n\
</body>\r\n\
</html>\r\n\
";

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
        IdpHttpCore* core = client->data;
        core->http_parser.data = client;
        int result = llhttp_execute(&core->http_parser, buf->base, nread);
        if (HPE_OK != result) {
            fprintf(stderr, "Invalid request received\n");
            uv_close((uv_handle_t*)client, NULL);
        }
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
    client->data = core;
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

static int idp_http_core_on_message_complete(llhttp_t* parser) {
    uv_write_t* response = malloc(sizeof(uv_write_t));
    if (NULL == response) {
        fprintf(stderr, "Couldn't allocate memory for write stream\n");
        exit(1);
    }

    memset(response, 0, sizeof(*response));

    uv_stream_t* client = parser->data;
    uv_buf_t response_buffer = uv_buf_init(strdup(NOT_FOUND_RESPONSE),
        strlen(NOT_FOUND_RESPONSE));
    uv_write(response, client, &response_buffer, 1, echo_write);

    return HPE_OK;
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

    llhttp_settings_init(&core->parser_settings);
    core->parser_settings.on_message_complete =
        idp_http_core_on_message_complete;
    llhttp_init(&core->http_parser, HTTP_BOTH, &core->parser_settings);

    return core;
}

void idp_http_core_add_port(IdpHttpCore* core, int port) {
    core->port = port;
}

void idp_http_core_shutdown(IdpHttpCore* core) {
    llhttp_reset(&core->http_parser);
    // TODO: Check if uv handle is active here?
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
