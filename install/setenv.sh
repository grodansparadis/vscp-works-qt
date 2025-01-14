#!/bin/bash

QT_BASE_DIR=~/Qt/6.8.1/
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH

if [[ $(uname -m) == "x86_64" ]]; then
  export LD_LIBRARY_PATH=$QT_BASE_DIR/gcc_64/lib/:$QT_BASE_DIR/:$LD_LIBRARY_PATH
else
  export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/i386-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
fi

export PKG_CONFIG_PATH=$QT_BASE_DIR/gcc_64/lib/pkgconfig:$PKG_CONFIG_PATH

TEST=`echo $0 | grep wrapper`
if [ "$TEST" != "" ]; then
   exec `echo $0 | sed s/-wrapper//` $*
fi