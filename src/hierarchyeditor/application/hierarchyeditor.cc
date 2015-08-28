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

/* Bibliothèques à vérifier...*/

#include <anatomist/application/hierarchyeditor.h>
#include <anatomist/control/nodeselectioncontrol.h>
#include <anatomist/action/nodeselectionaction.h>

#include <anatomist/object/Object.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/color/Material.h>
#include <anatomist/control/wControl.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/actions.h>
#include <anatomist/misc/error.h>
#include <anatomist/observer/observcreat.h>
#include <aims/graph/graphmanip.h>
#include <aims/io/datatypecode.h>
#include <aims/utility/converter_bucket.h>
#include <aims/vector/vector.h>

#include <anatomist/window/glwidget.h>
#include <anatomist/controler/view.h>

#include <anatomist/control/qObjTree.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>

#include <cartobase/stream/fileutil.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/winFactory.h>

#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <cartobase/stream/fileutil.h>


#include <qdialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <qlabel.h>
#include <qobject.h>
#include <qcursor.h>
#include <iostream>

#include <anatomist/browser/qwObjectBrowser.h>

#include <anatomist/control/listboxeditor.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

/* Truc inutile, juste pour initialiser les variables et charger la librairie*/
static bool initHierarchyEditor()
{
	cout<<"OK SUPER"<<endl;
  HierarchyEditor	*a = new HierarchyEditor;
  a->init();
  QObjectBrowser::registerAttributeEditor( "CorticalFoldArg","type",QObjectBrowser::floatEditor );
  return( true );
}

static bool garbage = initHierarchyEditor();
/********/
   HierarchyEditor::HierarchyEditor()  : Module()
   {
	cout<<"Constructeur MODULE"<<endl;
   }

   HierarchyEditor::~HierarchyEditor()
   {
	cout<<"Destructeur MODULE"<<endl;
   }


std::string
HierarchyEditor::name() const
{
  return QT_TRANSLATE_NOOP( "ControlWindow", "Graph Editor" );
}

std::string
HierarchyEditor::description() const
{
  return QT_TRANSLATE_NOOP( "ControlWindow", "Graph Editor" );
 }

void HierarchyEditor::actionsDeclaration()
{
  ActionDictionary::instance()->addAction("NodeSelectionAction", NodeSelectionAction::creator) ;

}

void HierarchyEditor::controlsDeclaration()
{
cout<<"control1"<<endl;
  ControlDictionary::instance()->addControl("NodeSelectionControl", NodeSelectionControl::creator, 500 ) ;
cout<<"control2"<<endl;
  /* Contexte d'activation du contrôle : quand il y a un graph uniquement*/
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::GRAPHOBJECT ),
					  "NodeSelectionControl" ) ;
cout<<"control3"<<endl;

  QPixmap	p;
  if( p.load( Settings::findResourceFile( "icons/edit_graph.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "NodeSelectionControl", p );

}


Hierarchy* HierarchyEditor::newHierarchy( const string & /*selectName*/,
                                          const string & syntax )
{

  Tree	*tr = new Tree( true, syntax );
  Hierarchy	*hie = new Hierarchy( tr) ;
  string ty = "hierarchy";
  //hie->setFileName( selectName ) ;
  string nm = askName( ty);
  hie->setName( nm );
  //hie->setName( theAnatomist->makeObjectName( selectName ) );
  theAnatomist->registerObject( hie );

  //tr->setAttribute( syntax + "_VERSION", string( "1.0" ) );

  return( hie );
}

string
HierarchyEditor::askName( const string & type, const string& originalName )
{
  string message("Enter ") ;
  message += type ;
  message += string(" name") ;
  QDialog * nameSetter = new QDialog;
  nameSetter->setModal( true );
  nameSetter->setWindowTitle( message.c_str() ) ;
  nameSetter->setMinimumSize( 400, 30 ) ;
  QLineEdit * lineEdition= new QLineEdit( QString( originalName.c_str() ),
					  nameSetter/*, message.c_str()*/ ) ;
  lineEdition->setMinimumWidth(400) ;
  lineEdition->setMinimumHeight(50) ;
  QObject::connect( lineEdition , SIGNAL(  returnPressed( ) ), nameSetter,
		    SLOT( accept ( ) ) ) ;
  int res = nameSetter->exec();
  delete nameSetter;
  if( res )
  {
    if( string( "" ) == lineEdition->text().toStdString() )
      return "Unknown" ;
    return lineEdition->text().toStdString();
  }
  return "Unknown" ;
}


