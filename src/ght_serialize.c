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
    w->type = GHT_IO_FILE;
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
    w->type = GHT_IO_MEM;
    *writer = w;
    return GHT_OK;
}

GhtErr
ght_write(GhtWriter *writer, const void *bytes, size_t bytesize)
{
    if ( writer->type == GHT_IO_MEM )
    {
        bytebuffer_append(writer->bytebuffer, bytes, bytesize);
        return GHT_OK;
    }
    else if (writer->type == GHT_IO_FILE )
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

GhtErr
ght_writer_get_size(GhtWriter *writer)
{
}

GhtErr
ght_reader_new_file(const char *filename, const GhtSchema *schema, GhtReader **reader)
{
    GhtReader *r;
    r = ght_malloc(sizeof(GhtReader));
    memset(r, 0,sizeof(GhtReader));
    r->type = GHT_IO_FILE;
    r->filename = ght_strdup(filename);
    r->schema = schema;
    *reader = r;
    return GHT_OK;
}

GhtErr
ght_reader_new_mem(const uint8_t *bytes_start, size_t bytes_size, const GhtSchema *schema, GhtReader **reader)
{
    GhtReader *r;
    r = ght_malloc(sizeof(GhtReader));
    memset(r, 0,sizeof(GhtReader));
    r->type = GHT_IO_MEM;
    r->bytes_start = bytes_start;
    r->bytes_current = bytes_start;
    r->bytes_size = bytes_size;
    r->schema = schema;
    *reader = r;
    return GHT_OK;
}

GhtErr
ght_read(GhtReader *reader, void *bytes, size_t read_size)
{
    if ( reader->type == GHT_IO_MEM )
    {
        if ( reader->bytes_current - reader->bytes_start + read_size > reader->bytes_size )
        {
            ght_error("%s: attempting to read past the end of the byte buffer", __func__);
            return GHT_ERROR;
        }
        memcpy(bytes, reader->bytes_current, read_size);
        reader->bytes_current += read_size;
        return GHT_OK;
    }
    else if (reader->type == GHT_IO_FILE )
    {
        ght_error("%s: file reading currently unimplemented", __func__);
        return GHT_ERROR;
    }
    else
    {
        ght_error("%s: unknown reader type %d", __func__, reader->type);
        return GHT_ERROR;
    }    
}

