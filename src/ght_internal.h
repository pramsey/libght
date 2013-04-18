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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include "pstdint.h"
#endif

#define GHT_TRY(functioncall) { if ( (functioncall) == GHT_ERROR ) return GHT_ERROR; }
#define GHT_NUM_TYPES 11

typedef enum
{
    GHT_NONE,
    GHT_GLOBAL,
    GHT_SAME,
    GHT_CHILD,
    GHT_SPLIT
} GhtHashMatch;

typedef enum
{   
    GHT_DUPES_NO = 0,
    GHT_DUPES_YES = 1
} GhtDuplicates;

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
    -1,                                 /* GHT_UNKNOWN */
    sizeof(int8_t),  sizeof(uint8_t),   /* GHT_INT8,   GHT_UINT8, */
    sizeof(int16_t), sizeof(uint16_t),  /* GHT_INT16,  GHT_UINT16 */
    sizeof(int32_t), sizeof(uint32_t),  /* GHT_INT32,  GHT_UINT32 */
    sizeof(int64_t), sizeof(uint64_t),  /* GHT_INT64,  GHT_UINT64 */
    sizeof(double),  sizeof(float)      /* GHT_DOUBLE, GHT_FLOAT */
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
GhtErr ght_node_insert_node(GhtNode *node, GhtNode *node_to_insert, GhtDuplicates duplicates);

/** Set the hash string on a node, takes ownership of hash */
GhtErr ght_node_set_hash(GhtNode *node, GhtHash *hash);

/** Create a new code from a coordinate */
GhtErr ght_node_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNode **node);

/** Create a new node from a hash */
GhtErr ght_node_new_from_hash(GhtHash *hash, GhtNode **node);

/** Fill a stringbuffer with a printout of the node tree */
GhtErr ght_node_to_string(GhtNode *node, stringbuffer_t *sb, int level);

/** How many leaf nodes in this tree? */
GhtErr ght_node_count_leaves(const GhtNode *node, int *count);

/** Delete an attribute from the node (frees the attribute) */
GhtErr ght_node_delete_attribute(GhtNode *node, const GhtDimension *dim);

/** Add a new attribute to the node */
GhtErr ght_node_add_attribute(GhtNode *node, GhtAttribute *attribute);

/** Move attributes to the highest level in the tree at which they apply to all children */
GhtErr ght_node_compact_attribute(GhtNode *node, const GhtDimension *dim, GhtAttribute *attr);

/** Create an empty nodelist */
GhtErr ght_nodelist_new(GhtNodeList **nodelist);

/** Add a new node to a nodelist */
GhtErr ght_nodelist_add_node(GhtNodeList *nl, GhtNode *node);

/** Free a nodelist, and optionally all the nodes referenced by the list */
GhtErr ght_nodelist_free_deep(GhtNodeList *nl);

/** Free a nodelist, but not the nodes it holds */
GhtErr ght_nodelist_free_shallow(GhtNodeList *nl);

/** Alocate a new attribute and fill in the value */
GhtErr ght_attribute_new(const GhtDimension *dim, double val, GhtAttribute **attr);

/** Free an attribtue */
GhtErr ght_attribute_free(GhtAttribute *attr);

/** Return the scaled and offset version of the packed attribute value */
GhtErr ght_attribute_get_value(const GhtAttribute *attr, double *val);

/** Set the packed attribute value */
GhtErr ght_attribute_set_value(GhtAttribute *attr, double val);

/** Write an appropriately formatted value into the stringbuffer_t */
GhtErr ght_attribute_to_string(const GhtAttribute *attr, stringbuffer_t *sb);

/** Size in bytes of an attribute type */
GhtErr ght_type_size(GhtType type, size_t *size);

/** Give a type string (eg "uint16_t"), return the GhtType number */
GhtErr ght_type_from_str(const char *str, GhtType *type);

/** Create an empty dimension */
GhtErr ght_dimension_new(GhtDimension **dim);

/** Create a schema from an XML document */
GhtErr ght_schema_from_xml_str(const char *xmlstr, GhtSchema **schema);

/** Find the GhtDimension corresponding to a name */
GhtErr ght_schema_get_dimension_by_name(const GhtSchema *schame, const char *name, GhtDimension **dim);

/** Find the GhtDimension corresponding to an index */
GhtErr ght_schema_get_dimension_by_index(const GhtSchema *schema, int i, GhtDimension **dim);

/** Free an existing schema */
GhtErr ght_schema_free(GhtSchema *schema);

#endif /* _GHT_INTERNAL_H */
