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


#include <anatomist/stdmod/stdmod.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/window3D/trackObliqueSlice.h>
#include <anatomist/window3D/transformer.h>
#include <anatomist/window3D/trackcut.h>
#include <anatomist/window3D/labeleditaction.h>
#include <anatomist/browser/browsercontrol.h>
#include <anatomist/controler/controldictionary.h>
#include <anatomist/controler/controlmanager.h>
#include <anatomist/controler/actiondictionary.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>

// commands
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cApplyBuiltinReferential.h>
#include <anatomist/commands/cAskTexExtrema.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cCamera.h>
#include <anatomist/commands/cClosePipe.h>
#include <anatomist/commands/cCloseWindow.h>
#include <anatomist/commands/cChangePalette.h>
#include <anatomist/commands/cControlsParams.h>
#include <anatomist/commands/cCreateControlWindow.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cDeleteAll.h>
#include <anatomist/commands/cDeleteElement.h>
#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/commands/cDuplicateObject.h>
#include <anatomist/commands/cEventFilter.h>
#include <anatomist/commands/cExit.h>
#include <anatomist/commands/cExportTexture.h>
#include <anatomist/commands/cExternalReference.h>
#include <anatomist/commands/cExtractTexture.h>
#include <anatomist/commands/cFusion2DParams.h>
#include <anatomist/commands/cFusion3DParams.h>
#include <anatomist/commands/cFusionInfo.h>
#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/commands/cGenerateTexture.h>
//#include <anatomist/commands/cGetGraphObjectNames.h>
#include <anatomist/commands/cGetInfo.h>
#include <anatomist/commands/cGraphDisplayProperties.h>
#include <anatomist/commands/cGraphParams.h>
#include <anatomist/commands/cGroupObjects.h>
#include <anatomist/commands/cLinkedCursor.h>
#include <anatomist/commands/cLinkWindows.h>
#include <anatomist/commands/cLoadGraphSubObjects.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cNewId.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/commands/cNewPalette.h>
#include <anatomist/commands/cObjectInfo.h>
#include <anatomist/commands/cOutput.h>
#include <anatomist/commands/cPopupPalette.h>
#include <anatomist/commands/cReloadObject.h>
#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/commands/cSaveObject.h>
#include <anatomist/commands/cSaveTransformation.h>
#include <anatomist/commands/cSelect.h>
#include <anatomist/commands/cSelectByHierarchy.h>
#include <anatomist/commands/cServer.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/commands/cSetMaterial.h>
#include <anatomist/commands/cSetObjectPalette.h>
#include <anatomist/commands/cShowObject.h>
#include <anatomist/commands/cSliceParams.h>
#include <anatomist/commands/cTexturingParams.h>
#include <anatomist/commands/cWindowConfig.h>

using namespace anatomist;
using namespace std;


StdModule::StdModule() : Module()
{
}


StdModule::~StdModule()
{
}


string StdModule::name() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", "Core module" ) );
}


string StdModule::description() const
{
  return( QT_TRANSLATE_NOOP( "ControlWindow", 
			     "Internal to the core Anatomist distribution - "
			     "always present" ) );
}


void StdModule::actionsDeclaration()
{
  //	actions

  LinkAction		la;
  ActionDictionary::instance()->addAction( la.name(), &LinkAction::creator );
  MenuAction		ma;
  ActionDictionary::instance()->addAction( ma.name(), &MenuAction::creator );
  SelectAction		sa;
  ActionDictionary::instance()->addAction( sa.name(), &SelectAction::creator );
  Zoom3DAction		za;
  ActionDictionary::instance()->addAction( za.name(), &Zoom3DAction::creator );
  Translate3DAction	ta;
  ActionDictionary::instance()->addAction( ta.name(), 
					   &Translate3DAction::creator );
  Trackball		tb;
  ActionDictionary::instance()->addAction( tb.name(), &Trackball::creator );
  ContinuousTrackball	ctb;
  ActionDictionary::instance()->addAction( ctb.name(), 
                                           &ContinuousTrackball::creator );
  TrackOblique		to;
  ActionDictionary::instance()->addAction( to.name(), &TrackOblique::creator );
  TrackObliqueSlice	ts;
  ActionDictionary::instance()->addAction( ts.name(), 
					   &TrackObliqueSlice::creator );
  Sync3DAction		sca;
  ActionDictionary::instance()->addAction( sca.name(), 
					   &Sync3DAction::creator );

  KeyFlightAction	fa;
  ActionDictionary::instance()->addAction( fa.name(), 
					   &KeyFlightAction::creator );

  Transformer		ta2;
  ActionDictionary::instance()->addAction( ta2.name(), 
					   &Transformer::creator );
  TranslaterAction	ta3;
  ActionDictionary::instance()->addAction( ta3.name(), 
					   &TranslaterAction::creator );
  PlanarTransformer		ta4;
  ActionDictionary::instance()->addAction( ta4.name(), 
					   &PlanarTransformer::creator );
  ResizerAction	ta5;
  ActionDictionary::instance()->addAction( ta5.name(), 
					   &ResizerAction::creator );
  TrackCutAction	tc;
  ActionDictionary::instance()->addAction( tc.name(), 
					   &TrackCutAction::creator );
  CutSliceAction	cs;
  ActionDictionary::instance()->addAction( cs.name(), 
					   &CutSliceAction::creator );

  MovieAction		mova;
  ActionDictionary::instance()->addAction( mova.name(), 
					   &MovieAction::creator );

  SliceAction		sla;
  ActionDictionary::instance()->addAction( sla.name(), &SliceAction::creator );

  DragObjectAction		doa;
  ActionDictionary::instance()->addAction( doa.name(), 
                                           &DragObjectAction::creator );

  WindowActions		wsa;
  ActionDictionary::instance()->addAction( wsa.name(), 
                                           &WindowActions::creator );

  LabelEditAction lea;
  ActionDictionary::instance()->addAction( lea.name(),
                             &LabelEditAction::creator );


  //	Commands
  AddObjectCommand::initSyntax();
  ApplyBuiltinReferentialCommand::initSyntax();
  AskTexExtremaCommand::initSyntax();
  AssignReferentialCommand::initSyntax();
  CameraCommand::initSyntax();
  ControlsParamsCommand::initSyntax();
  ClosePipeCommand::initSyntax();
  CloseWindowCommand::initSyntax();
  ChangePaletteCommand::initSyntax();
  CreateControlWindowCommand::initSyntax();
  CreateWindowCommand::initSyntax();
  DeleteAllCommand::initSyntax();
  DeleteElementCommand::initSyntax();
  DeleteObjectCommand::initSyntax();
  DuplicateObjectCommand::initSyntax();
  EventFilterCommand::initSyntax();
  ExitCommand::initSyntax();
  ExportTextureCommand::initSyntax();
  ExternalReferenceCommand::initSyntax();
  ExtractTextureCommand::initSyntax();
  Fusion2DParamsCommand::initSyntax();
  Fusion3DParamsCommand::initSyntax();
  FusionInfoCommand::initSyntax();
  FusionObjectsCommand::initSyntax();
  GenerateTextureCommand::initSyntax();
  //GetGraphObjectNamesCommand::initSyntax();
  GetInfoCommand::initSyntax();
  GraphDisplayPropertiesCommand::initSyntax();
  GraphParamsCommand::initSyntax();
  GroupObjectsCommand::initSyntax();
  LinkedCursorCommand::initSyntax();
  LinkWindowsCommand::initSyntax();
  LoadGraphSubObjectsCommand::initSyntax();
  LoadObjectCommand::initSyntax();
  LoadTransformationCommand::initSyntax();
  NewIdCommand::initSyntax();
  NewPaletteCommand::initSyntax();
  ObjectInfoCommand::initSyntax();
  OutputCommand::initSyntax();
  PopupPaletteCommand::initSyntax();
  ReloadObjectCommand::initSyntax();
  RemoveObjectCommand::initSyntax();
  SaveObjectCommand::initSyntax();
  SaveTransformationCommand::initSyntax();
  SelectCommand::initSyntax();
  SelectByHierarchyCommand::initSyntax();
  ServerCommand::initSyntax();
  SetControlCommand::initSyntax();
  SetMaterialCommand::initSyntax();
  SetObjectPaletteCommand::initSyntax();
  ShowObjectCommand::initSyntax();
  SliceParamsCommand::initSyntax();
  TexturingParamsCommand::initSyntax();
  WindowConfigCommand::initSyntax();
}


void StdModule::controlsDeclaration()
{
  //	controls

  Control3D		c3;
  ControlDictionary::instance()->addControl( c3.name(), &Control3D::creator, 
					     c3.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "", c3.name() );
  Select3DControl	c3s;
  ControlDictionary::instance()->addControl( c3s.name(), 
					     &Select3DControl::creator, 
					     c3s.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "", c3s.name() );
  FlightControl		fc;
  ControlDictionary::instance()->addControl( fc.name(), 
					     &FlightControl::creator, 
					     fc.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "", fc.name() );
  ObliqueControl	co;
  ControlDictionary::instance()->addControl( co.name(), 
					     &ObliqueControl::creator, 
					     co.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "", co.name() );
  TransformControl	tc;
  ControlDictionary::instance()->addControl( tc.name(), 
					     &TransformControl::creator, 
					     tc.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "", tc.name() );
  CutControl	cc;
  ControlDictionary::instance()->addControl( cc.name(), 
					     &CutControl::creator, 
					     cc.priority() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "CutMesh", 
					  cc.name() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "Slice", 
					  cc.name() );
  ControlManager::instance()->addControl( "QAGLWidget3D", "ClippedObject",
                                          cc.name() );

  SelectBrowserControl	sbc;
  ControlDictionary::instance()->addControl( sbc.name(), 
                                             &SelectBrowserControl::creator,
                                             sbc.priority() );
  ControlManager::instance()->addControl( "Browser", "", sbc.name() );

  //	Icons
  {
  QPixmap	p;
  if( p.load( Settings::findResourceFile(
              "icons/trackball.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( c3.name(), p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/oblique.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( co.name(), p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/select.xpm" ).c_str() ) )
    {
      IconDictionary::instance()->addIcon( c3s.name(), p );
      IconDictionary::instance()->addIcon( sbc.name(), p );
    }
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/flight.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( fc.name(), p );

  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/window-axial-small.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "axial", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/window-coronal-small.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "coronal", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/window-sagittal-small.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "sagittal", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/window-oblique-small.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "oblique", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/window-3d-small.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "3D", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/control-transfo.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( tc.name(), p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/control-cut.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( cc.name(), p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile(
              "icons/direct_ref_mark.png" ).c_str() ) )
    IconDictionary::instance()->addIcon( "direct_ref_mark", p );
  }
  {
  QPixmap       p;
  if( p.load( Settings::findResourceFile( "icons/roi.xpm" ).c_str() ) )
    IconDictionary::instance()->addIcon( "RoiControl", p );
  }

  {
    QPixmap       p;
    if( p.load( Settings::findResourceFile(
      "icons/meshPaint/SurfpaintControl.png" ).c_str() ) )
      IconDictionary::instance()->addIcon( "SurfpaintToolsControl", p );
  }

}
