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


#include <anatomist/window3D/trackObliqueSlice.h>
#include <anatomist/controler/view.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/processor/event.h>
#include <anatomist/window3D/boxviewslice.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


TrackObliqueSlice::TrackObliqueSlice() : TrackOblique()
{
}


TrackObliqueSlice::TrackObliqueSlice( const TrackObliqueSlice & a ) 
  : TrackOblique( a )
{
}


TrackObliqueSlice::~TrackObliqueSlice()
{
}


Action* TrackObliqueSlice::creator()
{
  return( new TrackObliqueSlice );
}


string TrackObliqueSlice::name() const
{
  return( "TrackObliqueSlice" );
}


void TrackObliqueSlice::moveTrackball( int x, int y, int, int )
{
  Quaternion	q = rotation( x, y ).inverse() * _beginslice;

  q.norm();

  AWindow3D	*w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w3 )
    return;

  w3->setSliceQuaternion( q );

  // send event
  Object	ex = Object::value( Dictionary() );
  ex->setProperty( "_window", Object::value( (AWindow *) w3 ) );
  vector<float>	vf(4);
  Point4df	qv = q.vector();
  vf[0] = qv[0];
  vf[1] = qv[1];
  vf[2] = qv[2];
  vf[3] = qv[3];
  ex->setProperty( "slice_quaternion", Object::value( vf ) );
  Point3df	pos = w3->getPosition();
  vf.erase( vf.begin() + 3 );
  vf[0] = pos[0];
  vf[1] = pos[1];
  vf[2] = pos[2];
  ex->setProperty( "position", Object::value( vf ) );
  OutputEvent	ev( "Slice", ex );
  ev.send();

  _boxviewslice->moveTrackball( x, y );
  Trackball::moveTrackballInternal( x, y );
  w3->refreshNow();
}

