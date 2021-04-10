
#include "cuda_iface.h"
#include "cuda_mfields.h"
#include "cuda_bits.h"

#include "psc_fields_cuda.h"

#undef dprintf
#if 0
#define dprintf(...) mprintf(__VA_ARGS__)
#else
#define dprintf(...)                                                           \
  do {                                                                         \
  } while (0)
#endif

MfieldsCuda::MfieldsCuda(const Grid_t& grid, int n_fields, Int3 ibn)
  : MfieldsBase{grid, n_fields, ibn}, grid_{&grid}
{
  dprintf("CMFLDS: ctor\n");
  cmflds_ = new cuda_mfields(grid, n_fields, ibn);
}

MfieldsCuda::~MfieldsCuda()
{
  dprintf("CMFLDS: dtor\n");
  delete cmflds_;
}

int MfieldsCuda::n_comps() const
{
  return cmflds_->n_comps();
}

int MfieldsCuda::n_patches() const
{
  return cmflds_->n_patches();
}

void MfieldsCuda::reset(const Grid_t& new_grid)
{
  dprintf("CMFLDS: reset\n");
  MfieldsBase::reset(new_grid);
  Int3 ibn = -cmflds()->ib();
  int n_comps = cmflds()->n_comps();
  delete cmflds_;
  cmflds_ = new cuda_mfields(new_grid, n_comps, ibn);
  grid_ = &new_grid;
}

int MfieldsCuda::index(int m, int i, int j, int k, int p) const
{
  return cmflds_->index(m, i, j, k, p);
}

gt::gtensor_span_device<MfieldsCuda::real_t, 5> MfieldsCuda::gt()
{
  return gt::adapt_device(cmflds_->storage().data().get(),
                          cmflds_->storage().shape());
}

MfieldsCuda::Patch::Patch(MfieldsCuda& mflds, int p) : mflds_(mflds), p_(p) {}

MfieldsCuda::Accessor MfieldsCuda::Patch::operator()(int m, int i, int j, int k)
{
  return {mflds_, mflds_.index(m, i, j, k, p_)};
}

MfieldsCuda::Accessor::Accessor(MfieldsCuda& mflds, int idx)
  : mflds_(mflds), idx_(idx)
{}

MfieldsCuda::Accessor::operator real_t() const
{
  return mflds_.cmflds_->get_value(idx_);
}

MfieldsCuda::real_t MfieldsCuda::Accessor::operator=(real_t val)
{
  mflds_.cmflds_->set_value(idx_, val);
  return val;
}

MfieldsCuda::real_t MfieldsCuda::Accessor::operator+=(real_t val)
{
  val += mflds_.cmflds_->get_value(idx_);
  mflds_.cmflds_->set_value(idx_, val);
  return val;
}

HMFields hostMirror(const MfieldsCuda& mflds)
{
  return hostMirror(*mflds.cmflds());
}

void copy(const MfieldsCuda& mflds, HMFields& hmflds)
{
  copy(*mflds.cmflds(), hmflds);
}

void copy(const HMFields& hmflds, MfieldsCuda& mflds)
{
  copy(hmflds, *mflds.cmflds());
}
