#include "shaderFactory.h"
#include "shader.h"


bool 
Vr::ShaderFactory::registerShader( const std::string& name,
		  		   Vr::ShaderFactory::ShaderCreator creator )
{

  if ( creator && _shaderCreators.find( name ) == _shaderCreators.end() )
  {

    _shaderCreators.insert( std::make_pair( name, creator ) );
    return true;

  }

  return false;

}


Vr::Shader* Vr::ShaderFactory::create( const std::string& name )
{

  std::map< std::string, Vr::ShaderFactory::ShaderCreator >::const_iterator
    s = _shaderCreators.find( name );

  if ( s != _shaderCreators.end() )
  {

    return (*s->second)();

  }

  return NULL;

}
