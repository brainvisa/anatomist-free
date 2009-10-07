/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */
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
