/***********************************************************************
* cu_ght_schema.c
*
*        Testing for the schema API functions
*
***********************************************************************/

#include "CUnit/Basic.h"
#include "cu_tester.h"

/* GLOBALS ************************************************************/

static GhtSchema *schema = NULL;
static const char *xmlfile = "test/data/pdal-schema.xml";
static char *xmlstr = NULL;

/* Setup/teardown for this suite */
static int
init_suite(void)
{
    GhtErr result;
    schema = NULL;
    xmlstr = file_to_str(xmlfile);
    result = ght_schema_from_xml_str(xmlstr, &schema);

    return result;
}

static int
clean_suite(void)
{
    if ( schema )
        ght_schema_free(schema);
        
    if ( xmlstr ) 
        ght_free(xmlstr);
        
    return 0;
}


/* TESTS **************************************************************/

static void
test_schema_from_xml()
{
//    ght_schema_free(schema);
//    ght_free(xmlstr);

    // char *schemastr = ght_schema_to_json(schema);
    // printf("ndims %d\n", schema->ndims);
    // printf("name0 %s\n", schema->dims[0]->name);
    // printf("%s\n", schemastr);

//  CU_ASSERT(schema != NULL);
}

static void
test_schema_size()
{
//  size_t sz = schema->size;
//  CU_ASSERT_EQUAL(sz, 37);
}



/* REGISTER ***********************************************************/

CU_TestInfo schema_tests[] =
{
    GHT_TEST(test_schema_from_xml),
    GHT_TEST(test_schema_size),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo schema_suite = {"schema", init_suite, clean_suite, schema_tests};
