#include "vrShader.h"
#include "shaderFactory.h"

#include <anatomist/window/glcaps.h>

using namespace anatomist;

Vr::VRShader::VRShader() : Shader()
{
}


std::string Vr::VRShader::getName() const
{

  return getStaticName();

}


std::string Vr::VRShader::getStaticName()
{

  return "VRShader";

}


void Vr::VRShader::setBlending()
{

  glEnable( GL_BLEND );
  GLCaps::glBlendEquation( GL_FUNC_ADD );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::VRShader::getSlices( const float* m, const Vr::Vector3d&, const int )
{

  return sl.getSlices( m );

}


RegisterShader( VRShader );
