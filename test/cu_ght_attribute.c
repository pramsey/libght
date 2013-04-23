/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

static GhtSchema *simpleschema = NULL;
static const char *schemafile = "test/data/simple-schema.xml";

/* Setup/teardown for this suite */
static int
init_suite(void)
{
    char *xmlstr = file_to_str(schemafile);
    if ( ! xmlstr ) return GHT_ERROR;
    return ght_schema_from_xml_str(xmlstr, &simpleschema);
}

static int
clean_suite(void)
{
    if ( simpleschema )
        ght_schema_free(simpleschema);
        
    return 0;
}

GhtNodeList *
tsv_file_to_node_list(const char *fname, const GhtSchema *schema)
{
    GhtNodeList *nodelist;
    char *ptr_start, *ptr_end, *tmp;
    char *filestr = file_to_str(fname);
    double dblval[16]; /* Only going to handle files 16 columns wide */
    int field_num = 0;

    if ( ! filestr ) return NULL;

    ght_nodelist_new(&nodelist);
    
    ptr_start = ptr_end = filestr;
    
    while( 1 ) 
    {
        if ( *ptr_end == '\t' || *ptr_end == '\n' || *ptr_end == '\0' )
        {
            char ptr_tmp = *ptr_end;
            *ptr_end = '\0';
            dblval[field_num] = atof(ptr_start);
            *ptr_end = ptr_tmp;
            ptr_start = ptr_end;
            if ( *ptr_end == '\n' || ! ptr_end ) 
            {
                int i;
                GhtCoordinate coord;
                GhtNode *node;
                
                if ( schema->num_dims != field_num + 1 )
                    return NULL;
                
                coord.x = dblval[0];
                coord.y = dblval[1];
                
                ght_node_new_from_coordinate(&coord, 16, &node);
                
                for ( i = 2; i < schema->num_dims; i++ )
                {
                    GhtAttribute *a;
                    ght_attribute_new_from_double(schema->dims[i], dblval[i], &a);
                    ght_node_add_attribute(node, a);
                }
                
                ght_nodelist_add_node(nodelist, node);
                field_num = 0;
            }
            else field_num++;
            /* All done! */
            if ( *ptr_end == '\0' ) break;
        }
        ptr_end++;
    }
    return nodelist;
}

/* TESTS ***********************************************************/

static void
test_ght_build_node_with_attributes(void)
{
    GhtAttribute *a;
    GhtCoordinate coord;
    GhtNode *node;
    stringbuffer_t *sb = stringbuffer_create();
    
    /* X, Y */
    coord.x = -127;
    coord.y = 45;
    ght_node_new_from_coordinate(&coord, 16, &node);
    /* Z */
    ght_attribute_new_from_double(simpleschema->dims[2], 1231.2, &a);
    ght_node_add_attribute(node, a);
    /* Intensity */
    ght_attribute_new_from_double(simpleschema->dims[3], 3, &a);
    ght_node_add_attribute(node, a);
    
    ght_node_to_string(node, sb, 0);
    CU_ASSERT_STRING_EQUAL("c0j8n012j80252h0  Z=1231.2:Intensity=3\n", stringbuffer_getstring(sb));
    // printf("%s\n", stringbuffer_getstring(sb));
    stringbuffer_destroy(sb);
    ght_node_free(node);
}

static void
test_ght_build_tree_with_attributes(void)
{
    int i;
    static const char *simpledata = "test/data/simple-data.tsv";   
    GhtNodeList *nodelist;
    GhtNode *root, *node;
    GhtErr err;
    GhtAttribute attr;
    stringbuffer_t *sb;
    double d;
    
    /* Read a nodelist from a TSV file */
    nodelist = tsv_file_to_node_list(simpledata, simpleschema);
    CU_ASSERT_EQUAL(nodelist->num_nodes, 8);
    
    /* Build node list into a tree */
    root = nodelist->nodes[0];
    for ( i = 1; i < nodelist->num_nodes; i++ )
    {
        err = ght_node_insert_node(root, nodelist->nodes[i], GHT_DUPES_YES);
    }

    /* Write the tree to string:
        c0n0e
          q
            m
              m7
                dvy8yz9  Z=123.4:Intensity=5
                ky667sj  Z=123.4:Intensity=5
              qw00rg068  Z=123.4:Intensity=5
            hekkhnhj3b  Z=123.4:Intensity=5
            6myj870p99  Z=123.3:Intensity=5
            46jybv17y1  Z=123.4:Intensity=5
          r
            980jtyf1dh  Z=123.4:Intensity=5
            2khvpfu13f  Z=123.4:Intensity=5
    */
    sb = stringbuffer_create();
    ght_node_to_string(root, sb, 0);
    // printf("\n%s\n", stringbuffer_getstring(sb));
    stringbuffer_destroy(sb);

    /* Compact the tree on both attributes:
        c0n0e  Intensity=5
          q
            m  Z=123.4
              m7
                dvy8yz9
                ky667sj
              qw00rg068
            hekkhnhj3b  Z=123.4
            6myj870p99  Z=123.3
            46jybv17y1  Z=123.4
          r  Z=123.4
            980jtyf1dh
            2khvpfu13f  
    */
    sb = stringbuffer_create();
    ght_node_compact_attribute(root, simpleschema->dims[2], &attr);
    ght_node_compact_attribute(root, simpleschema->dims[3], &attr);
    ght_node_to_string(root, sb, 0);
    // printf("\n%s\n", stringbuffer_getstring(sb));
    stringbuffer_destroy(sb);
    
    /* Check that Intensity=5 has migrated all the way to the top of the tree */
    CU_ASSERT_STRING_EQUAL(root->attributes->dim->name, "Intensity");
    ght_attribute_get_value(root->attributes, &d);
    CU_ASSERT_DOUBLE_EQUAL(d, 5, 0.00000001);
    
    ght_node_free(root);
    ght_nodelist_free_shallow(nodelist);
}


/* REGISTER ***********************************************************/

CU_TestInfo attribute_tests[] =
{
    GHT_TEST(test_ght_build_tree_with_attributes),
    GHT_TEST(test_ght_build_node_with_attributes),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo attributes_suite = {"attributes", init_suite, clean_suite, attribute_tests};

