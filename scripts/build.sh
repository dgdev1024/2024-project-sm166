#!/bin/bash
premake5 gmake
make -j2 -C generated/ $@
