#include <anatomist/surface/shaderModuleRegistry.h>
#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/depthPeelingEffect.h>
#include <anatomist/misc/error.h>
#include <iostream>

using namespace anatomist;

std::map<ShaderModuleId, carto::rc_ptr<IShaderModule> > &
ShaderModuleRegistry::modules()
{
  // built once, on first use (thread-safe static init since C++11)
  static std::map<ShaderModuleId, carto::rc_ptr<IShaderModule> > m = []
  {
    std::map<ShaderModuleId, carto::rc_ptr<IShaderModule> > r;
    auto add = [ &r ]( IShaderModule * mod )
    {
      r[ mod->id() ] = carto::rc_ptr<IShaderModule>( mod );
    };
    add( new BlinnPhongIlluminationModel );
    add( new DepthPeelingEffect );
    return r;
  }();
  return m;
}

std::vector<carto::rc_ptr<IShaderModule> >
ShaderModuleRegistry::resolve( const std::set<ShaderModuleId> & ids )
{
  std::vector<carto::rc_ptr<IShaderModule> > out;
  for( ShaderModuleId id : ids )
  {
    auto it = modules().find( id );
    if( it != modules().end() )
      out.push_back( it->second );
    else
      AWarning( "ShaderModuleRegistry::resolve: unknown shader module" );
  }
  return out;
}

void ShaderModuleRegistry::printModules()
{
  for( const auto & entry : modules() )
    entry.second->printModule();
}