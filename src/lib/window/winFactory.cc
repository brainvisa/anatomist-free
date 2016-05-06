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


#include <anatomist/window/winFactory.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/control/controlMenuHandler.h>
#include <anatomist/control/wControl.h>
#include <anatomist/selection/qSelMenu.h>


using namespace anatomist;
using namespace carto;
using namespace std;

//	static member variables have been moved to
//	<anatomist/application/statics.h>


bool AWindowFactory::initTypes()
{
  if( !TypeNames.empty() )
    return( false );

  TypeNames[ AWindow::AXIAL_WINDOW    ] = QT_TRANSLATE_NOOP( "ControlWindow",
							     "Axial" );
  TypeNames[ AWindow::CORONAL_WINDOW  ] = QT_TRANSLATE_NOOP( "ControlWindow",
							     "Coronal" );
  TypeNames[ AWindow::SAGITTAL_WINDOW ] = QT_TRANSLATE_NOOP( "ControlWindow",
							     "Sagittal" );
  TypeNames[ AWindow::WINDOW_3D       ] = QT_TRANSLATE_NOOP( "ControlWindow",
							     "3D" );

  TypeID[ "Axial"    ] = AWindow::AXIAL_WINDOW;
  TypeID[ "Coronal"  ] = AWindow::CORONAL_WINDOW;
  TypeID[ "Sagittal" ] = AWindow::SAGITTAL_WINDOW;
  TypeID[ "3D"       ] = AWindow::WINDOW_3D;

  Creators[ AWindow::AXIAL_WINDOW    ]
      = rc_ptr<AWindowCreator>( new AWindowCreatorFunc( createAxial ) );
  Creators[ AWindow::CORONAL_WINDOW  ]
      = rc_ptr<AWindowCreator>( new AWindowCreatorFunc( createCoronal ));
  Creators[ AWindow::SAGITTAL_WINDOW ]
      = rc_ptr<AWindowCreator>( new AWindowCreatorFunc( createSagittal ));
  Creators[ AWindow::WINDOW_3D       ]
      = rc_ptr<AWindowCreator>( new AWindowCreatorFunc( create3D ));

  CtrlWinVisibleTypes.insert( AWindow::AXIAL_WINDOW );
  CtrlWinVisibleTypes.insert( AWindow::SAGITTAL_WINDOW );
  CtrlWinVisibleTypes.insert( AWindow::CORONAL_WINDOW );
  CtrlWinVisibleTypes.insert( AWindow::WINDOW_3D );

  return( true );
}


AWindow* AWindowFactory::createWindow( const string & type, void *dock,
                                       carto::Object params )
{
  map<string, int>::const_iterator	it = TypeID.find( type );

  if( it == TypeID.end() )
    return( 0 );
  return createWindow( (*it).second, dock, params );
}


AWindow* AWindowFactory::createWindow( int type, void *dock,
                                       carto::Object params )
{
  map<int, rc_ptr<AWindowCreator> >::const_iterator
      it = Creators.find( type );
  if( it == Creators.end() )
    return 0;
  AWindow *win = (*it->second)( dock, params );
  if( win )
    applyCommonOptions( win, params );
  return win;
}


void AWindowFactory::applyCommonOptions( AWindow* win, Object params )
{
  bool hidden = false;
  if( params )
    try
    {
      hidden = bool( (int) params->getProperty( "hidden" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( !hidden )
  {
    win->show();
    QWidget* qw = dynamic_cast<QWidget *>( win );
    if( qw && qw->parentWidget() )
        qw->parentWidget()->resize( qw->parentWidget()->sizeHint() );
  }
}


AWindow* AWindowFactory::createAxial( void *dock, carto::Object params )
{
  QWidget	*dk = static_cast<QWidget *>( dock );
  Qt::WindowFlags	f = 0;
  if( dk )
    f |= Qt::Window;
  if( params )
    try
    {
      f = Qt::WindowFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  QAWindow	*w = new AWindow3D( AWindow3D::Axial, dk, params, f );
  w->setAttribute( Qt::WA_DeleteOnClose );
  return w;
}


AWindow* AWindowFactory::createCoronal( void *dock, carto::Object params )
{
  QWidget       *dk = static_cast<QWidget *>( dock );
  Qt::WindowFlags    f = 0;
  if( dk )
    f |= Qt::Window;
  if( params )
    try
    {
      f = Qt::WindowFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  QAWindow	*w = new AWindow3D( AWindow3D::Coronal, dk, params, f );
  w->setAttribute( Qt::WA_DeleteOnClose );
  return w;
}


AWindow* AWindowFactory::createSagittal( void *dock, carto::Object params )
{
  QWidget       *dk = static_cast<QWidget *>( dock );
  Qt::WindowFlags    f = 0;
  if( dk )
    f |= Qt::Window;
  if( params )
    try
    {
      f = Qt::WindowFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  QAWindow	*w = new AWindow3D( AWindow3D::Sagittal, dk, params, f );
  w->setAttribute( Qt::WA_DeleteOnClose );
  return w;
}


AWindow* AWindowFactory::create3D( void *dock, carto::Object params )
{
  QWidget       *dk = static_cast<QWidget *>( dock );
  Qt::WindowFlags    f = 0;
  if( dk )
    f |= Qt::Window;
  if( params )
    try
    {
      f = Qt::WindowFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  QAWindow	*w = new AWindow3D( AWindow3D::ThreeD, dk, params, f );
  w->setAttribute( Qt::WA_DeleteOnClose );

  return w;
}


bool AWindowFactory::exist( int type )
{
  map<int, string>::const_iterator	it = TypeNames.find( type );
  if( it == TypeNames.end() )
    return( false );
  return( true );
}


bool AWindowFactory::exist( const string & type )
{
  map<string, int>::const_iterator	it = TypeID.find( type );

  if( it == TypeID.end() )
    return( false );
  return( true );
}


string AWindowFactory::typeString( int type, int subtype )
{
  if( subtype != 0 )
    type = subtype;

  map<int, string>::const_iterator	it = TypeNames.find( type );
  if( it == TypeNames.end() )
    return( "" );
  return( (*it).second );
}


int AWindowFactory::typeID( const string & type )
{
  map<string, int>::const_iterator	it = TypeID.find( type );

  if( it == TypeID.end() )
    return( 0 );
  return( (*it).second );
}


int AWindowFactory::registerType( const string & type, WinCreator creator,
                                  bool visibleinCtrlWin )
{
  return registerType( type, new AWindowCreatorFunc( creator ),
                       visibleinCtrlWin );
}


int AWindowFactory::registerType( const string & type,
                                  AWindowCreator *creator,
                                  bool visibleinCtrlWin )
{
  map<string, int>::const_iterator	it = TypeID.find( type );

  if( it != TypeID.end() )
    {
      return( (*it).second );
    }

  int	itype = AWindow::OTHER + 1;
  map<int, string>::const_iterator	in = TypeNames.find( itype ),
    fn=TypeNames.end();

  for( ; in!=fn && (*in).first==itype; ++in, ++itype ) {}
  TypeID[ type ] = itype;
  TypeNames[ itype ] = type;
  Creators[ itype ] = rc_ptr<AWindowCreator>( creator );
  if( visibleinCtrlWin )
    CtrlWinVisibleTypes.insert( itype );

  ControlWindow* cw = theAnatomist->getControlWindow();
  if( cw )
  {
    AControlMenuHandler* mh = cw->menuHandler();
    mh->addWindowType( type, itype );
    if( visibleinCtrlWin )
      cw->setWindowTypeVisible( itype, true );
  }

  return( itype );
}


set<string> AWindowFactory::types()
{
  set<string>	t;
  map<string, int>::const_iterator	it, ft = TypeID.end();

  for( it=TypeID.begin(); it!=ft; ++it )
    t.insert( (*it).first );

  return( t );
}


bool AWindowFactory::hasControlWindowButton( int type )
{
  set<int>::const_iterator i = CtrlWinVisibleTypes.find( type );
  return i != CtrlWinVisibleTypes.end();
}


void AWindowFactory::setHasControlWindowButton( int type, bool visible )
{
  if( visible )
    CtrlWinVisibleTypes.insert( type );
  else
    CtrlWinVisibleTypes.erase( type );
  ControlWindow* cw = theAnatomist->getControlWindow();
  if( cw )
    cw->setWindowTypeVisible( type, visible );
}


AWindowCreator::~AWindowCreator()
{
}


AWindowCreatorFunc::~AWindowCreatorFunc()
{
}


AWindow* AWindowCreatorFunc::operator () ( void *dock, Object options ) const
{
  return _func( dock, options );
}

