///////////////////////////////////////////////////////////////////////////////
// NAME:            vector.h
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     A simple variable length container.
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

#ifndef IDP_VECTOR_H
#define IDP_VECTOR_H

typedef struct IdpVector IdpVector;
typedef void IdpVectorFreeFn(void*);

IdpVector* idp_vector_new(size_t member_size, IdpVectorFreeFn* free_func);
void idp_vector_free(IdpVector* vector);

void* idp_vector_reserve(IdpVector* vector);
void idp_vector_remove(IdpVector* vector, size_t index);
size_t idp_vector_length(IdpVector* vector);
void* idp_vector_get(IdpVector* vector, size_t index);

typedef struct IdpVectorIter {
    IdpVector* vector;
    size_t index;
} IdpVectorIter;

void idp_vector_iter_init(IdpVectorIter* iter, IdpVector* vector);
void* idp_vector_iter_next(IdpVectorIter* iter);

#endif // IDP_VECTOR_H

///////////////////////////////////////////////////////////////////////////////
