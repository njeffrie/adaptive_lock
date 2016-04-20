#!/bin/bash

cd $SCRATCH
execdir=/home/njeffrie/adaptive_lock/tests
exe=ticket_lock_basic

cp ${execdir}/${exe} ${exe}

./${exe}
