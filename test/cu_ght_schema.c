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
test_schema_xml()
{
    char *mystr, *str;
    GhtErr result;
    GhtSchema *myschema = NULL;

    result = ght_schema_to_xml_str(schema, &str);
    CU_ASSERT_EQUAL(result, GHT_OK);
    
    result = ght_schema_from_xml_str(str, &myschema);
    CU_ASSERT_EQUAL(result, GHT_OK);

    result = ght_schema_to_xml_str(myschema, &mystr);
    CU_ASSERT_EQUAL(result, GHT_OK);
    
    CU_ASSERT_STRING_EQUAL(str, mystr);
    ght_free(str);
    ght_free(mystr);
    ght_schema_free(myschema);
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
    GHT_TEST(test_schema_xml),
    CU_TEST_INFO_NULL
};

CU_SuiteInfo schema_suite = {"schema", init_suite, clean_suite, schema_tests};
