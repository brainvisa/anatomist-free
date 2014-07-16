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


#include <anatomist/application/fileDialog.h>
#include <anatomist/control/wControl.h>
#include <anatomist/window/Window.h>
#include <anatomist/object/Object.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/processor/event.h>
#include <anatomist/reference/Transformation.h>
#include <qfiledialog.h>
#include <algorithm>
#include <assert.h>

using namespace anatomist;
using namespace carto;
using namespace std;


bool AWindow::_leftRightDisplay = false;
bool AWindow::_hasCursor = true;
int AWindow::_cursorSize = 20;
bool AWindow::_useDefaultCursorColor = true;
AimsRGB AWindow::_cursorColor = AimsRGB( 255, 255, 255 );
float AWindow::_selectTolerence = 2.;


struct AWindow::Private
{
  Private();
  bool	destroying;
  bool	toolsvisible;
  int	hascursor;
  static map<AWindow::Type, set<unsigned> > & typecounts()
  {
    static map<AWindow::Type, set<unsigned> > tc;
    return tc;
  }
};


//--- methods -----------------------------------------------------------------


AWindow::Private::Private()
  : destroying( false ), toolsvisible( true ), hascursor( -1 )

{
}



AWindow::AWindow() : 
  _id(0),
  _refresh(false),
  _lookupChanged( true ), 
  _time( 0 ), 
  _referential( theAnatomist->centralReferential() ), 
  _geometry( 0 ), 
  _title( "Window" ), 
  _instNumber( -1 ), 
  _group( 0 ), 
  _position( Point3df( 0, 0, 0 ) ), 

  d( new Private ) 
{
  assert(theAnatomist);
  theAnatomist->registerWindow( this );
}

AWindow::~AWindow()
{
  d->destroying = true;
  disableRefCount();
  // send event
  Object	ex( (GenericObject *) 
		    new ValueObject<Dictionary> );
  ex->setProperty( "_window", this );
  set<string>	disc;
  disc.insert( "window" );
  OutputEvent	ev( "CloseWindow", ex, false, disc );
  ev.send();

  while( !_objects.empty() )
    unregisterObject( _objects.front().get() );
  if( _referential )
    _referential->RemoveWindow( this );
  theAnatomist->unregisterWindow(this);
  cleanupObserver();
  delete d;
  delete _geometry;
}


AimsRGB AWindow::cursorColor()
{
  return _cursorColor;
}


void AWindow::setCursorColor( const AimsRGB & cursCol )
{
  _cursorColor = cursCol;
}


void AWindow::displayTalairach()
{
  list<shared_ptr<AObject> >::iterator obj;

  for (obj = _objects.begin(); obj != _objects.end(); ++obj)
    (*obj)->printTalairachCoord( _position, getReferential() );
}

void AWindow::iconify()
{
}


void AWindow::unIconify()
{
  show();
}


void AWindow::show()
{
  theAnatomist->NotifyMapWindow( this );
}

void AWindow::hide()
{
  theAnatomist->NotifyUnmapWindow(this);
}

void AWindow::update(const Observable*, void* )
{
  Refresh();
}

void AWindow::unregisterObservable( Observable* obs )
{
  Observer::unregisterObservable( obs );
  AObject	*obj = dynamic_cast<AObject *>( obs );
  if( obj )
  {
    unregisterObject( obj );
    Refresh();
  }
}

void AWindow::Refresh()
{
  if( d->destroying )
    return;
  if( lookupChanged() )
    {
      setTitleWindow();
      setupWindow();
      setLookupChanged( false );
    }
}

void AWindow::registerObject( AObject* object, bool temporaryObject, int pos )
{
  if( _sobjects.find( object ) == _sobjects.end() )  
  {
    if( pos < 0 || (unsigned) pos >= _sobjects.size() )
      _objects.push_back( shared_ptr<AObject>(
                          shared_ptr<AObject>::Weak, object ) );
    else
    {
      list<carto::shared_ptr<AObject> >::iterator io;
      int i = 0;
      for( io=_objects.begin(); i<pos; ++io, ++i ) {}
      _objects.insert( io, shared_ptr<AObject>(
                       shared_ptr<AObject>::Weak, object ) );
    }
    _sobjects.insert( object );
    object->registerWindow( this );
    object->addObserver((Observer*)this);
    setLookupChanged( true );
    if( !temporaryObject )
      {
        // send event
        Object	ex( (GenericObject *)
                          new ValueObject<Dictionary> );
        ex->setProperty( "_window", this );
        ex->setProperty( "_object", object );
        OutputEvent	ev( "AddObject", ex );
        ev.send();
      }
  }
  if( temporaryObject )
    _tempObjects.insert( object );
}

void AWindow::unregisterObject( AObject* object )
{
  set<AObject *>::iterator is = _sobjects.find( object );
  if( is == _sobjects.end() )
    return;

  list<shared_ptr<AObject> >::iterator i
      = find( _objects.begin(),_objects.end(), object );
  assert( i != _objects.end() );
  _sobjects.erase( is );
  shared_ptr<AObject> so( shared_ptr<AObject>::Weak, object );
  _objects.erase( i );
  if( so.get() ) // object may be destroyed now
  {
    object->deleteObserver((Observer*)this);
    object->unregisterWindow( this );
  }
  is = _tempObjects.find( object );
  if( is != _tempObjects.end() )
    _tempObjects.erase( is );
  else
    {
      // send event
      Object	ex( (GenericObject *) 
		    new ValueObject<Dictionary> );
      ex->setProperty( "_window", this );
      ex->setProperty( "_object", object );
      static set<string>	discr;
      if( discr.empty() )
        discr.insert( "object" );
      OutputEvent	ev( "RemoveObject", ex, false, discr );
      ev.send();
    }
  setLookupChanged();
}


void AWindow::setTitle( const string & title )
{
  _title = title;
}



void AWindow::setReferential(Referential *ref )
{
  if( _referential ) _referential->RemoveWindow(this);
  _referential = ref;
  if( _referential ) _referential->AddWindow(this);
  showReferential();
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->NotifyWindowChange(this);
  setupWindow();
  Refresh();	// ?? should not be done here
}

void AWindow::setWindowGeometry(Geometry *geom )
{
  if( _geometry != geom )
    {
      delete _geometry;
    }
  _geometry = geom;
  setupWindow();
  //Refresh();	// ?? should not be done here
}

void AWindow::setTitleWindow()
{
  if( d->destroying )
    return;
  const unsigned MAXLEN = 40;
  set<AObject *> sobjects;
  set<AObject *>::const_iterator io, eo = _sobjects.end();
  // skip hidden and temporary objects
  for( io=_sobjects.begin(); io!=eo; ++io )
    if( theAnatomist->hasObject( *io ) && !isTemporary( *io ) )
      sobjects.insert( *io );
  string newtitle = Title().substr( 0, Title().find( ':' ) ) + ": " 
  		    + theAnatomist->catObjectNames( sobjects );
  if( newtitle.size() > MAXLEN )
    newtitle.replace( 37, newtitle.size(), "..." );

  setTitle( newtitle );

  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->NotifyWindowChange(this);
}


void AWindow::selectObject( float x, float y, float z, float t, int modifier )
{
  SelectFactory::select( this, Point3df( x, y, z ), t, _selectTolerence, 
			 modifier );
}


void AWindow::button3clicked( int x, int y )
{
  //	passe le b�b� � une impl�mentation externe (Qt ?)
  SelectFactory::factory()->handleSelectionMenu( this, x, y );
}


bool AWindow::hasObject( AObject * obj ) const
{
  return( _sobjects.find( obj ) != _sobjects.end() );
}


void AWindow::findObjectsAt( float x, float y, float z, float t, 
                             set<AObject *>& shown, set<AObject *>& hidden )
{
  SelectFactory::findObjectsAt( this, Point3df( x, y, z ), t, 
				_selectTolerence, shown, hidden, "default" );
}


AObject* AWindow::objectAt( float x, float y, float z, float t )
{
  set<AObject *>	shown, hidden;

  findObjectsAt( x, y, z, t, shown, hidden );
  if( shown.size() > 0 )
    return( *shown.begin() );
  if( hidden.size() > 0 )
    return( *hidden.begin() );
  return( 0 );
}


void AWindow::recordCbk( void* clientdata )
{
  AWindow	*win = (AWindow *) clientdata;

  win->recordImages();
}


void AWindow::recordImages()
{
  switch( recordingState() )
    {
    case DISABLED:
      cerr << "This window can't record images\n";
      break;
    case ON:
      stopRecord();
      break;
    case OFF:
      startRecord();
      break;
    }
}


void AWindow::startRecord()
{
  // isn't this obsolete ???

  QString filter = "JPEG base name"; filter += " (*)";
  QString caption = "Record JPEG images";

  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( caption );
  fd.setFileMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;

  QStringList filenames = fd.selectedFiles();
  if ( !filenames.isEmpty() )
  {
    QString filename = filenames[0];
    startRecord( filename.toStdString() );
  }
}


Point3df AWindow::getPosition() const
{
  return( _position );
}


void AWindow::setPosition( const Point3df& position ,
                           const Referential * orgref )
{
  anatomist::Transformation *tra = theAnatomist->getTransformation(orgref,
      getReferential());
  Point3df pos;

  if (tra)
    pos = tra->transform(position);
  else
    pos = position;
  if( pos != _position )
  {
    _position = pos;
    SetRefreshFlag();
  }
}


void AWindow::setTime( float time )
{
  if( time != _time )
  {
    _time = time;
    SetRefreshFlag();
  }
}


void AWindow::showReferential()
{
}


bool AWindow::isTemporary( AObject* o ) const
{
  return( _tempObjects.find( o ) != _tempObjects.end() );
}


void AWindow::showToolBars( int x )
{
  switch( x )
    {
    case 0:
      d->toolsvisible = false;
      break;
    case 1:
      d->toolsvisible = true;
      break;
    default:
      d->toolsvisible = !d->toolsvisible;
    }
}


bool AWindow::toolBarsVisible() const
{
  return d->toolsvisible;
}


void AWindow::setFullScreen( int )
{
}


bool AWindow::isFullScreen() const
{
  return false;
}


void AWindow::setHasCursor( int x )
{
  d->hascursor = x;
  SetRefreshFlag();
}


int AWindow::hasSelfCursor() const
{
  return d->hascursor;
}


bool AWindow::hasCursor() const
{
  if( d->hascursor >= 0 )
    return d->hascursor > 0;
  return hasGlobalCursor();
}


const string & AWindow::baseTitle() const
{
  static string bt = "AWindow";
  return bt;
}


const set<unsigned> & AWindow::typeCount() const
{
  return d->typecounts()[type()];
}


set<unsigned> & AWindow::typeCount()
{
  return d->typecounts()[type()];
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( AWindow * )
INSTANTIATE_GENERIC_OBJECT_TYPE( set<AWindow *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( vector<AWindow *> )
INSTANTIATE_GENERIC_OBJECT_TYPE( list<AWindow *> )


