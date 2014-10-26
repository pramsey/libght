#------------------------------------------------------------------------------
# basics
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.7)

project (GHT)

#------------------------------------------------------------------------------
# versions
#------------------------------------------------------------------------------

set (GHT_VERSION_MAJOR 0)
set (GHT_VERSION_MINOR 1)
set (GHT_VERSION_PATCH 1)

# full path for identification
set (GHT_VERSION "${GHT_VERSION_MAJOR}.${GHT_VERSION_MINOR}.${GHT_VERSION_PATCH}")

# API breaking changes can happen at minor version update
set (GHT_LIB_SOVERSION "${GHT_VERSION_MAJOR}.${GHT_VERSION_MINOR}")

#------------------------------------------------------------------------------
# internal cmake settings
#------------------------------------------------------------------------------

set (CMAKE_COLOR_MAKEFILE ON)

# Path to additional CMake modules
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})


#------------------------------------------------------------------------------
# use default install location if not specified
#------------------------------------------------------------------------------

if (NOT DEFINED LIB_INSTALL_DIR)
  set (LIB_INSTALL_DIR lib)
# Override this default 'lib' with 'lib64' if:
#  - we are on Linux system but NOT cross-compiling
#  - we are NOT on debian
#  - we are on a 64 bits system
# reason is: amd64 ABI: http://www.x86-64.org/documentation/abi.pdf
# Note that the future of multi-arch handling may be even
# more complicated than that: http://wiki.debian.org/Multiarch
  if(CMAKE_SYSTEM_NAME MATCHES "Linux"
      AND NOT CMAKE_CROSSCOMPILING
      AND NOT EXISTS "/etc/debian_version")
    if(NOT DEFINED CMAKE_SIZEOF_VOID_P)
      message(AUTHOR_WARNING
        "Unable to determine default CMAKE_INSTALL_LIBDIR directory because no target architecture is known. "
        "Please enable at least one language before including GNUInstallDirs.")
    else()
      if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(LIB_INSTALL_DIR "lib64")
      endif()
    endif()
  endif()
  set(CMAKE_INSTALL_LIBDIR "${LIB_INSTALL_DIR}" CACHE PATH "object code libraries (${LIB_INSTALL_DIR})")
endif()


SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#SET(CMAKE_MACOSX_RPATH ON)

#------------------------------------------------------------------------------
# check for headers
#------------------------------------------------------------------------------

include (CheckIncludeFiles)

check_include_files (stdint.h HAVE_STDINT_H)
check_include_files (getopt.h HAVE_GETOPT_H)

#------------------------------------------------------------------------------
# all the tools use the API
#------------------------------------------------------------------------------

# ght.h is in the src directory
include_directories ("${PROJECT_SOURCE_DIR}/src")

#------------------------------------------------------------------------------
# must find libxml2
#------------------------------------------------------------------------------

find_package (LibXml2 REQUIRED)
mark_as_advanced (CLEAR LIBXML2_INCLUDE_DIR)
mark_as_advanced (CLEAR LIBXML2_LIBRARIES)
include_directories (${LIBXML2_INCLUDE_DIR})

#------------------------------------------------------------------------------
# need libLAS and Proj4 for file translation tools
#------------------------------------------------------------------------------

find_package (LibLAS)
find_package (PROJ4)
find_package (CUnit)

#------------------------------------------------------------------------------
# generate config include
#------------------------------------------------------------------------------

configure_file (
  "${PROJECT_SOURCE_DIR}/src/ght_config.h.in"
  "${PROJECT_BINARY_DIR}/src/ght_config.h"
  )

include_directories ("${PROJECT_BINARY_DIR}/src")

#------------------------------------------------------------------------------
# testing
#------------------------------------------------------------------------------

enable_testing()

#------------------------------------------------------------------------------
# build the library, then the test suite
#------------------------------------------------------------------------------

add_subdirectory (src) 
add_subdirectory (test) 
add_subdirectory (tools) 

