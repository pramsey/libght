/***********************************************************************
* cu_tester.h
*
*        Testing harness for libght
*
***********************************************************************/

#include "ght_internal.h"

#define GHT_TEST(test_func) { #test_func, test_func }

/* Read a file (XML) into a cstring */
char* file_to_str(const char *fname);

