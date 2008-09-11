#ifndef _slicing_h_
#define _slicing_h_


#include <anatomist/qtvr3/cubeZbuffer.h>
#include <anatomist/qtvr3/vector3d.h>

#include <list>
#include <map>


namespace Vr
{


class Slicing
{

  public:

    Slicing();
    virtual ~Slicing() {}

    void setMaxSlices( int );
    std::list< Vector3d > getSlice( const float*, float );
    std::map< float, std::list< Vector3d > >& getSlices( const float* );
    std::map< float, std::list< Vector3d > >& getSlab( const float*, 
	  					       float, int );

  private:

    void getOneSlice( float );

    int maxNbSlices;
    CubeZBuffer cubeZBuffer;
    std::map< float, std::list< Vector3d > > slices;

};


}


#endif
