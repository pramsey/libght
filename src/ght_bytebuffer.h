/**********************************************************************
 * bytebuffer.h
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

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H 1

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BYTEBUFFER_STARTSIZE 1028

typedef struct
{
    size_t capacity;
    uint8_t *bytes_end;
    uint8_t *bytes_start;
}
bytebuffer_t;

extern bytebuffer_t *bytebuffer_create(void);
extern void bytebuffer_destroy(bytebuffer_t *bb);
extern void bytebuffer_append(bytebuffer_t *bb, const uint8_t *buffer, size_t buffer_size);
extern size_t bytebuffer_getsize(bytebuffer_t *s);
extern const uint8_t *bytebuffer_getbytes(bytebuffer_t *bb);
extern uint8_t *bytebuffer_getbytescopy(bytebuffer_t *bb);
extern size_t bytebuffer_getsize(bytebuffer_t *bb);

#include "ght_internal.h"

#endif /* _BYTEBUFFER_H */