#include "cubeZbuffer.h"

#include <limits>


Vr::CubeZBuffer::CubeZBuffer() 
  : zMin( std::numeric_limits< float >::min() ),
    zMax( std::numeric_limits< float >::max() )
{
}


void Vr::CubeZBuffer::compute( const float* m )
{

  int i;

  zMin = std::numeric_limits< float >::max();
  zMax = std::numeric_limits< float >::min();
  vz[ 0 ] = m[ 8 ];
  vz[ 1 ] = m[ 9 ];
  vz[ 2 ] = m[ 10 ];
  vz[ 3 ] = m[ 14 ];

  for ( i = 0; i < 8; i++ )
  {

    zi[ i ] = m[ 14 ] + ( ( ( i & 0x1 ) ? m[ 8 ] : 0.0f ) +
                          ( ( i & 0x2 ) ? m[ 9 ] : 0.0f ) +
                          ( ( i & 0x4 ) ? m[ 10 ] : 0.0f ) );

    if ( zi[ i ] > zMax )
    {

      zMax = zi[ i ];

    }

    if ( zi[ i ] < zMin )
    {

      zMin = zi[ i ];

    }

  }

}


float Vr::CubeZBuffer::getMinZ() const
{

  return zMin;

}


float Vr::CubeZBuffer::getMaxZ() const
{

  return zMax;

}


float Vr::CubeZBuffer::getVertexZ( int i ) const
{

  return zi[ i ];

}


int Vr::CubeZBuffer::getKey( float z ) const
{

  int i, key = 0;

  for ( i = 0; i < 8; i++ )
  {

    key += ( zi[ i ] > z ) ? 1 << i : 0;

  }

  return key;

}


float Vr::CubeZBuffer::getStep( int n ) const
{

  return ( zMax - zMin ) / (float)n;

}


float Vr::CubeZBuffer::getZ( const Vr::Vector3d& v ) const
{

  return ( vz[ 0 ] * v.v[ 0 ] + vz[ 1 ] * v.v[ 1 ] + 
	   vz[ 2 ] * v.v[ 2 ] + vz[ 3 ] );

}
