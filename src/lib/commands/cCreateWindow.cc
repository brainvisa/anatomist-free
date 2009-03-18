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

#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/qwinblock.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/event.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <qapplication.h>
#include <aims/qtcompat/qwidgetlist.h>
#include <qwidget.h>
#include <anatomist/window3D/window3D.h>

using namespace anatomist;
using namespace carto;
using namespace std;


CreateWindowCommand::CreateWindowCommand( const string & type, int id, 
					  CommandContext* context, 
					  const vector<int> & geom, 
                                          int blockid, QWidget *block,
                                          int cols,
                                          Object options )
  : RegularCommand(), SerializingCommand( context ), _type( type ), _win( 0 ), 
    _id( id ), _geom( geom ), _blockid( blockid ), _block( block ),
    _cols(cols),
    _options( options )
{
}


CreateWindowCommand::~CreateWindowCommand()
{
}


bool CreateWindowCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "CreateWindow" ];
  
  s[ "type"          ] = Semantic( "string", true );
  s[ "res_pointer"   ] = Semantic( "int", true );
  s[ "block_columns" ] = Semantic( "int", false );
  s[ "geometry"      ] = Semantic( "int_vector", false );
  s[ "block"         ] = Semantic( "int", false );
  s[ "options"       ] = Semantic( "dictionary", false );
  Registry::instance()->add( "CreateWindow", &read, ss );
  return( true );
}


void CreateWindowCommand::doit()
{
  if( _block )
    {
      // check if the block still exists
      QWidget		*w =  (QWidget *) _block;
#if QT_VERSION >= 0x040000
      QWidgetList	wl4 = qApp->topLevelWidgets();
      QWidget		*w2 = 0;
      int		i, n = wl4.size();
      for( i=0; i < n && (w2=wl4.at(i)) != w; ++i );
      if( w != w2 )
        {
          wl4 = qApp->allWidgets();
          for( i=0, n=wl4.size(); i < n && (w2=wl4.at(i)) != w; ++i );
        }
#else
      QWidgetList	*wl = qApp->topLevelWidgets();
      QWidgetListIt it( *wl );
      QWidget		*w2;
      while( (w2=it.current()) != 0 && w2 != w )
        ++it;
      delete wl;
      if( w != w2 )
        {
          wl = qApp->allWidgets();
          it = *wl;
          while( (w2=it.current()) != 0 && w2 != w )
            ++it;
          delete wl;
        }
#endif
      if( w != w2 )
        _block = 0; // not living anymore
    }
  if( _blockid != 0 && !_block )
    {
      // create a block widget
      QAWindowBlock	*dk = new QAWindowBlock(NULL, NULL,
			Qt::WType_TopLevel | Qt::WDestructiveClose, _cols);
      dk->show();
      _block = dk;
      if( _blockid > 0 && context() && context()->unserial )
	context()->unserial->registerPointer( (void *) _block,
					_blockid, "Widget" );
    }
  else if( _blockid == 0 )
    _block = 0;

  _win = AWindowFactory::createWindow( _type, (QWidget *) _block,
		  					_options );
  if( _win )
    {
      if( _id >= 0 && context() && context()->unserial )
	context()->unserial->registerPointer( _win, _id, "AWindow" );
      if( _geom.size() == 4 )
	_win->setGeometry( _geom[0], _geom[1], _geom[2], _geom[3] );
      // send event
      if (_block){
        ((QAWindowBlock*)_block)->addWindowToBlock((AWindow3D *)_win);
      
      }
      Object	ex( (GenericObject *) 
		    new ValueObject<Dictionary> );
      ex->setProperty( "_window", Object::value( _win ) );
      ex->setProperty( "type", Object::value( _type ) );
      OutputEvent	ev( "CreateWindow", ex );
      ev.send();
    }
}


void CreateWindowCommand::undoit()
{
  delete _win;
  _win = 0;
}


Command* CreateWindowCommand::read( const Tree & com, CommandContext* context )
{
  string	type;
  int		id, blockid = 0, cols = 2;
  vector<int>	geom;
  void		*ptr = 0;

  if( !com.getProperty( "type", type )
      || !com.getProperty( "res_pointer", id ) )
    return( 0 );
  com.getProperty( "geometry", geom );
  com.getProperty( "block", blockid );
  com.getProperty( "block_columns", cols );
  if( blockid > 0 )
    ptr = context->unserial->pointer( blockid, "Widget" );
  Object	options;
  try
    {
      options = com.getProperty( "options" );
    }
  catch( ... )
    {
    }
  return new CreateWindowCommand( type, id, context, geom,
		  blockid, (QWidget *) ptr, cols, options );
}


void CreateWindowCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "type", _type );
  t->setProperty( "res_pointer", ser->serialize( _win ) );
  if( _blockid != 0 && _block )
    t->setProperty( "block", ser->serialize( _block ) );
  if( _options.get() )
    t->setProperty( "options", _options );
  com.insert( t );
}
