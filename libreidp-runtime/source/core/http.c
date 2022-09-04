///////////////////////////////////////////////////////////////////////////////
// NAME:            http.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of the public HTTP core API.
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
    for (size_t i = 0; NULL != request->headers[i].name; ++i) {
        if (name == request->headers[i].name) {
            return request->headers[i].value;
        }
    }

    return NULL;
}

const char* idp_http_request_get_body(IdpHttpRequest* request)
{ return request->body; }

size_t idp_http_request_get_body_length(IdpHttpRequest* request)
{ return request->body_length; }

///////////////////////////////////////////////////////////////////////////////
// Response
////

static const size_t HEADER_SLOTS = 16;
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
    response->headers_capacity = HEADER_SLOTS;
    response->headers = malloc(
        response->headers_capacity * sizeof(IdpHttpHeader));
    if (NULL == response->headers) {
        fprintf(stderr, "Couldn't allocate memory for HTTP response!\n");
        free(response);
        exit(1);
    }
    memset(response->headers, 0,
        response->headers_capacity * sizeof(IdpHttpHeader));

    response->code = code;
    idp_http_response_set_header(response, strdup("Server"),
        strdup("libreidp"));
    idp_http_response_set_header(response, strdup("Content-Length"),
        strdup("0"));
    idp_http_response_set_header(response, strdup("Connection"),
        strdup("close"));

    return response;
}

void idp_http_response_free(IdpHttpResponse* response) {
    free(response->body);
    for (size_t i = 0; i < response->headers_length; ++i) {
        free(response->headers[i].name);
        free(response->headers[i].value);
    }
    free(response->headers);
    free(response);
}

void idp_http_response_set_header(IdpHttpResponse* response, char* header,
    char* value)
{
    if (response->headers_length >= response->headers_capacity) {
        const size_t capacity = 2 * response->headers_capacity;
        IdpHttpHeader* headers = realloc(response->headers,
            capacity * sizeof(IdpHttpHeader));
        if (NULL == headers) {
            fprintf(stderr, "Couldn't reallocate HTTP response headers!\n");
            exit(1);
        }

        response->headers = headers;
        response->headers_capacity = capacity;
    }

    size_t this_header = response->headers_length++;
    response->headers[this_header].name = header;
    response->headers[this_header].value = value;
}

void idp_http_response_set_body(IdpHttpResponse* response, char* body,
    size_t length)
{
    response->body = body;
    response->body_length = length;
    for (size_t i = 0; i < response->headers_length; ++i) {
        if (!strcmp(response->headers[i].name, "Content-Length")) {
            free(response->headers[i].value);
            char content_length[64] = {0};
            snprintf(content_length, sizeof(content_length), "%ld", length);
            response->headers[i].value = strdup(content_length);
        }
    }
}

size_t idp_http_response_get_string_length(IdpHttpResponse* response) {
    const char* code_string = idp_http_response_code_to_str(response->code);

    // Summary
    size_t total_length = strlen(HTTP_VERSION) + strlen(" ")
        + strlen(code_string) + strlen("\r\n");

    // Headers
    for (size_t i = 0; i < response->headers_length; ++i) {
        total_length += strlen(response->headers[i].name)
            + strlen(": ") + strlen(response->headers[i].value)
            + strlen("\r\n");
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

    for (size_t i = 0; i < response->headers_length; ++i) {
        strcat(response_string, response->headers[i].name);
        strcat(response_string, ": ");
        strcat(response_string, response->headers[i].value);
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
