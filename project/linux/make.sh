#!/bin/bash

make clean

cp -rf Makefile_with_c++0x Makefile

make

cp -rf Makefile_without_c++0x Makefile

make

cp -rf Makefile_with_c++0x Makefile

make
