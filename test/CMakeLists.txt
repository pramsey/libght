#------------------------------------------------------------------------------
# test suite build
#------------------------------------------------------------------------------

set (GHT_TEST_SOURCES 
	cu_ght_core.c
	cu_ght_schema.c
	cu_ght_attribute.c
	cu_ght_tree.c
	cu_tester.c
	)

set (GHT_TEST_HEADERS
	cu_tester.h
	)

set (GHT_TEST_DATA
	data/pdal-schema.xml
	data/simple-schema.xml
	)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${CUNIT_INCLUDE_DIR}")

add_executable(cu_tester ${GHT_TEST_SOURCES} ${GHT_TEST_HEADERS})
target_link_libraries (cu_tester libght-static ${CUNIT_LIBRARIES})

add_test(cu_tester cu_tester)

#install (FILES "${GHT_TEST_DATA}" DESTINATION share)
