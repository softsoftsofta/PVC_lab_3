#!/bin/bash

sed -i "5,+0 s|.*|export HW_THREADS_CONFIG=8|g" launch.sh
./launch.sh
