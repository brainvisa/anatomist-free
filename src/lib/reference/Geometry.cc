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


//--- header files ------------------------------------------------------------

#include <anatomist/reference/Geometry.h>


using namespace anatomist;
using namespace std;


//--- methods -----------------------------------------------------------------

Geometry::Geometry( const vector<float> & steps, const vector<int> & dimMin,
                    const vector<int> & dimMax )
  : _size( steps ), _dimMin( dimMin ), _dimMax( dimMax )
{
  cout << "Geometry steps size: " << steps.size() << endl;
  if( steps.size() >= 4 )
    cout << "step t: " << steps[3] << endl;
  if( _size.size() < 4 )
    _size.resize( 4, 1. );
}


Geometry::Geometry()
  : _size( 4, 1.f ), _dimMin( 4, 1 ), _dimMax( 4, 1 )
{
  _dimMin[0] = 0;
  _dimMin[1] = 0;
  _dimMin[2] = 0;
  _dimMin[3] = 0;
  _dimMax[0] = 0;
  _dimMax[1] = 0;
  _dimMax[2] = 0;
  _dimMax[3] = 0;
}

Geometry::Geometry(Point3df size,Point4dl dimMin,Point4dl dimMax)
  : _size( 4, 1.f ), _dimMin( 4, 1 ), _dimMax( 4, 1 )
{
  _size[0] = size[0];
  _size[1] = size[1];
  _size[2] = size[2];
  _dimMin[0] = dimMin[0];
  _dimMin[1] = dimMin[1];
  _dimMin[2] = dimMin[2];
  _dimMin[3] = dimMin[3];
  _dimMax[0] = dimMax[0];
  _dimMax[1] = dimMax[1];
  _dimMax[2] = dimMax[2];
  _dimMax[3] = dimMax[3];
}

Geometry::Geometry( const Geometry & g )
{
  *this = g;
}

Geometry::~Geometry()
{ 
}

Geometry & Geometry::operator = ( const Geometry & g )
{
  if( &g != this )
    {
      _size = g._size;
      if( _size.size() < 4 )
        _size.resize( 4, 1. );
      cout << "Geometry=, steps size: " << _size.size() << endl;
      if( _size.size() >= 4 )
        cout << "step T: " << _size[3] << endl;
      _dimMin = g._dimMin;
      _dimMax = g._dimMax;
    }

  return( *this );
}


void Geometry::SetSize( Point3df size )
{
  _size.resize( 4 );
  _size[0] = size[0];
  _size[1] = size[1];
  _size[2] = size[2];
  _size[3] = 1.f;
}


void Geometry::setStepSize( const vector<float> & steps )
{
  _size = steps;
  if( _size.size() < 4 )
    _size.resize( 4, 1. );
}


void Geometry::SetDimMin( Point4dl dimMin )
{
  _dimMin.resize( 4 );
  _dimMin[0] = dimMin[0];
  _dimMin[1] = dimMin[1];
  _dimMin[2] = dimMin[2];
  _dimMin[3] = dimMin[3];
}

void Geometry::SetDimMax( Point4dl dimMax )
{
  _dimMax.resize( 4 );
  _dimMax[0] = dimMax[0];
  _dimMax[1] = dimMax[1];
  _dimMax[2] = dimMax[2];
  _dimMax[3] = dimMax[3];
}


Point4dl Geometry::DimMin() const
{
  Point4dl d;
  int i, n = _dimMin.size();
  for( i=0; i<std::min(4, n); ++i )
    d[i] = _dimMin[i];
  for( ; i<4; ++i )
    d[i] = 1;
  return d;
}


Point4dl Geometry::DimMax() const
{
  Point4dl d;
  int i, n = _dimMax.size();
  for( i=0; i<std::min(4, n); ++i )
    d[i] = _dimMax[i];
  for( ; i<4; ++i )
    d[i] = 1;
  return d;
}


void Geometry::setDimMin( const std::vector<int> & dimMin )
{
  _dimMin = dimMin;
}


void Geometry::setDimMax( const std::vector<int> & dimMax )
{
  _dimMax = dimMax;
}

