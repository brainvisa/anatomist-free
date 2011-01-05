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


#include <anatomist/surface/fusiontexsurf.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/window/viewstate.h>
#include <qobject.h>

using namespace anatomist;
using namespace std;


string FusionTexSurfMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionTexSurfMethod" ) );
}


int FusionTexSurfMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  GLComponent				*o1, *o2;
  set<AObject *>::const_iterator	io;

  io = obj.begin();
  o1 = (*io)->glAPI();
  if( !o1 || o1->sliceableAPI() )
    return 0;
  ++io;
  o2 = (*io)->glAPI();
  if( !o2 || o2->sliceableAPI() )
    return 0;

  ViewState	s( 0 );
  if( o1->glNumPolygon( s ) != 0 && o2->glNumTextures() != 0 )
    return 80;
  if( o2->glNumPolygon( s ) != 0 && o1->glNumTextures() != 0 )
    return 130;

  return 0;
}


AObject* FusionTexSurfMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io = obj.begin();
  AObject				*o1 = *io;

  ++io;

  return( new ATexSurface( o1, *io ) );
}
