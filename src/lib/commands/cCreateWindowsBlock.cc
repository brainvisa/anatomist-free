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

#include <anatomist/commands/cCreateWindowsBlock.h>
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
#include <QWidgetList>
#include <qwidget.h>
#include <anatomist/application/Anatomist.h>

using namespace anatomist;
using namespace carto;
using namespace std;


CreateWindowsBlockCommand::CreateWindowsBlockCommand(
  int id,
  CommandContext* context,
  const vector<int> & geom,
  int cols, int rows )
  : RegularCommand(), SerializingCommand( context ),
    _id( id ), _geom( geom ), _cols(cols), _rows( rows )
{
}


CreateWindowsBlockCommand::~CreateWindowsBlockCommand()
{
}


bool CreateWindowsBlockCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "CreateWindowsBlock" ];
  
  s[ "res_pointer"   ] = Semantic( "int", true );
  s[ "block_columns" ] = Semantic( "int", false );
  s[ "block_rows"    ] = Semantic( "int", false );
  s[ "geometry"      ] = Semantic( "int_vector", false );
  Registry::instance()->add( "CreateWindowsBlock", &read, ss );
  return( true );
}


void CreateWindowsBlockCommand::doit()
{
    // create a block widget
    int colsrows = _rows;
    bool inrows = false;
    if( colsrows == 0 && _cols > 0 )
    {
      inrows = false;
      colsrows = _cols;
    }
    QAWindowBlock	*dk
      = new QAWindowBlock( theAnatomist->getQWidgetAncestor(), NULL,
                           Qt::Window, colsrows, inrows );
    dk->show();
    _block = dk;
    if( _id > 0 && context() && context()->unserial )
      context()->unserial->registerPointer( (void *) _block, _id,
                                            "Widget" );
    if( _geom.size() == 4 )
      _block->setGeometry( _geom[0], _geom[1], _geom[2], _geom[3] );

  Object    ex( (GenericObject *) new ValueObject<Dictionary> );
    ex->setProperty( "_block", Object::value( _block ) );
    OutputEvent       ev( "CreateWindowBlock", ex );
    ev.send();
}


void CreateWindowsBlockCommand::undoit()
{
  delete _block;
  _block = 0;
}


Command* CreateWindowsBlockCommand::read( const Tree & com,
                                          CommandContext* context )
{
  int		id, cols = 0, rows = 0;
  vector<int>	geom;

  if( !com.getProperty( "res_pointer", id ) )
    return 0;
  com.getProperty( "geometry", geom );
  com.getProperty( "block_columns", cols );
  com.getProperty( "block_rows", rows );
  Object	options;
  return new CreateWindowsBlockCommand( id, context, geom, cols, rows );
}


void CreateWindowsBlockCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  t->setProperty( "res_pointer", ser->serialize( _block ) );
  if( _cols )
    t->setProperty( "block_columns", _cols );
  else if( _rows )
    t->setProperty( "block_rows", _rows );
  com.insert( t );
}
