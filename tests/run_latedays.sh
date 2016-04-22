#!/bin/bash

export OMP_NUM_THREADS=10
rm harness.sh.*

qsub harness.sh -n
