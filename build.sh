#!/bin/bash

pushd ./build

gcc -fms-extensions ../src/main.c

popd
