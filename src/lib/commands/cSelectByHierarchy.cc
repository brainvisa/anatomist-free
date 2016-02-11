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

#include <anatomist/commands/cSelectByHierarchy.h>
#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/processor/Registry.h>
#include <anatomist/processor/unserializer.h>
#include <anatomist/processor/Serializer.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/processor/context.h>
#include <anatomist/processor/Processor.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <vector>

using namespace anatomist;
using namespace carto;
using namespace std;


SelectByHierarchyCommand::SelectByHierarchyCommand( Hierarchy* hie, 
						    const set<string> & names,
						    int group, int modifiers )
  : RegularCommand(), _hie( hie ), _names( names ), _group( group ), 
    _modifiers( modifiers )
{
}


SelectByHierarchyCommand::~SelectByHierarchyCommand()
{
}


bool SelectByHierarchyCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "SelectByHierarchy" ];
  
  s[ "hierarchy"    ] = Semantic( "int", false );
  s[ "nomenclature" ] = Semantic( "int", false );
  s[ "names"        ] = Semantic( "string", true );
  s[ "group"        ] = Semantic( "int", false );
  s[ "modifiers"    ] = Semantic( "string", false );
  Registry::instance()->add( "SelectByHierarchy", &read, ss );

  SyntaxSet	ssn;
  ssn[ "SelectByNomenclature" ] = s;
  Registry::instance()->add( "SelectByNomenclature", &read, ssn );

  return true;
}


void SelectByHierarchyCommand::doit()
{
  using carto::shared_ptr;

  cout << "SelectByNomenclatureCommand\n";
  cout << "nomenclature : " << _hie->name() << endl;
  cout << "select : " << _names.size() << " names\n";
  cout << "group : " << _group << endl;
  cout << "modifiers : " << _modifiers << endl;

  string	attrib, gattrib;

  if( GraphParams::graphParams() )
    attrib = GraphParams::graphParams()->attribute;

  set<AWindow *>	win = theAnatomist->getWindowsInGroup( _group );
  set<AWindow *>::const_iterator	iw, fw=win.end();
  string				synt;
  set<AObject *>::const_iterator	io, fo;
  AGraph				*ag;
  Graph					*g;
  set<Graph *>				gr;
  set<string>				vallist;
  set<string>::iterator			is, es;
  MObject				*mobj;
  MObject::const_iterator		im, fm;
  set<Tree *>::iterator			it, et;
  set<AObject *>			tosel;

  if( !_hie->tree()->getProperty( "graph_syntax", synt ) )
    return;

  for( is=_names.begin(), es=_names.end(); is!=es; ++is )
    {
      //cout << "name to select : " << *is << endl;
      set<Tree *>	nodes = _hie->tree()->getElementsWith( "name", *is );
      //cout << "found " << nodes.size() << " trees\n";
      for( it=nodes.begin(), et=nodes.end(); it!=et; ++it )
	{
	  _hie->namesUnder( *it, vallist );
	  if( vallist.size() == 0 )
	    //if( !descr.ao->getProperty( "name", val ) )
	    return;

	  //	find graphs in windows of the same group
	  for( iw=win.begin(); iw!=fw; ++iw )
	    {
	      set<AObject *>	obj = (*iw)->Objects();
	      for( io=obj.begin(), fo=obj.end(); io!=fo; ++io )
		{
		  if( ( ag = dynamic_cast<AGraph *>( *io ) ) )
		    {
		      g = ag->graph();
		      if( g->getSyntax() == synt )	// matching syntax ?
			gr.insert( g );
		    }
		  //	and graphs contained in objects (fusions...) in windows
		  else if( (mobj = dynamic_cast<MObject *>( *io ) ) )
		    for( im=mobj->begin(), fm=mobj->end(); im!=fm; ++im )
		      if( ( ag = dynamic_cast<AGraph *>( *im ) ) )
			{
			  g = ag->graph();
			  if( g->getSyntax() == synt )	// matching syntax ?
			    gr.insert( g );
			}
		}
	    }

	  //	now select nodes in the graphs
	  set<Graph *>::const_iterator	ig, fg=gr.end();
	  set<Vertex *>			sv;
	  set<Vertex *>::const_iterator	iv, fv;
	  shared_ptr<AObject>		obj;
	  set<string>::const_iterator	is, fs=vallist.end();
	  string			ws, name;
	  string::size_type		pos;
	  bool				done;

	  //cout << "graphs : " << gr.size() << endl;

	  for( ig=gr.begin(); ig!=fg; ++ig )
          {
            gattrib = attrib;
            // if there a per-graph nomenclature property setting, use it
            (*ig)->getProperty( "label_property", gattrib );
	    for( iv=(*ig)->begin(), fv=(*ig)->end(); iv!=fv; ++iv )
	      if( (*iv)->getProperty( "ana_object", obj ) 
		  && (*iv)->getProperty( attrib, name ) )
		{
		  done = false;
		  while( !name.empty() && !done )
		    {
		      pos = name.find( '+' );
		      if( pos == string::npos )
			pos = name.size();
		      ws = name.substr( 0, pos );
		      name.erase( 0, pos+1 );

		      for( is=vallist.begin(); is!=fs; ++is )
			{
			  if( ws == *is )
			    {
			      tosel.insert( obj.get() );
			      done = true;
			      break;
			    }
			  /*sv = (*ig)->getVerticesWith( attrib, *is );
			    for( iv=sv.begin(), fv=sv.end(); iv!=fv; ++iv )
			    if( (*iv)->getProperty( "ana_object", obj ) )
			    tosel.insert( obj );*/
			}
		    }
		}
          }
	}
    }

  // cout << "select " << tosel.size() << " objects\n";
  SelectFactory	*fac = SelectFactory::factory();
  int	mod = _modifiers;
  if( mod == Remove )
    mod = Add;
  fac->select( (SelectFactory::SelectMode) mod, _group, tosel );
  fac->refresh();
  if( _modifiers == Remove )
    {
      // cout << "remove objects\n";
      RemoveObjectCommand	*c = new RemoveObjectCommand( tosel, win );
      c->execute();
    }
}


Command* SelectByHierarchyCommand::read( const Tree & com, 
					 CommandContext* context )
{
  set<string>		names;
  int			group = 0, mod = 0, id;
  string		modif = "add";
  string		nm;

  if( !com.getProperty( "nomenclature", id ) 
      && !com.getProperty( "hierarchy", id ) )
    {
      cerr << "SelectByNomenclatureCommand : either 'nomenclature' or " 
        "'hierarchy' attribute is needed\n";
      return 0;
    }
  AObject	*ho = (AObject *) context->unserial->pointer( id, "AObject" );
  Hierarchy	*hie = dynamic_cast<Hierarchy *>( ho );
  if( !hie )
    {
      cerr << "SelectByHierarchyCommand : hierarchy not found\n";
      return 0;
    }

  com.getProperty( "names", nm );
  com.getProperty( "group", group );
  com.getProperty( "modifiers", modif );
  if( modif == "add" || modif.empty() )
    mod = Add;
  else if( modif == "toggle" )
    mod = Toggle;
  else if( modif == "remove" )
    mod = Remove;

  string::size_type	pos = 0, pos2 = 0;
  while( ( pos = nm.find( ' ', pos2 ) ) != string::npos )
    {
      names.insert( nm.substr( pos2, pos - pos2 ) );
      pos2 = pos + 1;
    }
  if( pos2 < nm.length() )
    names.insert( nm.substr( pos2, nm.length() - pos2 ) );

  return( new SelectByHierarchyCommand( hie, names, group, mod ) );
}


void SelectByHierarchyCommand::write( Tree & com, Serializer* ser ) const
{
  Tree				*t = new Tree( true, name() );
  vector<int>			oid, uid;
  set<string>::const_iterator	in, en=_names.end();

  if( _group != 0 )
    t->setProperty( "group", _group );
  string	modif[] = { "set", "add", "toggle", "remove" };
  if( _modifiers > 0 && _modifiers <= Remove && _modifiers != Add )
    t->setProperty( "modifiers", modif[ _modifiers ] );

  t->setProperty( "nomenclature", ser->serialize( _hie ) );

  string	names;
  for( in=_names.begin(); in!=en; ++in )
    {
      if( !names.empty() )
	names += ' ';
      names += *in;
    }
  if( !names.empty() )
    t->setProperty( "names", names );
  com.insert( t );
}




