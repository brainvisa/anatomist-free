#ifndef _cubeZbuffer_h_
#define _cubeZbuffer_h_


#include <anatomist/qtvr3/vector3d.h>


namespace Vr
{


class CubeZBuffer
{

  public:

    CubeZBuffer();
    virtual ~CubeZBuffer() {}

    void compute( const float* );
    float getMinZ() const;
    float getMaxZ() const;
    float getVertexZ( int ) const;
    int getKey( float ) const;
    float getStep( int ) const;
    float getZ( const Vector3d& ) const;
 
  private:

    float vz[ 4 ];
    float zi[ 8 ];
    float zMin;
    float zMax;

};


}


#endif
