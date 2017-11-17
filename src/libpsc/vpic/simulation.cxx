
#include "simulation.h"

extern vpic_simulation *simulation;

// ----------------------------------------------------------------------
// C wrappers

Simulation *Simulation_create()
{
  return new Simulation;
}

void Simulation_delete(Simulation *sim)
{
  delete sim;
}

// ----------------------------------------------------------------------
// diagnostics

void Simulation_diagnostics_init(struct Simulation *sim, int interval)
{
  sim->pDiag_ = new globals_diag(interval);
}

void Simulation_diagnostics_setup(struct Simulation *sim)
{
  sim->pDiag_->setup();
}

void Simulation_diagnostics_run(struct Simulation *sim, struct psc_harris *sub)
{
  sim->pDiag_->run();
}

// ----------------------------------------------------------------------
// Rng

void Simulation_rngPool_seed(struct Simulation *sim, int base)
{
  sim->rng_pool.seed(base, 0);
}

Rng *Simulation_rngPool_get(struct Simulation *sim, int n)
{
  return sim->rng_pool[n];
}

double Rng_uniform(struct Rng *rng, double lo, double hi)
{
  return rng->uniform(lo, hi);
}

double Rng_normal(struct Rng *rng, double mu, double sigma)
{
  return rng->normal(mu, sigma);
}
