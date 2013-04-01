#!/bin/sh

find . -type f -name CMakeCache.txt -exec rm {} ';'
find . -type d -name CMakeFiles -exec rm -r {} ';'
find . -type f -name cmake_install.cmake -exec rm {} ';'
find . -type f -name Makefile -exec rm {} ';'


