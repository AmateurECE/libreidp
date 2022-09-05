///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the HTTP core.
//
// CREATED:         08/22/2022
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
#include <string.h>
#include <llhttp.h>
#include <libreidp/priv/core/http.h>

static const int DEFAULT_BACKLOG = 10;
static const char* DEFAULT_LISTEN_ADDRESS = "0.0.0.0";

///////////////////////////////////////////////////////////////////////////////
// Helpers
////

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
// Routing
////

static void idp_http_core_route_request(IdpHttpCore* core,
    IdpHttpContext* context, IdpHttpRequest* request)
{
    IdpHttpResponse* response = idp_http_response_new(IDP_HTTP_404_NOT_FOUND);
    idp_http_context_set_response(context, response, IDP_HTTP_RESPONSE_OWNING);
}

///////////////////////////////////////////////////////////////////////////////
// Connection Handling
////

static void idp_http_core_connection_write(uv_write_t* req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }

    free(req);
}

static int idp_http_core_on_message_complete(llhttp_t* parser) {
    uv_write_t* response_stream = malloc(sizeof(uv_write_t));
    if (NULL == response_stream) {
        fprintf(stderr, "Couldn't allocate memory for write stream\n");
        exit(1);
    }
    memset(response_stream, 0, sizeof(*response_stream));

    uv_stream_t* client = parser->data;
    IdpHttpCore* core = client->data;

    // Route the request to the handler
    IdpHttpContext context = {0};
    idp_http_core_route_request(core, &context, NULL);
    if (NULL == context.response) {
        return HPE_OK;
    }

    // Serialize the response object
    size_t length = idp_http_response_get_string_length(context.response);
    char* response_string = idp_http_response_to_string(context.response);
    if (IDP_HTTP_RESPONSE_OWNING == context.ownership) {
        idp_http_response_free(context.response);
    }

    // Write the response to the client
    uv_buf_t response_buffer = uv_buf_init(response_string, length);
    uv_write(response_stream, client, &response_buffer, 1,
        idp_http_core_connection_write);

    return HPE_OK;
}

static void idp_http_core_allocate_buffer(
    uv_handle_t* handle __attribute__((unused)),
    size_t suggested_size,
    uv_buf_t* buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void idp_http_core_connection_read(uv_stream_t* client, ssize_t nread,
    const uv_buf_t* buf)
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

static void idp_http_core_on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
    IdpHttpCore* core = server->data;
    client->data = core;
    uv_tcp_init(core->loop, client);
    if (0 == uv_accept(server, (uv_stream_t*)client)) {
        uv_read_start((uv_stream_t*)client, idp_http_core_allocate_buffer,
            idp_http_core_connection_read);
    }
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

    // HTTP parser
    llhttp_settings_init(&core->parser_settings);
    core->parser_settings.on_message_complete =
        idp_http_core_on_message_complete;
    llhttp_init(&core->http_parser, HTTP_BOTH, &core->parser_settings);

    // HTTP routes
    static const size_t ROUTES_STEP = 10;
    core->routes_capacity = ROUTES_STEP;
    core->routes = malloc(ROUTES_STEP * sizeof(IdpHttpRoute));
    if (NULL == core->routes) {
        perror("couldn't allocate memory for HTTP routes");
        free(core);
        exit(1);
    }

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
        idp_http_core_on_new_connection);
    if (0 != result) {
        return idp_http_core_result_error_from_libuv(
            IDP_HTTP_CORE_LISTEN_ERROR, result);
    }

    return (IdpHttpCoreResult){.ok = true};
}

///////////////////////////////////////////////////////////////////////////////
// Request
////

IdpHttpRequest* idp_http_request_new() {
    IdpHttpRequest* request = malloc(sizeof(IdpHttpRequest));
    if (NULL == request) {
        fprintf(stderr, "Failed to allocate memory for request!\n");
        exit(1);
    }

    memset(request, 0, sizeof(IdpHttpRequest));
    static const size_t DEFAULT_HEADER_SLOTS = 16;
    request->headers = malloc(DEFAULT_HEADER_SLOTS * sizeof(IdpHttpHeader));
    if (NULL == request->headers) {
        free(request);
        fprintf(stderr, "Failed to allocate memory for request headers!\n");
        exit(1);
    }
    memset(request->headers, 0, DEFAULT_HEADER_SLOTS * sizeof(IdpHttpHeader));

    return request;
}

void idp_http_request_free(IdpHttpRequest* request) {
    free(request->headers);
    free(request);
}

///////////////////////////////////////////////////////////////////////////////
