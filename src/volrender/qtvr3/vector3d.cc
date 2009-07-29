#include "vector3d.h"


Vr::Vector3d::Vector3d()
{

  v[ 0 ] = v[ 1 ] = v[ 2 ] = 0.0f;

}


Vr::Vector3d::Vector3d( float x, float y, float z )
{

  v[ 0 ] = x;
  v[ 1 ] = y;
  v[ 2 ] = z;

}
