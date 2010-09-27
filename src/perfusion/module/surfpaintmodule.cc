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

#include <anatomist/module/surfpaintmodule.h>
#include <anatomist/action/surfpaintaction.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <qtranslator.h>

#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/window/Window.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/object/actions.h>
#include <anatomist/misc/error.h>

#include <aims/mesh/surface.h>
#include <anatomist/surface/triangulated.h>
#include <aims/mesh/texture.h>
#include <anatomist/surface/texture.h>

using namespace anatomist;
using namespace std;


static bool initSurfpaintModule()
{
  //cout << "initSurfpaintModule\n";
  SurfpaintModule	*a = new SurfpaintModule;
  a->init();
  return( true );
}

static bool garbage = initSurfpaintModule();


SurfpaintModule::SurfpaintModule() : Module()
{
}


SurfpaintModule::~SurfpaintModule()
{
}


string SurfpaintModule::name() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "Surfpaint" ) );
}


string SurfpaintModule::description() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "picking and painting a texture value on the mesh" ) );
}

void SurfpaintModule::objectsDeclaration() { }

void SurfpaintModule::viewsDeclaration() { }

void SurfpaintModule::actionsDeclaration()
{
  //std::cout << "Surfpaint actions\n" << std::endl;
  ActionDictionary::instance()->addAction("SurfpaintColorPickerAction", SurfpaintColorPickerAction::creator ) ;
  ActionDictionary::instance()->addAction("SurfpaintBrushAction", SurfpaintBrushAction::creator ) ;
  ActionDictionary::instance()->addAction("SurfpaintEraseAction", SurfpaintEraseAction::creator ) ;
  ActionDictionary::instance()->addAction("SurfpaintShortestPathAction", SurfpaintShortestPathAction::creator ) ;

}

void SurfpaintModule::controlsDeclaration()
{
  //std::cout << "Surfpaint control\n" << std::endl;
  ControlDictionary::instance()->addControl("SurfpaintColorPickerControl",SurfpaintColorPickerControl::creator, 555 ) ;
  ControlDictionary::instance()->addControl("SurfpaintBrushControl",SurfpaintBrushControl::creator, 556 ) ;
  ControlDictionary::instance()->addControl("SurfpaintEraseControl",SurfpaintEraseControl::creator, 557 ) ;
  ControlDictionary::instance()->addControl("SurfpaintShortestPathControl",SurfpaintShortestPathControl::creator, 558 ) ;

  //ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TRIANG ),"SurfpaintBrushControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TEXSURFACE ),"SurfpaintBrushControl" ) ;

  //ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TRIANG ),"SurfpaintColorPickerControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TEXSURFACE ),"SurfpaintColorPickerControl" ) ;

  //ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TRIANG ),"SurfpaintEraseControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TEXSURFACE ),"SurfpaintEraseControl" ) ;

  //ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TRIANG ),"SurfpaintShortestPathControl" ) ;
  ControlManager::instance()->addControl( "QAGLWidget3D", AObject::objectTypeName( AObject::TEXSURFACE ),"SurfpaintShortestPathControl" ) ;


  {
    QPixmap p;
    if( p.load( ( Settings::globalPath() + "/icons/meshPaint/colorpicker.png" ).c_str() ) )
      IconDictionary::instance()->addIcon( "SurfpaintColorPickerControl", p );
  }

  {
  QPixmap p;
  if( p.load( ( Settings::globalPath() + "/icons/meshPaint/paintbrush.png" ).c_str() ) )
    IconDictionary::instance()->addIcon( "SurfpaintBrushControl", p );
  }

  {
  QPixmap p;
  if( p.load( ( Settings::globalPath() + "/icons/meshPaint/build.png" ).c_str() ) )
    IconDictionary::instance()->addIcon( "buildIcon", p );
  }

  {
  QPixmap p;
  if( p.load( ( Settings::globalPath() + "/icons/meshPaint/erase.png" ).c_str() ) )
    IconDictionary::instance()->addIcon( "SurfpaintEraseControl", p );
  }

  {
  QPixmap p;
  if( p.load( ( Settings::globalPath() + "/icons/meshPaint/shortestpath.png" ).c_str() ) )
    IconDictionary::instance()->addIcon( "SurfpaintShortestPathControl", p );
  }
}
