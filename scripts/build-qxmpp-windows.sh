#!/bin/bash

SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"

${SCRIPT_FOLDER}/build-qxmpp.sh /opt/Qt/5.12.6/mingw32 " \
        -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-mingw32.cmake  \
        -DCYGWIN=1\
    "
