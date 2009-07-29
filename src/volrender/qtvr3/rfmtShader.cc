#include "rfmtShader.h"
#include "shaderFactory.h"


Vr::RfmtShader::RfmtShader() : Shader()
{
}


std::string Vr::RfmtShader::getName() const
{ 

  return getStaticName();

}


std::string Vr::RfmtShader::getStaticName()
{

  return "RfmtShader";

}


std::map< float, std::list< Vr::Vector3d > >& 
Vr::RfmtShader::getSlices( const float* m, const Vr::Vector3d& v3d, const int )
{

  Vr::CubeZBuffer c;

  c.compute( m );
  float pz = c.getZ( v3d );
  std::list< Vr::Vector3d > v = sl.getSlice( m, pz );
  slices.clear();
  slices.insert( std::make_pair( pz, v ) );

  return slices;

}


RegisterShader( RfmtShader );
