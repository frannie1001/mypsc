
#include "../psc_push_particles/inc_defs.h"

#define DIM DIM_YZ
#define BLOCKSIZE_X 1
#define BLOCKSIZE_Y 4
#define BLOCKSIZE_Z 4
#define EM_CACHE EM_CACHE_NONE
#define CALC_J CALC_J_1VB_SPLIT

#define SFX(s) s ## _yz

#include "cuda2_push_mprts.cu"

