#!/bin/bash

export ARRAY_SIZE_CONFIG=2000
export RUNS_CONFIG=10
export HW_THREADS_CONFIG=1

sed -i "3,+0 s|.*|export ARRAY_SIZE=$ARRAY_SIZE_CONFIG|g" serial.lsf
sed -i "4,+0 s|.*|export RUNS=$RUNS_CONFIG|g" serial.lsf
sed -i "5,+0 s|.*|export HW_THREADS=1|g" serial.lsf

sed -i "3,+0 s|.*|export ARRAY_SIZE=$ARRAY_SIZE_CONFIG|g" parallel.lsf
sed -i "4,+0 s|.*|export RUNS=$RUNS_CONFIG|g" parallel.lsf
sed -i "5,+0 s|.*|export HW_THREADS=$HW_THREADS_CONFIG|g" parallel.lsf
sed -i "9,+0 s|.*|#BSUB -n $HW_THREADS_CONFIG|g" parallel.lsf
sed -i "11,+0 s|.*|#BSUB -oo logs/output_$HW_THREADS_CONFIG.out|g" parallel.lsf
sed -i "12,+0 s|.*|#BSUB -eo logs/error_$HW_THREADS_CONFIG.err|g" parallel.lsf

if [ $HW_THREADS_CONFIG -lt 2 ]; then

    if ! command -v bsub > /dev/null; then
        echo "Не получилось найти BSUB, запускаем напрямую"
        ./serial.lsf
    else
        bsub < ./serial.lsf
    fi

else

    if ! command -v bsub > /dev/null; then
        echo "Не получилось найти BSUB, запускаем напрямую"
        ./parallel.lsf
    else
        bsub < ./parallel.lsf
    fi
    
fi
