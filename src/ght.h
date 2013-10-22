/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include "ght_core.h"

#ifndef _GHT_H
#define _GHT_H

typedef void* GhtDimensionPtr;
typedef void* GhtSchemaPtr;
typedef void* GhtWriterPtr;
typedef void* GhtReaderPtr;
typedef void* GhtTreePtr;
typedef void* GhtNodeListPtr;
typedef void* GhtNodePtr;
typedef void* GhtAttributePtr;
typedef GhtConfig* GhtConfigPtr;


/***********************************************************************
*   MEMORY MANAGEMENT
*/

/* Global function signatures for memory/logging handlers. */
typedef void* (*GhtAllocator)(size_t size);
typedef void* (*GhtReallocator)(void *mem, size_t size);
typedef void  (*GhtDeallocator)(void *mem);
typedef void  (*GhtMessageHandler)(const char *string, va_list ap);

/** Initialize memory/message handling */
void ght_set_handlers(GhtAllocator allocator, GhtReallocator reallocator,
                      GhtDeallocator deallocator, GhtMessageHandler error_handler,
                      GhtMessageHandler info_handler, GhtMessageHandler warn_handler);

/***********************************************************************
*   NODE
*/

/** Create a new node from a hash */
GhtErr ght_node_new_from_hash(const GhtHash *hash, GhtNodePtr *node);

/** Create a new code from a coordinate */
GhtErr ght_node_new_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNodePtr *node);

/** Get the coordinates represented by the node */
GhtErr ght_node_get_coordinate(const GhtNodePtr node, GhtCoordinate *coord);

/** Add a new attribute to the node */
GhtErr ght_node_add_attribute(GhtNodePtr node, GhtAttributePtr attribute);

/** Get the attribute list handing off the node */
GhtErr ght_node_get_attributes(const GhtNodePtr node, GhtAttributePtr *attr);


/***********************************************************************
*   NODELIST
*/

/** Create an empty nodelist */
GhtErr ght_nodelist_new(int capacity, GhtNodeListPtr *nodelist);

/** How many nodes in this GhtNodeList? */
GhtErr ght_nodelist_get_num_nodes(const GhtNodeListPtr nodelist, int *num_nodes);

/** Get a GhtNode by index number */
GhtErr ght_nodelist_get_node(const GhtNodeListPtr nodelist, int index, GhtNodePtr *node);

/** Add a new node to a nodelist */
GhtErr ght_nodelist_add_node(GhtNodeListPtr nodelist, GhtNodePtr node);

/** Free a nodelist, and optionally all the nodes referenced by the list */
GhtErr ght_nodelist_free_deep(GhtNodeListPtr nodelist);

/** Free a nodelist, but not the nodes it holds */
GhtErr ght_nodelist_free_shallow(GhtNodeListPtr nodelist);


/***********************************************************************
*   ATTRIBUTE
*/

/** Allocate a new attribute and fill in the value from a double */
GhtErr ght_attribute_new_from_double(const GhtDimensionPtr dim, double val, GhtAttributePtr *attr);

/** Read the next attribute from an attribute list */
GhtErr ght_attribute_get_next(const GhtAttributePtr attr, GhtAttributePtr *nextattr);

/** Get the dimension associated with a GhtAttribute */
GhtErr ght_attribute_get_dimension(const GhtAttributePtr attr, const GhtDimensionPtr *dim);

/** Return the scaled and offset version of the packed attribute value */
GhtErr ght_attribute_get_value(const GhtAttributePtr attr, double *val);

/***********************************************************************
*   DIMENSION
*/

/** Create a populated dimension */
GhtErr ght_dimension_new_from_parameters(const char *name, const char *desc, GhtType type, double scale, double offset, GhtDimensionPtr *dim);

/** What's the name of this dimension? */
GhtErr ght_dimension_get_name(const GhtDimensionPtr dim, const char **name);

/** What's the type of this dimension? */
GhtErr ght_dimension_get_type(const GhtDimensionPtr dim, GhtType *type);

/** What's the index of this dimension? */
GhtErr ght_dimension_get_index(const GhtDimensionPtr dim, int *index);


/***********************************************************************
*   SCHEMA
*/

/** Allocate a blank GhtSchemaPtr */
GhtErr ght_schema_new(GhtSchemaPtr *schema);

/** Write out an XML representation of a GhtSchemaPtr */
GhtErr ght_schema_to_xml_file(const GhtSchemaPtr schema, const char *filename);

/** Write out an XML representation of a GhtSchemaPtr */
GhtErr ght_schema_from_xml_file(const char *filename, GhtSchemaPtr *schema);

/** Append a GhtDimension to the GhtSchemaPtr */
GhtErr ght_schema_add_dimension(GhtSchemaPtr schema, GhtDimensionPtr dim);

/** How many dimensions in this schema? */
GhtErr ght_schema_get_num_dimensions(const GhtSchemaPtr schema, unsigned int *num_dims);

/** Read a dimension */
GhtErr ght_schema_get_dimension_by_index(const GhtSchemaPtr schema, int i, GhtDimensionPtr *dim);

/** Read a dimension */
GhtErr ght_schema_get_dimension_by_name(const GhtSchemaPtr schema, const char *name, GhtDimensionPtr *dim);

/** Free an existing schema */
GhtErr ght_schema_free(GhtSchemaPtr schema);


/***********************************************************************
*   TREE
*/

/** Allocate a new tree and initialize config parameters */
GhtErr ght_tree_new(const GhtSchemaPtr a, GhtTreePtr *tree);

/** Build a tree from a linear nodelist */
GhtErr ght_tree_from_nodelist(const GhtSchemaPtr schema, GhtNodeListPtr nlist, GhtConfigPtr config, GhtTreePtr *tree);

/** Free a GhtTree from memory, including nodes and schema */
GhtErr ght_tree_free(GhtTreePtr tree);

/** Add a GhtNode to a GhtTreePtr */
GhtErr ght_tree_insert_node(GhtTreePtr tree, GhtNodePtr node);

/** Write a GhtTree to memory or file */
GhtErr ght_tree_write(const GhtTreePtr tree, GhtWriterPtr writer);

/** Read the top level hash key off the GhtTreePtr */
GhtErr ght_tree_get_hash(const GhtTreePtr tree, GhtHash **hash);

/** Read the schema from the GhtTree */
GhtErr ght_tree_get_schema(const GhtTreePtr tree, GhtSchemaPtr *schema);

/** Read the point cound from the GhtTree */
GhtErr ght_tree_get_numpoints(const GhtTreePtr tree, int *numpoints);

/** Calculate the spatial extent of a GhtTree */
GhtErr ght_tree_get_extent(const GhtTreePtr tree, GhtArea *area);

/** Allocate new tree with only nodes that meet the filter condition */
GhtErr ght_tree_filter_greater_than(const GhtTreePtr tree, const char *dimname, double value, GhtTreePtr *tree_filtered);

/** Allocate new tree with only nodes that meet the filter condition */
GhtErr ght_tree_filter_less_than(const GhtTreePtr tree, const char *dimname, double value, GhtTreePtr *tree_filtered);

/** Allocate new tree with only nodes that meet the filter condition */
GhtErr ght_tree_filter_between(const GhtTreePtr tree, const char *dimname, double value1, double value2, GhtTreePtr *tree_filtered);

/** Allocate new tree with only nodes that meet the filter condition */
GhtErr ght_tree_filter_equal(const GhtTreePtr tree, const char *dimname, double value, GhtTreePtr *tree_filtered);

/** Compact all the attributes from 'Z' onwards */
GhtErr ght_tree_compact_attributes(GhtTreePtr tree);

/** Write a GhtTree to memory or file */
GhtErr ght_tree_write(const GhtTreePtr tree, GhtWriterPtr writer);

/** Write a GhtTree to memory of file */
GhtErr ght_tree_read(GhtReaderPtr reader, GhtTreePtr *tree);

/** Set up a tree configuration with defaults */
GhtErr ght_config_init(GhtConfigPtr config);

/** Take in a tree and output a populated GhtNodeList, creates complete copy of data */
GhtErr ght_tree_to_nodelist(const GhtTreePtr tree, GhtNodeListPtr nodelist);


/***********************************************************************
*   WRITER
*/

/** Create a new file-based writer */
GhtErr ght_writer_new_file(const char *filename, GhtWriterPtr *writer);

/** Create a new memory-backed writer */
GhtErr ght_writer_new_mem(GhtWriterPtr *writer);

/** Read current size of writen bytes */
GhtErr ght_writer_get_size(GhtWriterPtr writer, size_t *size);

/** Copy bytes from memory writer into external buffer */
GhtErr ght_writer_get_bytes(GhtWriterPtr writer, unsigned char *bytes);

/** Close filehandle if necessary and free all memory along with writer */
GhtErr ght_writer_free(GhtWriterPtr writer);



/***********************************************************************
*   READER
*/

/** Create a new file-based reader */
GhtErr ght_reader_new_file(const char *filename, const GhtSchemaPtr schema, GhtReaderPtr *reader);

/** Create a new memory-based reader */
GhtErr ght_reader_new_mem(const unsigned char *bytes_start, size_t bytes_size, const GhtSchemaPtr schema, GhtReaderPtr *reader);

/** Close filehandle if necessary and free all memory along with reader */
GhtErr ght_reader_free(GhtReaderPtr reader);



#endif