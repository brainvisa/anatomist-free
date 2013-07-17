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


#include <anatomist/window3D/trackOblique.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/processor/event.h>
#include <anatomist/window3D/boxviewslice.h>
#include <iostream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


TrackOblique::TrackOblique() 
  : Trackball(), _boxviewslice( new BoxViewSlice( this ) )
{
}


TrackOblique::TrackOblique( const TrackOblique & a )
  : Trackball( a ), _boxviewslice( new BoxViewSlice( this ) )
{
}


TrackOblique::~TrackOblique()
{
  delete _boxviewslice;
}


Action* TrackOblique::creator()
{
  return( new TrackOblique );
}


string TrackOblique::name() const
{
  return( "TrackOblique" );
}


Quaternion TrackOblique::beginQuaternion() const
{
  return _beginslice;
}


void TrackOblique::beginTrackball( int x, int y, int globalX, int globalY )
{
  AWindow3D	* w = dynamic_cast<AWindow3D *>( view()->aWindow() );

  /*const Point4df	q = ((QAGLWidget *) view())->quaternion().vector();
  cout << "quat : " << q[0] << ", " << q[1] << ", " << q[2] << ", " 
  << q[3] << endl;*/
  Trackball::beginTrackball( x, y, globalX, globalY );

  if( w )
    _beginslice = w->sliceQuaternion();

  _boxviewslice->beginTrackball( x, y );
}


Quaternion TrackOblique::rotation( int x, int y )
{
  if( _beginx < 0 || _beginy < 0 )
    return( Quaternion( 0, 0, 0, 1 ) );

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "TrackOblique operating on wrong view type -- error\n";
      return( Quaternion( 0, 0, 0, 1 ) );
    }

  float	dimx = w->width();
  float	dimy = w->height();

  Quaternion	q 
    = w->quaternion().inverse() * 
    initQuaternion( ( 2. * _beginx - dimx ) / dimx,
		    ( dimy - 2. * _beginy ) / dimy,
		    ( 2. * x - dimx ) / dimx,
		    ( dimy - 2. * y ) / dimy ).inverse() 
    * w->quaternion();

  Point4df	vec = q.vector();
  // we must invert Z since we're transforming to an indirect referential
  float zfac = w->invertedZ() ? 1 : -1;
  q = Quaternion( zfac * vec[0], zfac * vec[1], -vec[2], vec[3] );

  //cout << "rotation axis : " << q.axis() << ", angle : " 
  //     << q.angle() * 180. / M_PI << endl;
  return( q );
}


void TrackOblique::moveTrackball( int x, int y, int, int )
{
  Quaternion	q = rotation( x, y ) * _beginslice;

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
  Point3df	pos = w3->GetPosition();
  vf.erase( vf.begin() + 3 );
  vf[0] = pos[0];
  vf[1] = pos[1];
  vf[2] = pos[2];
  ex->setProperty( "position", Object::value( vf ) );
  OutputEvent	ev( "Slice", ex );
  ev.send();

  _boxviewslice->moveTrackball( x, y );
  w3->refreshNow();
}


void TrackOblique::endTrackball( int x, int y, int globalX, int globalY )
{
  _boxviewslice->endTrackball( x, y );

  Trackball::endTrackball( x, y, globalX, globalY );
}

