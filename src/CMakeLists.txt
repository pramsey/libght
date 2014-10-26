#------------------------------------------------------------------------------
# library build
#------------------------------------------------------------------------------

set ( GHT_SOURCES
	ght_attribute.c	
	ght_hash.c	
	ght_mem.c	
	ght_node.c	
	ght_schema.c	
	ght_serialize.c	
	ght_tree.c
	ght_util.c
	ght_bytebuffer.c
	ght_stringbuffer.c
	)

set (GHT_HEADERS
	ght.h		
	ght_config.h	
	ght_internal.h	
	ght_bytebuffer.h
	ght_stringbuffer.h
	ght_stdint.h	
	)

add_library (libght SHARED ${GHT_SOURCES} ${GHT_HEADERS})
add_library (libght-static STATIC ${GHT_SOURCES} ${GHT_HEADERS})

set_target_properties (libght 
	PROPERTIES
		OUTPUT_NAME "ght"
		PREFIX "lib"
		CLEAN_DIRECT_OUTPUT 1
		DEFINE_SYMBOL GHT_DLL_EXPORT
		VERSION ${GHT_VERSION}
		SOVERSION ${GHT_LIB_SOVERSION}
		CLEAN_DIRECT_OUTPUT 1
	)

set_target_properties (libght-static 
	PROPERTIES
		OUTPUT_NAME "ght"
		PREFIX "lib"
		CLEAN_DIRECT_OUTPUT 1
	)

target_link_libraries (libght xml2)
target_link_libraries (libght-static xml2)

install (TARGETS libght DESTINATION ${LIB_INSTALL_DIR})
install (TARGETS libght-static DESTINATION ${LIB_INSTALL_DIR})

install (FILES "${PROJECT_SOURCE_DIR}/src/ght.h" DESTINATION include)

install (FILES "${PROJECT_SOURCE_DIR}/src/ght_core.h" DESTINATION include)

