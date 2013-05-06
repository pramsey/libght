/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#define GHT_MAX_HASH_LENGTH    18
#define GHT_FORMAT_VERSION      1


/***********************************************************************
* Common enumerations and structures found in both the public   
* and private header files.
************************************************************************/

typedef enum
{
    GHT_OK = 0,
    GHT_ERROR = 1,
    GHT_WARNING = 2,
    GHT_DONE = (1<<8),
    GHT_INCOMPLETE = (2<<8)
} GhtErr;

typedef enum
{
    GHT_UNKNOWN = 0,
    GHT_INT8    = 1,  GHT_UINT8  = 2,
    GHT_INT16   = 3,  GHT_UINT16 = 4,
    GHT_INT32   = 5,  GHT_UINT32 = 6,
    GHT_INT64   = 7,  GHT_UINT64 = 8,
    GHT_DOUBLE  = 9,  GHT_FLOAT  = 10
} GhtType;

#define GHT_TRY(functioncall) { if ( (functioncall) == GHT_ERROR ) { return GHT_ERROR; } }

typedef struct
{
    double x;
    double y;
} GhtCoordinate;

typedef struct
{
    double min;
    double max;
} GhtRange;

typedef struct
{
    GhtRange x;
    GhtRange y;
} GhtArea;

typedef struct
{
    unsigned char  allow_duplicates;
    unsigned char  max_hash_length;
    unsigned char  version;
    unsigned char  endian;
} GhtConfig;

/* So we can alias char* to GhtHash* */
typedef char GhtHash;

/* Access version information */
int ght_version_major(void);
int ght_version_minor(void);
int ght_version_patch(void);
char * ght_version(void);

