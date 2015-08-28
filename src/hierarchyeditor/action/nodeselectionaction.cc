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

#include <anatomist/action/nodeselectionaction.h>
#include <anatomist/control/nodeselectioncontrol.h>
#include <anatomist/application/hierarchyeditor.h>
#include <anatomist/object/Object.h>
#include <cartobase/object/object.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/Window.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/controler/view.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/control/graphParams.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#include <anatomist/commands/cSelectByHierarchy.h>
#include <anatomist/processor/Processor.h>

#include <cartobase/stream/fileutil.h>

using namespace std;
using namespace anatomist ;
using namespace aims;


Action*
NodeSelectionAction::creator()
{
	//cout<<"Creator Action"<<endl;
  return  new NodeSelectionAction ;
}

string NodeSelectionAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "NodeSelectionAction" );
}


/* Constr. */

	NodeSelectionAction::NodeSelectionAction() : action_browser(NULL), action_hie(0)
	{
		graph_set=false;
		count_fold=0;
		//cout<<"Constructeur NodeSelectionACTION"<<endl;
	}

	NodeSelectionAction::~NodeSelectionAction()
	{

		//cout<<"Destructeur NodeSelectionACTION"<<endl;
	}


/*Remove a node from the sub-graph*/

bool NodeSelectionAction::removeVertex( Vertex * V )
{
	if( list_vertex.find(V) == list_vertex.end() )
		return false;

	/*getElementsWith(nom Attribut, valeur, true)*/

	try{
		static_cast <Tree*> ( action_hie->attributed() )->remove( list_vertex[V]) ;
	}
	catch( std::range_error )
	{
		cout<<"Not in the hierarchy"<<endl;
	}

	list_vertex.erase(V);       //update list of vertex in subgraph
	action_hie->setChanged();
	action_hie->notifyObservers();
	return true;
}


/*Add a node in the sub-graph*/

bool NodeSelectionAction::addVertex( Vertex * V )
{
	if( list_vertex.find(V) != list_vertex.end() )
		return false;
	Tree * newT = new Tree(true,"sTree");
	cout<<"new tree created"<<endl;

	/*Recup du graph parent*/
	AObject * t_g = 0;

	V->getProperty("ana_object", t_g);
	AObject::ParentList & m_l = t_g->Parents();
	MObject* m_o = *m_l.begin();

	Graph* g = ((AGraph*)m_o)->graph();
	cout << "graph syntax : " << g->getSyntax() << endl;
	action_hie->attributed()->setProperty( "graph_syntax",g->getSyntax() );

	/***Voila**/

	static_cast <Tree*> ( action_hie->attributed() )-> insert(newT) ;
	cout<<"Node added to action_hie"<<endl;

	string temp_name;
	string temp_label;
	//V->getProperty(GraphParams::graphParams()->attribute, temp_name);
	V->getProperty("name", temp_name);
	V->getProperty("label", temp_label);

	newT->setSyntax("sTree");
	newT->setProperty( "name", temp_name );
	newT->setProperty( "label", temp_label );
	cout<<"Syntax set"<<endl;
	//graph_graph_vertex_h::
	newT->setProperty("comments",string("No comment"));
	newT->setProperty("parallel_coord", 0.0F);
	newT->setProperty("meridian_coord", 0.0F);  //sans le F, entier par d�faut (F=float)
	newT->setProperty("confidence",100);
	newT->setProperty("type",string("Both"));
	cout<<"Attributes set"<<endl;


	//list_vertex[V]=action_graph->graph()->addVertex(synt);
	//list_vertex[V] = action_graph->graph()-> cloneVertex(newV) ;  //copy with attributes
	list_vertex[V] =newT;
	cout<<"Node added to list"<<endl;


	//SelectFactory::factory()->select((unsigned int)action_browser->Group(),newT);
	action_hie->setChanged();
	action_hie->notifyObservers();

	set<string> set_name;
	//string tp;
	//action_hie->attributed()->getProperty( "label", tp ),
	set_name.insert( temp_label );
	cout<<"Insertion ok"<<endl;
	Command *c = new SelectByHierarchyCommand( action_hie, set_name, 0, 1 );
	cout<<"Creation commande ok"<<endl;
	theProcessor->execute( c );
	cout<<"Execution ok"<<endl;
	//action_hie->setChanged();
	//action_hie->notifyObservers();

//Pour "red�plier" le browser, s�lectionner explicitement le dernier noeud pris en compte

	return true;
}


/* Treatment of selected sulci */
void NodeSelectionAction::add( int x, int y, int , int  )
{
	/*Check if the objects are still existing*/

	if( !theAnatomist->hasWindow(action_browser) || !theAnatomist->hasObject(action_hie) )
	{
		cerr << "No object for this action\n";
		return;
	}

	/*Selection*/
        GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
        AWindow	    *aw = view()->aWindow();
	if( !w )
	{
		cerr << "SelectAction operating on wrong view type -- error\n";
		return;
	}

	Point3df pos;
	if( w->positionFromCursor( x, y, pos ) )
	{
		cout << "Position : " << pos << endl;
		aw->selectObject( pos[0], pos[1], pos[2],
                aw->getTime(),
		SelectFactory::Normal );
	}

 /*Acces � la selection*/
	cout<<"acces a la Selection"<<endl;

	std::map<unsigned, std::set<AObject *> >  current_element = SelectFactory::factory()->selected() ;
	std::set<AObject *>::iterator im ;

	unsigned num = aw->Group();
	std::set<AObject *> & temp_num = current_element[num];

	im=temp_num.begin();

	if( im == temp_num.end())  /*if selection is empty*/

		return;

	/* Le nouveau graph, s'il n'existe pas, devrait �tre cr�� ICI*/

	AGraphObject * tempObj ;
	//AGraph * temp_graph ;
	Vertex * current_vertex ;
	AObject * selectObj = (*im);

	//temp_graph = dynamic_cast<AGraph * > ( selectObj );
	tempObj = dynamic_cast<AGraphObject * > ( selectObj );

	/*If selection isn't vertex*/
	if(tempObj == 0)
		return;


	current_vertex = (Vertex *) tempObj->attributed();

	if( !addVertex(current_vertex) )
	{
		cerr << "warning : sulci already selected\n";
		return;
	}

	action_browser->registerObject( action_hie ) ;

	select(x,y,0,0);
}

void NodeSelectionAction::remove( int x, int y, int , int  )
{
	/*Check if the objects are still existing*/
	if( !theAnatomist->hasWindow(action_browser) || !theAnatomist->hasObject(action_hie) )
	{
		cerr << "No object for this action\n";
		return;
	}

	/*Selection*/
        GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
        AWindow     *aw = w->aWindow();
	if( !w )
	{
		cerr << "SelectAction operating on wrong view type -- error\n";
		return;
	}

	Point3df pos;
	if( w->positionFromCursor( x, y, pos ) )
	{
		cout << "Position : " << pos << endl;
		aw->selectObject( pos[0], pos[1], pos[2],
		aw->getTime(),
		SelectFactory::Normal );
	}

	/*Acces � la selection*/

	std::map<unsigned, std::set<AObject *> >  current_element = SelectFactory::factory()->selected() ;
	std::set<AObject *>::iterator im ;

	unsigned num = aw->Group();
	std::set<AObject *> & temp_num = current_element[num];

	im=temp_num.begin();

	if( im == temp_num.end())  /*if selection is empty*/
		return;

	AGraphObject * tempObj ;
	Vertex * current_vertex ;
	AObject * selectObj = (*im);

	tempObj = dynamic_cast<AGraphObject * > ( selectObj );

	/*If selection isn't vertex*/
	if(tempObj == 0)
		return;

	current_vertex = (Vertex *) tempObj->attributed();

	if( !removeVertex(current_vertex) )
	{
		cerr << "warning : sulci is not in sub-hierarchy\n";
		return;
	}

	action_browser->registerObject( action_hie ) ;
}

void NodeSelectionAction::select( int x, int y, int , int   )
{
	cout<<"SELECT"<<endl;
	/*Check if the objects are still existing*/
		if( !theAnatomist->hasWindow(action_browser) || !theAnatomist->hasObject(action_hie) )
	{
		cerr << "No object for this action\n";
		return;
	}
		/*Selection*/
        GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
        AWindow     *aw = w->aWindow();
	if( !w )
	{
		cerr << "SelectAction operating on wrong view type -- error\n";
		return;
	}
	Point3df pos;
	if( w->positionFromCursor( x, y, pos ) )
	{
		cout << "Position : " << pos << endl;
		aw->selectObject( pos[0], pos[1], pos[2],
		aw->getTime(),
		SelectFactory::Normal );
	}
	
	/*Acces � la selection*/

	std::map<unsigned, std::set<AObject *> >  current_element = SelectFactory::factory()->selected() ;
	std::set<AObject *>::iterator im ;

	unsigned num = w->aWindow()->Group();
	std::set<AObject *> & temp_num = current_element[num];

	im=temp_num.begin();

	if( im == temp_num.end())  /*if selection is empty*/

		return;

	/* Le nouveau graph, s'il n'existe pas, devrait �tre cr�� ICI*/

	AGraphObject * tempObj ;
	//AGraph * temp_graph ;
	Vertex * current_vertex ;
	AObject * selectObj = (*im);

	//temp_graph = dynamic_cast<AGraph * > ( selectObj );
	tempObj = dynamic_cast<AGraphObject * > ( selectObj );

	/*If selection isn't vertex*/
	if(tempObj == 0)
		return;


	current_vertex = (Vertex *) tempObj->attributed();

	/*hierarchy temporaire pour s�lectionner les noeuds dans tous les graphs du m�me groupe*/
	Tree	*trt = new Tree( true, "hierarchy" );
	Hierarchy	*hiera = new Hierarchy( trt) ;
	string tyt = "hierarchy";
	//hie->setFileName( selectName ) ;
	//string nm = askName( ty );
	hiera->setName( tyt);
	hiera->attributed()->setProperty( "graph_syntax",(string("CorticalFoldArg") ) );
	//hie->setName( theAnatomist->makeObjectName( selectName ) );
	theAnatomist->registerObject( hiera );
	
	Tree * newT = new Tree(true,"sTree");
	
	static_cast <Tree*> ( hiera->attributed() )-> insert(newT) ;

	string temp_name;
	string temp_label;
	
	current_vertex->getProperty(GraphParams::graphParams()->attribute, temp_label);
	cout<<"Node added and label="<<temp_label<<endl;

	newT->setSyntax("sTree");
	newT->setProperty( "name", temp_label );
	
	set<string> set_name;
	//string tp;
	//hiera->attributed()->getProperty( "label", tp ),
	set_name.insert( temp_label );
	
	
	//select(x,y,0,0);
	Command *c = new SelectByHierarchyCommand( hiera, set_name ,0,1);//, (unsigned int)action_browser->Group() );
	cout<<"Creation commande ok"<<endl;
	theProcessor->execute( c );
	cout<<"Execution ok"<<endl;
	hiera->setChanged();
	hiera->notifyObservers();

	action_browser->Refresh( ) ;
	theAnatomist->unregisterObject( hiera );
	
	cout<<"END SELECT"<<endl;
}

void NodeSelectionAction::setHie( Hierarchy * g )
{
	action_hie = g;
}

void NodeSelectionAction::setBrowser( AWindow * b )
{
	action_browser = b;
}

Hierarchy * NodeSelectionAction::getHie( )
{
	return action_hie ;
}

AWindow * NodeSelectionAction::getBrowser( )
{
	return action_browser ;
}

