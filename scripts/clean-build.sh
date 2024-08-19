#!/bin/bash
rm -rf build generated
premake5 gmake
make -j2 -C generated/ $@
