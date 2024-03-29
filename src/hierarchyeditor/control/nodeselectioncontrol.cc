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

#include <anatomist/application/hierarchyeditor.h>
#include <anatomist/control/nodeselectioncontrol.h>
#include <anatomist/action/nodeselectionaction.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/misc/error.h>
#include <qlabel.h>
#include <qobject.h>
#include <qcursor.h>
#include <iostream>

using namespace anatomist ;
using namespace carto;
using namespace std;


Control *
NodeSelectionControl::creator( )
{
	//cout<<"creator Control"<<endl;
  NodeSelectionControl * ns = new NodeSelectionControl() ;

  return ( ns ) ;
}


NodeSelectionControl::NodeSelectionControl()
		: Control( 201, QT_TRANSLATE_NOOP( "ControlledWindow", "NodeSelectionControl" ) )
		{
	//cout<<"Constructeur NodeSelectionCONTROL"<<endl;
	}

NodeSelectionControl::NodeSelectionControl( const NodeSelectionControl & c )
  : Control(c)
  {
	//cout<<"Constructeur COPIE NodeSelectionCONTROL"<<endl;
  }

NodeSelectionControl::~NodeSelectionControl()
{
	//cout<<"Destructor NODESSELECTIONCONTROL"<<endl;
}


void
NodeSelectionControl::eventAutoSubscription( ActionPool * actionPool )
{

	//cout<<"EVENT SUSCRIPTION"<<endl;
  mousePressButtonEventSubscribe
    ( Qt::RightButton, Qt::NoModifier,
      MouseActionLinkOf<MenuAction>( actionPool->action( "MenuAction" ),
				     &MenuAction::execMenu ) );

	mousePressButtonEventSubscribe
    	( Qt::LeftButton, Qt::NoModifier,
      	MouseActionLinkOf<NodeSelectionAction>( actionPool->action( "NodeSelectionAction" ),
						&NodeSelectionAction::select ) );
						
/*
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<SelectAction>( actionPool->action( "SelectAction" ),
				       &SelectAction::execSelect ) );
*/
				       
				       /*
  mouseLongEventSubscribe
    ( Qt::LeftButton, Qt::NoModifier,
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
	&LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
	&LinkAction::execLink ),
      MouseActionLinkOf<LinkAction>( actionPool->action( "LinkAction" ),
	&LinkAction::endLink ), true );
*/

  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ShiftModifier,
      MouseActionLinkOf<NodeSelectionAction>( actionPool->action( "NodeSelectionAction" ),
				      &NodeSelectionAction::add ) );
				      /*
      MouseActionLinkOf<NodeSelectionControl>( actionPool->action( "NodeSelectionAction" ),
				      _add ) );

			*/
  mousePressButtonEventSubscribe
    ( Qt::LeftButton, Qt::ControlModifier,
      MouseActionLinkOf<NodeSelectionAction>( actionPool->action( "NodeSelectionAction" ),
				      &NodeSelectionAction::remove ) );

  // sort triangles by depth
  keyPressEventSubscribe( Qt::Key_D, Qt::NoModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::sort ),
                          "sort_polygons_by_depth" );

  keyPressEventSubscribe( Qt::Key_D, Qt::ControlModifier,
                          KeyActionLinkOf<SortMeshesPolygonsAction>
                          ( actionPool->action( "SortMeshesPolygonsAction" ),
                            &SortMeshesPolygonsAction::toggleAutoSort ),
                          "auto_sort_polygons_by_depth" );

  // rotation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::NoModifier,
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
				    &Trackball::beginTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
				    &Trackball::moveTrackball ),
      MouseActionLinkOf<Trackball>( actionPool->action( "Trackball" ),
				    &Trackball::endTrackball ), true );

  // zoom

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ShiftModifier,
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
				       &Zoom3DAction::beginZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
				       &Zoom3DAction::moveZoom ),
      MouseActionLinkOf<Zoom3DAction>( actionPool->action( "Zoom3DAction" ),
				       &Zoom3DAction::endZoom ), true );

  //	translation

  mouseLongEventSubscribe
    ( Qt::MiddleButton, Qt::ControlModifier,
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
	&Translate3DAction::beginTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
	&Translate3DAction::moveTranslate ),
      MouseActionLinkOf<Translate3DAction>
      ( actionPool->action( "Translate3DAction" ),
	&Translate3DAction::endTranslate ), true ) ;

/*Creation of action*/
	mySelection = dynamic_cast<NodeSelectionAction *>( actionPool->action( "NodeSelectionAction" ) );
}


void
NodeSelectionControl::doAlsoOnSelect( ActionPool * /* pool */ )
{

	//OPENS A NEW BROWSER AND CREATES BROWSER & GRAPH
  if(mySelection)
    {
	Hierarchy * hi;
	AWindow * bw;
	if( !theAnatomist->hasWindow( mySelection->getBrowser() ) )
	{
		bw = AWindowFactory::createWindow("Browser") ;
		hi = HierarchyEditor::newHierarchy( "New hierarchy", "hierarchy") ;
	}
	else
	{
		bw = mySelection->getBrowser();
		if( mySelection->getBrowser()->hasObject( mySelection->getHie() ) )
		//if( mySelection->getBrowser()->hasObject( ) )
		{
			hi=mySelection->getHie();
		}
		else
		{
			hi = HierarchyEditor::newHierarchy( "New hierarchy", "hierarchy") ;
			mySelection->setHie(hi);
		}
	}

	mySelection->setHie(hi);
	mySelection->setBrowser(bw);
	mySelection->getBrowser()->registerObject( hi );
    }

}


void
NodeSelectionControl::doAlsoOnDeselect ( ActionPool * /* pool */ )
{
	//CLOSES AND SAVES/DOESN'T SAVE THE NEW GRAPH
	if( !(mySelection->getBrowser()->hasObject( mySelection->getHie() ) ) )
	{}
	else
	{
		cout<<"SAVE procedure";
	}
}
