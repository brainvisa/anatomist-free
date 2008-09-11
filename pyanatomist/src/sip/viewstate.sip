
namespace anatomist
{

class ViewState
{
%TypeHeaderCode
#include <anatomist/window/viewstate.h>
%End

public:
  ViewState( float t = 0, anatomist::AWindow * = 0 );
  virtual ~ViewState();

  virtual anatomist::SliceViewState *sliceVS();

  float       time;
  anatomist::AWindow *window;
};


class SliceViewState : anatomist::ViewState
{
%TypeHeaderCode
#include <anatomist/window/viewstate.h>
#ifndef PYAIMS_AIMSVECTOR_FLOAT_3_DEFINED
#define PYAIMS_AIMSVECTOR_FLOAT_3_DEFINED
typedef AimsVector<float, 3> AimsVector_FLOAT_3;
#endif
%End

public:
  SliceViewState( float t = 0, bool slicewanted = false,
                  const Point3df & pos = Point3df(),
                  const aims::Quaternion* orient = 0,
                  const anatomist::Referential* wref = 0,
                  const anatomist::Geometry* wgeom = 0,
                  const aims::Quaternion* vorient = 0,
                  anatomist::AWindow * = 0 );

  virtual ~SliceViewState();

  virtual anatomist::SliceViewState *sliceVS();

  bool                          wantslice;
  Point3df                position;
  const aims::Quaternion        *orientation;
  const anatomist::Referential  *winref;
  const anatomist::Geometry     *wingeom;
  const aims::Quaternion        *vieworientation;
};

};

