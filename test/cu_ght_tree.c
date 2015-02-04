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
static GhtNode *root;

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
        return ght_schema_free(simpleschema);

    return 0;
}

static GhtTree *
tsv_string_to_tree(char *str, const GhtSchema *schema)
{
    GhtNodeList *nodelist;
    GhtTree *tree;
    GhtConfig config;
    char *ptr_start, *tmp;
    char *ptr_end;
    char *filestr = str;
    double dblval[16]; /* Only going to handle files 16 columns wide */
    int field_num = 0;

    ght_nodelist_new(16, &nodelist);
    
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
    
    ght_config_init(&config);
    if ( ght_tree_from_nodelist(schema, nodelist, &config, &tree) != GHT_OK )
      return NULL;
    ght_tree_compact_attributes(tree);
    ght_nodelist_free_shallow(nodelist);
    
    return tree;
}

static GhtTree *
tsv_file_to_tree(const char *fname, const GhtSchema *schema)
{
    char *filestr = file_to_str(fname);
    if ( ! filestr ) return NULL;
    GhtTree *tree = tsv_string_to_tree(filestr, schema);
    ght_free(filestr);
    return tree;
}

static void
test_ght_tree_extent(void)
{
    static const char *simpledata = "test/data/simple-data.tsv";   
    GhtTree *tree;
    GhtErr err;
    GhtArea area;
    // stringbuffer_t *sb;
    
    /* Read a nodelist from a TSV file */
    tree = tsv_file_to_tree(simpledata, simpleschema);
    CU_ASSERT_EQUAL(tree->num_nodes, 8);
    err = ght_tree_get_extent(tree, &area);
    CU_ASSERT_EQUAL(err, GHT_OK);
    // printf("%g %g, %g %g\n", area.x.min, area.y.min, area.x.max, area.y.max);
    CU_ASSERT_DOUBLE_EQUAL(area.x.min, -126.419, 0.001);
    CU_ASSERT_DOUBLE_EQUAL(area.y.min, 45.1212, 0.0001);
    CU_ASSERT_DOUBLE_EQUAL(area.x.max, -126.412, 0.001);
    CU_ASSERT_DOUBLE_EQUAL(area.y.max, 45.1291, 0.0001);

    ght_tree_free(tree);
}

static void
test_ght_tree_empty(void)
{
    GhtTree *tree1, *tree2;
    GhtErr err;
    GhtWriter *writer;
    GhtReader *reader;
    uint8_t *bytes;
    size_t bytes_size;

    /* Empty tree */
    err = ght_tree_new(simpleschema, &tree1);
    CU_ASSERT_EQUAL(err, GHT_OK);
    
    /* Serialize it */
    err = ght_writer_new_mem(&writer);
    err = ght_tree_write(tree1, writer);
    CU_ASSERT_EQUAL(err, GHT_ERROR);

    ght_tree_free(tree1);
    ght_writer_free(writer);
}

static void
test_ght_tree_filter(void)
{
    static const char *simpledata = "test/data/simple-data.tsv";   
    GhtTree *tree1, *tree2;
    GhtErr err;
    
    /* Read a nodelist from a TSV file */
    tree1 = tsv_file_to_tree(simpledata, simpleschema);
    CU_ASSERT_EQUAL(tree1->num_nodes, 8);

    err = ght_tree_filter_greater_than(tree1, "Z", 123.35, &tree2);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_EQUAL(tree2->num_nodes, 7);    
    ght_tree_free(tree2);
    
    err = ght_tree_filter_less_than(tree1, "Z", 123.35, &tree2);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_EQUAL(tree2->num_nodes, 1);    
    ght_tree_free(tree2);

    err = ght_tree_filter_less_than(tree1, "Z", 103.35, &tree2);
    CU_ASSERT_EQUAL(err, GHT_OK);
    CU_ASSERT_EQUAL(tree2->num_nodes, 0);   
    ght_tree_free(tree2);
    
    ght_tree_free(tree1);
}

static void
test_ght_tree_xxx(void)
{
    GhtTree *tree;
    char tsv[256];
    sprintf(tsv, "18\t-45.5\t123.4\t5\n170\t-45\t123.4\t5\n");
    tree = tsv_string_to_tree(tsv, simpleschema);
    CU_ASSERT_FATAL( tree != NULL );
    printf("Num nodes: %d", tree->num_nodes);
    CU_ASSERT_EQUAL(tree->num_nodes, 2);
    ght_tree_free(tree);
}

/* REGISTER ***********************************************************/

CU_TestInfo tree_tests[] =
{
    GHT_TEST(test_ght_tree_extent),
    GHT_TEST(test_ght_tree_empty),
    GHT_TEST(test_ght_tree_filter),
    GHT_TEST(test_ght_tree_xxx),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo tree_suite = {"tree", init_suite, clean_suite, tree_tests};

