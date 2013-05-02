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

char machine_endian(void); /* from ght_util.c */

GhtErr
ght_tree_new(const GhtSchema *schema, GhtTree **tree)
{
    GhtTree *t;
    t = ght_malloc(sizeof(GhtTree));
    memset(t, 0, sizeof(GhtTree));
    t->config.allow_duplicates = GHT_DUPES_YES;
    t->config.max_hash_length  = GHT_MAX_HASH_LENGTH;
    t->schema = schema;
    *tree = t;
}

GhtErr
ght_tree_free(GhtTree *tree)
{
    assert(tree);
    if ( tree->root )
        ght_node_free(tree->root);
    ght_free(tree);
}

GhtErr
ght_tree_get_hash(const GhtTree *tree, GhtHash **hash)
{
    if ( ! tree->root || ! tree->root->hash ) 
        return GHT_ERROR;
        
    *hash = tree->root->hash;
    return GHT_OK;
}

GhtErr
ght_tree_compact_attributes(GhtTree *tree)
{
    int i;
    GhtAttribute attr;

    /* for 'Z 'and all other attributes... */
    for ( i = 2; i < tree->schema->num_dims; i++ )
    {
        ght_node_compact_attribute(tree->root, tree->schema->dims[i], &attr);
    }
    return GHT_OK;
}



GhtErr
ght_tree_insert_node(GhtTree *tree, GhtNode *node)
{
    if ( ! tree->root )
    {
        tree->root = node;
    }
    else
    {
        GHT_TRY(ght_node_insert_node(tree->root, node, tree->config.allow_duplicates));
    }
    tree->num_nodes++;
    return GHT_OK;
}

GhtErr
ght_tree_write(const GhtTree *tree, GhtWriter *writer)
{
    uint8_t version = GHT_FORMAT_VERSION;
    char endian = machine_endian();

    assert(tree);
    assert(writer);
    
    if ( ! tree->root )
        return GHT_ERROR;
    
    /* Endianness */
    GHT_TRY(ght_write(writer, &endian, 1)); /* 0 = Big, 1 = Little */
    
    /* File format version */
    GHT_TRY(ght_write(writer, &version, 1));
    
    /* Maximum hash length in this tree */
    GHT_TRY(ght_write(writer, &(tree->config.max_hash_length), 1));
    
    return ght_node_write(tree->root, writer);
}

GhtErr
ght_tree_from_nodelist(const GhtSchema *schema, GhtNodeList *nlist, GhtConfig *config, GhtTree **tree)
{
    int i;
    GhtTree *t;
    GhtNode *root;
    GhtErr err;
    
    for ( i = 0; i < nlist->num_nodes; i++ )
    {
        GhtNode *node = nlist->nodes[i];
        /* In case we need to free things when we're partway through, */
        /* make sure only one structure holds ownership of a node at a */
        /* time. */
        nlist->nodes[i] = NULL;
        if ( ! node ) continue;
        if ( i == 0 )
        {
            root = node;
            continue;
        }
        else
        {
            err = ght_node_insert_node(root, node, config->allow_duplicates);
            /* If we have an error, that's a big problem. The nodes underneath */
            /* the GhtNodeList have now been mutated during the insertion */
            /* process, and there are also new interior nodes lying around too */
            /* We just free *everything* and make sure the caller backs all the */
            /* way out. */
            if ( err == GHT_ERROR )
            {
                ght_node_free(root); /* Deep free, all nodes and children */
                ght_nodelist_free_deep(nlist); /* We NULL'ed out all the nodes we put into the tree */
                return GHT_ERROR;
            }
        }
    }
    
    GHT_TRY(ght_tree_new(schema, tree));
    t->num_nodes = nlist->num_nodes;
    t->root = root;
    t->schema = schema;
    t->config = *config;
    
    *tree = t;
    return GHT_OK;
}

GhtErr
ght_tree_to_nodelist(const GhtTree *tree, GhtNodeList *nodelist)
{
    GhtHash h[GHT_MAX_HASH_LENGTH+1];
    
    if ( ! tree->root ) return GHT_ERROR;
    
    return ght_node_to_nodelist(tree->root, nodelist, NULL, h);
}


