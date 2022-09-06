///////////////////////////////////////////////////////////////////////////////
// NAME:            vector.c
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Implementation of a simple vector.
//
// CREATED:         09/05/2022
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libreidp/vector.h>

typedef struct IdpVector {
    char* data;
    size_t length;
    size_t capacity;
    size_t member_size;
    void (*free_func)(void*);
} IdpVector;

static const size_t DEFAULT_CAPACITY = 16;

///////////////////////////////////////////////////////////////////////////////
// Vector
////

IdpVector* idp_vector_new(size_t member_size, void (*free_func)(void*)) {
    IdpVector* vector = malloc(sizeof(IdpVector));
    if (NULL == vector) {
        perror("couldn't allocate memory for vector!");
        exit(1);
    }

    memset(vector, 0, sizeof(IdpVector));
    vector->capacity = DEFAULT_CAPACITY;
    vector->member_size = member_size;
    vector->free_func = free_func;
    vector->data = malloc(DEFAULT_CAPACITY * member_size);
    if (NULL == vector->data) {
        perror("couldn't allocate memory for vector!");
        free(vector);
        exit(1);
    }

    return vector;
}

void idp_vector_free(IdpVector* vector) {
    if (NULL != vector->free_func) {
        IdpVectorIter iterator = {0};
        idp_vector_iter_init(&iterator, vector);
        void* data = NULL;
        while (NULL != (data = idp_vector_iter_next(&iterator))) {
            vector->free_func(data);
        }
    }

    free(vector->data);
    free(vector);
}

void* idp_vector_reserve(IdpVector* vector) {
    if (vector->length >= vector->capacity) {
        const size_t capacity = vector->capacity * 2;
        void* data = realloc(vector->data, capacity * vector->member_size);
        if (NULL == data) {
            perror("couldn't realloc vector data!");
            exit(1);
        }

        vector->data = data;
        vector->capacity = capacity;
    }

    return idp_vector_get(vector, vector->length++);
}

void idp_vector_remove(IdpVector* vector, size_t index) {
    if (vector->length == 0 || index >= vector->length) {
        return;
    }

    void* old = vector->data + (vector->member_size * index);
    if (NULL != vector->free_func) {
        vector->free_func(old);
    }

    void* new = vector->data + (vector->member_size * vector->length - 1);
    memcpy(old, new, vector->member_size);
    memset(new, 0, vector->member_size);
    --vector->length;
}

size_t idp_vector_length(IdpVector* vector)
{ return vector->length; }

void* idp_vector_get(IdpVector* vector, size_t index)
{
    if (index >= vector->length) {
        return NULL;
    }

    return vector->data + (index * vector->member_size);
}

///////////////////////////////////////////////////////////////////////////////
// Iterator
////

void idp_vector_iter_init(IdpVectorIter* iter, IdpVector* vector) {
    iter->vector = vector;
    iter->index = 0;
}

void* idp_vector_iter_next(IdpVectorIter* iter)
{ return idp_vector_get(iter->vector, iter->index++); }

///////////////////////////////////////////////////////////////////////////////
