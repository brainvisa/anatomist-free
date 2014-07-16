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

#include <cstdlib>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/window/glwidgetmanager.h>
#include <math.h>
#include <qtimer.h>

using namespace anatomist;
using namespace aims;
using namespace std;


Action * 
Trackball::creator()
{
  return new Trackball ;
}


Trackball::Trackball()
  : Action(), _beginx( -1 ), _beginy( -1 ), _beginquat( 0, 0, 0, 0 )
{
}


Trackball::Trackball( const Trackball & a ) 
  : Action( a ), _beginx( a._beginx ), _beginy( a._beginy ), 
    _beginquat( a._beginquat )
{
}


Trackball::~Trackball()
{
}


string Trackball::name() const
{
  return "Trackball";
}


QWidget* Trackball::actionView( QWidget* )
{
  return 0;
}


bool Trackball::viewableAction() const
{
  return false;
}


void Trackball::beginTrackball( int x, int y, int, int )
{
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Trackball operating on wrong view type -- error" << endl;
      return;
    }

  _beginx = x;
  _beginy = y;
  _beginquat = w->quaternion();
}


void Trackball::endTrackball( int, int, int, int )
{
  _beginx = -1;
  _beginy = -1;

  /*GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
    if( w )
       cout << "quat : " << w->quaternion().vector() << endl;*/
}


void Trackball::moveTrackball( int x, int y, int, int )
{
  if( moveTrackballInternal( x, y ) )
    {
      GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
      if( w )
        ((AWindow3D *) w->aWindow())->refreshLightViewNow();
    }
}


bool Trackball::moveTrackballInternal( int x, int y )
{
  if( _beginx < 0 || _beginy < 0 || ( x == _beginx && y == _beginy ) )
    return false;

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Trackball operating on wrong view type -- error" << endl;
      return false;
    }

  float	dimx = w->width();
  float	dimy = w->height();

  Quaternion	q 
    = initQuaternion( ( 2. * _beginx - dimx ) / dimx,
		      ( dimy - 2. * _beginy ) / dimy,
		      ( 2. * x - dimx ) / dimx,
		      ( dimy - 2. * y ) / dimy )
    * _beginquat;
  q.norm();
  w->setQuaternion( q );
  return true;
}

Quaternion Trackball::initQuaternion( float x1, float y1, float x2, float y2 )
{
  if( x1 == x2 && y1 == y2 )
    return Quaternion( 0, 0, 0, 1 );

  Point4df	a( x1, y1, tbProj2Sphere( 0.8, x1, y1 ), 0 );
  Point4df	b( x2, y2, tbProj2Sphere( 0.8, x2, y2 ), 0 );
  Point4df	c = Quaternion::cross( b, a );
  float		t = (a - b).norm() / 1.6;

  if (t > 1.0) t = 1.0;
  if (t < -1.0) t = -1.0;

  float		phi = 2.0 * ::asin(t);
  Quaternion	q;

  q.fromAxis( Point3df( c[0], c[1], c[2] ), phi );

  return q;
}


float Trackball::tbProj2Sphere( float r, float x, float y )
{
  float d, t, z;

  d = sqrt( x*x + y*y );

  if( d < r * M_SQRT1_2 )
    z = sqrt( r*r - d*d );
  else
    { 
      t = r / M_SQRT2;
      z = t*t/d;
    }

  return z;
}


void Trackball::setCenter()
{
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Trackball operating on wrong view type -- error\n";
      return;
    }

  AWindow	*win = view()->aWindow();
  Point3df	pos = win->getPosition();
  w->setRotationCenter( pos );
  ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void Trackball::showRotationCenter()
{
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Trackball operating on wrong view type -- error\n";
      return;
    }

  Point3df		p = w->rotationCenter();
  vector<float>		vp;
  vp.push_back( p[0] );
  vp.push_back( p[1] );
  vp.push_back( p[2] );

  LinkedCursorCommand	*c = new LinkedCursorCommand( view()->aWindow(), vp );
  theProcessor->execute( c );
}


// -----------

struct ContinuousTrackball::Private
{
  Private();
  Private( const Private & );
  ~Private();

  bool		running;
  int		timestep;
  QTimer	*timer;
  Quaternion	lastrot;
  bool		frozen;
};


ContinuousTrackball::Private::Private()
  : running( false ), timestep( 20 ), timer( 0 ), frozen( false )
{
}


ContinuousTrackball::Private::Private( const Private & other )
  : running( false ), timestep( other.timestep ), timer( 0 ), frozen( false )
{
}


ContinuousTrackball::Private::~Private()
{
}


ContinuousTrackball::ContinuousTrackball()
  : QObject(), Trackball(), d( new Private )
{
}


ContinuousTrackball::ContinuousTrackball( const ContinuousTrackball & a )
  : QObject(), Trackball( a ), d( new Private( *a.d ) )
{
}


ContinuousTrackball::~ContinuousTrackball()
{
  delete d;
}


Action * ContinuousTrackball::creator()
{
  return new ContinuousTrackball;
}


string ContinuousTrackball::name() const
{
  return "ContinuousTrackball";
}


void ContinuousTrackball::beginTrackball( int x, int y, int globalX, 
                                          int globalY )
{
  d->frozen = true;
  Trackball::beginTrackball( x, y, globalX, globalY );
}


void ContinuousTrackball::moveTrackball( int x, int y, int globalX, 
                                         int globalY )
{
  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Trackball operating on wrong view type -- error" << endl;
      return;
    }
  Quaternion	q1 = w->quaternion();

  Trackball::moveTrackball( x, y, globalX, globalY );

  d->lastrot = w->quaternion() * q1.inverse();
}


void ContinuousTrackball::endTrackball( int x, int y, int globalX, 
                                        int globalY )
{
  Trackball::endTrackball( x, y, globalX, globalY );

  if( !d->running )
    return;

  if( !d->timer )
    {
      d->timer = new QTimer( this );
      connect( d->timer, SIGNAL( timeout() ), this, SLOT( goOn() ) );
    }
  d->timer->stop();
  //d->timer->changeInterval( d->timestep );
  d->frozen = false;
  d->timer->setSingleShot( true );
  d->timer->start( d->timestep );
}


void ContinuousTrackball::goOn()
{
  if( d->frozen )
    return;

  GLWidgetManager* w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
    return;

  w->setQuaternion( d->lastrot * w->quaternion() );
  ((AWindow3D *) w->aWindow())->refreshLightViewNow();

  d->timer->setSingleShot( true );
  d->timer->start( d->timestep );
}


void ContinuousTrackball::stop()
{

  if( d->timer )
    d->timer->stop();
  d->running = false;
}


void ContinuousTrackball::startOrStop()
{
  if( d->running )
    stop();
  else
    {
      d->running = true;
    }
}


// -----------


float KeyFlightAction::_maxAngle = M_PI/8;
float KeyFlightAction::_maxSpeed = 10;


Action* 
KeyFlightAction::creator() 
{
  return new KeyFlightAction ;
}

KeyFlightAction::KeyFlightAction() 
  : Action(), _angle( 0 ), _speed( 0 ), _angleChanged( false ), _auto( false )
{
}


KeyFlightAction::KeyFlightAction( const KeyFlightAction & a ) 
  : Action(), _angle( a._angle ), _speed( a._speed ), _angleChanged( false ), 
    _auto( false )
{
}


KeyFlightAction::~KeyFlightAction()
{
}


string KeyFlightAction::name() const
{
  return "KeyFlightAction" ;
}


QWidget* KeyFlightAction::actionView( QWidget* )
{
  return 0;
}


bool KeyFlightAction::viewableAction() const
{
  return false;
}


void KeyFlightAction::up()
{
  // cout << "up\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  Quaternion	q = w->quaternion();
  float		s = -sin( _angle );
  //Point3df	ax = q.apply( Point3df( 1, 0, 0 ) ) * s;

  //Quaternion	r = Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );
  Quaternion	r = Quaternion( s, 0, 0, cos( _angle ) );
  q = r * q;
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::down()
{
  //cout << "down\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  Quaternion	q = w->quaternion();
  float		s = sin( _angle );
  //  Point3df	ax = q.apply( Point3df( 1, 0, 0 ) ) * s;

  //Quaternion	r = Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );
  Quaternion	r = Quaternion( s, 0, 0, cos( _angle ) );
  q = r * q;
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::left()
{
  //cout << "left\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  Quaternion	q; // = w->quaternion();
  float		s = sin( _angle );
  //  Point3df	ax = q.apply( Point3df( 0, 1, 0 ) ) * s;

  //  q += Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );
  q = Quaternion( 0, s, 0, cos( _angle ) ) * w->quaternion();
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::right()
{
  //cout << "right\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  Quaternion	q; // = w->quaternion();
  float		s = -sin( _angle );
  //Point3df	ax = q.apply( Point3df( 0, 1, 0 ) ) * s;

  //q += Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );
  q = Quaternion( 0, s, 0, cos( _angle ) ) * w->quaternion();
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::spinLeft()
{
  //cout << "left\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  /*Quaternion	q = w->quaternion();
  float		s = sin( _angle );
  Point3df	ax = q.apply( Point3df( 0, 0, 1 ) ) * s;

  q += Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );*/

  Quaternion	q 
    = Quaternion( 0, 0, sin( _angle ), cos( _angle ) ) * w->quaternion();
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::spinRight()
{
  //cout << "left\n";
  GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "KeyFlightAction operating on wrong view type -- error\n";
      return;
    }

  increaseAngleSpeed();

  /*Quaternion	q = w->quaternion();
  float		s = -sin( _angle );
  Point3df	ax = q.apply( Point3df( 0, 0, 1 ) ) * s;

  q += Quaternion( ax[0], ax[1],  ax[2], cos( _angle ) );
  */

  Quaternion	q 
    = Quaternion( 0, 0, -sin( _angle ), cos( _angle ) ) * w->quaternion();
  q.norm();

  w->setQuaternion( q );

  if( _auto )
    _angleChanged = true;
  else
    ((AWindow3D *) w->aWindow())->refreshLightViewNow();
}


void KeyFlightAction::boost()
{
  _speed = _maxSpeed 
    * ( 1. - 0.95 * ( _maxSpeed - fabs( _speed ) ) / _maxSpeed )
    * ( _speed < 0 ? -1 : 1 );
}


void KeyFlightAction::brake()
{
  _speed = 0.8 * _speed;
}


void KeyFlightAction::reverse()
{
  if( _speed >= 0 && _speed < 1e-2 )
    {
      _speed = -1e-2;
      boost();
    }
  else if( _speed < 0 && _speed > -1e-2 )
    {
      _speed = 0;
      boost();
    }
}


void KeyFlightAction::release()
{
  cout << "KeyFlightAction::release\n";
  _angle = 0;
}


void KeyFlightAction::runStep()
{
  cout << "run\n";
  _auto = true;
  if( _speed != 0 || _angleChanged )
    {
      GLWidgetManager	* w = dynamic_cast<GLWidgetManager *>( view() );

      if( !w )
	{
	  cerr << "KeyFlightAction operating on wrong view type -- error\n";
	  return;
	}

      Point3df		c = w->rotationCenter();
      const Quaternion	& q = w->quaternion().inverse();
      Point3df		p = q.apply( Point3df( 0, 0, -_speed ) );
      p[2] = -p[2];	// invert Z axis
      //cout << "avance : " << p << endl;
      w->setRotationCenter( c + p );
      ((AWindow3D *) w->aWindow())->refreshLightViewNow();
      _angleChanged = false;
    }
}


void KeyFlightAction::stop()
{
  cout << "stop\n";
  _speed = 0;
  _auto = false;
}


void KeyFlightAction::increaseAngleSpeed()
{
  _angle = _maxAngle * ( 1. - 0.95 * ( _maxAngle - _angle ) / _maxAngle );
  //cout << "angle : " << _angle << endl;
}


void KeyFlightAction::decreaseAngleSpeed()
{
  _angle = _maxAngle * 0.8 * _angle;
}
