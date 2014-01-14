#include "ggcm_mhd_step_cweno_private.h"

#include "ggcm_mhd_private.h"
#include "ggcm_mhd_crds.h"
#include "ggcm_mhd_diag.h"

#include <mrc_domain.h>
#include <mrc_ddc.h>
#include <mrc_io.h>

#include <assert.h>
#include <stdio.h>
#include <math.h>


static void
ggcm_mhd_step_cweno_calc_rhs(struct ggcm_mhd_step *step, struct mrc_fld *rhs,
			     struct mrc_fld *fld)
{
  struct ggcm_mhd *mhd = step->mhd;  
  
  //fill_ghost_fld(mhd, fld);

#ifdef DEBUG
  {
    static struct ggcm_mhd_diag *diag;
    static int cnt;
    if (!diag) {
      diag = ggcm_mhd_diag_create(ggcm_mhd_comm(mhd));
      ggcm_mhd_diag_set_param_obj(diag, "mhd", mhd);
      ggcm_mhd_diag_set_param_string(diag, "run", "fld");
      ggcm_mhd_diag_set_param_string(diag, "fields", "rr1:rv1:uu1:b1:divb:pp_full:b");
      ggcm_mhd_diag_setup(diag);
      ggcm_mhd_diag_view(diag);
    }
    ggcm_mhd_diag_run_now(diag, fld, DIAG_TYPE_3D, cnt++);
  }
#endif

  struct mrc_fld *flux[3];
  for (int f = 0; f < 3; f++) {
    flux[f] = ggcm_mhd_get_fields(mhd, "flux", 8);
  }
 
  calc_cweno_fluxes(mhd, flux, fld);

#if SEMICONSV   
  
  /*
  struct mrc_fld *J_cc = ggcm_mhd_get_fields(mhd, "J_cc", 3); 
  struct mrc_fld *E_cc = ggcm_mhd_get_fields(mhd, "E_cc", 3); 
  calc_jxB(mhd, jxB, J_cc);  
  calc_jdotE(mhd, jdotE, J_cc, E_cc);
  calc_sc_source_terms( , jxB, jdotE); 
  */
  calc_semiconsv_rhs(mhd, rhs, flux);//, fld); 
  
#else 
  calc_neg_divg(mhd, rhs, flux);
#endif 

  calc_ct_rhs(mhd, rhs, flux);
  struct mrc_fld *r = mrc_fld_get_as(rhs, "float");
  assert(mhd->fld->_data_type == MRC_NT_FLOAT);
  struct mrc_fld *f = mrc_fld_get_as(mhd->fld, mrc_fld_type(mhd->fld));

  mrc_fld_foreach(f, ix,iy,iz, 1, 1) {
    for (int m = 0; m < 8; m++) {
      MRC_F3(r, m, ix,iy,iz) *= MRC_F3(f, _YMASK, ix,iy,iz);
    }
  } mrc_fld_foreach_end;

  mrc_fld_put_as(r, rhs);
  mrc_fld_put_as(f, mhd->fld);


#ifdef DEBUG
  {
    static struct ggcm_mhd_diag *diag;
    static int cnt;
    if (!diag) {
      diag = ggcm_mhd_diag_create(ggcm_mhd_comm(mhd));
      ggcm_mhd_diag_set_param_obj(diag, "mhd", mhd);
      ggcm_mhd_diag_set_param_string(diag, "run", "rhs");
      ggcm_mhd_diag_set_param_string(diag, "fields", "rr1:rv1:uu1:b1:divb");
      ggcm_mhd_diag_setup(diag);
      ggcm_mhd_diag_view(diag);
    }
    ggcm_mhd_diag_run_now(diag, rhs, DIAG_TYPE_3D, cnt++);
  }
#endif

  for (int f = 0; f < 3; f++) {
    mrc_fld_destroy(flux[f]);
  }

}

// ----------------------------------------------------------------------
// ggcm_mhd_step subclass "cweno"

struct ggcm_mhd_step_ops ggcm_mhd_step_cweno_ops = {
  .name        = "cweno",
  .calc_rhs    = ggcm_mhd_step_cweno_calc_rhs,
};

