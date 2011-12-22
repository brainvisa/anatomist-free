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

#include <anatomist/commands/cWindowBlock.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/window/qwinblock.h>
#include <anatomist/processor/context.h>
// #include <anatomist/processor/event.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <qapplication.h>
#include <aims/qtcompat/qwidgetlist.h>
#include <qwidget.h>

using namespace anatomist;
using namespace carto;
using namespace std;


WindowBlockCommand::WindowBlockCommand( int id,
                                        CommandContext* context,
                                        QWidget *block,
                                        int cols, int rows,
                                        bool makerect, float rectratio,
                                        const vector<int> & geom )
  : RegularCommand(), SerializingCommand( context ), _id( id ),
    _block( block ), _cols(cols), _rows( rows ), _rect( makerect ),
    _rectratio( rectratio ), _geom( geom )
{
}


WindowBlockCommand::~WindowBlockCommand()
{
}


bool WindowBlockCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "WindowBlock" ];

  s[ "block"           ] = Semantic( "int", true );
  s[ "block_columns"   ] = Semantic( "int", false );
  s[ "block_rows"      ] = Semantic( "int", false );
  s[ "geometry"        ] = Semantic( "int_vector", false );
  s[ "make_rectangle"  ] = Semantic( "int", false );
  s[ "rectangle_ratio" ] = Semantic( "float", false );
  Registry::instance()->add( "WindowBlock", &read, ss );
  return( true );
}


void WindowBlockCommand::doit()
{
  QAWindowBlock *block = 0;
  if( _block )
  {
    // check if the block exists
    QWidget           *w =  _block;
    QWidgetList       wl4 = qApp->topLevelWidgets();
    QWidget           *w2 = 0;
    int               i, n = wl4.size();
    for( i=0; i < n && (w2=wl4.at(i)) != w; ++i );
    if( w != w2 )
      {
        wl4 = qApp->allWidgets();
        for( i=0, n=wl4.size(); i < n && (w2=wl4.at(i)) != w; ++i );
      }
    if( w != w2 )
      _block = 0; // not living anymore
  }
  if( _id != 0 && !_block )
  {
    // create a block widget
    int colsrows = _cols;
    bool inrows = true;
    if( colsrows == 0 && _rows > 0 )
    {
      inrows = false;
      colsrows = _rows;
    }
    block = new QAWindowBlock( 0, 0,
      Qt::WType_TopLevel | Qt::WDestructiveClose, colsrows, inrows );
    block->show();
    _block = block;
    if( _id > 0 && context() && context()->unserial )
      context()->unserial->registerPointer( (void *) _block, _id, "Widget" );
  }
  else if( _id == 0 )
    _block = 0;
  else if( _block )
  {
    block = dynamic_cast<QAWindowBlock *>( _block );
    if( block )
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
        block->setColsOrRows( inrows, colsrows );
    }
  }
  if( _block )
  {
    if( block && _rect )
      block->arrangeInRect( _rectratio );
    if( _geom.size() == 4 )
      _block->setGeometry( _geom[0], _geom[1], _geom[2], _geom[3] );
  }
}


Command* WindowBlockCommand::read( const Tree & com, CommandContext* context )
{
  string        type;
  int           id, cols = 0, rows = 0, rect = 0;
  vector<int>   geom;
  void          *ptr = 0;
  float         rectratio = 1.;

  if( !com.getProperty( "block", id ) )
    return 0;
  com.getProperty( "geometry", geom );
  com.getProperty( "block_columns", cols );
  com.getProperty( "block_rows", rows );
  com.getProperty( "make_rectangle", rect );
  com.getProperty( "rectangle_ratio", rectratio );
  if( id > 0 )
    ptr = context->unserial->pointer( id, "Widget" );
  return new WindowBlockCommand( id, context, (QWidget *) ptr, cols, rows,
                                 (bool) rect, rectratio, geom );
}


void WindowBlockCommand::write( Tree & com, Serializer* ser ) const
{
  Tree  *t = new Tree( true, name() );

  if( _id != 0 && _block )
    t->setProperty( "block", ser->serialize( _block ) );
  if( _cols )
    t->setProperty( "block_columns", _cols );
  else if( _rows )
    t->setProperty( "block_rows", _rows );
  if( _rect )
    t->setProperty( "make_rectangle", (int) 1 );
  if( _rectratio != 1. )
    t->setProperty( "rectangle_ratio", _rectratio );
  com.insert( t );
}
