#!/bin/sh
# This script will attempt to migrate all files from rocksdb as can be
# downloaded from https://github.com/facebook/rocksdb/releases

# PRECONDITION: $PWD=/path/to/gorocksdb

if [ "$1" == "" ]; then
  echo "give directory for rocksdb source as arg"
  exit 2
fi

rocksdir=$1 # root of rocksdb, relative or abs

cd $rocksdir
make rocksdb.cc
cp rocksdb.cc rocksdb.h ..
cp includes/rocksdb/c.h ../rocksdb
