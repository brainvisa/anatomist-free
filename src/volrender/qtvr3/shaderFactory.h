#ifndef _shaderFactory_h_
#define _shaferFactory_h_


#include <anatomist/qtvr3/vrsingleton.h>

#include <map>
#include <string>


namespace Vr
{


class Shader;


class ShaderFactory : public VrSingleton< ShaderFactory >
{

  public:

    typedef Shader* (*ShaderCreator)();

    bool registerShader( const std::string&, ShaderCreator );
    Shader* create( const std::string& );

  private:

    std::map< std::string, ShaderCreator > _shaderCreators;

};


}


#define RegisterShader( S ) \
  static bool init_##S = Vr::ShaderFactory::instance().registerShader( \
				 	       Vr::S::getStaticName(), \
					       &Vr::S::create )


#endif
