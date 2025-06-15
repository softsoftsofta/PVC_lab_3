#!/bin/bash

module load mpi/openmpi-x86_64
mpicc main.c -o main
