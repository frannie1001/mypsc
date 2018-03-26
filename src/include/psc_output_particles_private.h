
#ifndef PSC_OUTPUT_PARTICLES_PRIVATE_H
#define PSC_OUTPUT_PARTICLES_PRIVATE_H

#include <psc_output_particles.h>
#include "vec3.hxx"

struct PscOutputParticlesParams
{
  const char *data_dir;
  const char *basename;
  int every_step;
  Int3 lo;
  Int3 hi;
  bool use_independent_io;
  const char *romio_cb_write;
  const char *romio_ds_write;
};

struct psc_output_particles
{
  struct mrc_obj obj;

  // parameters
  PscOutputParticlesParams params;
};

struct psc_output_particles_ops {
  MRC_SUBCLASS_OPS(struct psc_output_particles);
  void (*run)(struct psc_output_particles *output_particles,
	      struct psc_mparticles *particles);
};

// ======================================================================

#define psc_output_particles_ops(output_particles) ((struct psc_output_particles_ops *)((output_particles)->obj.ops))

#endif
