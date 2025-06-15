#pragma once
#ifndef VEC_OPER_H
#define VEC_OPER_H

#include "config.h"
#include "random.h"

#include <stdint.h>
#include <stddef.h>

#include <mpi.h>

#define MPI_SYNC(arr, rank, mpi_size, arr_size, width) {\
  if(rank == 0)\
    for(uint32_t i=1; i<mpi_size; ++i)\
      MPI_Recv(&arr[i*width], MIN(width, arr_size - rank*width), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);\
  else\
    MPI_Send(arr, MIN(width, arr_size - rank*width), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);\
}

#define MPI_BROADCAST(arr, arr_size, rank, source, mpi_size, width) {\
  if(rank == source) {\
    for(uint32_t i=0; i<mpi_size; ++i)\
      if(i != source)\
        MPI_Send(&arr[i*width], MIN(width, arr_size - i*width), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);\
  } else\
    MPI_Recv(arr, MIN(width, arr_size - rank*width), MPI_UNSIGNED_CHAR, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);\
}

void Summ(arr_t* arr1, arr_t* arr2, arr_t* arr_out, const uint32_t size);
void Diff(arr_t* arr1, arr_t* arr2, arr_t* arr_out, const uint32_t size);
void Mult(arr_t* arr1, arr_t* arr2, arr_t* arr_out, const uint32_t size);
void Div(arr_t* arr1, arr_t* arr2, arr_t* arr_out, const uint32_t size);

#endif // !VEC_OPER_H
