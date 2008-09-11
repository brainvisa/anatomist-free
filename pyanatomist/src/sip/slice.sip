
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <anatomist/object/sliceable.h>
#endif
%End


namespace anatomist
{

class Sliceable : anatomist::GLComponent /Abstract/
{
%TypeHeaderCode
#include <anatomist/object/sliceable.h>
%End

public:
  Sliceable();
  virtual ~Sliceable();
  virtual rc_ptr_Volume_RGBA rgbaVolume( const anatomist::SliceViewState* = 0,
      int tex = 0 ) const;
  virtual void rgbaVolume( Volume_RGBA &, const anatomist::SliceViewState* = 0,
      int tex = 0 ) const;
};



class SliceableObject : anatomist::AObject, anatomist::Sliceable /Abstract/
{
%TypeHeaderCode
#include <anatomist/object/sliceable.h>
%End

public:
  SliceableObject();
  virtual ~SliceableObject();
};

};
