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
                
                ght_node_from_coordinate(&coord, 16, &node);
                
                for ( i = 2; i < schema->num_dims; i++ )
                {
                    GhtAttribute *a;
                    ght_attribute_new(schema->dims[i], dblval[i], &a);
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
test_ght_build_tree_with_attributes(void)
{
    static const char *simpledata = "test/data/simple-data.tsv";
    
    GhtNodeList *nodelist;
    
    nodelist = tsv_file_to_node_list(simpledata, simpleschema);
    
    ght_nodelist_free_deep(nodelist);
}


/* REGISTER ***********************************************************/

CU_TestInfo attribute_tests[] =
{
    GHT_TEST(test_ght_build_tree_with_attributes),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo attributes_suite = {"attributes", init_suite, clean_suite, attribute_tests};

