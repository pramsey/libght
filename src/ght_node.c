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
#include <float.h>

/** New, empty, nodelist */
GhtErr
ght_nodelist_new(GhtNodeList **nodelist)
{
    GhtNodeList *nl;
    assert(nodelist);
    nl = ght_malloc(sizeof(GhtNodeList));
    if ( ! nl ) return GHT_ERROR;
    memset(nl, 0, sizeof(GhtNodeList));
    nl->nodes = NULL;
    nl->num_nodes = 0;
    nl->max_nodes = 0;
    *nodelist = nl;
    return GHT_OK;
}

/** Free just the containing structure, leaving the nodes */
GhtErr
ght_nodelist_free_shallow(GhtNodeList *nl)
{
    if ( nl->nodes )
        ght_free(nl->nodes);

    ght_free(nl);
}

/** Free all the nodes, then the containing stuff */
GhtErr
ght_nodelist_free_deep(GhtNodeList *nl)
{
    int i;
    if ( nl->nodes )
    {
        for ( i = 0; i < nl->num_nodes; i++ )
        {
            if ( nl->nodes[i] )
                ght_node_free(nl->nodes[i]);
        }
    }
    return ght_nodelist_free_shallow(nl);
}

/** Add node, adding memory space as necessary. */
GhtErr
ght_nodelist_add_node(GhtNodeList *nl, GhtNode *node)
{
    // assert(nl);
    // assert(node);
    /* First time, initialize */
    if ( nl->max_nodes == 0 )
    {
        nl->nodes = ght_malloc(sizeof(GhtNode*) * 8);
        nl->max_nodes = 8;
    }

    /* Node list is full, so expand it */
    if ( nl->num_nodes == nl->max_nodes )
    {
        nl->max_nodes *= 2;
        nl->nodes = ght_realloc(nl->nodes, sizeof(GhtNode*) * nl->max_nodes);
        if ( ! nl->nodes ) return GHT_ERROR;
    }

    /* Something wrong with memory? */
    if ( ! nl->nodes ) return GHT_ERROR;

    /* Add the node to list */
    nl->nodes[nl->num_nodes] = node;
    nl->num_nodes++;
    return GHT_OK;
}

/** Some nodelist utility functions */
static int
ght_node_is_leaf(const GhtNode *node)
{
    return (! node->children) || (node->children->num_nodes == 0);
}

static int
ght_node_num_children(const GhtNode *node)
{
    if ( ! node->children)
        return 0;
    return node->children->num_nodes;
}

static GhtErr
ght_node_new(GhtNode **node)
{
    GhtNode *n = ght_malloc(sizeof(GhtNode));
    if ( ! n ) return GHT_ERROR;
    memset(n, 0, sizeof(GhtNode));
    n->children = NULL;
    n->attributes = NULL;
    n->hash = NULL;
    *node = n;
    return GHT_OK;
}

GhtErr
ght_node_set_hash(GhtNode *node, GhtHash *hash)
{
    if ( node->hash )
        ght_free(node->hash);
    node->hash = hash;
    return GHT_OK;
}

/** Create new node, taking ownership of hash parameter */
GhtErr
ght_node_new_from_hash(GhtHash *hash, GhtNode **node)
{
    GHT_TRY(ght_node_new(node));
    GHT_TRY(ght_node_set_hash(*node, ght_strdup(hash)));
    return GHT_OK;
}

/** Create new node, taking ownership of hash parameter */
GhtErr
ght_node_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNode **node)
{
    GhtHash *hash;
    assert(node != NULL);
    assert(coord != NULL);
    GHT_TRY(ght_hash_from_coordinate(coord, resolution, &hash));
    GHT_TRY(ght_node_new(node));
    GHT_TRY(ght_node_set_hash(*node, hash));
    return GHT_OK;
}

static GhtErr
ght_node_add_child(GhtNode *parent, GhtNode *child)
{
    if ( ! parent->children )
    {
        ght_nodelist_new(&(parent->children));
    }
    return ght_nodelist_add_node(parent->children, child);
}

static GhtErr
ght_node_transfer_attributes(GhtNode *node_from, GhtNode *node_to)
{
    size_t sz;
    
    /* Nothing to transfer */
    if ( ! node_from->attributes )
        return GHT_OK;
        
    /* Uh oh, something is already there */
    if ( node_to->attributes )
        return GHT_ERROR;
        
    node_to->attributes = node_from->attributes;
    node_from->attributes = NULL;
    
    return GHT_OK;
}

/**
* Recursive function, walk down from parent node, looking for
* appropriate insertion point for node_to_insert. If duplicates,
* and duplicate leaf, insert as hash-less "attribute only" node.
* ["abcdefg", "abcdeff", "abcdddd", "abbbeee"] becomes
* "ab"->["c"->["d"->["ddd","ef"->["g","f"]]],"b"]
*/
GhtErr
ght_node_insert_node(GhtNode *node, GhtNode *node_to_insert, GhtDuplicates duplicates)
{
    GhtHash *node_leaf, *node_to_insert_leaf;
    GhtErr err;
    GhtHashMatch matchtype;

    /* NULL hash implies this node is a faux node for duplicate points */
    if ( ! node->hash )
        return GHT_INCOMPLETE;

    /* matchtype in (GHT_NONE, GHT_GLOBAL, GHT_SAME, GHT_CHILD, GHT_SPLIT) */
    /* NONE and GLOBAL come back with GHT_ERROR, so we don't handle them yet */
    GHT_TRY(ght_hash_leaf_parts(node->hash, node_to_insert->hash, GHT_MAX_HASH_LENGTH,
                                &matchtype, &node_leaf, &node_to_insert_leaf));

    /* Insert node is child of node, either explicitly, or implicitly for */
    /* the "" hash which serves as a master parent */
    /* "abcdef" is a GHT_CHILD or "abc", and gets added as "def" */
    if ( matchtype == GHT_CHILD || matchtype == GHT_GLOBAL )
    {
        int i;
        ght_node_set_hash(node_to_insert, ght_strdup(node_to_insert_leaf));
        for ( i = 0; i < ght_node_num_children(node); i++ )
        {
            err = ght_node_insert_node(node->children->nodes[i], node_to_insert, duplicates);
            /* Node added to one of the children */
            if ( err == GHT_OK ) return GHT_OK;
        }
        /* Node didn't fit any of the children, so add it at this level */
        return ght_node_add_child(node, node_to_insert);
    }

    if ( matchtype == GHT_SAME )
    {
        if ( duplicates )
        {
            /* Duplicate node entry, to hang attributes off of, strip hash */
            /* and then use this node as the parent */
            ght_free(node_to_insert->hash);
            node_to_insert->hash = NULL;
            return ght_node_add_child(node, node_to_insert);
        }
        else
        {
            /* Average / median the duplicates onto parent here? */
            return GHT_OK;
        }
    }

    /* "abcdef" and "abcghi" need to GHT_SPLIT, into "abc"->["def", "ghi"] */
    if ( matchtype == GHT_SPLIT )
    {
        /* We need a new node to hold that part of the parent that is not shared */
        GhtNode *another_node_to_insert;
        GHT_TRY(ght_node_new_from_hash(node_leaf, &another_node_to_insert));
        /* Move attributes to the new child */
        GHT_TRY(ght_node_transfer_attributes(node, another_node_to_insert));
        
        /* Any children of the parent need to move down the tree with the unique part of the hash */
        if ( node->children )
        {
            another_node_to_insert->children = node->children;
            node->children = NULL;
        }
        /* Null-terminate parent hash at end of shared part */
        *node_leaf = '\0';
        /* Pull the non-shared part of insert node hash to the front */
        memmove(node_to_insert->hash, node_to_insert_leaf, strlen(node_to_insert_leaf)+1);
        /* Add the unique portion of the parent to the parent */
        GHT_TRY(ght_node_add_child(node, another_node_to_insert));
        /* Add the unique portion of the insert node to the parent */
        GHT_TRY(ght_node_add_child(node, node_to_insert));
        /* Done! */
        return GHT_OK;
    }

    /* Don't get here */
    return GHT_ERROR;

}


GhtErr
ght_node_to_string(GhtNode *node, stringbuffer_t *sb, int level)
{
    int i = 0;
    stringbuffer_aprintf(sb, "%*s%s", 2*level, "", node->hash);
    if ( node->attributes )
    {
        stringbuffer_append(sb, "  ");
        ght_attributelist_to_string(node->attributes, sb);
    }
    stringbuffer_append(sb, "\n");
    for ( i = 0; i < ght_node_num_children(node); i++ )
    {
        GHT_TRY(ght_node_to_string(node->children->nodes[i], sb, level + 1));
    }
    return GHT_OK;
}


GhtErr
ght_node_count_leaves(const GhtNode *node, int *count)
{
    int i;
    GhtErr err;
    if ( ght_node_is_leaf(node) )
    {
        *count += 1;
        return GHT_OK;
    }
    for ( i = 0; i < ght_node_num_children(node); i++ )
    {
        GHT_TRY(ght_node_count_leaves(node->children->nodes[i], count));
    }
    return GHT_OK;
}

GhtErr
ght_node_free(GhtNode *node)
{
    int i;
    const int deep = 1;
    assert(node != NULL);

    if ( node->attributes )
        GHT_TRY(ght_attributelist_free(node->attributes));

    if ( node->children )
        GHT_TRY(ght_nodelist_free_deep(node->children));

    if ( node->hash )
        GHT_TRY(ght_hash_free(node->hash));

    ght_free(node);
}

GhtErr
ght_node_add_attribute(GhtNode *node, GhtAttribute *attribute)
{
    if ( ! node->attributes )
    {
        GHT_TRY(ght_attributelist_new(&(node->attributes)));
    }
    GHT_TRY(ght_attributelist_add_attribute(node->attributes, attribute));
    return GHT_OK;
}

GhtErr
ght_node_delete_attribute(GhtNode *node, const GhtDimension *dim)
{
    if ( ! node->attributes )
        return GHT_ERROR;
    
    GHT_TRY(ght_attributelist_delete_attribute(node->attributes, dim));
    
    if ( node->attributes->num_attributes == 0 )
    {
        GHT_TRY(ght_attributelist_free(node->attributes));
        node->attributes = NULL;
    }
    return GHT_OK;
}

static GhtErr
ght_node_compact_attribute_with_delta(GhtNode *node, const GhtDimension *dim, double delta, GhtAttribute *compacted_attribute)
{
    int i;
    
    /* This is an internal node, see if all the children share a value in this dimension */
    if ( node->children && node->children->num_nodes > 0 )
    {
        double minval = DBL_MAX;
        double maxval = -1 * DBL_MAX;
        double totval = 0.0;
        int node_count = 0;
        
        /* Figure out the range of values for this dimension in child nodes */
        for ( i = 0; i < node->children->num_nodes; i++ )
        {
            GhtAttribute attr;
            GhtErr err;
            err = ght_node_compact_attribute_with_delta(node->children->nodes[i], dim, delta, &attr);
            if ( err == GHT_OK )
            {
                double d;
                GHT_TRY(ght_attribute_get_value(&attr, &d));
                (d < minval) ? (minval = d) : 0;
                (d > maxval) ? (maxval = d) : 0;
                totval += d;
                node_count++;
            }
            else
            {
                continue;
            }
        }
        
        /* If the range is narrow, and we got values from all our children, compact them */
        if ( (maxval-minval) < delta && node_count == node->children->num_nodes )
        {
            double val = (minval+maxval)/2.0;
            GhtAttribute *myattr;
            for ( i = 0; i < node->children->num_nodes; i++ )
            {
                ght_node_delete_attribute(node->children->nodes[i], dim);
            }
            ght_attribute_new(dim, val, &myattr);
            memcpy(compacted_attribute, myattr, sizeof(GhtAttribute));
            ght_node_add_attribute(node, myattr);
            return GHT_OK;
        }
        return GHT_ERROR;
    }
    /* This is a leaf node, send the attribute value up to the caller */
    else
    {
        if ( ! node->attributes ) return GHT_ERROR;
        for ( i = 0; i < node->attributes->num_attributes; i++ )
        {
            if ( dim == node->attributes->attributes[i]->dim )
            {
                memcpy(compacted_attribute, node->attributes->attributes[i], sizeof(GhtAttribute));
                return GHT_OK;
            }
        }
        return GHT_ERROR;
    }
}

GhtErr
ght_node_compact_attribute(GhtNode *node, const GhtDimension *dim, GhtAttribute *attr)
{
    return ght_node_compact_attribute_with_delta(node, dim, 10e-8, attr);
}
