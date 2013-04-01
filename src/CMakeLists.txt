#------------------------------------------------------------------------------
# library build
#------------------------------------------------------------------------------

configure_file ("ght_config.h.in" "ght_config.h")

file(GLOB_RECURSE GHT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE GHT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(libght SHARED ${GHT_SOURCES} ${GHT_HEADERS})
add_library(libght-static STATIC ${GHT_SOURCES} ${GHT_HEADERS})

set_target_properties(libght 
	PROPERTIES
		OUTPUT_NAME "ght"
		PREFIX "lib"
		CLEAN_DIRECT_OUTPUT 1
		DEFINE_SYMBOL GHT_DLL_EXPORT
		VERSION ${GHT_VERSION}
		CLEAN_DIRECT_OUTPUT 1
	)

set_target_properties(libght-static 
	PROPERTIES
		OUTPUT_NAME "ght"
		PREFIX "lib"
		CLEAN_DIRECT_OUTPUT 1
	)

target_link_libraries(libght xml2)
target_link_libraries(libght-static xml2)

install (TARGETS libght DESTINATION lib)
install (TARGETS libght-static DESTINATION lib)
install (FILES "${PROJECT_SOURCE_DIR}/src/ght.h" DESTINATION include)
