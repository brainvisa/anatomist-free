#ifndef _mipShader_h_
#define _mipShader_h_


#include <anatomist/qtvr3/shader.h>
#include <anatomist/qtvr3/creator.h>


namespace Vr
{


class MIPShader : public Shader, 
                  public Creator< MIPShader, Shader >
{

  public:

    std::string getName() const;
    static std::string getStaticName();
    void setBlending();
    std::map< float, std::list< Vector3d > >& getSlices( 
	  		      const float*,
			      const Vector3d& v = Vector3d( 0.0f, 0.0f, 0.0f ),
			      const int n = 1  );

  protected:

    friend class Creator< MIPShader, Shader >;

    MIPShader();

};


}


#endif
