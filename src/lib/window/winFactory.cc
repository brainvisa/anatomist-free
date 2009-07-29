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


#include <anatomist/window/winFactory.h>
#include <anatomist/window3D/window3D.h>


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
  
  Creators[ AWindow::AXIAL_WINDOW    ] = createAxial;
  Creators[ AWindow::CORONAL_WINDOW  ] = createCoronal;
  Creators[ AWindow::SAGITTAL_WINDOW ] = createSagittal;
  Creators[ AWindow::WINDOW_3D       ] = create3D;

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
  map<int, WinCreator>::const_iterator	it = Creators.find( type );
  if( it == Creators.end() )
    return( 0 );
  return (*it).second( dock, params );
}


AWindow* AWindowFactory::createAxial( void *dock, carto::Object params )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( params )
    try
    {
      f = Qt::WFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  AWindow	*w = new AWindow3D( AWindow3D::Axial, dk, params, f );
  w->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return( w );
}


AWindow* AWindowFactory::createCoronal( void *dock, carto::Object params )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( params )
    try
    {
      f = Qt::WFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  AWindow	*w = new AWindow3D( AWindow3D::Coronal, dk, params, f );
  w->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return( w );
}


AWindow* AWindowFactory::createSagittal( void *dock, carto::Object params )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( params )
    try
    {
      f = Qt::WFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  AWindow	*w = new AWindow3D( AWindow3D::Sagittal, dk, params, f );
  w->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return( w );
}


AWindow* AWindowFactory::create3D( void *dock, carto::Object params )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( params )
    try
    {
      f = Qt::WFlags( (int) params->getProperty( "wflags" )->getScalar() );
    }
    catch( ... )
    {
    }

  if( dock )
    f = 0;
  AWindow3D	*w = new AWindow3D( AWindow3D::ThreeD, dk, params, f );
  w->show();
  if( dk )
    dk->resize( dk->sizeHint() );

  return( w );
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


int AWindowFactory::registerType( const string & type, WinCreator creator )
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
  Creators[ itype ] = creator;

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
