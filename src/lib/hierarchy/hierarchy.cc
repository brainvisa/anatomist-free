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


#include <anatomist/control/qObjTree.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/graph/Graph.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#include <anatomist/object/oReader.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <aims/def/path.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/config/verbose.h>
#include <cartobase/config/paths.h>
#include <qpixmap.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


SyntaxSet	*Hierarchy::_syntax = 0;
Tree		*Hierarchy::_optionTree = 0;

struct Hierarchy::Private
{
  Private() {}
  mutable map<string, list<Tree *> > nodesByName;
  PropertySet::Signal::slot_list_type::iterator propslot;
};


struct Hierarchy::PrivateStatic
{
  map<string, list<Hierarchy *> > nomBySyntax;
};


Hierarchy::PrivateStatic* Hierarchy::staticStruct()
{
  static PrivateStatic ps;
  return &ps;
}


int Hierarchy::registerClass()
{
  ObjectReader::registerLoader( "hie", loadHierarchy );

  int type = registerObjectType( "NOMENCLATURE" );

  return( type );
}


Hierarchy::Hierarchy( Tree* tr )
  : AObject(), AttributedAObject(), d( new Private ),
  _tree( rc_ptr<Tree>( tr ) )
{
  _type = _classType;
  //_objMenu    = new ObjectMenu( "Nomenclature", 1 );
  //_optionMenu = new list<OptionMenu>;

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    char str[200];
    sprintf( str, "%s", Settings::findResourceFile(
        "icons/list_hierarchy.xpm" ).c_str() );
    if( !QObjectTree::TypeIcons[ _type ].load( str ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Nomenclature";
  }

  string hsynt;
  if( attributed()->getProperty( "graph_syntax", hsynt ) )
    staticStruct()->nomBySyntax[ hsynt ].push_back( this );
  // listen to properties changes
  d->propslot = _tree->getValue().getSignalPropertyChanged().connect(
    ::sigc::mem_fun( *this, &Hierarchy::slotPropertyChanged ) );
}


Hierarchy::Hierarchy( rc_ptr<Tree> tr )
  : AObject(), AttributedAObject(), d( new Private ), _tree( tr )
{
  _type = _classType;
  //_objMenu    = new ObjectMenu( "Nomenclature", 1 );
  //_optionMenu = new list<OptionMenu>;

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
  {
    char str[200];
    sprintf( str, "%s", Settings::findResourceFile(
        "icons/list_hierarchy.xpm" ).c_str() );
    if( !QObjectTree::TypeIcons[ _type ].load( str ) )
    {
      QObjectTree::TypeIcons.erase( _type );
      cerr << "Icon " << str << " not found\n";
    }

    QObjectTree::TypeNames[ _type ] = "Nomenclature";
  }

  string hsynt;
  if( attributed()->getProperty( "graph_syntax", hsynt ) )
    staticStruct()->nomBySyntax[ hsynt ].push_back( this );
  // listen to properties changes
  d->propslot = _tree->getValue().getSignalPropertyChanged().connect(
    ::sigc::mem_fun( *this, &Hierarchy::slotPropertyChanged ) );
}


Hierarchy::~Hierarchy()
{
  // remove properties callback
  _tree->getValue().getSignalPropertyChanged().sigcslots().erase(
    d->propslot );

  string hsynt;
  if( attributed()->getProperty( "graph_syntax", hsynt ) )
  {
    list<Hierarchy *> & lh = staticStruct()->nomBySyntax[ hsynt ];
    list<Hierarchy *>::iterator i = find( lh.begin(), lh.end(), this );
    if( i != lh.end() )
    {
      lh.erase( i );
      if( lh.empty() )
        staticStruct()->nomBySyntax.erase( hsynt );
    }
  }
  cleanup();
  delete d;
}


AObject* Hierarchy::loadHierarchy( const string & filename,
                                   ObjectReader::PostRegisterList &,
                                   Object options )
{
  if( !options.isNull() )
    try
      {
        Object restricted = options->getProperty( "restrict_object_types" );
        if( !restricted->hasProperty( "Hierarchy" ) )
          return 0;
      }
    catch( ... )
      {
      }

  Tree* tr = new Tree;

  if( !_syntax )
    {
      //cout << "init Hierarchy syntax\n";
      _syntax = new SyntaxSet;
      string sname = Path::singleton().syntax() + "/hierarchy.stx";
      try
	{
	  SyntaxReader	sr( sname );
	  sr.read( *_syntax );
	}
      catch( exception & e )
	{
	  cerr << e.what() << endl;
	  sname = Settings::findResourceFile( "syntax/hierarchy.stx" );

	  try
	    {
	      SyntaxReader	sr( sname );
	      sr.read( *_syntax );
              if( carto::verbose > 0 )
	        cout << "Loaded syntax " << sname << ", OK" << endl;
	    }
	  catch( exception & e )
	    {
	      cerr << "Cannot read syntax for Hierarchy objects :\n";
	      cerr << e.what() << endl;
	    }
	}
    }

  try
    {
      string	fname;
      if( FileUtil::fileStat( filename ).find( '+' ) == string::npos )
        fname = filename + ".hie";
      else
        fname = filename;
      TreeReader	trd( fname, *_syntax );

      trd >> *tr;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      delete tr;
      return( 0 );
    }

  return new Hierarchy( tr );
}


Tree* Hierarchy::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, "File" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Save" );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
    }
  return( _optionTree );
}


GenericObject* Hierarchy::attributed()
{
  return( _tree.get() );
}


bool Hierarchy::save( const string & filename )
{
  if( !_syntax )
    {
      //cout << "init Hierarchy syntax\n";
      _syntax = new SyntaxSet;
      string sname = Paths::findResourceFile( "syntax/hierarchy.stx" );
      try
      {
        SyntaxReader	sr( sname );
        sr.read( *_syntax );
      }
      catch( exception & e )
      {
        cerr << "Cannot read syntax for Hierarchy objects :\n";
        cerr << e.what() << endl;
      }
    }

  try
    {
      string	fname;
      if( FileUtil::extension( filename ).empty() )
        fname = filename + ".hie";
      else
        fname = filename;
      TreeWriter tw( fname, *_syntax );
      tw << *_tree;
    }
  catch( exception & e )
    {
      cerr << e.what() << "\nsave aborted\n";
      return( false );
    }
  return( true );
}


void Hierarchy::namesUnder( Tree* tr, set<string> & ss )
{
  string	str;

  if( tr->getProperty( "name", str ) )
    ss.insert( str );

  Tree::const_iterator	it, ft=tr->end();

  for( it=tr->begin(); it!=ft; ++it )
    namesUnder( (Tree *) *it, ss );
}


Hierarchy* Hierarchy::findMatchingNomenclature( const AObject* obj,
    const AGraph **agr )
{
  if( agr )
    *agr = 0;
  const AGraph *ag = dynamic_cast<const AGraph *>( obj );
  ParentList::const_iterator ip, ep;
  if( !ag )
  {
    const ParentList & pl = obj->parents();
    for( ip=pl.begin(), ep=pl.end(); ip!=ep; ++ip )
    {
      obj = *ip;
      ag = dynamic_cast<const AGraph *>( obj );
      if( ag )
        break;
    }
  }
  if( !ag )
    return 0;
  if( agr )
    *agr = ag;
  const SyntaxedInterface
      *si = ag->attributed()->getInterface<SyntaxedInterface>();
  string synt = si->getSyntax();

  // now find a nomenclature with syntax synt
/*  set<AObject *>	objs = theAnatomist->getObjects();
  set<AObject *>::const_iterator	io, fo=objs.end();
  Hierarchy				*hie;
  GenericObject			        *hao;
  string				hsynt;

  for( io=objs.begin(); io!=fo; ++io )
  {
    hie = dynamic_cast<Hierarchy *>( *io );
    if( hie )
    {
      hao = hie->attributed();
      if( hao->getProperty( "graph_syntax", hsynt ) && hsynt == synt )
        return hie;
    }
  }*/

  map<string,list<Hierarchy *> >::const_iterator
    ilh = staticStruct()->nomBySyntax.find( synt );
  if( ilh != staticStruct()->nomBySyntax.end() && !ilh->second.empty() )
    return ilh->second.front();

  return 0; // no matching nomenclature found
}


  Tree* Hierarchy::findNamedNode( const std::string & name,
                                const list<Tree*> ** parents ) const
{
  if( d->nodesByName.empty() )
  {
    // fill nodes table
    string tname;
    list<pair<Tree::const_iterator, Tree::const_iterator> > lit;
    list<Tree *> parentstack;
    if( _tree->begin() != _tree->end() )
      lit.push_back( make_pair( _tree->begin(), _tree->end() ) );
    while( !lit.empty() )
    {
      pair<Tree::const_iterator, Tree::const_iterator> & pit = lit.back();
      if( pit.first == pit.second )
      {
        if( !parentstack.empty() )
          parentstack.pop_front();
        lit.pop_back();
      }
      else
      {
        Tree *t = static_cast<Tree *>( *pit.first );
        parentstack.push_front( t );
        if( t->getProperty( "name", tname ) )
          d->nodesByName[ tname ] = parentstack;
        if( t->begin() != t->end() )
          lit.push_back( make_pair( t->begin(), t->end() ) );
        else
          parentstack.pop_front();
        ++pit.first;
      }
    }
  }
  map<string, list<Tree *> >::const_iterator it = d->nodesByName.find( name );
  if( it == d->nodesByName.end() )
  {
    if( parents )
      *parents = 0;
    return 0;
  }
  if( parents )
    *parents = &it->second;
  return it->second.front();
}


void Hierarchy::slotPropertyChanged( const Object& sender,
                                     const string& propertyName,
                                     const Object& oldValue )
{
  if( propertyName == "graph_syntax" )
  {
    string oldsynt, newsynt;
    try
    {
      oldsynt = oldValue->getString();
      newsynt = attributed()->getProperty( propertyName )->getString();
    }
    catch( ... )
    {
      cout << "exception while getting syntax values\n";
      return;
    }
    if( oldsynt != newsynt )
    {
      list<Hierarchy *> & lh = staticStruct()->nomBySyntax[ oldsynt ];
      list<Hierarchy *>::iterator i = find( lh.begin(), lh.end(), this );
      if( i != lh.end() )
      {
        lh.erase( i );
        if( lh.empty() )
          staticStruct()->nomBySyntax.erase( oldsynt );
      }
      staticStruct()->nomBySyntax[ newsynt ].push_back( this );
    }
  }
}


void Hierarchy::internalUpdate()
{
  d->nodesByName.clear();
  AObject::internalUpdate();
}

