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

GhtErr
ght_tree_from_nodelist(const GhtSchema *schema, GhtNodeList *nlist, GhtDuplicates duplicates, GhtTree **tree)
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
            err = ght_node_insert_node(root, node, duplicates);
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
    
    t = ght_malloc(sizeof(GhtTree));
    t->num_nodes = nlist->num_nodes;
    t->root = root;
    t->schema = schema;
    
    *tree = t;
    return GHT_OK;
}



