#ifndef __EXCLUSIVE_SCAN__
#define __EXCLUSIVE_SCAN__

#include <iostream>
#include <cuda_runtime_api.h>

template <int thread_block>
__device__ __inline__ int exclusive_scan(uint element, uint *sum)
{

    __shared__ uint buffer[thread_block];
    __shared__ uint block_offset;

    if (threadIdx.x == 0)
        memset(buffer, 0, sizeof(uint) * 16 * 16);

    __syncthreads();

    buffer[threadIdx.x] = element;

    __syncthreads();

    int s1, s2;

    for (s1 = 1, s2 = 1; s1 < thread_block; s1 <<= 1)
    {
        s2 |= s1;
        if ((threadIdx.x & s2) == s2)
            buffer[threadIdx.x] += buffer[threadIdx.x - s1];

        __syncthreads();
    }

    for (s1 >>= 2, s2 >>= 1; s1 >= 1; s1 >>= 1, s2 >>= 1)
    {
        if (threadIdx.x != thread_block - 1 && (threadIdx.x & s2) == s2)
            buffer[threadIdx.x + s1] += buffer[threadIdx.x];

        __syncthreads();
    }

    if (threadIdx.x == 0 && buffer[thread_block - 1] > 0)
        block_offset = atomicAdd(sum, buffer[thread_block - 1]);

    __syncthreads();

    int offset;
    if (threadIdx.x == 0)
    {
        if (buffer[threadIdx.x] == 0)
            offset = -1;
        else
            offset = block_offset;
    }
    else
    {
        if (buffer[threadIdx.x] == buffer[threadIdx.x - 1])
            offset = -1;
        else
            offset = block_offset + buffer[threadIdx.x - 1];
    }

    return offset;
}

#endif