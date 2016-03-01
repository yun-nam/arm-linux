#!/bin/bash

echo "NOTE -- you should build this as ARM binaries"

ARCH=`uname -m`
if [ "$ARCH" = "armv7l" ]
then
    CC=gcc
else
    CC=arm-linux-gnueabi-gcc
fi

$CC -marm prefetch.S pcrelative.S unifiedTest.c -o test

echo 'Finished building Tests.'
