/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include "ght_internal.h"
#include "ght_mem.h"

struct GhtContext_t
{
    GhtAllocator alloc;
    GhtReallocator realloc;
    GhtDeallocator free;
    GhtMessageHandler err;
    GhtMessageHandler warn;
    GhtMessageHandler info;
};

static struct GhtContext_t ght_context;


static void *
default_allocator(size_t size)
{
    return malloc(size);
}

static void
default_freeor(void *mem)
{
    free(mem);
}

static char *
default_strdup(const char *str)
{
    return strdup(str);
}

static void *
default_reallocator(void *mem, size_t size)
{
    return realloc(mem, size);
}

static void
default_msg_handler(const char *label, const char *fmt, va_list ap)
{
    char newfmt[1024];
    memset(newfmt, 0, 1024);
    snprintf(newfmt, 1024, "%s%s\n", label, fmt);
    newfmt[1023] = '\0';
    vfprintf(stderr, newfmt, ap);
}

static void
default_info_handler(const char *fmt, va_list ap)
{
    default_msg_handler("INFO: ", fmt, ap);
}

static void
default_warn_handler(const char *fmt, va_list ap)
{
    default_msg_handler("WARNING: ", fmt, ap);
}

static void
default_error_handler(const char *fmt, va_list ap)
{
    default_msg_handler("ERROR: ", fmt, ap);
    va_end(ap);
    exit(1);
}


void ght_init(void)
{
    ght_context.alloc   = default_allocator;
    ght_context.realloc = default_reallocator;
    ght_context.free    = default_freeor;
    ght_context.err     = default_error_handler;
    ght_context.warn    = default_warn_handler;
    ght_context.info    = default_info_handler;

    xmlGcMemSetup(
        (xmlFreeFunc)    ght_free,
        (xmlMallocFunc)  ght_malloc,
        (xmlMallocFunc)  ght_malloc,
        (xmlReallocFunc) ght_realloc,
        (xmlStrdupFunc)  ght_strdup
    );
}


void ght_set_handlers(GhtAllocator allocator, GhtReallocator reallocator,
                      GhtDeallocator deallocator, GhtMessageHandler error_handler,
                      GhtMessageHandler info_handler, GhtMessageHandler warn_handler)
{
    ght_context.alloc = allocator;
    ght_context.realloc = reallocator;
    ght_context.free = deallocator;
    ght_context.err = error_handler;
    ght_context.warn = warn_handler;
    ght_context.info = info_handler;

    xmlGcMemSetup(
        (xmlFreeFunc)    ght_free,
        (xmlMallocFunc)  ght_malloc,
        (xmlMallocFunc)  ght_malloc,
        (xmlReallocFunc) ght_realloc,
        (xmlStrdupFunc)  ght_strdup
    );
}

void ght_set_allocator(GhtAllocator allocator)
{
    ght_context.alloc = allocator;
}

void ght_set_deallocator(GhtDeallocator deallocator)
{
    ght_context.free = deallocator;
}

void *
ght_malloc(size_t size)
{
    void *mem = ght_context.alloc(size);
    if ( ! mem )
    {
        ght_error("%s: unable to allocate %zu bytes", __func__, size);
        return NULL;
    }
    memset(mem, 0, size); /* Always clean memory */
    return mem;
}

char *
ght_strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *newstr = ght_malloc(len);
    if ( ! newstr )
    {
        ght_error("%s: memory allocation failed", __func__);
        return NULL;
    }
    memcpy(newstr, str, len);
    return newstr;
}

void *
ght_realloc(void * mem, size_t size)
{
    void *newmem = ght_context.realloc(mem, size);
    if ( ! newmem )
    {
        ght_error("%s: unable to reallocate %zu bytes", __func__, size);
        return NULL;
    }
    return newmem;
}

void
ght_free(void * mem)
{
    ght_context.free(mem);
}

void
ght_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (*ght_context.err)(fmt, ap);
    va_end(ap);
}

void
ght_info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (*ght_context.info)(fmt, ap);
    va_end(ap);
}

void
ght_warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (*ght_context.warn)(fmt, ap);
    va_end(ap);
}



