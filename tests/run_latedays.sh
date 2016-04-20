#!/bin/bash

export OMP_NUM_THREADS=24
rm harness.sh.*

qsub harness.sh -n
