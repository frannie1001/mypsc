
#pragma once

#include "dim.hxx"
#include "inc_defs.h"

template<typename dim>
struct CacheFieldsNone;

template<typename dim>
struct CacheFields;

template<typename MP, typename MF, typename D,
	 typename IP, typename O,
	 typename CALCJ = opt_calcj_esirkepov,
	 typename OPT_EXT = opt_ext_none,
	 typename CF = CacheFieldsNone<D>>
struct push_p_config
{
  using mparticles_t = MP;
  using mfields_t = MF;
  using dim = D;
  using ip = IP;
  using order = O;
  using calcj = CALCJ;
  using ext = OPT_EXT;
  using CacheFields = CF;
};

#include "psc_particles_double.h"
#include "psc_fields_c.h"

template<typename dim>
using Config2nd = push_p_config<PscMparticlesDouble, PscMfieldsC, dim, opt_ip_2nd, opt_order_2nd>;

using Config2ndDoubleYZ = push_p_config<PscMparticlesDouble, PscMfieldsC, dim_yz, opt_ip_2nd, opt_order_2nd,
					opt_calcj_esirkepov, opt_ext_none, CacheFields<dim_yz>>;

using Config1stXZ = push_p_config<PscMparticlesDouble, PscMfieldsC, dim_xz, opt_ip_1st, opt_order_1st>;
using Config1stYZ = push_p_config<PscMparticlesDouble, PscMfieldsC, dim_yz, opt_ip_1st, opt_order_1st>;
