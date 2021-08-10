#include "hip/hip_runtime.h"
/*
 * test_grid.cu
 *
 *  Created on: Dec 20, 2018
 *      Author: dkoes
 */

#define BOOST_TEST_MODULE grid_cuda_test
#include <boost/test/unit_test.hpp>
#include <thrust/reduce.h>
#include <thrust/execution_policy.h>

#include "libmolgrid/grid.h"

using namespace libmolgrid;


BOOST_AUTO_TEST_CASE( constructors )
{
  //this is mostly a compilation test
  const int SIZE = 256*8;
  float *f = NULL;
  double *d = NULL;
  hipMalloc(&f, SIZE);
  hipMemset(f, 0, SIZE);
  hipMalloc(&d, SIZE);
  hipMemset(d, 0, SIZE);

  Grid1fCUDA g1f(f, 100);
  Grid2fCUDA g2f(f, 100, 1);
  Grid4fCUDA g4f(f, 2, 1, 2, 25);
  Grid6fCUDA g6f(f, 1,2,1,2,25,1);

  Grid1dCUDA g1d(d,3);
  Grid2dCUDA g2d(d,64,2);
  Grid4dCUDA g4d(d,2,2,2,16);

  BOOST_CHECK_EQUAL(g1f.data(),g6f.data());
  BOOST_CHECK_EQUAL(g2d.data(),g4d.data());
  BOOST_CHECK_NE((void*)g4f.data(),(void*)g4d.data());

  hipFree(f);
  hipFree(d);

  hipError_t error = hipGetLastError();
  BOOST_CHECK_EQUAL(error,hipSuccess);
}

__global__ void copyFtoD(Grid2fCUDA f, Grid2dCUDA d) {
  d(blockIdx.x,threadIdx.x) = f(blockIdx.x,threadIdx.x);
}

BOOST_AUTO_TEST_CASE( direct_indexing )
{
  const int N = 256;
  float *f = NULL;
  double *d = NULL;
  hipMalloc(&f, N*sizeof(float));
  hipMemset(f, 0, N*sizeof(float));
  hipMalloc(&d, N*sizeof(double));
  hipMemset(d, 0, N*sizeof(double));

  float fhost[N] = {-1,};
  double dhost[N] = {-1,};

  //write to f, have a kernel copy to d using indexing, check
  for(unsigned i = 0; i < N; i++) {
    fhost[i] = i;
  }
  hipMemcpy(f,fhost,N*sizeof(float),hipMemcpyHostToDevice);

  Grid2fCUDA F(f, 8, 32);
  Grid2dCUDA D(d, 8, 32);

  hipLaunchKernelGGL(copyFtoD, dim3(8), dim3(32), 0, 0, F,D);

  hipMemcpy(dhost,d,N*sizeof(double), hipMemcpyDeviceToHost);

  for(unsigned i = 0; i < N; i++) {
    BOOST_CHECK_EQUAL(dhost[i], i);
  }

  hipError_t error = hipGetLastError();
  BOOST_CHECK_EQUAL(error,hipSuccess);
}

__global__ void copyFtoD3(Grid3fCUDA f, Grid3dCUDA d) {
  d[threadIdx.x][threadIdx.y][threadIdx.z] = f[threadIdx.x][threadIdx.y][threadIdx.z];
}

__global__ void copyFtoD1(Grid1fCUDA f, Grid1dCUDA d) {
  d[threadIdx.x] = f[threadIdx.x];
}

BOOST_AUTO_TEST_CASE( indirect_indexing )
{
  const int N = 256;
  float *f = NULL;
  double *d = NULL;
  hipMalloc(&f, N*sizeof(float));
  hipMemset(f, 0, N*sizeof(float));
  hipMalloc(&d, N*sizeof(double));
  hipMemset(d, 0, N*sizeof(double));

  float fhost[N] = {-1,};
  double dhost[N] = {-1,};

  //write to f, have a kernel copy to d using indexing, check
  for(unsigned i = 0; i < N; i++) {
    fhost[i] = i;
  }
  hipMemcpy(f,fhost,N*sizeof(float),hipMemcpyHostToDevice);

  Grid3fCUDA F(f, 8, 16, 2);
  Grid3dCUDA D(d, 8, 16, 2);

  dim3 threads(8,16,2);
  hipLaunchKernelGGL(copyFtoD3, dim3(1), dim3(threads), 0, 0, F,D);

  hipMemcpy(dhost,d,N*sizeof(double), hipMemcpyDeviceToHost);

  for(unsigned i = 0; i < N; i++) {
    BOOST_CHECK_EQUAL(dhost[i], i);
  }

  Grid1fCUDA F1 = F[3][5];
  hipMemset(F1.data(), 0, F1.dimensions()[0]*sizeof(float));

  Grid1dCUDA D1 = D[0][0];

  hipLaunchKernelGGL(copyFtoD1, dim3(1), dim3(2), 0, 0, F1,D1);

  float fsum = thrust::reduce(thrust::device, f, f+256);
  double dsum = thrust::reduce(thrust::device, d, d+256);

  BOOST_CHECK_EQUAL(fsum,32427);
  BOOST_CHECK_EQUAL(dsum,32639);
  hipFree(f);
  hipFree(d);
  hipError_t error = hipGetLastError();
  BOOST_CHECK_EQUAL(error,hipSuccess);
}

