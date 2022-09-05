///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the HTTP core.
//
// CREATED:         08/22/2022
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
// Message Parsing
////

static int idp_http_core_on_url(llhttp_t* parser, const char* data,
    size_t length)
{}

static int idp_http_core_on_header_field(llhttp_t* parser, const char* data,
    size_t length)
{}

static int idp_http_core_on_header_value(llhttp_t* parser, const char* data,
    size_t length)
{}

static int idp_http_core_on_body(llhttp_t* parser, const char* data,
    size_t length)
{}

///////////////////////////////////////////////////////////////////////////////
// Message Handling
////

static IdpHttpCoreResult idp_http_core_route_request(IdpHttpCore* core,
    IdpHttpContext* context, IdpHttpRequest* request)
{
    const char* path = idp_http_request_get_path(request);
    IdpHttpRequestType request_type = idp_http_request_get_type(request);

    IdpVectorIter iterator = {0};
    IdpHttpRoute* route = NULL;
    idp_vector_iter_init(&iterator, core->routes);
    while (NULL != (route = idp_vector_iter_next(&iterator))) {
        if (!strcmp(path, route->path) && request_type == route->request_type)
        {
            return route->handler(request, context, route->handler_data);
        }
    }

    IdpHttpResponse* response = idp_http_response_new(IDP_HTTP_404_NOT_FOUND);
    idp_http_context_set_response(context, response, IDP_HTTP_RESPONSE_OWNING);
    return (IdpHttpCoreResult){.ok = true};
}

typedef struct IdpHttpRequestCompletionResult {
    bool ok;
    bool empty;
    uv_buf_t buffer;
} IdpHttpRequestCompletionResult;

static IdpHttpRequestCompletionResult idp_http_core_complete_request(
    IdpHttpCore* core, IdpHttpRequest* request)
{
    IdpHttpContext context = {0};
    IdpHttpCoreResult result = idp_http_core_route_request(core, &context,
        request);
    if (!result.ok) {
        fprintf(stderr, "ERROR: %s\n", result.error.message.borrowed);
        if (result.error.owned) {
            free(result.error.message.owned);
        }
        return (IdpHttpRequestCompletionResult){.ok = true, .empty = true};
    } else if (NULL == context.response) {
        return (IdpHttpRequestCompletionResult){.ok = true, .empty = true};
    }

    // Serialize the response object
    size_t length = idp_http_response_get_string_length(context.response);
    char* response_string = idp_http_response_to_string(context.response);
    if (IDP_HTTP_RESPONSE_OWNING == context.ownership) {
        idp_http_response_free(context.response);
    }

    return (IdpHttpRequestCompletionResult){
        .ok = true, .empty = false,
        .buffer = uv_buf_init(response_string, length),
    };
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

    IdpHttpContext* context = parser->data;
    IdpHttpRequestCompletionResult response = idp_http_core_complete_request(
        context->core, context->request);
    if (response.ok && !response.empty) {
        uv_write(response_stream, (uv_stream_t*)&context->client,
            &response.buffer, 1, idp_http_core_connection_write);
    }

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
        IdpHttpContext* context = client->data;
        int result = llhttp_execute(&context->parser, buf->base, nread);
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

    IdpHttpCore* core = server->data;
    IdpHttpContext* context = idp_vector_reserve(core->connections);
    uv_tcp_init(core->loop, &context->client);
    if (0 == uv_accept(server, (uv_stream_t*)&context->client)) {
        llhttp_init(&context->parser, HTTP_BOTH, &core->parser_settings);
        context->parser.data = context;
        context->core = core;
        context->request = idp_http_request_new();
        context->client.data = context;

        uv_read_start((uv_stream_t*)&context->client,
            idp_http_core_allocate_buffer, idp_http_core_connection_read);
    } else {
        idp_vector_remove(core->connections,
            idp_vector_length(core->connections) - 1);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Public API
////

void idp_http_context_free(IdpHttpContext* context) {
    if (IDP_HTTP_RESPONSE_OWNING == context->ownership) {
        idp_http_response_free(context->response);
    }
}

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
    core->parser_settings.on_url = idp_http_core_on_url;
    core->parser_settings.on_header_field = idp_http_core_on_header_field;
    core->parser_settings.on_header_value = idp_http_core_on_header_value;
    core->parser_settings.on_body = idp_http_core_on_body;

    core->routes = idp_vector_new(sizeof(IdpHttpRoute),
        (IdpVectorFreeFn*)idp_http_route_free);
    core->connections = idp_vector_new(sizeof(IdpHttpContext),
        (IdpVectorFreeFn*)idp_http_context_free);
    return core;
}

void idp_http_core_add_port(IdpHttpCore* core, int port) {
    core->port = port;
}

void idp_http_core_shutdown(IdpHttpCore* core) {
    // TODO: Check if uv handle is active here?
    idp_vector_free(core->routes);
    idp_vector_free(core->connections);
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
    request->headers = idp_vector_new(sizeof(IdpHttpHeader),
        (IdpVectorFreeFn*)idp_http_header_free);
    return request;
}

void idp_http_request_free(IdpHttpRequest* request) {
    idp_vector_free(request->headers);
    free(request);
}

///////////////////////////////////////////////////////////////////////////////
