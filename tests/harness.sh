#!/bin/bash

cd $SCRATCH
execdir=/home/njeffrie/adaptive_lock/tests
exe=lock_test

cp ${execdir}/${exe} ${exe}

./${exe}
