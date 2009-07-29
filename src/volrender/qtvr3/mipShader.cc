#include "mipShader.h"
#include "shaderFactory.h"

#include <anatomist/window/glcaps.h>

using namespace anatomist;

Vr::MIPShader::MIPShader() : Shader() 
{
}


std::string Vr::MIPShader::getName() const 
{ 

  return getStaticName();

}


std::string Vr::MIPShader::getStaticName()
{

  return "MIPShader";

}


void Vr::MIPShader::setBlending()
{

  glEnable( GL_BLEND );
  GLCaps::glBlendEquation( GL_MAX );
  glBlendFunc( GL_SRC_ALPHA, GL_DST_ALPHA );

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::MIPShader::getSlices( const float* m, const Vr::Vector3d&, const int )
{

  return sl.getSlices( m );

}


RegisterShader( MIPShader );
