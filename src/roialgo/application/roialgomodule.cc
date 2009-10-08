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


#include <anatomist/application/roialgomodule.h>
#include <anatomist/control/dynsegmentcontrol.h>
#include <anatomist/action/dynsegmentaction.h>
#include <anatomist/action/morphomath.h>

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
#include <aims/graph/graphmanip.h>
#include <aims/io/datatypecode.h>
#include <aims/utility/converter_bucket.h>
#include <aims/vector/vector.h>

#include <graph/tree/tree.h>
#include <graph/graph/graph.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

static bool initRoiAlgoModule()
{
  RoiAlgoModule	*a = new RoiAlgoModule;
  a->init();
  return( true );
}

static bool garbage = initRoiAlgoModule();

std::string 
RoiAlgoModule::name() const
{ 
  return QT_TRANSLATE_NOOP( "ControlWindow", "Regions of Interest Algos" );
}

std::string 
RoiAlgoModule::description() const 
{ 
  return QT_TRANSLATE_NOOP( "ControlWindow", 
			    "Segmentation and manipulation of regions of "
			    "interest : Algorithms module" );
} 

RoiAlgoModule::RoiAlgoModule() : Module() { }

RoiAlgoModule::~RoiAlgoModule() 
{ }

void RoiAlgoModule::objectsDeclaration() { }

void RoiAlgoModule::objectPropertiesDeclaration()
{
}

void RoiAlgoModule::viewsDeclaration()
{

}

void RoiAlgoModule::actionsDeclaration()
{
  ActionDictionary::instance()->addAction("DynSegmentAction", RoiDynSegmentAction::creator ) ;
  ActionDictionary::instance()->addAction("MorphoMathAction", RoiMorphoMathAction::creator ) ;
}

void RoiAlgoModule::controlsDeclaration()
{
  ControlDictionary::instance()->addControl("DynSegmentControl",
					    RoiDynSegmentControl::creator, 106 ) ;
  
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::BUCKET ), 
					  "DynSegmentControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::VOLUME ), 
					  "DynSegmentControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::GRAPHOBJECT ), 
					  "DynSegmentControl" ) ;
  
  
  QPixmap	p;
  if( p.load( ( Settings::globalPath() + "/icons/dynsegment.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "DynSegmentControl", p );
}

void RoiAlgoModule::controlGroupsDeclaration()
{

}
