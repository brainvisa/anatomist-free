#ifndef _shader_h_
#define _shader_h_


#include <anatomist/qtvr3/slicing.h>

#include <string>


namespace Vr
{


class Shader
{

  public:

    Shader();
    virtual ~Shader();

    virtual std::string getName() const;
    void setMaxSlices( int );
    virtual void setBlending();
    virtual std::map< float, std::list< Vector3d > >&
      getSlices( const float*, const Vector3d&, const int ) = 0;

  protected:

    Slicing sl;

};


}


#endif
