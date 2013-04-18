/**********************************************************************
 * bytebuffer.c
 *
 * Copyright 2013 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************/

#include "bytebuffer.h"

/* Use appropriate allocators for this deployment */
//#define malloc ght_malloc
//#define free ght_free
//#define realloc ght_realloc


/**
* Allocate a new bytebuffer_t. Use bytebuffer_destroy to free.
*/
static bytebuffer_t*
bytebuffer_create_with_size(size_t size)
{
    bytebuffer_t *s;

    s = malloc(sizeof(bytebuffer_t));
    s->bytes_start = malloc(size);
    s->bytes_end = s->bytes_start;
    s->capacity = size;
    memset(s->bytes_start,0,size);
    return s;
}

/**
* Allocate a new bytebuffer_t. Use bytebuffer_destroy to free.
*/
bytebuffer_t*
bytebuffer_create(void)
{
    return bytebuffer_create_with_size(BYTEBUFFER_STARTSIZE);
}


/**
* Free the bytebuffer_t and all memory managed within it.
*/
void
bytebuffer_destroy(bytebuffer_t *s)
{
    if ( s->bytes_start ) free(s->bytes_start);
    if ( s ) free(s);
}


/**
* If necessary, expand the bytebuffer_t internal buffer to accomodate the
* specified additional size.
*/
static inline void
bytebuffer_makeroom(bytebuffer_t *s, size_t size_to_add)
{
    size_t current_size = (s->bytes_end - s->bytes_start);
    size_t capacity = s->capacity;
    size_t required_size = current_size + size_to_add;

    while (capacity < required_size)
        capacity *= 2;

    if ( capacity > s->capacity )
    {
        s->bytes_start = realloc(s->bytes_start, capacity);
        s->capacity = capacity;
        s->bytes_end = s->bytes_start + current_size;
    }
}

/**
* Append the specified string to the bytebuffer_t.
*/
void
bytebuffer_append(bytebuffer_t *s, const uint8_t *a, size_t a_size)
{
    bytebuffer_makeroom(s, a_size);
    memcpy(s->bytes_end, a, a_size);
    s->bytes_end += a_size;
}


/**
* Returns a reference to the internal string being managed by
* the stringbuffer. The current string will be null-terminated
* within the internal string.
*/
const uint8_t*
bytebuffer_getbytes(bytebuffer_t *s)
{
    return s->bytes_start;
}

size_t
bytebuffer_getsize(bytebuffer_t *s)
{
    return (s->bytes_end - s->bytes_start);
}

/**
* Returns a newly allocated string large enough to contain the
* current state of the string. Caller is responsible for
* freeing the return value.
*/
uint8_t*
bytebuffer_getbytescopy(bytebuffer_t *s)
{
    size_t size = bytebuffer_getsize(s);
    uint8_t *b = malloc(size);
    memcpy(b, s->bytes_start, size);
    return b;
}


