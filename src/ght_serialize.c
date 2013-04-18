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


GhtErr
ght_writer_new_file(const char *filename, GhtWriter **writer)
{
    GhtWriter *w;
    w = ght_malloc(sizeof(GhtWriter));
    memset(w, 0,sizeof(GhtWriter));
    w->filename = ght_strdup(filename);
    w->type = GHT_WRITER_FILE;
    *writer = w;
    return GHT_OK;
}

GhtErr
ght_writer_new_mem(GhtWriter **writer)
{
    GhtWriter *w;
    w = ght_malloc(sizeof(GhtWriter));
    memset(w, 0,sizeof(GhtWriter));
    w->bytebuffer = bytebuffer_create();
    w->type = GHT_WRITER_MEM;
    *writer = w;
    return GHT_OK;
}

GhtErr
ght_write(GhtWriter *writer, uint8_t *bytes, size_t bytesize)
{
    if ( writer->type == GHT_WRITER_MEM )
    {
        bytebuffer_append(writer->bytebuffer, bytes, bytesize);
        return GHT_OK;
    }
    else if (writer->type == GHT_WRITER_FILE )
    {
        ght_error("%s: file writing currently unimplemented", __func__);
        return GHT_ERROR;
    }
    else
    {
        ght_error("%s: unknown writer type %d", __func__, writer->type);
        return GHT_ERROR;
    }    
}


