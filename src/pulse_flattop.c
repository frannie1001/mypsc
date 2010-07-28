
#include "psc_pulse.h"

#include <math.h>

#define VAR(x) (void *)offsetof(struct psc_p_pulse_z1_flattop_param, x)

static struct param psc_p_pulse_z1_flattop_descr[] = {
  { "pulse_xm"      , VAR(xm)              , PARAM_DOUBLE(5.  * 1e-6)     },
  { "pulse_ym"      , VAR(ym)              , PARAM_DOUBLE(5.  * 1e-6)     },
  { "pulse_zm"      , VAR(zm)              , PARAM_DOUBLE(-4. * 1e-6)     },
  { "pulse_dxm"     , VAR(dxm)             , PARAM_DOUBLE(1.5 * 1e-6)     },
  { "pulse_dym"     , VAR(dym)             , PARAM_DOUBLE(1.5 * 1e-6)     },
  { "pulse_dzm"     , VAR(dzm)             , PARAM_DOUBLE(1.5 * 1e-6)     },
  { "pulse_zb"      , VAR(zb)              , PARAM_DOUBLE(1.5 * 1e-6)     },
  {},
};

#undef VAR

static void
psc_pulse_p_z1_flattop_setup(struct psc_pulse *pulse)
{
  struct psc_p_pulse_z1_flattop_param *prm = pulse->ctx;

  // normalization
  prm->xm /= psc.coeff.ld;
  prm->ym /= psc.coeff.ld;
  prm->zm /= psc.coeff.ld;
  prm->dxm /= psc.coeff.ld;
  prm->dym /= psc.coeff.ld;
  prm->dzm /= psc.coeff.ld;
  prm->zb /= psc.coeff.ld;
}

double
psc_pulse_p_z1_flattop(struct psc_pulse *pulse, 
		       double xx, double yy, double zz, double tt)
{
  struct psc_p_pulse_z1_flattop_param *prm = pulse->ctx;

  double xl = xx;
  double yl = yy;
  double zl = zz - tt;

  double xr = xl - prm->xm;
  double yr = yl - prm->ym;
  double zr = zl - prm->zm;

  return sin(zr)
    * exp(-sqr(xr/prm->dxm))
    * exp(-sqr(yr/prm->dym))
    * 1. / (1.+exp((fabs(zr)-prm->zb)/prm->dzm));
}

struct psc_pulse_ops psc_pulse_ops_p_z1_flattop = {
  .name       = "p_z1_flattop",
  .ctx_size   = sizeof(struct psc_p_pulse_z1_flattop_param),
  .ctx_descr  = psc_p_pulse_z1_flattop_descr,
  .setup      = psc_pulse_p_z1_flattop_setup,
  .field      = psc_pulse_p_z1_flattop,
};

struct psc_pulse *
psc_pulse_p_z1_flattop_create(struct psc_p_pulse_z1_flattop_param *prm)
{
  return psc_pulse_create(&psc_pulse_ops_p_z1_flattop, prm);
}

