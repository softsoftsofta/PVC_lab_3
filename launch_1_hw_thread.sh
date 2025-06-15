#!/bin/bash

sed -i "5,+0 s|.*|export HW_THREADS_CONFIG=1|g" launch.sh
./launch.sh
