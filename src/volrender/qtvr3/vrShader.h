#ifndef _vrShader_h_
#define _vrShader_h_


#include <anatomist/qtvr3/shader.h>
#include <anatomist/qtvr3/creator.h>


namespace Vr
{


class VRShader : public Shader, 
                 public Creator< VRShader, Shader >
{

  public:

    std::string getName() const;
    static std::string getStaticName();
    void setBlending();
    std::map< float, std::list< Vector3d > >& getSlices( 
			      const float*,
			      const Vector3d& v = Vector3d( 0.0f, 0.0f, 0.0f ),
			      const int n = 1 );

  protected:

    friend class Creator< VRShader, Shader >;

    VRShader();

};


}


#endif
