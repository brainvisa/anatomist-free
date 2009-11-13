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


#include <anatomist/window3D/trackcut.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/object/Object.h>
#include <anatomist/object/selfsliceable.h>
#include <anatomist/controler/view.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/processor/event.h>
#include <iostream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


TrackCutAction::TrackCutAction() : Trackball()
{
}


TrackCutAction::TrackCutAction( const TrackCutAction & a ) : Trackball( a )
{
}


TrackCutAction::~TrackCutAction()
{
}


Action* TrackCutAction::creator()
{
  return new TrackCutAction;
}


string TrackCutAction::name() const
{
  return( "TrackCutAction" );
}


void TrackCutAction::beginTrackball( int x, int y, int globalX, int globalY )
{
  Trackball::beginTrackball( x, y, globalX, globalY );

  _begin = Quaternion( 0, 0, 0, 1 );
  _cuts.clear();
  _slices.clear();
}


Quaternion TrackCutAction::rotation( int x, int y )
{
  if( _beginx < 0 || _beginy < 0 )
    return( Quaternion( 0, 0, 0, 1 ) );

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "TrackCutAction operating on wrong view type -- error\n";
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


void TrackCutAction::moveTrackball( int x, int y, int, int )
{
  Quaternion	q = rotation( x, y ) * _begin;

  q.norm();

  AWindow				*w = view()->window();
  set<AObject *>			obj = w->Objects();
  set<AObject *>::iterator		io, eo = obj.end();
  SelfSliceable				*sl;
  map<AObject *, Point3df>::iterator	ic, ec = _cuts.end();
  Point3df				n, center, bmin, bmax;
  GLWidgetManager	*glw = dynamic_cast<GLWidgetManager *>( view() );
  map<SelfSliceable *, AObject *>		sls;
  map<SelfSliceable *, AObject *>::iterator	isls, esls = sls.end();
  AObject					*o;

  if( glw )
    center = glw->rotationCenter();

  SelectFactory	*sf = SelectFactory::factory();

  for( io=obj.begin(); io!=eo; ++io )
  {
    sl = dynamic_cast<SelfSliceable *>( *io );
    if( sl && sf->isSelected( w->Group(), *io ) )
      sls[ sl ] = *io;
  }
  if( sls.empty() )
    for( io=obj.begin(); io!=eo; ++io )
    {
      sl = dynamic_cast<SelfSliceable *>( *io );
      if( sl )
        sls[ sl ] = *io;
    }

  for( isls=sls.begin(); isls!=esls; ++isls )
    {
      sl = isls->first;
      o = isls->second;
      map<AObject *, Quaternion>::iterator	is = _slices.find( o );
      Quaternion				q2;
      if( is == _slices.end() )
        {
          q2 = sl->quaternion();
          _slices[ o ] = q2;
        }
      else
        q2 = is->second;
      sl->setQuaternion( q * q2 );
      o->notifyObservers( this );

      // send event
      Object	ex = Object::value( Dictionary() );
      set<AObject *>	nobj;
      nobj.insert( o );
      ex->setProperty( "_objects", Object::value( nobj ) );
      vector<float>	vf(4);
      Point4df	qv = sl->quaternion().vector();
      vf[0] = qv[0];
      vf[1] = qv[1];
      vf[2] = qv[2];
      vf[3] = qv[3];
      ex->setProperty( "slice_quaternion", Object::value( vf ) );
      Point3df	pos = sl->offset();
      vf.erase( vf.begin() + 3 );
      vf[0] = pos[0];
      vf[1] = pos[1];
      vf[2] = pos[2];
      ex->setProperty( "position", Object::value( vf ) );
      OutputEvent	ev( "ObjectSlice", ex );
      ev.send();
    }
}


void TrackCutAction::axialSlice()
{
  setSlice( Quaternion( 0, 0, 0, 1 ) );
}


void TrackCutAction::coronalSlice()
{
  static const float	c = 1. / sqrt( 2. );
  setSlice( Quaternion( c, 0, 0, c ) );
}


void TrackCutAction::sagittalSlice()
{
  setSlice( Quaternion( -0.5, -0.5, -0.5, 0.5 ) );
}


void TrackCutAction::setSlice( const Quaternion & q )
{
  Point3df	plane = q.applyInverse( Point3df( 0, 0, 1 ) );
  AWindow			*w = view()->window();
  set<AObject *>		obj = w->Objects();
  set<AObject *>::iterator	io, eo = obj.end();
  SelfSliceable			*sl;
  GLWidgetManager	*glw = dynamic_cast<GLWidgetManager *>( view() );
  Point3df			n, center, bmin, bmax;
  map<SelfSliceable *, AObject *>               sls;
  map<SelfSliceable *, AObject *>::iterator     isls, esls = sls.end();

  if( glw )
    center = glw->rotationCenter();

  SelectFactory	*sf = SelectFactory::factory();

  for( io=obj.begin(); io!=eo; ++io )
  {
    sl = dynamic_cast<SelfSliceable *>( *io );
    if( sl && sf->isSelected( w->Group(), *io ) )
      sls[ sl ] = *io;
  }
  if( sls.empty() )
    for( io=obj.begin(); io!=eo; ++io )
  {
    sl = dynamic_cast<SelfSliceable *>( *io );
    if( sl )
      sls[ sl ] = *io;
  }

  for( isls=sls.begin(); isls!=esls; ++isls )
  {
    sl = isls->first;
    sl->setQuaternion( q );
    isls->second->notifyObservers( this );

    // send event
    Object	ex = Object::value( Dictionary() );
    set<AObject *>	nobj;
    nobj.insert( *io );
    ex->setProperty( "_objects", Object::value( nobj ) );
    vector<float>	vf(4);
    Point4df	qv = q.vector();
    vf[0] = qv[0];
    vf[1] = qv[1];
    vf[2] = qv[2];
    vf[3] = qv[3];
    ex->setProperty( "slice_quaternion", Object::value( vf ) );
    OutputEvent	ev( "ObjectSlice", ex );
    ev.send();
  }
}


// -------------

CutSliceAction::CutSliceAction() : Action()
{
}


CutSliceAction::CutSliceAction( const CutSliceAction & a ) : Action( a )
{
}


CutSliceAction::~CutSliceAction()
{
}


Action* CutSliceAction::creator()
{
  return new CutSliceAction;
}


string CutSliceAction::name() const
{
  return( "CutSliceAction" );
}


void CutSliceAction::beginTrack( int, int y, int, int )
{
  _y = y;
}


void CutSliceAction::moveTrack( int, int y, int, int )
{
  if( y == _y )
    return;
  GLWidgetManager * wid = dynamic_cast<GLWidgetManager *>( view() );

  if( !wid )
  {
    cerr << "CutSliceAction operating on wrong view type -- error\n";
    return;
  }

  float dimy = wid->height();
  Point3df  dims = wid->windowBoundingMax() - wid->windowBoundingMin();
  float scl = max( max( dims[0], dims[1] ), dims[2] ) / dimy;

  AWindow				*w = view()->window();
  set<AObject *>			obj = w->Objects();
  set<AObject *>::iterator		io, eo = obj.end();
  SelfSliceable				*sl;
  Quaternion				q;
  Point3df				p;
  map<SelfSliceable *, AObject *>		sls;
  map<SelfSliceable *, AObject *>::iterator	isls, esls = sls.end();

  SelectFactory	*sf = SelectFactory::factory();

  for( io=obj.begin(); io!=eo; ++io )
  {
    sl = dynamic_cast<SelfSliceable *>( *io );
    if( sl && sf->isSelected( w->Group(), *io ) )
      sls[ sl ] = *io;
  }
  if( sls.empty() )
    for( io=obj.begin(); io!=eo; ++io )
  {
    sl = dynamic_cast<SelfSliceable *>( *io );
    if( sl )
      sls[ sl ] = *io;
  }

  for( isls=sls.begin(); isls!=esls; ++isls )
  {
    sl = isls->first;
    q = sl->quaternion();
    p = q.apply( Point3df( 0, 0, scl * ( y - _y ) ) );
    sl->setOffset( sl->offset() + p );
    isls->second->notifyObservers( this );

    // send event
    Object	ex = Object::value( Dictionary() );
    set<AObject *>	nobj;
    nobj.insert( isls->second );
    ex->setProperty( "_objects", Object::value( nobj ) );
    vector<float>	vf(3);
    vf[0] = p[0];
    vf[1] = p[1];
    vf[2] = p[2];
    ex->setProperty( "slice_offset", Object::value( vf ) );
    OutputEvent	ev( "ObjectSlice", ex );
    ev.send();
  }
  _y = y;
}


