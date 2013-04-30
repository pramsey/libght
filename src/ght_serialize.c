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

/** Supplement to c file functions, from ght_util.c */
int fexists(const char *filename);


GhtErr
ght_writer_new_file(const char *filename, GhtWriter **writer)
{
    GhtWriter *w;
    FILE *file;
    
    if ( ! filename )
    {
        ght_error("%s: null filename provided", __func__);
        return GHT_ERROR;
    }
    
    if ( fexists(filename) )
    {
        ght_error("%s: output file %s already exists", __func__, filename);
        return GHT_ERROR;
    }
    
    file = fopen(filename, "w");
    
    if ( ! file )
    {
        ght_error("%s: unable to open file %s for writing", __func__, filename);
        return GHT_ERROR;
    }
    
    w = ght_malloc(sizeof(GhtWriter));
    memset(w, 0,sizeof(GhtWriter));
    w->file = file;
    w->filename = ght_strdup(filename);
    w->filesize = 0;
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
ght_writer_free(GhtWriter *writer)
{
    if ( ! writer ) return GHT_ERROR;
    if ( writer->type == GHT_IO_MEM )
    {
        bytebuffer_destroy(writer->bytebuffer);
    }
    else if ( writer->type == GHT_IO_FILE )
    {
        if ( writer->file )
            fclose(writer->file);
        if ( writer->filename )
            ght_free(writer->filename);
    }

    ght_free(writer);
    return GHT_OK;
}

GhtErr
ght_write(GhtWriter *writer, const void *bytes, size_t bytesize)
{
    assert(writer);
    if ( writer->type == GHT_IO_MEM )
    {
        bytebuffer_append(writer->bytebuffer, bytes, bytesize);
        return GHT_OK;
    }
    else if (writer->type == GHT_IO_FILE )
    {
        size_t wsz;
        wsz = fwrite(bytes, 1, bytesize, writer->file);
        if ( wsz != bytesize )
            return GHT_ERROR;
        writer->filesize += wsz;
        return GHT_OK;
    }
    else
    {
        ght_error("%s: unknown writer type %d", __func__, writer->type);
        return GHT_ERROR;
    }    
}

GhtErr
ght_writer_get_size(GhtWriter *writer, size_t *size)
{
    if ( writer->type == GHT_IO_MEM )
    {
        *size = bytebuffer_getsize(writer->bytebuffer);
    }
    else if ( writer->type == GHT_IO_FILE )
    {
        *size = writer->filesize;
    }
    else
    {
        return GHT_ERROR;
    }
    return GHT_OK;   
}

GhtErr
ght_writer_get_bytes(GhtWriter *writer, uint8_t *bytes)
{
    if ( writer->type == GHT_IO_MEM )
    {
        memcpy(bytes, bytebuffer_getbytes(writer->bytebuffer), bytebuffer_getsize(writer->bytebuffer));
        return GHT_OK;   
    }
    else
    {
        return GHT_ERROR;
    }
}

GhtErr
ght_reader_new_file(const char *filename, const GhtSchema *schema, GhtReader **reader)
{
    GhtReader *r;
    FILE *file;

    if ( ! filename )
    {
        ght_error("%s: null filename provided", __func__);
        return GHT_ERROR;
    }
    
    if ( ! fexists(filename) )
    {
        ght_error("%s: file %s does not exist", __func__, filename);
        return GHT_ERROR;
    }
    
    file = fopen(filename, "r");
    
    if ( ! file )
    {
        ght_error("%s: unable to open file %s for reading", __func__, filename);
        return GHT_ERROR;
    }
    
    r = ght_malloc(sizeof(GhtReader));
    memset(r, 0,sizeof(GhtReader));

    r->file = file;
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
ght_reader_free(GhtReader *reader)
{
    if ( reader->type == GHT_IO_FILE )
    {
        if ( reader->file )
            fclose(reader->file);
        if ( reader->filename )
            ght_free(reader->filename);
    }
    ght_free(reader);
    return GHT_OK;    
}

GhtErr
ght_read(GhtReader *reader, void *bytes, size_t read_size)
{
    assert(reader);
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
        size_t rsz;
        rsz = fread(bytes, 1, read_size, reader->file);
        if ( rsz != read_size )
        {
            if ( feof(reader->file) )
            {
                ght_warn("%s: reader reached end of file prematurely", __func__);
                return GHT_OK;
            }
            else if ( ferror(reader->file) )
            {
                ght_error("%s: reader error", __func__);
                return GHT_ERROR;
            }
        }
        
        return GHT_OK;
    }
    else
    {
        ght_error("%s: unknown reader type %d", __func__, reader->type);
        return GHT_ERROR;
    }    
}

