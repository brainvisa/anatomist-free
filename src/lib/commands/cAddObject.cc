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

//--- header files ------------------------------------------------------------

#include <anatomist/commands/cAddObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/Window.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/processor/context.h>
#include <anatomist/graph/Graph.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>


using namespace anatomist;
using namespace carto;
using namespace std;

//-----------------------------------------------------------------------------

AddObjectCommand::AddObjectCommand( const set<AObject *> & objL, 
                                    const set<AWindow*> & winL,
                                    bool addchildren, bool addnodes,
                                    bool addrels ) 
  : RegularCommand(), _objL( objL ), _winL( winL ),
    _addchildren( addchildren ),_addnodes( addnodes ), _addrels( addrels )
{
}


AddObjectCommand::~AddObjectCommand()
{
}


bool AddObjectCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "AddObject" ];
  
  s[ "objects" ].type = "int_vector";
  s[ "objects" ].needed = true;
  s[ "windows" ].type = "int_vector";
  s[ "windows" ].needed = true;
  s[ "add_graph_nodes" ] = Semantic( "int", false );
  s[ "add_graph_relations" ] = Semantic( "int", false );
  s[ "add_children" ] = Semantic( "int", false );
  Registry::instance()->add( "AddObject", &read, ss );
  return( true );
}


namespace
{

  void addobject( AObject* o, const set<AObject *> & obj, 
                  const set<AWindow *> & win, bool addchildren, bool addnodes,
                  bool addrels )
  {
    if( theAnatomist->hasObject( o ) )
    {
      AObject::ParentList::iterator	ip, ep;
      set<AObject *>::const_iterator	eo = obj.end();
      AObject::ParentList		& parents = o->Parents();
      for( ip=parents.begin(), ep=parents.end(); ip!=ep; ++ip )
        if( obj.find( *ip ) != eo )
          addobject( *ip, obj, win, false, false, false );

      set<AWindow*>::const_iterator w, ew = win.end();
      for( w=win.begin(); w!=ew; ++w )
        (*w)->registerObject( o );

      if( addchildren )
      {
        MObject *mo = dynamic_cast<MObject *>( o );
        if( mo )
        {
          MObject::iterator i, e = mo->end();
          set<AWindow*>::const_iterator w, ew = win.end();
          for( i=mo->begin(); i!=e; ++i )
            for( w=win.begin(); w!=ew; ++w )
              (*w)->registerObject( *i );
        }
      }
      else
      {
        AGraph *ag = dynamic_cast<AGraph *>( o );
        if( ag )
        {
          Graph *g = ag->graph();
          shared_ptr<AObject> ao;
          if( addnodes )
          {
            cout << "addnodes\n";
            Graph::const_iterator iv, ev = g->end();
            for( iv=g->begin(); iv!=ev; ++iv )
              if( (*iv)->getProperty( "ana_object", ao ) )
                for( w=win.begin(); w!=ew; ++w )
                  (*w)->registerObject( ao.get() );
          }
          if( addrels )
          {
            const set<Edge *> & edg = g->edges();
            set<Edge *>::const_iterator ie, ee = edg.end();
            for( ie=edg.begin(); ie!=ee; ++ie )
              if( (*ie)->getProperty( "ana_object", ao ) )
                for( w=win.begin(); w!=ew; ++w )
                  (*w)->registerObject( ao.get() );
          }
        }
      }
    }
  }

}


void
AddObjectCommand::doit()
{
  set<AObject*>::iterator o, fo= _objL.end();
  set<AWindow*>::iterator w, w2, fw = _winL.end();

  w=_winL.begin();
  while( w != fw )
    if( !theAnatomist->hasWindow( *w ) )
    {
      w2 = w;
      ++w;
      _winL.erase( w2 );
    }
    else
      ++w;

  set<MObject *>::const_iterator	ip, ep;
  for( o=_objL.begin(); o!=fo; ++o )
    if( theAnatomist->hasObject( *o ) )
      addobject( *o, _objL, _winL, _addchildren, _addnodes, _addrels );

  theAnatomist->Refresh();
}


void
AddObjectCommand::undoit()
{
}


Command* AddObjectCommand::read( const Tree & com, CommandContext* context )
{
  vector<int>		obj, win;
  set<AObject *>	objL;
  set<AWindow *>	winL;
  unsigned		i, n;
  void			*ptr;
  int                   addch = 0, addnodes = 0, addrels = 0;

  if( !com.getProperty( "objects", obj ) )
    return( 0 );
  if( !com.getProperty( "windows", win ) )
    return( 0 );
  com.getProperty( "add_children", addch );
  com.getProperty( "add_graph_nodes", addnodes );
  com.getProperty( "add_graph_relations", addrels );

  for( i=0, n=obj.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( obj[i], "AObject" );
      if( ptr )
	objL.insert( (AObject *) ptr );
      else
	cerr << "object id " << obj[i] << " not found\n";
    }
  for( i=0, n=win.size(); i<n; ++i )
    {
      ptr = context->unserial->pointer( win[i], "AWindow" );
      if( ptr )
	winL.insert( (AWindow *) ptr );
      else
	cerr << "window id " << win[i] << " not found\n";
    }

  if( !objL.empty() && !winL.empty() )
    return( new AddObjectCommand( objL, winL, addch, addnodes, addrels ) );
  else
    return( 0 );
}


void AddObjectCommand::write( Tree & com, Serializer* ser ) const
{
  Tree	*t = new Tree( true, name() );

  set<AObject *>::const_iterator	io;
  set<AWindow *>::const_iterator	iw;
  vector<int>				obj, win;

  for( io=_objL.begin(); io!=_objL.end(); ++io ) 
    obj.push_back( ser->serialize( *io ) );

  for( iw=_winL.begin(); iw!=_winL.end(); ++iw )
    win.push_back( ser->serialize( *iw ) );

  t->setProperty( "objects", obj );
  t->setProperty( "windows", win );
  if( _addchildren )
    t->setProperty( "add_children", 1 );
  else
  {
    if( _addnodes )
      t->setProperty( "add_graph_nodes", 1 );
    if( _addrels )
      t->setProperty( "add_graph_relations", 1 );
  }
  com.insert( t );
}
