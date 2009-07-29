#include "shader.h"


Vr::Shader::Shader()
{
}


Vr::Shader::~Shader()
{
}


std::string Vr::Shader::getName() const 
{ 

  return "unknown"; 

}

void Vr::Shader::setMaxSlices( int nbSlices )
{

  sl.setMaxSlices( nbSlices );

}

void Vr::Shader::setBlending()
{
}
