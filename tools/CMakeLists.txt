#------------------------------------------------------------------------------
# file and loading tools build
#   can only build the las tools if liblas is found
#------------------------------------------------------------------------------

include_directories ("${PROJECT_SOURCE_DIR}/src")

if (LIBLAS_FOUND AND PROJ4_FOUND)

  set (LAS2GHT_SOURCES 
  	las2ght.c
  	)

  include_directories ("${LIBLAS_INCLUDE_DIR}")
  include_directories ("${PROJ4_INCLUDE_DIR}")
  add_executable(las2ght ${LAS2GHT_SOURCES} ${LAS2GHT_HEADERS})
  target_link_libraries (las2ght libght-static las_c proj)
  install (PROGRAMS las2ght DESTINATION bin)
  MESSAGE(STATUS "las2ght build enabled")

else()

  MESSAGE(STATUS "las2ght build DISABLED")
  MESSAGE(STATUS "las2ght utility requires LibLAS and Proj4")

endif ()
