#include "sumShader.h"
#include "shaderFactory.h"

#include <anatomist/window/glcaps.h>

using namespace anatomist;

Vr::SumShader::SumShader() : Shader()
{
}


std::string Vr::SumShader::getName() const
{ 

  return getStaticName();

}


std::string Vr::SumShader::getStaticName()
{

  return "SumShader";

}


void Vr::SumShader::setBlending()
{

  glEnable( GL_BLEND );
  GLCaps::glBlendEquation( GL_FUNC_ADD );
  glBlendFunc( GL_ONE, GL_ONE );

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::SumShader::getSlices( const float* m, const Vr::Vector3d&, const int )
{

  return sl.getSlices( m );

}


RegisterShader( SumShader );
