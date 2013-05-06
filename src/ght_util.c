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

int fexists(const char *filename); /* ght_util.c */
char machine_endian(void); /* ght_util.c */

/* Our static character->number map. Anything > 15 is invalid */
static uint8_t hex2char[256] = {
	/* not Hex characters */
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	/* 0-9 */
	0,1,2,3,4,5,6,7,8,9,20,20,20,20,20,20,
	/* A-F */
	20,10,11,12,13,14,15,20,20,20,20,20,20,20,20,20,
	/* not Hex characters */
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	/* a-f */
	20,10,11,12,13,14,15,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	/* not Hex characters (upper 128 characters) */
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20
};


GhtErr
bytes_from_hexbytes(const char *hexbuf, size_t hexsize, uint8_t **bytes)
{
	uint8_t *buf = NULL;
	register uint8_t h1, h2;
	int i;

	if( hexsize % 2 )
	{
		ght_error("Invalid hex string, length (%d) has to be a multiple of two!", hexsize);
        return GHT_ERROR;
	}

	buf = ght_malloc(hexsize/2);

	if( ! buf )
	{
		ght_error("Unable to allocate memory buffer.");
		return GHT_ERROR;
    }

	for( i = 0; i < hexsize/2; i++ )
	{
		h1 = hex2char[(int)hexbuf[2*i]];
		h2 = hex2char[(int)hexbuf[2*i+1]];
		if( h1 > 15 )
		{
			ght_error("Invalid hex character (%c) encountered", hexbuf[2*i]);
			return GHT_ERROR;
        }
		if( h2 > 15 )
		{
			ght_error("Invalid hex character (%c) encountered", hexbuf[2*i+1]);
			return GHT_ERROR;
        }
		/* First character is high bits, second is low bits */
		buf[i] = ((h1 & 0x0F) << 4) | (h2 & 0x0F);
	}
    *bytes = buf;
    return GHT_OK;
}

GhtErr
hexbytes_from_bytes(const uint8_t *bytebuf, size_t bytesize, char **hexbytes)
{
	char *buf = ght_malloc(2*bytesize + 1); /* 2 chars per byte + null terminator */
	int i;
	char *ptr = buf;

	for ( i = 0; i < bytesize; i++ )
	{
		int incr = snprintf(ptr, 3, "%02X", bytebuf[i]);
		if ( incr < 0 )
		{
			ght_error("write failure in hexbytes_from_bytes");
			return GHT_ERROR;
		}
		ptr += incr;
	}

    *hexbytes = buf;
    return GHT_OK;
}

int
fexists(const char *filename)
{
    FILE *file;
    if ( file = fopen(filename, "r") )
    {
        fclose(file);
        return 1;
    }
    return 0;
}

char
machine_endian(void)
{
        static int check_int = 1; /* dont modify this!!! */
        return *((char *) &check_int); /* 0 = big endian | xdr, */
                                       /* 1 = little endian | ndr */
}

int 
ght_version_major(void)
{
    return GHT_VERSION_MAJOR;
}

int 
ght_version_minor(void)
{
    return GHT_VERSION_MINOR;
}

int 
ght_version_patch(void)
{
    return GHT_VERSION_PATCH;
}

char *
ght_version(void)
{
    char buf[32];
    snprintf(buf, 32, "%d.%d.%d", GHT_VERSION_MAJOR, GHT_VERSION_MINOR, GHT_VERSION_PATCH);
    return ght_strdup(buf);
}

