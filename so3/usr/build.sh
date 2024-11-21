#!/bin/bash

function usage {
  echo "$0 [OPTIONS]"
  echo "  -c        Clean"
  echo "  -d        Debug build"
  echo "  -v        Verbose"
  echo "  -s        Single core"
  echo "  -h        Print this help"
}

function install_file_elf {
  [ -f $1 ] && echo "Installing $1" && mv $BUILDDIR/src/*.elf $BUILDDIR/deploy && mv $BUILDDIR/src/**/*.elf $BUILDDIR/deploy
}

function install_file_root {
  [ -f $1 ] && echo "Installing $1" && cp $1 $BUILDDIR/deploy
}

function install_directory_root {
  [ -d $1 ] && echo "Installing $1" && cp -R $1 $BUILDDIR/deploy
}

function install_file_directory {
  [ -f $1 ] && echo "Installing $1 into $2" && mkdir -p $BUILDDIR/deploy/$2 && cp $1 $BUILDDIR/deploy/$2
}


while read var; do
if [ "$var" != "" ]; then
  export $(echo $var | sed -e 's/ //g' -e /^$/d -e 's/://g' -e /^#/d)
fi
done < ../../build.conf

echo Platform is ${PLATFORM}

clean=n
debug=y
verbose=n
singlecore=n

while getopts cdhvs option
  do
    case "${option}"
      in
        c) clean=y;;
        d) debug=y;;
	    v) verbose=y;;
        s) singlecore=y;;
        h) usage && exit 1;;
    esac
  done

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`
BUILDDIR=$SCRIPTPATH/build/$PLATFORM

if [ $clean == y ]; then
  echo "Cleaning $BUILDDIR"
  rm -rf $BUILDDIR
  exit
fi

if [ $debug == y ]; then
  build_type="Debug"
else
  build_type="Release"
fi

echo "Starting $build_type build"
mkdir -p $BUILDDIR

cd $BUILDDIR

if [ "$PLATFORM" == "virt32" -o "$PLATFORM" == "rpi4" ]; then
cmake -Wno-dev --no-warn-unused-cli -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_TOOLCHAIN_FILE=$SCRIPTPATH/arm_toolchain.cmake $SCRIPTPATH
fi
if [ "$PLATFORM" == "virt64" -o "$PLATFORM" == "rpi4_64" ]; then
cmake -Wno-dev --no-warn-unused-cli -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_TOOLCHAIN_FILE=$SCRIPTPATH/aarch64_toolchain.cmake $SCRIPTPATH
fi
if [ $singlecore == y ]; then
    NRPROC=1
else
    NRPROC=$((`cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l` + 1))
fi
if [ $verbose == y ]; then
	make VERBOSE=1 -j1
else
	make -j$NRPROC
fi
cd -


mkdir -p $BUILDDIR/deploy/

# SO3 shell
install_directory_root usr/out

install_file_elf
