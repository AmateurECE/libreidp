///////////////////////////////////////////////////////////////////////////////
// NAME:            http.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     HTTP core--responsible for providing extensible HTTP
//                  routing and handling services to plugins.
//
// CREATED:         08/22/2022
//
// LAST EDITED:     09/09/2022
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

#ifndef IDP_HTTP_H
#define IDP_HTTP_H

#include <stdbool.h>
#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////
// Configuration
////

// Configuration for the HTTP core
typedef struct IdpHttpCoreConfig {
    int default_port;
} IdpHttpCoreConfig;

///////////////////////////////////////////////////////////////////////////////
// Query Parameters
////

// A single query parameter
typedef struct IdpHttpParam {
    const char* name;
    size_t name_length;

    const char* value;
    size_t value_length;
} IdpHttpParam;

// Iterator over borrowed string
typedef struct IdpHttpParamsIter {
    const char* string;
    const char* index;

    IdpHttpParam param;
} IdpHttpParamsIter;

void idp_http_params_iter_init(IdpHttpParamsIter* iter, const char* string);
IdpHttpParam* idp_http_params_iter_next(IdpHttpParamsIter* iter);

///////////////////////////////////////////////////////////////////////////////
// Request
////

typedef enum IdpHttpRequestType {
    IDP_HTTP_GET,
    IDP_HTTP_POST,
} IdpHttpRequestType;

typedef struct IdpHttpRequest IdpHttpRequest;

IdpHttpRequestType idp_http_request_get_type(IdpHttpRequest* request);
const char* idp_http_request_get_path(IdpHttpRequest* request);
const char* idp_http_request_get_header(IdpHttpRequest* request,
                                        const char* name);
const char* idp_http_request_get_body(IdpHttpRequest* request);
size_t idp_http_request_get_body_length(IdpHttpRequest* request);

///////////////////////////////////////////////////////////////////////////////
// Response
////

typedef enum IdpHttpResponseCode {
    IDP_HTTP_200_OK,
    IDP_HTTP_404_NOT_FOUND,
} IdpHttpResponseCode;

const char* idp_http_response_code_to_str(IdpHttpResponseCode code);

typedef struct IdpHttpResponse IdpHttpResponse;

IdpHttpResponse* idp_http_response_new(IdpHttpResponseCode code);
void idp_http_response_free(IdpHttpResponse* response);
void idp_http_response_set_header(IdpHttpResponse* response, char* header,
                                  char* value);
void idp_http_response_set_body(IdpHttpResponse* response, char* body,
                                size_t length);
size_t idp_http_response_get_string_length(IdpHttpResponse* response);
char* idp_http_response_to_string(IdpHttpResponse* response);

///////////////////////////////////////////////////////////////////////////////
// Context
////

// This object holds state for each request while the request is executing.
typedef struct IdpHttpContext IdpHttpContext;

typedef enum IdpHttpResponseOwnership {
    IDP_HTTP_RESPONSE_OWNING,
    IDP_HTTP_RESPONSE_BORROWING,
} IdpHttpResponseOwnership;

// Tell the HTTP core whether it's responsible for free(3)'ing the response.
void idp_http_context_set_response(IdpHttpContext* context,
                                   IdpHttpResponse* response,
                                   IdpHttpResponseOwnership ownership);

///////////////////////////////////////////////////////////////////////////////
// Core Executor
////

// The HTTP core executor
typedef struct IdpHttpCore IdpHttpCore;

typedef enum IdpHttpCoreError {
    IDP_HTTP_CORE_OK,
    IDP_HTTP_CORE_PATH_EXISTS,
    IDP_HTTP_CORE_LISTEN_ERROR,
} IdpHttpCoreError;

typedef struct IdpHttpCoreResult {
    bool ok;
    union {
        struct {
            IdpHttpCoreError error;
            bool owned;
            union {
                const char* borrowed;
                char* owned;
            } message;
        } error;
    };
} IdpHttpCoreResult;

typedef IdpHttpCoreResult IdpHttpHandlerCallback(IdpHttpRequest* request,
                                                 IdpHttpContext* context,
                                                 void* handler_state);

IdpHttpCoreResult idp_http_core_add_route(IdpHttpCore* core,
                                          IdpHttpRequestType request_type,
                                          char* path,
                                          IdpHttpHandlerCallback* handler,
                                          void* handler_state);

#endif // IDP_HTTP_H

///////////////////////////////////////////////////////////////////////////////
