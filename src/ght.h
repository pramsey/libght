/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#ifndef _GHT_H
#define _GHT_H

#include "libxml/xmlmemory.h"
#include "ght_config.h"

/* Up to double/int64 */
#define GHT_ATTRIBUTE_MAX_SIZE  8
#define GHT_MAX_HASH_LENGTH    18


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

typedef enum
{   
    GHT_DUPES_NO = 0,
    GHT_DUPES_YES = 1
} GhtDuplicates;

/* So we can alias char* to GhtHash* */
typedef char GhtHash;

typedef struct
{
    int position;
    char *name;
    char *description;
    GhtType type;
    double scale;
    double offset;
} GhtDimension;

typedef struct
{
    int num_dims;
    int max_dims;
    GhtDimension **dims;
} GhtSchema;

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

typedef struct GhtAttribute_t
{
    const GhtDimension *dim;
    struct GhtAttribute_t *next;
    char val[GHT_ATTRIBUTE_MAX_SIZE];
} GhtAttribute;

typedef struct
{
    double min;
    double max;
    double sum;
    int count;
    GhtType type;
    const GhtDimension *dim;
} GhtAttributeStats;

struct GhtNodeList_t;

typedef struct
{
    GhtHash *hash;
    struct GhtNodeList_t *children;
    GhtAttribute *attributes;
} GhtNode;

typedef struct GhtNodeList_t
{
    int num_nodes;
    int max_nodes;
    GhtNode **nodes;
} GhtNodeList;

typedef struct
{
    GhtDuplicates  allow_duplicates;
    unsigned char  max_hash_length;
} GhtConfig;

typedef struct
{
    const GhtSchema *schema;
    GhtNode *root;
    int num_nodes;
    GhtConfig config;
} GhtTree;



typedef GhtHash* GhtHashPtr;
typedef GhtCoordinate* GhtCoordinatePtr;
typedef GhtArea* GhtAreaPtr;
typedef GhtNode* GhtNodePtr;
typedef GhtDimension* GhtDimensionPtr;
typedef GhtSchema* GhtSchemaPtr;


/* Global function signatures for memory/logging handlers. */
typedef void* (*GhtAllocator)(size_t size);
typedef void* (*GhtReallocator)(void *mem, size_t size);
typedef void  (*GhtDeallocator)(void *mem);
typedef void  (*GhtMessageHandler)(const char *string, va_list ap);


/** Initialize memory/message handling with defaults (malloc/free/printf) */
void   ght_init(void);


GhtErr ght_schema_from_xml_str(const char *xmlstr, GhtSchema **schema);
GhtErr ght_schema_to_xml_str(const GhtSchema *schema, char **xml_str, size_t *xml_str_size);
GhtErr ght_schema_free(GhtSchema *schema);


#endif