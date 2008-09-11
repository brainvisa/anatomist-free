/* Copyright (c) 2007 CEA
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

#include <anatomist/commands/cLoadGraphSubObjects.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/context.h>
#include <anatomist/selection/selectFactory.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


LoadGraphSubObjectsCommand::LoadGraphSubObjectsCommand
( const set<AObject *> & graphs,int mask )
  : WaitCommand(), _graphs( graphs ), _objectsmask( mask )
{
}


LoadGraphSubObjectsCommand::~LoadGraphSubObjectsCommand()
{
}


bool LoadGraphSubObjectsCommand::initSyntax()
{
  SyntaxSet     ss;
  Syntax        & s = ss[ "LoadGraphSubObjects" ];

  s[ "objects"          ] = Semantic( "int_vector", true );
  s[ "objects_mask"     ].type = "int";
  Registry::instance()->add( "LoadGraphSubObjects", &read, ss );
  return( true );
}


void LoadGraphSubObjectsCommand::doit()
{
  set<AObject *>::iterator      i, e = _graphs.end();
  AGraph                        *gr;

  for( i=_graphs.begin(); i!=e; ++i )
    if( ( gr = dynamic_cast<AGraph *>( *i ) ) )
    {
      gr->loadSubObjects( _objectsmask );
      gr->notifyObservers( gr );
    }
}


Command* LoadGraphSubObjectsCommand::read( const Tree & com, 
                                           CommandContext* context )
{
  vector<int>           obj;
  set<AObject *>        aobj;
  int                   mask = 3;
  unsigned              i, n;
  void                  *ptr;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );
  com.getProperty( "objects_mask", mask );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
        aobj.insert( (AObject *) ptr );
      else
        cerr << "object id " << obj[i] << " not found\n";
    }

  return new LoadGraphSubObjectsCommand( aobj, mask );
}


void LoadGraphSubObjectsCommand::write( Tree & com, Serializer* ser ) const
{
  Tree                          *t = new Tree( true, name() );

  set<AObject *>::const_iterator        io;
  vector<int>                           obj;
  for( io=_graphs.begin(); io!=_graphs.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  t->setProperty( "objects", obj );
  if( _objectsmask != 3 )
    t->setProperty( "objects_mask", _objectsmask );

  com.insert( t );
}


