/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <anatomist/browser/qObjBrowserWid.h>
#include <anafold/fgraph/afgHelpers.h>
#include <anafold/fgraph/afgraph.h>
#include <anatomist/browser/attDescr.h>
#include <anatomist/application/syntax.h>
#include <si/global/global.h>
#include <si/domain/domain.h>
#include <si/graph/cgraph.h>
#include <si/model/model.h>
#include <si/graph/vertexclique.h>
#include <cartobase/object/sreader.h>
#include <graph/tree/tree.h>
#include <stdio.h>

using namespace anatomist;
using namespace sigraph;
using namespace carto;
using namespace std;


void AFGHelpers::init()
{
  static bool _initialized = false;

  //cout << "AFGHelpers::init\n";
  if( _initialized )
    {
      //cout << "already initialized.\n";
      return;
    }

  _initialized = true;

  // almost obsolete
  try
    {
      SyntaxSet		msyntax;
      SyntaxReader	sr( si().basePath() + "/config/model_internal.stx" );

      sr.read( msyntax );
      SyntaxRepository::mergeSyntaxes( SyntaxRepository::internalSyntax(), 
				       msyntax );
    }
  catch( exception & e )
    {
      //cerr << e.what() << endl;
    }

  try
    {
      SyntaxSet		syntax;
      SyntaxReader	sr( si().basePath() + "/config/fold_internal.stx" );

      sr.read( syntax );
      SyntaxRepository::mergeSyntaxes( SyntaxRepository::internalSyntax(), 
				       syntax );
    }
  catch( exception & e )
    {
      //cerr << e.what() << endl;
    }
  // end of obsolete stuff

  AttDescr	*descr = AttDescr::descr();
  AttDescr::HelperSet	help;

  help[ "clique_p_set_p" ] = &printCliqueSet;

  descr->addHelpers( help );

  AttDescr::ListHelperSet	listH;

  listH[ "domain_p" ] = &printDomain;
  listH[ "model_p" ] = &printModel;

  descr->addListHelpers( listH );

  QObjectBrowserWidget::objectHelpers[ AFGraph::classType() ] 
    = describeAFGraph;
}


//	Helpers for model graphs


void AFGHelpers::printCliqueSet( QObjectBrowserWidget*, 
				 const GenericObject & ao, 
				 const string & semantic, string & stre )
{
  set<Clique*>			*val;
  set<Clique*>::const_iterator	ic, fc;
  string			name;

  if( !ao.getProperty( semantic, val ) )
    stre = "** error **";

  for( ic=val->begin(), fc=val->end(); ic!=fc; ++ic )
    {
      if( !(*ic)->getProperty( "label", name ) )
	name = "unnamed";
      stre += name + ' ';
    }
}


void AFGHelpers::printDomain( QObjectBrowserWidget* br, 
			      const GenericObject & ao, 
			      const string & semantic, Q3ListViewItem *parent, 
			      const AttDescr* ad, bool regist )
{
  Domain	*val;

  if( !ao.getProperty( semantic, val ) )
    AttDescr::printError( parent, semantic.c_str() );
  else
    {
      Q3ListViewItem	*item;
      Tree		tr;

      val->buildTree( tr );
      item = br->itemFor( parent, QObjectBrowserWidget::ATTRIBUTE, semantic, 
			  regist );
      if( !item )
	{
	  item = new Q3ListViewItem( parent, semantic.c_str(), "domain_p", 
				    tr.getSyntax().c_str() );
	  if( regist )
	    br->registerAttribute( item );
	}
      else
	{
	  item->setText( 0, semantic.c_str() );
	  item->setText( 1, "domain_p" );
	  item->setText( 2, tr.getSyntax().c_str() );
	}
      ad->describeTreeInside( br, &tr, item, false );
    }
}


void AFGHelpers::printModel( QObjectBrowserWidget* br, 
			     const GenericObject & ao, 
			     const string & semantic, Q3ListViewItem *parent, 
			     const AttDescr* ad, bool regist )
{
  Model	*val;

  if( !ao.getProperty( semantic, val ) )
    AttDescr::printError( parent, semantic.c_str() );
  else
    {
      Q3ListViewItem	*item;

      Tree	tr;
      val->buildTree( tr );

      item = br->itemFor( parent, QObjectBrowserWidget::ATTRIBUTE, semantic, 
			  regist);
      if( !item )
	{
	  item = new Q3ListViewItem( parent, semantic.c_str(), "model_p", 
				    tr.getSyntax().c_str() );
	  if( regist )
	    br->registerAttribute( item );
	}
      else
	{
	  item->setText( 0, semantic.c_str() );
	  item->setText( 1, "model_p" );
	  item->setText( 2, tr.getSyntax().c_str() );
	}
      ad->describeTreeInside( br, &tr, item, false );
    }
}


void AFGHelpers::describeAFGraph( QObjectBrowserWidget* br, AObject* obj, 
				  Q3ListViewItem *parent )
{
  AFGraph	*fg = dynamic_cast<AFGraph *>( obj );

  assert( fg );
  //br->insertObject( parent, fg->model() );
  //br->insertObject( parent, fg->folds() );

  CGraph	*folds = dynamic_cast<CGraph *>( fg->folds()->graph() );
  assert( folds );
  const set<Clique *>	& cl = folds->cliques();
  assert( &cl );
  char			id[100];
  set<Clique *>::const_iterator	ic, fc=cl.end();

  sprintf( id, "%u", (unsigned) cl.size() );

  Q3ListViewItem	*cliques = br->itemFor( parent, "Cliques" );
  if( !cliques )
    cliques = new Q3ListViewItem( parent, "Cliques", 0, id );
  else
    {
      cliques->setText( 1, 0 );
      cliques->setText( 2, id );
    }

  Q3ListViewItem	*item, *subitem, *citem;
  Clique	*c;
  string	name, str, synt, potential;
  AttDescr	*descr = AttDescr::descr();
  VertexClique::const_iterator	iv, fv;
  Vertex	*v;
  double	pot;
  bool		newgo;

  for( ic=cl.begin(); ic!=fc; ++ic )
    {
      c = *ic;
      if( !c->getProperty( "label", name ) )
      {
	if( c->getProperty( "label1", name ) 
	    && c->getProperty( "label2", str ) )
	  {
	    name = string( "edge " ) + name + " - " + str;
	  }
	else
	  name = "unnamed";
      }

      synt = c->getSyntax();
      if( ( !fg->isMapWeighted() 
	  && c->getProperty( "potential_unweighted", pot ) ) 
	  || c->getProperty( "potential", pot ) )
	{
	  sprintf( id, "%f", pot );
	  potential = id;
	}
      else potential = "";
      citem = br->itemFor( cliques, c, true );
      if( !citem )
	{
	  citem = new Q3ListViewItem( cliques, name.c_str(), synt.c_str(), 
				     potential.c_str() );
	  br->registerGObject( citem, c );
	  newgo = true;
	}
      else
	{
	  /*cout << "item found : " << citem << " for " << c << ", " << name 
	    << ", " << synt << ", " << potential << endl;*/
	  citem->setText( 0, name.c_str() );
	  citem->setText( 1, synt.c_str() );
	  citem->setText( 2, potential.c_str() );
	  newgo = false;
	}
      descr->describeAttributes( br, citem, c, true );

      sprintf( id, "Size : %d", ((const VertexClique *) c)->size() );
      if( !newgo )
	item = br->itemFor( citem, "Nodes" );
      else
	item = 0;
      if( !item )
	{
	  item = new Q3ListViewItem( citem, "Nodes", 0, id );
	  newgo = true;
	}

      if( newgo )
	for( iv=((const VertexClique*) c)->begin(), 
	       fv=((const VertexClique*) c)->end(); iv!=fv; ++iv )
	  {
	    v = *iv;
	    if( !v->getProperty( "name", name ) )
	      name = "unnamed";
	    if( !v->getProperty( "label", str ) )
	      str = "";
	    subitem = br->itemFor( item, name );
	    if( !subitem )
	      subitem = new Q3ListViewItem( item, name.c_str(), 
					    v->getSyntax().c_str(), 0, 
					    str.c_str() );
	    //describeAttributes( subitem, v );
	  }
    }
}




