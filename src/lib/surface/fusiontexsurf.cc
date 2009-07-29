/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
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


bool FusionTexSurfMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return false;

  GLComponent				*o1, *o2;
  set<AObject *>::const_iterator	io;

  io = obj.begin();
  o1 = (*io)->glAPI();
  if( !o1 || o1->sliceableAPI() )
    return false;
  ++io;
  o2 = (*io)->glAPI();
  if( !o2 || o2->sliceableAPI() )
    return false;

  ViewState	s( 0 );
  if( o1->glNumPolygon( s ) != 0 && o2->glNumTextures() != 0 )
    return true;
  if( o2->glNumPolygon( s ) != 0 && o1->glNumTextures() != 0 )
    return true;

  return false;
}


AObject* FusionTexSurfMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io = obj.begin();
  AObject				*o1 = *io;

  ++io;

  return( new ATexSurface( o1, *io ) );
}
