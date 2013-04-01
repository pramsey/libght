GeoHashTree for Point Cloud Data
================================

A file format and library for storing and rapidly accessing point cloud data, in particular LIDAR data.

GeoHashTree organizes points into a tree structure for fast spatial access. The tree structure itself encodes the significant bits of at each node, so child nodes can omit them. The result is a smaller file than if all the points were stored with full precision. Each node includes statistical information about the children below (e.g. average/median Z value) permitting fast overview generation. Additional attributes are attached to the tree at parent nodes, below which all children share the attribute value. This reduces duplicate data storage further.

The advantage of a GeoHashTree file over a LAS file is fast access and filtering, since the tree encodes useful information at each node to speed searches over the full set of points in the file. LASZ zipped files can be smaller, but will be less efficient at overviews, searching and sub-setting. GHT is a good working format for applications that will be filtering and querying large sets of LIDAR data.

Requires
========

- [LibXML2](http://www.xmlsoft.org/downloads.html) for handling schema documents
- [CUnit](http://cunit.sourceforge.net/) for running unit tests
- [CMake](http://www.cmake.org/cmake/resources/software.html) to build

Build
========

- LibXML2 for handling schema documents
- CUnit for running unit tests

Utilities
=========

TBD