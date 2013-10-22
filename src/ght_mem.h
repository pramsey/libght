/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#ifndef _GHT_MEM_H
#define _GHT_MEM_H

/* Global function signatures for memory/logging handlers. */
typedef void* (*GhtAllocator)(size_t size);
typedef void* (*GhtReallocator)(void *mem, size_t size);
typedef void  (*GhtDeallocator)(void *mem);
typedef void  (*GhtMessageHandler)(const char *string, va_list ap);

/** Set the malloc handler */
void   ght_set_allocator(GhtAllocator allocator);

/** Set the free handler */
void   ght_set_deallocator(GhtDeallocator deallocator);

#endif /* _GHT_MEM_H */