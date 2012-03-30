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
#include <anatomist/application/Anatomist.h>

using namespace anatomist;
using namespace carto;
using namespace std;


CreateWindowCommand::CreateWindowCommand( const string & type, int id, 
					  CommandContext* context, 
					  const vector<int> & geom, 
                                          int blockid, QWidget *block,
                                          int cols, int rows,
                                          Object options )
  : RegularCommand(), SerializingCommand( context ), _type( type ), _win( 0 ), 
    _id( id ), _geom( geom ), _blockid( blockid ), _block( block ),
    _cols(cols), _rows( rows ),
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
  s[ "block_rows"    ] = Semantic( "int", false );
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
      QWidget		*w =  _block;
      QWidgetList	wl4 = qApp->topLevelWidgets();
      QWidget		*w2 = 0;
      int		i, n = wl4.size();
      for( i=0; i < n && (w2=wl4.at(i)) != w; ++i );
      if( w != w2 )
        {
          wl4 = qApp->allWidgets();
          for( i=0, n=wl4.size(); i < n && (w2=wl4.at(i)) != w; ++i );
        }
      if( w != w2 )
        _block = 0; // not living anymore
    }
  if( _blockid != 0 && !_block )
    {
      // create a block widget
      int colsrows = _rows;
      bool inrows = false;
      if( colsrows == 0 && _cols > 0 )
      {
        inrows = false;
        colsrows = _cols;
      }
      QAWindowBlock	*dk = new QAWindowBlock( theAnatomist->getQWidgetAncestor(), NULL,
        Qt::Window, colsrows, inrows );
      dk->show();
      _block = dk;
      if( _blockid > 0 && context() && context()->unserial )
        context()->unserial->registerPointer( (void *) _block, _blockid,
                                              "Widget" );
    }
  else if( _blockid == 0 )
    _block = 0;
  else if( _block )
  {
    QAWindowBlock *qb = dynamic_cast<QAWindowBlock *>( _block );
    if( qb )
    {
      // check if block colums/rows should be changed or not
      int inrows = true;
      int colsrows = _cols;
      if( _cols == 0 && _rows != 0 )
      {
        inrows = false;
        colsrows = _rows;
      }
      if( colsrows > 0 )
        qb->setColsOrRows( inrows, colsrows );
    }
  }

  _win = AWindowFactory::createWindow( _type, _block, _options );
  if( _win )
    {
      if( _id >= 0 && context() && context()->unserial )
        context()->unserial->registerPointer( _win, _id, "AWindow" );
      if( _geom.size() == 4 )
        _win->setGeometry( _geom[0], _geom[1], _geom[2], _geom[3] );
      // send event
      if( _block )
      {
        QAWindowBlock *qwb = dynamic_cast<QAWindowBlock *>( _block );
        if( qwb )
          qwb->addWindowToBlock((AWindow3D *)_win);
      }
      Object	ex( (GenericObject *) new ValueObject<Dictionary> );
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
  int		id, blockid = 0, cols = 0, rows = 0;
  vector<int>	geom;
  void		*ptr = 0;

  if( !com.getProperty( "type", type )
      || !com.getProperty( "res_pointer", id ) )
    return( 0 );
  com.getProperty( "geometry", geom );
  com.getProperty( "block", blockid );
  com.getProperty( "block_columns", cols );
  com.getProperty( "block_rows", rows );
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
		  blockid, (QWidget *) ptr, cols, rows, options );
}


void CreateWindowCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "type", _type );
  t->setProperty( "res_pointer", ser->serialize( _win ) );
  if( _blockid != 0 && _block )
    t->setProperty( "block", ser->serialize( _block ) );
  if( _cols )
    t->setProperty( "block_columns", _cols );
  else if( _rows )
    t->setProperty( "block_rows", _rows );
  if( _options.get() )
    t->setProperty( "options", _options );
  com.insert( t );
}
