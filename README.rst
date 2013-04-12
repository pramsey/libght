GeoHashTree for Point Cloud Data
================================

A file format and library for storing and rapidly accessing point cloud data, in particular LIDAR data.

GeoHashTree organizes points into a tree structure for fast spatial access. The tree structure itself encodes the significant bits of at each node, so child nodes can omit them. The result is a smaller file than if all the points were stored with full precision. Each node includes statistical information about the children below (e.g. average/median Z value) permitting fast overview generation. Additional attributes are attached to the tree at parent nodes, below which all children share the attribute value. This reduces duplicate data storage further.

The advantage of a GeoHashTree file over a LAS file is fast access and filtering, since the tree encodes useful information at each node to speed searches over the full set of points in the file. LASZ zipped files can be smaller, but will be less efficient at overviews, searching and sub-setting. GHT is a good working format for applications that will be filtering and querying large sets of LIDAR data.

Requires
========

- `LibXML2 <http://www.xmlsoft.org/downloads.html>`_ for handling schema documents
- `CUnit <http://cunit.sourceforge.net/>`_ for running unit tests
- `CMake <http://www.cmake.org/cmake/resources/software.html>`_ to build

Build
=====

CMake prefers builds "out of source", where all the generated files are created separately from the source directory. To make this happen, create a build directory, enter it, then invoke `cmake` with the source directory as the target argument. 

UNIX
----

::

    mkdir libght-build
    cd libght-build
    cmake ../libght-src
    make
    make test
    make install

Windows
-------

::

    mkdir libght-build
    cd libght-build
    cmake -G "NMake Makefiles" ..\libght-src
    nmake
    nmake install

To Do
=====

- File format header structure
  - Endianness flag (die if mismatch)
  - Version number (support legacy)

