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

#include <anatomist/surface/globject.h>
#include <anatomist/window/viewstate.h>

using namespace anatomist;
using namespace std;

struct AGLObject::Private
{
  Private() : poprefneeded( false )
  {}
  ~Private() {}

  bool poprefneeded;
};


AGLObject::AGLObject() : AObject(), GLComponent(), d( new Private )
{
}


AGLObject::~AGLObject()
{
  delete d;
}


const Material *AGLObject::glMaterial() const
{
  return &material();
}


const AObjectPalette* AGLObject::glPalette( unsigned ) const
{
  return getOrCreatePalette();
}


void AGLObject::glSetChanged( glPart p, bool x ) const
{
  //cout << "AGLObject::glSetChanged " << p << ", " << x << endl;
  GLComponent::glSetChanged( p, x );
  if( x )
    obsSetChanged( p );
}


void AGLObject::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}


void AGLObject::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}


std::string AGLObject::viewStateID( glPart part,
                                    const ViewState & state ) const
{
  // cout << "AGLObject::viewStateID " << part << ", smode: " << state.selectRenderMode << endl;
  string	s;
  vector<float> td = state.timedims;
  if( part == glTEXIMAGE || part == glTEXENV || part == glMATERIAL )
    return s;
  unsigned i, n = td.size();
  vector<float> bbmin( 3, 0. ), bbmax( 3, 0. );
  boundingBox( bbmin, bbmax );
  if( n > bbmax.size() - 3 )
    n = bbmax.size() - 3;

  for( i=0; i<n; ++i )
  {
    if( td[i] < bbmin[i + 3] )
      td[i] = bbmin[i + 3];
    if( td[i] > bbmax[i + 3] )
      td[i] = bbmax[i + 3];
  }
  if( n < bbmax.size() - 3 )
  {
    n = bbmax.size() - 3;
    for( ; i<n; ++i )
      td.push_back( bbmin[i + 3] );
  }

  const unsigned nf = sizeof( float );

  if( state.selectRenderMode != ViewState::glSELECTRENDER_NONE )
  {
    if( part == glPALETTE )
      return s;
    s.resize( nf * ( n + 1 ) + sizeof( glSelectRenderMode ) );
    (float &) s[0] = n;
    for( i=0; i<n; ++i )
      (float &) s[nf * ( i + 1 )] = td[i];
    (glSelectRenderMode &) s[nf * (n + 1)] = state.selectRenderMode;
    return s;
  }

  s.resize( nf * ( n + 1 ) );
  (float &) s[0] = n;
    for( i=0; i<n; ++i )
      (float &) s[nf * ( i + 1 )] = td[i];
  return s;
}


