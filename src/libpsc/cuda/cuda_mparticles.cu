
#include "cuda_mparticles.h"

#include <cstdio>
#include <cassert>

#define cudaCheck(ierr) do {						\
    if (ierr != cudaSuccess)						\
      fprintf(stderr, "IERR = %d (%s)\n", ierr, cudaGetErrorName(ierr)); \
    assert(ierr == cudaSuccess);					\
  } while(0)

// ----------------------------------------------------------------------
// cuda_mparticles_create

struct cuda_mparticles *
cuda_mparticles_create()
{
  struct cuda_mparticles *cmprts = 
    (struct cuda_mparticles *) calloc(1, sizeof(*cmprts));

  return cmprts;
}

// ----------------------------------------------------------------------
// cuda_mparticles_destroy

void
cuda_mparticles_destroy(struct cuda_mparticles *cmprts)
{
  free(cmprts);
}

// ----------------------------------------------------------------------
// cuda_mparticles_set_domain_info

void
cuda_mparticles_set_domain_info(struct cuda_mparticles *cuda_mprts,
				const struct cuda_domain_info *info)
{
  cuda_mprts->n_patches = info->n_patches;
  for (int d = 0; d < 3; d++) {
    cuda_mprts->mx[d] = info->mx[d];
    cuda_mprts->b_mx[d] = info->mx[d] / info->bs[d];
    cuda_mprts->dx[d] = info->dx[d];
    cuda_mprts->b_dxi[d] = 1.f / (info->bs[d] * info->dx[d]);
  }
  // cuda_mprts->nr_blocks_per_patch =
  //   cuda_mprts->b_mx[0] * cuda_mprts->b_mx[1] * cuda_mprts->b_mx[2];
  // cuda_mprts->nr_blocks = info->nr_patches * cuda_mprts->nr_blocks_per_patch;
}

// ----------------------------------------------------------------------
// cuda_mparticles_alloc

void
cuda_mparticles_alloc(struct cuda_mparticles *cmprts, unsigned int *n_prts_by_patch)
{
  cudaError_t ierr;

  cmprts->n_prts = 0;
  for (int p = 0; p < cmprts->n_patches; p++) {
    cmprts->n_prts += n_prts_by_patch[p];
  }

  cmprts->n_alloced = cmprts->n_prts * 1.4;
  unsigned int n_alloced = cmprts->n_alloced;

  ierr = cudaMalloc((void **) &cmprts->d_xi4, n_alloced * sizeof(float4)); cudaCheck(ierr);
  ierr = cudaMalloc((void **) &cmprts->d_pxi4, n_alloced * sizeof(float4)); cudaCheck(ierr);
  ierr = cudaMalloc((void **) &cmprts->d_alt_xi4, n_alloced * sizeof(float4)); cudaCheck(ierr);
  ierr = cudaMalloc((void **) &cmprts->d_alt_pxi4, n_alloced * sizeof(float4)); cudaCheck(ierr);
  ierr = cudaMalloc((void **) &cmprts->d_bidx, n_alloced * sizeof(unsigned int)); cudaCheck(ierr);
  ierr = cudaMalloc((void **) &cmprts->d_id, n_alloced * sizeof(unsigned int)); cudaCheck(ierr);

  ierr = cudaMalloc(&cmprts->d_n_prts_by_patch, cmprts->n_patches * sizeof(unsigned int)); cudaCheck(ierr);
}

// ----------------------------------------------------------------------
// cuda_mparticles_dealloc

void
cuda_mparticles_dealloc(struct cuda_mparticles *cmprts)
{
  cudaError_t ierr;

  ierr = cudaFree(cmprts->d_xi4); cudaCheck(ierr);
  ierr = cudaFree(cmprts->d_pxi4); cudaCheck(ierr);
  ierr = cudaFree(cmprts->d_alt_xi4); cudaCheck(ierr);
  ierr = cudaFree(cmprts->d_alt_pxi4); cudaCheck(ierr);
  ierr = cudaFree(cmprts->d_bidx); cudaCheck(ierr);
  ierr = cudaFree(cmprts->d_id); cudaCheck(ierr);

  ierr = cudaFree(cmprts->d_n_prts_by_patch); cudaCheck(ierr);
}

// ----------------------------------------------------------------------
// cuda_mparticles_swap_alt

void
cuda_mparticles_swap_alt(struct cuda_mparticles *cmprts)
{
  float4 *tmp_xi4 = cmprts->d_alt_xi4;
  float4 *tmp_pxi4 = cmprts->d_alt_pxi4;
  cmprts->d_alt_xi4 = cmprts->d_xi4;
  cmprts->d_alt_pxi4 = cmprts->d_pxi4;
  cmprts->d_xi4 = tmp_xi4;
  cmprts->d_pxi4 = tmp_pxi4;
}

