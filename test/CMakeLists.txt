#------------------------------------------------------------------------------
# test suite build
#------------------------------------------------------------------------------

file(GLOB_RECURSE GHT_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE GHT_TEST_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

include_directories ("${PROJECT_SOURCE_DIR}/src")

add_executable(cu_tester ${GHT_TEST_SOURCES} ${GHT_TEST_HEADERS})
target_link_libraries (cu_tester libght-static cunit)
