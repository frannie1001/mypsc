
#include "ggcm_mhd_bnd_private.h"

#include "ggcm_mhd_defs.h"
#include "ggcm_mhd_private.h"

#include <mrc_profile.h>
#include <mrc_fld_as_double_aos.h>

#include <assert.h>

#define MT MT_FULLY_CONSERVATIVE
#define SHIFT 0

#define ggcm_mhd_bnd_ops_inoutflow ggcm_mhd_bnd_ops_inoutflow_fc_double_aos
#define ggcm_mhd_bnd_sub_name "inoutflow_fc_double_aos"

#include "ggcm_mhd_bnd_inoutflow_common.c"