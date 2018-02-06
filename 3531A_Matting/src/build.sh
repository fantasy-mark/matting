#!/bin/sh

export PATH="/opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/bin:$PATH" 
export QT_DIR="/opt/qt4.8.6-Hi3531A-v400"

ROOT=`dirname "$0"`

make clean -C $ROOT/log
make -C $ROOT/log

make clean -C $ROOT/matting
make -C $ROOT/matting

#copy to board
echo '\033[32m' && md5sum $ROOT/matting/director && echo '\033[0m\c'
/bin/cp $ROOT/matting/director ~/share
echo '\033[31m\c' && md5sum ~/share/director && echo '\033[0m'
