#ifndef _rfmtShader_h_
#define _rfmtShader_h_


#include <anatomist/qtvr3/shader.h>
#include <anatomist/qtvr3/creator.h>


namespace Vr
{


class RfmtShader : public Shader,
                   public Creator< RfmtShader, Shader >
{

  public:

    std::string getName() const;
    static std::string getStaticName();
    std::map< float, std::list< Vector3d > >& getSlices( 
			      const float*,
			      const Vector3d& v = Vector3d( 0.0f, 0.0f, 0.0f ),
			      const int n = 1  );

  protected:

    friend class Creator< RfmtShader, Shader >;

    RfmtShader();

  private:

    std::map< float, std::list< Vector3d > > slices;

};


}


#endif
