///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the public HTTP core API.
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

#include <stdlib.h>
#include <string.h>
#include <libreidp/priv/core/http.h>

///////////////////////////////////////////////////////////////////////////////
// Request
////

const char* idp_http_request_get_path(IdpHttpRequest* request)
{ return request->path; }

const char* idp_http_request_get_header(IdpHttpRequest* request,
    const char* name)
{
    IdpVectorIter iterator = {0};
    IdpHttpHeader* header = NULL;
    idp_vector_iter_init(&iterator, request->headers);
    while (NULL != (header = idp_vector_iter_next(&iterator))) {
        if (name == header->name) {
            return header->value;
        }
    }

    return NULL;
}

IdpHttpRequestType idp_http_request_get_type(IdpHttpRequest* request)
{ return request->request_type; }

const char* idp_http_request_get_body(IdpHttpRequest* request)
{ return request->body; }

size_t idp_http_request_get_body_length(IdpHttpRequest* request)
{ return request->body_length; }

///////////////////////////////////////////////////////////////////////////////
// Response
////

static const char* HTTP_VERSION = "HTTP/1.1";

const char* idp_http_response_code_to_str(IdpHttpResponseCode code) {
    switch (code) {
    case IDP_HTTP_200_OK: return "200 OK";
    case IDP_HTTP_404_NOT_FOUND: return "404 Not Found";
    default: return "unknown";
    }
}

IdpHttpResponse* idp_http_response_new(IdpHttpResponseCode code) {
    IdpHttpResponse* response = malloc(sizeof(IdpHttpResponse));
    if (NULL == response) {
        fprintf(stderr, "Couldn't allocate memory for HTTP response!\n");
        exit(1);
    }

    memset(response, 0, sizeof(IdpHttpResponse));
    response->headers = idp_vector_new(sizeof(IdpHttpHeader),
        (IdpVectorFreeFn*)idp_http_header_free);

    response->code = code;
    idp_http_response_set_header(response, strdup("Server"),
        strdup("libreidp"));
    idp_http_response_set_header(response, strdup("Content-Length"),
        strdup("0"));
    idp_http_response_set_header(response, strdup("Connection"),
        strdup("close"));

    return response;
}

void idp_http_header_free(IdpHttpHeader* header) {
    free(header->name);
    free(header->value);
}

void idp_http_response_free(IdpHttpResponse* response) {
    free(response->body);
    idp_vector_free(response->headers);
    free(response);
}

void idp_http_response_set_header(IdpHttpResponse* response, char* name,
    char* value)
{
    IdpHttpHeader* header = idp_vector_reserve(response->headers);
    header->name = name;
    header->value = value;
}

void idp_http_response_set_body(IdpHttpResponse* response, char* body,
    size_t length)
{
    response->body = body;
    response->body_length = length;

    IdpVectorIter iterator = {0};
    IdpHttpHeader* header = NULL;
    idp_vector_iter_init(&iterator, response->headers);
    while (NULL != (header = idp_vector_iter_next(&iterator))) {
        if (!strcmp(header->name, "Content-Length")) {
            free(header->value);
            char content_length[64] = {0};
            snprintf(content_length, sizeof(content_length), "%ld", length);
            header->value = strdup(content_length);
        }
    }
}

size_t idp_http_response_get_string_length(IdpHttpResponse* response) {
    const char* code_string = idp_http_response_code_to_str(response->code);

    // Summary
    size_t total_length = strlen(HTTP_VERSION) + strlen(" ")
        + strlen(code_string) + strlen("\r\n");

    // Headers
    IdpVectorIter iterator = {0};
    IdpHttpHeader* header = NULL;
    idp_vector_iter_init(&iterator, response->headers);
    while (NULL != (header = idp_vector_iter_next(&iterator))) {
        total_length += strlen(header->name) + strlen(": ")
            + strlen(header->value) + strlen("\r\n");
    }

    // Body
    total_length += strlen("\r\n");
    if (0 != response->body_length) {
        total_length += response->body_length + strlen("\r\n");
    }

    return total_length;
}

char* idp_http_response_to_string(IdpHttpResponse* response) {
    const size_t string_length = idp_http_response_get_string_length(response);
    char* response_string = malloc(string_length + 1);
    memset(response_string, 0, string_length + 1);

    strcat(response_string, HTTP_VERSION);
    strcat(response_string, " ");
    strcat(response_string, idp_http_response_code_to_str(response->code));
    strcat(response_string, "\r\n");

    IdpVectorIter iterator = {0};
    IdpHttpHeader* header = NULL;
    idp_vector_iter_init(&iterator, response->headers);
    while (NULL != (header = idp_vector_iter_next(&iterator))) {
        strcat(response_string, header->name);
        strcat(response_string, ": ");
        strcat(response_string, header->value);
        strcat(response_string, "\r\n");
    }

    strcat(response_string, "\r\n");
    if (0 != response->body_length) {
        const size_t header_length = string_length - response->body_length
            - strlen("\r\n");
        memcpy(response_string + header_length, response->body,
            response->body_length);
        memcpy(response_string + header_length + response->body_length, "\r\n",
            strlen("\r\n"));
    }
    return response_string;
}

///////////////////////////////////////////////////////////////////////////////
// Context
////

// Tell the HTTP core whether it's responsible for free(3)'ing the response.
void idp_http_context_set_response(IdpHttpContext* context,
    IdpHttpResponse* response, IdpHttpResponseOwnership ownership)
{
    context->response = response;
    context->ownership = ownership;
}

///////////////////////////////////////////////////////////////////////////////
// Core
////

IdpHttpCoreResult idp_http_core_add_route(IdpHttpCore* core,
    IdpHttpRequestType request_type, char* path,
    IdpHttpHandlerCallback* handler, void* handler_state)
{
    IdpHttpRoute* route = idp_vector_reserve(core->routes);
    route->request_type = request_type;
    route->path = path;
    route->handler = handler;
    route->handler_data = handler_state;
    return (IdpHttpCoreResult){.ok = true};
}

///////////////////////////////////////////////////////////////////////////////
