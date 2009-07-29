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


#include <anatomist/selection/qSelectFactory.h>
#include <anatomist/selection/qSelectWid.h>
#include <anatomist/selection/qSelMenu.h>
#include <anatomist/selection/qwSelAttrib.h>
#include <anatomist/browser/qwObjectBrowser.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/processor/Processor.h>

#include <graph/graph/graph.h>
#include <qcursor.h>


using namespace anatomist;
using namespace carto;
using namespace std;


QSelectFactory::QSelectFactory() : _selMenu( 0 )
{
}


QSelectFactory::~QSelectFactory()
{
  delete _selMenu;
}


WSelectChooser* 
QSelectFactory::createSelectChooser( unsigned group, 
				     const set<AObject *> & objects ) const
{
  return( new QSelectWidget( group, objects, 0, "Selection" ) );
}


void QSelectFactory::handleSelectionMenu( AWindow* win, int, int, 
					  const Tree* specific )
{
  if( !_selMenu )
    {
      _selMenu = new QSelectMenu;
      connect( _selMenu, SIGNAL( viewSignal( anatomist::AWindow * ) ), this, 
	       SLOT( view( anatomist::AWindow * ) ) );
      connect( _selMenu, SIGNAL( unselectSignal( anatomist::AWindow * ) ), 
	       this, SLOT( unselect( anatomist::AWindow * ) ) );
      connect( _selMenu, SIGNAL( selectAllSignal( anatomist::AWindow * ) ), 
	       this, SLOT( selectAll( anatomist::AWindow * ) ) );
      connect( _selMenu, SIGNAL( removeSignal( anatomist::AWindow * ) ), this, 
	       SLOT( remove( anatomist::AWindow * ) ) );
      connect( _selMenu, 
	       SIGNAL( removeThisWinSignal( anatomist::AWindow * ) ), this, 
	       SLOT( removeFromThisWindow( anatomist::AWindow * ) ) );
      connect( _selMenu, SIGNAL( neighboursSignal( anatomist::AWindow * ) ), 
	       this, SLOT( neighbours( anatomist::AWindow * ) ) );
      connect( _selMenu, SIGNAL( selAttribSignal( anatomist::AWindow * ) ), 
	       this, SLOT( selAttrib( anatomist::AWindow * ) ) );
    }

  _selMenu->update( win, specific );
  _selMenu->popup( QCursor::pos() );
}


void QSelectFactory::view( AWindow* win )
{
  QObjectBrowser	*br = new QObjectBrowser;

  br->SetGroup( win->Group() );
  br->show();

  set<AObject *>		obj = win->Objects();
  set<AObject *>::iterator	io, fo=obj.end();

  for( io=obj.begin(); io!=fo; ++io )
    br->registerObject( *io );
  br->Refresh();
}


void QSelectFactory::unselect( AWindow* win )
{
  // cout << "unselect\n";

  unselectAll( win->Group() );
  refresh();
}


void QSelectFactory::selectAll( AWindow* win )
{
  //cout << "selectAll\n";

  SelectFactory::selectAll( win );
  refresh();
}


void QSelectFactory::remove( AWindow* win )
{
  SelectFactory::remove( win );
}


void QSelectFactory::removeFromThisWindow( AWindow* win )
{
  SelectFactory::removeFromThisWindow( win );
}


void QSelectFactory::neighbours( AWindow* win )
{
  cout << "neighbours\n";

  const map<unsigned, set<AObject *> >	& mo = selected();
  map<unsigned, set<AObject *> >::const_iterator im = mo.find( win->Group() );

  if( im == mo.end() )
    return;

  const set<AObject *>			obj = (*im).second;
  set<AObject *>::const_iterator	io, fo=obj.end();
  set<AObject *>			tosel;
  set<MObject *>::const_iterator	ip, fp;

  for( io=obj.begin(); io!=fo; ++io )
    if( (*io)->isMultiObject() 
	&& ((MObject *)*io)->MType() == AObject::GRAPHOBJECT )
      {
	AObject::ParentList & par = (*io)->Parents();

	for( ip=par.begin(), fp=par.end(); ip!=fp; ++ip )
	  if( (*ip)->MType() == AObject::GRAPH )
	    selectNeighbours( (AGraph *) *ip, (AGraphObject *) *io, tosel );
      }

  select( win->Group(), tosel );
  refresh();
}


void QSelectFactory::selectNeighbours( AGraph* graph, AGraphObject* go, 
				       set<AObject *> & toselect ) const
{
  Graph* gr = graph->graph();	// we deal with low-level object

  shared_ptr<AObject> sgo( shared_ptr<AObject>::Weak, (AObject *) go );
  set<Vertex *> sv = gr->getVerticesWith( "ana_object", sgo );

  if( sv.size() != 1 )
    {
      cerr << "QSelectFactory::selectNeighbours : found " << sv.size() 
	   << " vertices pointing to AGraphObject, should be 1\n";
      return;
    }

  Vertex			*v = *sv.begin();
  set<Vertex *>			nbr = v->neighbours();
  set<Vertex *>::const_iterator	in, fn = nbr.end();
  shared_ptr<AObject>		ago;

  //cout << nbr.size() << " neighbours\n";
  for( in=nbr.begin(); in!=fn; ++in )
    if( (*in)->getProperty( "ana_object", ago ) )
      //&& ( !ago->isMultiObject() || ((MObject *) ago)->size() > 0 ) )
      toselect.insert( ago.get() );
}


void QSelectFactory::selAttrib( AWindow* win )
{
  cout << "selAttrib\n";
  if( QSelAttrib::attributes().size() == 0 )
    {
      QSelAttrib::attributes().insert( "label" );
      QSelAttrib::attributes().insert( "name" );
    }

  QSelAttrib	sat( 0, "attribSel" );

  if( sat.exec() )
    {
      cout << "Attrib : " << sat.attribute() << endl;
      cout << "Value  : " << sat.value() << endl;

      const map<unsigned, set<AObject *> >	& mo = selected();
      map<unsigned, set<AObject *> >::const_iterator 
	im = mo.find( win->Group() );

      if( im == mo.end() )
	return;

      const set<AObject *>			obj = (*im).second;
      set<AObject *>::const_iterator	io, fo=obj.end();
      set<AObject *>			tosel;
      set<MObject *>::const_iterator	ip, fp;
      set<Graph *>			graphs;
      Graph				*gr;

      for( io=obj.begin(); io!=fo; ++io )
	if( (*io)->isMultiObject() 
	    && ((MObject *)*io)->MType() == AObject::GRAPHOBJECT )
	  {
	    //cout << "GObj " << (*io)->name() << endl;
	    AObject::ParentList & par = (*io)->Parents();

	    for( ip=par.begin(), fp=par.end(); ip!=fp; ++ip )
	      if( (*ip)->MType() == AObject::GRAPH )
		{
		  //cout << "graph\n";
		  gr = ((AGraph *)*ip)->graph();
		  if( graphs.find( gr ) == graphs.end() )
		    {
		      //cout << "sel in graph " << gr << endl;
		      graphs.insert( gr );
		      selectNodesWith( tosel, gr, sat.attribute(), 
				       sat.value() );
		    }
		}
	  }

      unselectAll( win->Group() );
      select( win->Group(), tosel );
      refresh();
    }
}


void QSelectFactory::selectNodesWith( set<AObject *> & tosel, 
				      const Graph* gr, const string & attr, 
				      const string & val )
{
  set<Vertex *> 	vert;

  try
    {
      vert = gr->getVerticesWith( attr, val );
    }
  catch( exception & e )
    {
      //cout << "exception, attr " << attr << " not of type " << val << endl;
      return;
    }

  set<Vertex *>::const_iterator	iv, fv = vert.end();
  shared_ptr<AObject>		ao;

  for( iv=vert.begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( "ana_object", ao ) )
      tosel.insert( ao.get() );
  cout << "nodes : " << tosel.size() << ", vertex : " << vert.size() << endl;
}
