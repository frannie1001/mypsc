
#ifndef PSC_PARTICLES_CUDA_H
#define PSC_PARTICLES_CUDA_H

#include "particles.hxx"
#include "particles_traits.hxx"
#include "psc_bits.h"
#include "particles_simple.hxx"

#include <vector>

struct BS144
{
  using x = std::integral_constant<unsigned int, 1>;
  using y = std::integral_constant<unsigned int, 4>;
  using z = std::integral_constant<unsigned int, 4>;
};

struct BS444
{
  using x = std::integral_constant<unsigned int, 4>;
  using y = std::integral_constant<unsigned int, 4>;
  using z = std::integral_constant<unsigned int, 4>;
};

using DParticleCuda = ParticleSimple<float>;

template<typename BS>
struct cuda_mparticles;

// ======================================================================
// PatchCuda

template<typename _Mparticles>
struct PatchCuda
{
  using Mparticles = _Mparticles;

  PatchCuda(Mparticles& mprts, int p)
    : mprts_(mprts), p_(p)
  {}
  
  const Grid_t& grid() const { return mprts_.grid(); }

protected:
  Mparticles& mprts_;
  int p_;
};

// ======================================================================
// InjectorBuffered

template<typename Mparticles>
struct InjectorBuffered
{
  using particle_t = typename Mparticles::particle_t;
  using real_t = typename particle_t::real_t;
  using Real3 = typename particle_t::Real3;
  using Double3 = Vec3<double>;
  
  struct Patch
  {
    Patch(InjectorBuffered& injector, int p)
      : injector_(injector), p_{p}, n_prts_{0} // FIXME, why (), {} does not work
    {
      assert(p_ == injector_.mprts_.injector_n_prts_by_patch_.size());
    }
    
    ~Patch()
    {
      injector_.mprts_.injector_n_prts_by_patch_.push_back(n_prts_);
      if (p_ == injector_.mprts_.n_patches() - 1) {
	injector_.mprts_.inject(injector_.mprts_.injector_buf_, injector_.mprts_.injector_n_prts_by_patch_);
	injector_.mprts_.injector_n_prts_by_patch_.clear();
	injector_.mprts_.injector_buf_.clear();
      }
    }
  
    void raw(const particle_t& prt)
    {
      injector_.mprts_.injector_buf_.push_back(prt);
      n_prts_++;
    }
    
    void operator()(const particle_inject& new_prt)
    {
      auto& patch = injector_.mprts_.grid().patches[p_];
      auto x = Double3::fromPointer(new_prt.x) - patch.xb;
      auto u = Double3::fromPointer(new_prt.u);
      raw({Real3(x), Real3(u), real_t(new_prt.w), new_prt.kind});
    }

    // FIXME do we want to keep this? or just have a particle_inject version instead?
    void raw(const std::vector<particle_t>& buf)
    {
      auto& injector_buf = injector_.mprts_.injector_buf_;
      injector_buf.insert(injector_buf.end(), buf.begin(), buf.end());
      n_prts_ += buf.size();
    }
    
  private:
    InjectorBuffered& injector_;
    const int p_;
    uint n_prts_;
  };
  
  InjectorBuffered(Mparticles& mprts)
    : mprts_{mprts}
  {}

  Patch operator[](int p) { return {*this, p}; }

private:
  Mparticles& mprts_;
};

// ======================================================================
// MparticlesCuda

template<typename _BS>
struct MparticlesCuda : MparticlesBase
{
  using Self = MparticlesCuda;
  using BS = _BS;
  using real_t = float;
  using particle_t = ParticleSimple<real_t>;
  using Real3 = Vec3<real_t>;
  using BndpParticle = DParticleCuda;
  using buf_t = std::vector<BndpParticle>;
  using CudaMparticles = cuda_mparticles<BS>;

  using is_cuda = std::true_type;
  
  MparticlesCuda(const Grid_t& grid);
  ~MparticlesCuda();

  int get_n_prts() const override;
  void get_size_all(uint *n_prts_by_patch) const override;
  void reset(const Grid_t& grid) override;

  void inject(const std::vector<particle_t>& buf, const std::vector<uint>& buf_n_by_patch);
  void dump(const std::string& filename);
  uint start(int p) const;
  bool check_after_push();

  // ----------------------------------------------------------------------
  // facility to access particles without conversion,
  // mostly for debugging (?)

  std::vector<particle_t> get_particles(int beg, int end) const;
  std::vector<particle_t> get_particles(int p) const;

  void define_species(const char *name, double q, double m,
		      double max_local_np, double max_local_nm,
		      double sort_interval, double sort_out_of_place)
  {}
  
  static const Convert convert_to_, convert_from_;
  const Convert& convert_to() override { return convert_to_; }
  const Convert& convert_from() override { return convert_from_; }

  CudaMparticles* cmprts() { return cmprts_; }

  struct Patch : PatchCuda<MparticlesCuda>
  {
    using Base = PatchCuda<MparticlesCuda>;

    using Base::Base;
    using Base::mprts_;
    using Base::p_;
    using Base::grid;
    
    struct const_accessor
    {
      using Double3 = Vec3<double>;
      
      const_accessor(const particle_t& prt, const Patch& prts)
	: prt_{prt}, prts_{prts}
      {}
      
      Real3 u()   const { return prt_.u(); }
      real_t w()  const { return prt_.qni_wni() / prts_.grid().kinds[prt_.kind()].q; }
      int kind()  const { return prt_.kind(); }
      
      Double3 position() const
      {
	auto& patch = prts_.grid().patches[prts_.p_];
	
	return patch.xb + Double3(prt_.x());
      }
    
    private:
      particle_t prt_;
      const Patch prts_;
    };
  
    struct const_accessor_range
    {
      struct const_iterator : std::iterator<std::random_access_iterator_tag,
					    const_accessor,  // value type
					    ptrdiff_t,       // difference type
					    const_accessor*, // pointer type
					    const_accessor&> // reference type
      
      {
	const_iterator(const Patch& prts, uint n)
	  : prts_{prts}, n_{n}
	{}
	
	bool operator==(const_iterator other) const { return n_ == other.n_; }
	bool operator!=(const_iterator other) const { return !(*this == other); }
	
	const_iterator& operator++() { n_++; return *this; }
	const_iterator operator++(int) { auto retval = *this; ++(*this); return retval; }
	const_accessor operator*() { return {prts_.get_particle(n_), prts_}; }
	
      private:
	const Patch prts_;
	uint n_;
      };
    
      const_accessor_range(const Patch& prts)
	: prts_(prts)
      {}

      const_iterator begin() const { return {prts_, 0}; }
      const_iterator end()   const { return {prts_, prts_.size()}; };
      
    private:
      const Patch prts_;
    };

    const ParticleIndexer<real_t>& particleIndexer() const { return mprts_.pi_; }

    particle_t get_particle(int n) const
    {
      uint off = mprts_.start(p_);
      auto cprts = mprts_.get_particles(off + n, off + n + 1);
      return cprts[0];
    }

    uint size() const
    {
      uint n_prts_by_patch[grid().n_patches()];
      mprts_.get_size_all(n_prts_by_patch);
      return n_prts_by_patch[p_];
    }

    const_accessor_range get() const { return {*this}; }
  };

  friend struct InjectorBuffered<MparticlesCuda>;
  
  Patch operator[](int p) { return {*this, p}; }
  InjectorBuffered<MparticlesCuda> injector() { return {*this}; }

private:
  CudaMparticles* cmprts_;
  ParticleIndexer<real_t> pi_;

  std::vector<uint> injector_n_prts_by_patch_;
  std::vector<particle_t> injector_buf_;
};

template<>
struct Mparticles_traits<MparticlesCuda<BS144>>
{
  static constexpr const char* name = "cuda";
  static MPI_Datatype mpi_dtype() { return MPI_FLOAT; }
};


template<>
struct Mparticles_traits<MparticlesCuda<BS444>>
{
  static constexpr const char* name = "cuda444";
  static MPI_Datatype mpi_dtype() { return MPI_FLOAT; }
};


#endif
