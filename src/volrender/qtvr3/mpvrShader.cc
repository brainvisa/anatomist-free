#include "mpvrShader.h"
#include "shaderFactory.h"

#include <anatomist/window/glcaps.h>

using namespace anatomist;

Vr::MPVRShader::MPVRShader() : Shader() 
{
}


std::string Vr::MPVRShader::getName() const
{

  return getStaticName();

}


std::string Vr::MPVRShader::getStaticName()
{

  return "MPVRShader";

}


void Vr::MPVRShader::setBlending()
{

  glEnable( GL_BLEND );
  GLCaps::glBlendEquation( GL_MAX );
  glBlendFunc( GL_SRC_ALPHA, GL_DST_ALPHA );

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::MPVRShader::getSlices( const float* m, 
                           const Vr::Vector3d& v3d, 
                           const int n )
{

  Vr::CubeZBuffer c;
  c.compute( m );

  return sl.getSlab( m, c.getZ( v3d ), n );

}


RegisterShader( MPVRShader );
