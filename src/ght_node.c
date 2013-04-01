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

/** New, empty, nodelist */
GhtErr
ght_nodelist_new(GhtNodeList **nodelist)
{
    GhtNodeList *nl;
    assert(nodelist);
    nl = ght_malloc(sizeof(GhtNodeList));
    if ( ! nl ) return GHT_ERROR;
    nl->nodes = NULL;
    nl->num_nodes = 0;
    nl->max_nodes = 0;
    *nodelist = nl;
    return GHT_OK;
}

/** If deep is set, free list and all nodes. Otherwise, just free
* over arching list and leave the nodes alone. */
GhtErr
ght_nodelist_free(GhtNodeList *nl, int deep)
{

    if ( nl->nodes )
    {
        if ( deep )
        {
            int i;
            for ( i = 0; i < nl->num_nodes; i++ )
            {
                ght_node_free(nl->nodes[i]);
            }
        }
        ght_free(nl->nodes);
    }
    ght_free(nl);
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


/** Create new node, taking ownership of hash parameter */
GhtErr
ght_node_new(GhtHash *hash, GhtNode **node)
{
    GhtNode *n = ght_malloc(sizeof(GhtNode));
    if ( ! n ) return GHT_ERROR;
    n->children = NULL;
    n->attributes = NULL;
    n->hash = ght_strdup(hash);
    *node = n;
    return GHT_OK;
}

/** Create new node, taking ownership of hash parameter */
GhtErr
ght_node_from_coordinate(const GhtCoordinate *coord, unsigned int resolution, GhtNode **node)
{
    GhtHash *hash;
    GhtNode *n;
    assert(node != NULL);
    assert(coord != NULL);
    GHT_TRY(ght_hash_from_coordinate(coord, resolution, &hash));
    n = ght_malloc(sizeof(GhtNode));
    if ( ! n ) return GHT_ERROR;
    n->children = NULL;
    n->attributes = NULL;
    n->hash = hash;
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

static GhtErr
ght_node_add_child(GhtNode *parent, GhtNode *child)
{
    if ( ! parent->children )
    {
        ght_nodelist_new(&(parent->children));
    }
    return ght_nodelist_add_node(parent->children, child);
}


/**
* Recursive function, walk down from parent node, looking for
* appropriate insertion point for node_to_insert. If duplicates,
* and duplicate leaf, insert as hash-less "attribute only" node.
* ["abcdefg", "abcdeff", "abcdddd", "abbbeee"] becomes
* "ab"->["c"->["d"->["ddd","ef"->["g","f"]]],"b"]
*/
GhtErr
ght_node_insert_node(GhtNode *node, GhtNode *node_to_insert, int duplicates)
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

    if ( matchtype == GHT_SPLIT )
    {
        /* We need a new node to hold that part of the parent that is not shared */
        GhtNode *another_node_to_insert;
        GHT_TRY(ght_node_new(node_leaf, &another_node_to_insert));
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
    int i;
    stringbuffer_aprintf(sb, "%*s%s\n", level, "", node->hash);
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
        ght_attributelist_free(node->attributes);

    if ( node->children )
        ght_nodelist_free(node->children, deep);

    if ( node->hash )
        ght_hash_free(node->hash);

    ght_free(node);
}

