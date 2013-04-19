/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*  Copyright (c) 2011 Lyo Kato <lyo.kato@gmail.com>
*
******************************************************************************/

#include "ght_internal.h"

#define MAX_HASH_LENGTH 22

#define REFINE_RANGE(range, bits, offset) \
    if (((bits) & (offset)) == (offset)) \
        (range)->min = ((range)->max + (range)->min) / 2.0; \
    else \
        (range)->max = ((range)->max + (range)->min) / 2.0;

#define SET_BIT(bits, mid, range, value, offset) \
    mid = ((range)->max + (range)->min) / 2.0; \
    if ((value) >= mid) { \
        (range)->min = mid; \
        (bits) |= (0x1 << (offset)); \
    } else { \
        (range)->max = mid; \
        (bits) |= (0x0 << (offset)); \
    }

static const char BASE32_ENCODE_TABLE[33] = "0123456789bcdefghjkmnpqrstuvwxyz";
static const char BASE32_DECODE_TABLE[44] =
{
    /* 0 */   0, /* 1 */   1, /* 2 */   2, /* 3 */   3, /* 4 */   4,
    /* 5 */   5, /* 6 */   6, /* 7 */   7, /* 8 */   8, /* 9 */   9,
    /* : */  -1, /* ; */  -1, /* < */  -1, /* = */  -1, /* > */  -1,
    /* ? */  -1, /* @ */  -1, /* A */  -1, /* B */  10, /* C */  11,
    /* D */  12, /* E */  13, /* F */  14, /* G */  15, /* H */  16,
    /* I */  -1, /* J */  17, /* K */  18, /* L */  -1, /* M */  19,
    /* N */  20, /* O */  -1, /* P */  21, /* Q */  22, /* R */  23,
    /* S */  24, /* T */  25, /* U */  26, /* V */  27, /* W */  28,
    /* X */  29, /* Y */  30, /* Z */  31
};

static const char NEIGHBORS_TABLE[8][33] =
{
    "p0r21436x8zb9dcf5h7kjnmqesgutwvy", /* NORTH EVEN */
    "bc01fg45238967deuvhjyznpkmstqrwx", /* NORTH ODD  */
    "bc01fg45238967deuvhjyznpkmstqrwx", /* EAST EVEN  */
    "p0r21436x8zb9dcf5h7kjnmqesgutwvy", /* EAST ODD   */
    "238967debc01fg45kmstqrwxuvhjyznp", /* WEST EVEN  */
    "14365h7k9dcfesgujnmqp0r2twvyx8zb", /* WEST ODD   */
    "14365h7k9dcfesgujnmqp0r2twvyx8zb", /* SOUTH EVEN */
    "238967debc01fg45kmstqrwxuvhjyznp"  /* SOUTH ODD  */
};

static const char BORDERS_TABLE[8][9] =
{
    "prxz",     /* NORTH EVEN */
    "bcfguvyz", /* NORTH ODD */
    "bcfguvyz", /* EAST  EVEN */
    "prxz",     /* EAST  ODD */
    "0145hjnp", /* WEST  EVEN */
    "028b",     /* WEST  ODD */
    "028b",     /* SOUTH EVEN */
    "0145hjnp"  /* SOUTH ODD */
};

GhtErr
ght_hash_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtHash **hash)
{
    int i;
    GhtHash *geohash;
    unsigned char bits = 0;
    double lon = coord->x;
    double lat = coord->y;
    double mid;
    GhtRange lat_range = {  -90,  90 };
    GhtRange lon_range = { -180, 180 };

    double val1, val2, val_tmp;
    GhtRange *range1, *range2, *range_tmp;

    assert(lat >= -90.0);
    assert(lat <= 90.0);
    assert(lon >= -180.0);
    assert(lon <= 180.0);
    assert(resolution <= MAX_HASH_LENGTH);

    geohash = (GhtHash*)ght_malloc(sizeof(GhtHash)*resolution+1);

    if (geohash == NULL)
        return GHT_ERROR;

    val1 = lon;
    range1 = &lon_range;
    val2 = lat;
    range2 = &lat_range;

    for (i=0; i < resolution; i++)
    {
        bits = 0;
        SET_BIT(bits, mid, range1, val1, 4);
        SET_BIT(bits, mid, range2, val2, 3);
        SET_BIT(bits, mid, range1, val1, 2);
        SET_BIT(bits, mid, range2, val2, 1);
        SET_BIT(bits, mid, range1, val1, 0);

        geohash[i] = BASE32_ENCODE_TABLE[bits];

        val_tmp   = val1;
        val1      = val2;
        val2      = val_tmp;
        range_tmp = range1;
        range1    = range2;
        range2    = range_tmp;
    }

    geohash[resolution] = '\0';
    *hash = geohash;

    return GHT_OK;
}


GhtErr
ght_area_from_hash(const GhtHash *hash, GhtArea *area)
{

    const char *p;
    unsigned char c;
    char bits;
    GhtRange *range1, *range2, *range_tmp;

    area->y.max =   90; /* latitude max */
    area->y.min =  -90; /* latitude min */
    area->x.max =  180; /* longitude max */
    area->x.min = -180; /* longitude min */

    range1 = &(area->x);
    range2 = &(area->y);

    p = (const char*)hash;

    while (*p != '\0')
    {
        c = toupper(*p++);
        if (c < 0x30)
        {
            return GHT_ERROR;
        }
        c -= 0x30;
        if (c > 43)
        {
            return GHT_ERROR;
        }
        bits = BASE32_DECODE_TABLE[c];
        if (bits == -1)
        {
            return GHT_ERROR;
        }

        REFINE_RANGE(range1, bits, 0x10);
        REFINE_RANGE(range2, bits, 0x08);
        REFINE_RANGE(range1, bits, 0x04);
        REFINE_RANGE(range2, bits, 0x02);
        REFINE_RANGE(range1, bits, 0x01);

        range_tmp = range1;
        range1    = range2;
        range2    = range_tmp;
    }
    return GHT_OK;
}

int
ght_hash_common_length(const GhtHash *a, const GhtHash *b, int max_len)
{
    int min_len;
    int index = 0;
    int a_len = strlen(a);
    int b_len = strlen(b);

    /* One of the arguments is the "magic empty hash" */
    if ( a_len == 0 || b_len == 0 )
        return 0;

    /* First chars differ! */
    if ( *a != *b )
        return -1;

    min_len = (a_len < b_len) ? a_len : b_len;

    if ( min_len < max_len )
        max_len = min_len;


    while(index < max_len)
    {
        if ( *a != *b )
            return index;

        a++;
        b++;
        index++;
    }
    return index;
}

/**
* Takes in a potential parent hash (a), and a potential child (b).
* Returns pointers to the start of the sub-hashes once the
* common parts are stripped.
* Match types are:
*   GHT_NONE, GHT_GLOBAL, GHT_SAME, GHT_CHILD, GHT_SPLIT
*/
GhtErr
ght_hash_leaf_parts(const GhtHash *a, const GhtHash *b, int maxlen,
                    GhtHashMatch *matchtype, GhtHash **a_leaf, GhtHash **b_leaf)
{
    const GhtHash *a_start = a;
    const GhtHash *b_start = b;

    /* Initialize return values */
    *a_leaf = (GhtHash*)a;
    *b_leaf = (GhtHash*)b;

    while( *a && *b && *a == *b && (a - a_start) < maxlen )
    {
        a++;
        b++;
    }

    /* Still on the first character... */
    if ( a == a_start )
    {
        /* First character is null terminator? */
        /* First has is the empty string! The "" hash. */
        if ( *a == 0 )
        {
            *matchtype = GHT_GLOBAL;
            return GHT_OK;
        }
        /* First character, but different. */
        /* The hashes don't match at all! */
        if ( *b == 0 || (*a && *b) )
        {
            *matchtype = GHT_NONE;
            return GHT_ERROR;
        }
    }
    /* Past the first character... */
    else
    {
        /* Both null terminators, so hashes are identical. */
        /* "abcdefg" & "abcdefg" */
        if ( *a == 0 && *b == 0 )
        {
            *matchtype = GHT_SAME;
            return GHT_OK;
        }
        /* A got to null terminator, but B didn't, so B is longer. */
        /* "abcd" & "abcdefg" become "abcd"->["efg"] */
        else if ( *a == 0 )
        {
            *b_leaf = (GhtHash*)b;
            *matchtype = GHT_CHILD;
            return GHT_OK;
        }
        /* B got to null terminator but A didn't? This shouldn't happen. */
        /* "abcdefg" & "abcd" */
        else if ( *b == 0 )
        {
            *matchtype = GHT_NONE;
            return GHT_ERROR;
        }
        /* A and B are both inside their respective hashes, we need to split */
        /* the parent hash up. */
        /* "abdefg" & "abdpqrs" become "abd"->["efg","pqrs"] */
        else
        {
            *a_leaf = (GhtHash*)a;
            *b_leaf = (GhtHash*)b;
            *matchtype = GHT_SPLIT;
            return GHT_OK;
        }
    }
    return GHT_ERROR;
}

GhtErr
ght_hash_free(GhtHash *hash)
{
    assert(hash != NULL);
    ght_free(hash);
}

GhtErr 
ght_hash_write(const GhtHash *hash, GhtWriter *writer)
{
    uint8_t hashlen = 0;

    /* Only try to read length if there's something there */
    if ( hash )
        hashlen = strlen(hash);
    
    /* Write the length. Write 0 if there's no hash, so we know it's missing */
    ght_write(writer, &hashlen, 1);
    
    /* Write the hash, omit the null terminator */
    if ( hashlen )
        ght_write(writer, hash, hashlen);
    
    return GHT_OK;    
}

GhtErr 
ght_hash_read(GhtReader *reader, GhtHash **hash)
{
    uint8_t hashlen;
    GhtHash *h = NULL;
    
    /* Anything there? */
    ght_read(reader, &hashlen, 1);
    
    /*  Yep, read it. */
    if ( hashlen )
    {
        h = ght_malloc(hashlen+1);
        ght_read(reader, h, hashlen);
        h[hashlen] = '\0';
    }
    
    *hash = h;
    return GHT_OK;
}
