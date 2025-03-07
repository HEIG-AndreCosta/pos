#!/bin/bash

set -e

while read var; do
if [ "$var" != "" ]; then
  export $(echo $var | sed -e 's/ //g' -e /^$/d -e 's/://g' -e /^#/d)
fi
# The following wizard ensure there is a final return line as read need it
done < <(cat ../build.conf; echo)

echo "******** Running for $PLATFORM ********"

SO3_SRC=../so3/so3
AVZ_TARGET=../../avz/build/$PLATFORM

clean=n
verbose=n

function usage {
  echo "$0 [OPTIONS]"
  echo "  -c        Clean"
  echo "  -v        Verbose"
  echo "  -h        Print this help"
}

while getopts chv option
  do
    case "${option}"
      in
        c) clean=y;;
        v) set -- ""; verbose=y;;
        h) usage && exit 1;;
    esac
  done

if [ $clean == y ]; then
  echo "Cleaning AVZ"
  cd $SO3_SRC
  make O=$AVZ_TARGET distclean
  rm $AVZ_TARGET/Makefile
  exit
fi

if [ ! -z $1 ]; then
  echo "Configuring AVZ with config: $1"
  cd $SO3_SRC
  make O=$AVZ_TARGET  $1
  exit
fi

# Default target
cd $SO3_SRC
if [ $verbose == y ]; then
    make O=$AVZ_TARGET  V=1
else
    make O=$AVZ_TARGET
fi
