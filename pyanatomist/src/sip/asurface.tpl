
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <anatomist/surface/surface.h>
#ifndef PYANA_ASURFACE_%Template1typecode%_DEFINED
#define PYANA_ASURFACE_%Template1typecode%_DEFINED
namespace anatomist
{
  typedef ASurface<%Template1%> ASurface_%Template1typecode%;
}
#endif
#endif
%End

namespace anatomist
{

class ASurface_%Template1typecode% : anatomist::AObject, anatomist::GLComponent
{
%TypeHeaderCode
#include <anatomist/surface/surface.h>
#ifndef PYANA_ASURFACE_%Template1typecode%_DEFINED
#define PYANA_ASURFACE_%Template1typecode%_DEFINED
namespace anatomist
{
  typedef ASurface<%Template1%> ASurface_%Template1typecode%;
}
#endif
#include <pyaims/vector/vector.h>
#include <pyanatomist/refglitem.h>
%End

public:
  ASurface_%Template1typecode%( const char *="" );
  virtual ~ASurface_%Template1typecode%();

  // const rc_ptr_AimsTimeSurface_%Template1typecode% surface() const;
  rc_ptr_AimsTimeSurface_%Template1typecode% surface();
  void setSurface( rc_ptr_AimsTimeSurface_%Template1typecode% );
  void setSurface( AimsTimeSurface_%Template1typecode% * /Transfer/ );
  // const AimsSurface_%Template1typecode% * surfaceOfTime( float ) const;
  bool isPlanar() const;
  virtual bool Is2DObject();
  virtual bool Is3DObject();
  virtual void UpdateMinAndMax();

  // redeclaration of pure vitual methods of AObject
  virtual anatomist::GLComponent* glAPI();
  int type() const;

private:
  ASurface_%Template1typecode%( const anatomist::ASurface_%Template1typecode% & );
};

};

