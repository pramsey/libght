/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
* 
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#ifndef _GHT_INTERNAL_H
#define _GHT_INTERNAL_H

#include "ght.h"
#include "stringbuffer.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define GHT_TRY(functioncall) { if ( (functioncall) == GHT_ERROR ) return GHT_ERROR; }

typedef enum  
{
    GHT_NONE,
    GHT_GLOBAL,
    GHT_SAME,
    GHT_CHILD,
    GHT_SPLIT
} GhtHashMatch;

typedef struct
{
    int num_pages;
    int max_pages;
    int num_elements;
    int elements_per_page;
    size_t element_size;
    unsigned char **pages;
} GhtMemoryPool;



static char *GhtTypeStrings[] =
{
    "unknown",
    "int8_t",  "uint8_t",
    "int16_t", "uint16_t",
    "int32_t", "uint32_t",
    "int64_t", "uint64_t",
    "double",  "float"
};

static size_t GhtTypeSizes[] =
{
    -1,    /* PC_UNKNOWN */
    1, 1,  /* PC_INT8, PC_UINT8, */
    2, 2,  /* PC_INT16, PC_UINT16 */
    4, 4,  /* PC_INT32, PC_UINT32 */
    8, 8,  /* PC_INT64, PC_UINT64 */
    8, 4   /* PC_DOUBLE, PC_FLOAT */
};


/** Allocate memory using runtime memory management */
void* ght_malloc(size_t size);
/** Free memory using runtime memory management */
void  ght_free(void *ptr);
/** String duplication using runtime memory management */
char* ght_strdup(const char *str);
/** Memory resizing using runtime memory management */
void* ght_realloc(void *ptr, size_t size);
/** Send error message and stop */
void ght_error(const char *fmt, ...);
/** Send info message */
void ght_info(const char *fmt, ...);
/** Send warning message */
void ght_warn(const char *fmt, ...);
        

/** Initialize memory/message handling */
void ght_set_handlers(GhtAllocator allocator, GhtReallocator reallocator,
                      GhtDeallocator deallocator, GhtMessageHandler error_handler,
                      GhtMessageHandler info_handler, GhtMessageHandler warn_handler);

/** Set the malloc handler */
void   ght_set_allocator(GhtAllocator allocator);

/** Set the free handler */
void   ght_set_deallocator(GhtDeallocator deallocator);

/**
* Calculate the amount of charaters two GhtHashes have in common. 
* Returns -1 for two full hashes with nothing in common.
* Returns 0 if one of the hashes is "", the "master hash".
*
* Examples:
*   ("abcdef", "abc", 3) => 3
*   ("abc", "abcdef", 3) => 3
*   ("abc", "", 3) => 0
*   ("abcdef", "abcdef", 2) => 2
*   ("abc", "1abc", 3) => -1
*/
int ght_hash_common_length(const GhtHash *a, const GhtHash *b, int max_len);

/** Generate hash, up to resolution characters in length */
GhtErr ght_hash_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtHash **hash);

/** Generate area, since hash of finite resolution bounds an area */
GhtErr ght_area_from_hash(const GhtHash *hash, GhtArea *area);

/** Release hash memory */
GhtErr ght_hash_free(GhtHash *hash);

/** 
* Find the common parts of two hash strings and return pointers 
* to the unique bits. Also returns a code indicating the kind
* of match between the hashes.
*  GHT_NONE, no match ("acbde" "12345")
*  GHT_GLOBAL, match with the global hash key ("" "abcde")
*  GHT_SAME, identical hashes  ("abcde" "abcde")
*  GHT_CHILD, hash b is child of a ("abc" "abcde")
*  GHT_SPLIT, hash a and b share a common prefix ("abcde" "abcpq")
*             so they will both need to be split
*/
GhtErr ght_hash_leaf_parts(const GhtHash *a, const GhtHash *b, int maxlen, GhtHashMatch *matchtype, GhtHash **a_leaf, GhtHash **b_leaf);

/** Free a node and all its children and attributes */
GhtErr ght_node_free(GhtNode *node);

/** Add node_to_insert to a tree of nodes headed by node */
GhtErr ght_node_insert_node(GhtNode *node, GhtNode *node_to_insert, int duplicates);

/** Set the hash string on a node, takes ownership of hash */
GhtErr ght_node_set_hash(GhtNode *node, GhtHash *hash);

/** Create a new code from a coordinate */
GhtErr ght_node_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNode **node);

/** Create a new node from a hash */
GhtErr ght_node_new(GhtHash *hash, GhtNode **node);

/** Fill a stringbuffer with a printout of the node tree */
GhtErr ght_node_to_string(GhtNode *node, stringbuffer_t *sb, int level);

/** How many leaf nodes in this tree? */
GhtErr ght_node_count_leaves(const GhtNode *node, int *count);

/** Create an empty nodelist */
GhtErr ght_nodelist_new(GhtNodeList **nodelist);

/** Add a new node to a nodelist */
GhtErr ght_nodelist_add_node(GhtNodeList *nl, GhtNode *node);

/** Free a nodelist, and optionally all the nodes referenced by the list */
GhtErr ght_nodelist_free(GhtNodeList *nl, int deep);

/** Free an attribute list */
GhtErr ght_attributelist_free(GhtAttributeList *attrs);



#endif /* _GHT_INTERNAL_H */