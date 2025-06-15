#!/bin/bash

sed -i "5,+0 s|.*|export HW_THREADS_CONFIG=32|g" launch.sh
./launch.sh
