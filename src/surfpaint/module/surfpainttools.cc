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

#include <anatomist/module/surfpainttools.h>
#include <anatomist/object/oReader.h>
#include <anatomist/control/wControl.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/application/settings.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/mesh/surfacegen.h>
#include <aims/geodesicpath/geodesicPath.h>
#include <cartobase/stream/fileutil.h>
#include <QToolBar>
#include <QToolButton>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qaction.h>
#include <QMenu>
#include <queue>
#include <float.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

struct SurfpaintTools::Private
{
  Private() {}

  list<unsigned> undone_listIndexVertexPathSP;
  list<unsigned> undone_listIndexVertexSelectSP;
  list<unsigned> numVerticesInPathElements;
  list<unsigned> undone_numVerticesInPathElements;

  list<unsigned> undone_listIndexVertexBrushPath;
  list<unsigned> undone_listIndexVertexHolesPath;
  list<unsigned> numIndexVertexHolesPath;
  list<unsigned> undone_numIndexVertexHolesPath;
  list<unsigned> holeVertexIndices;
  list<unsigned> undone_holeVertexIndices;
  list<unsigned> holeCount;
  list<unsigned> undone_holeCount;

  set<int> undone_listIndexVertexFill;

  list<rc_ptr<ATriangulated> > undone_pathObject;
  list<rc_ptr<ATriangulated> > undone_fillObject;
  list<rc_ptr<ATriangulated> > undone_holesObject;

  list<map<unsigned, float> > recorded_modifs;
  list<map<unsigned, float> > undone_modifs;
};


SurfpaintTools::SurfpaintTools()/* : Observer()*/
    : QWidget(theAnatomist->getQWidgetAncestor(), Qt::Window),
  d( new Private ),
  surfpaintTexInit( 0 ),
  win3D( 0 ),
  objselect( 0 ),
  tbTextureValue( 0 ),
  textureFloatSpinBox( 0 ),
  tbInfos3D( 0 ),
  IDPolygonSpinBox( 0 ),
  IDVertexSpinBox( 0 ),
  tbControls( 0 ),
  colorPickerAction( 0 ),
  selectionAction( 0 ),
  pathAction( 0 ),
  shortestPathAction( 0 ),
  sulciPathAction( 0 ),
  gyriPathAction( 0 ),
  paintBrushAction( 0 ),
  magicBrushAction( 0 ),
  validateEditAction( 0 ),
  eraseAction( 0 ),
  clearPathAction( 0 ),
  saveAction( 0 ),
  distanceAction( 0 ),
  constraintList( 0 ),
  toleranceSpinBox( 0 ),
  toleranceSpinBoxLabel( 0 ),
  textureValueMinSpinBox( 0 ),
  textureValueMaxSpinBox( 0 ),
  constraintPathSpinBox( 0 ),
  constraintPathSpinBoxLabel( 0 ),
  constraintPathValue( 0 ),
  toleranceValue( 0 ),
  stepToleranceValue( 0 ),
  IDActiveControl( 4 ),
  sp( 0 ),
  sp_sulci( 0 ),
  sp_gyri( 0 ),
  pathClosed( false ),
  mesh(0),
  at(0),
  options(0)
{
  changeControl(0);
  shortestPathSelectedType = "ShortestPath";
}

SurfpaintTools::~SurfpaintTools()
{
  delete sp;
  delete sp_sulci;
  delete sp_gyri;
  delete d;
}

void SurfpaintTools::popAllButtonPaintToolBar()
{
  colorPickerAction->setChecked(false);
  distanceAction->setChecked(false);
  selectionAction->setChecked(false);
  pathAction->setChecked(false);
  shortestPathAction->setChecked(false);
  sulciPathAction->setChecked(false);
  gyriPathAction->setChecked(false);
  paintBrushAction->setChecked(false);
  magicBrushAction->setChecked(false);
  eraseAction->setChecked(false);
}

void SurfpaintTools::colorPicker()
{
  popAllButtonPaintToolBar();
  colorPickerAction->setChecked(true);
  changeControl(1);
}

void SurfpaintTools::distance()
{
  popAllButtonPaintToolBar();
  distanceAction->setChecked(true);
  changeControl(7);
}

void SurfpaintTools::magicSelection()
{
  popAllButtonPaintToolBar();
  selectionAction->setChecked(true);
  changeControl(2);
}

void SurfpaintTools::path()
{
  if (shortestPathSelectedType.compare("ShortestPath") == 0)
    shortestPath();
  else if (shortestPathSelectedType.compare("SulciPath") == 0)
    sulciPath();
  else if (shortestPathSelectedType.compare("GyriPath") == 0)
    gyriPath();
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::shortestPath()
{
  popAllButtonPaintToolBar();
  string iconname = Settings::findResourceFile(
    "icons/meshPaint/shortest.png" );
  pathAction->setIcon(QIcon(iconname.c_str()));
  pathAction->setChecked(true);
  shortestPathSelectedType = "ShortestPath";
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::sulciPath()
{
  popAllButtonPaintToolBar();
  string iconname = Settings::findResourceFile( "icons/meshPaint/sulci.png" );
  pathAction->setIcon(QIcon(iconname.c_str()));
  pathAction->setChecked(true);
  shortestPathSelectedType = "SulciPath";
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::gyriPath()
{
  popAllButtonPaintToolBar();
  string iconname = Settings::findResourceFile( "icons/meshPaint/gyri.png" );
  pathAction->setIcon(QIcon(iconname.c_str()));
  pathAction->setChecked(true);
  shortestPathSelectedType = "GyriPath";
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::brush()
{
  popAllButtonPaintToolBar();
  paintBrushAction->setChecked(true);
  changeControl(4);
}

void SurfpaintTools::magicBrush()
{
  popAllButtonPaintToolBar();
  magicBrushAction->setChecked(true);
  changeControl(6);
}

void SurfpaintTools::validateEdit()
{
  int i;

  float texvalue = getTextureValueFloat();

  // fill closed region
  if (IDActiveControl == 2)
  {
    std::set<int>::iterator ite = listIndexVertexFill.begin();
    for (; ite != listIndexVertexFill.end(); ite++)
    {
      updateTextureValue(*ite, texvalue);
    }

    clearRegion();
  }

  // path fill
  if (IDActiveControl == 3)
  {
    for (unsigned i = 0; i < listIndexVertexPathSP.size(); i++)
    {
      updateTextureValue(listIndexVertexPathSP[i], texvalue);
    }

    clearPath();
  }

  // holes fill
  if (IDActiveControl == 6)
  {
    for (unsigned i = 0; i < listIndexVertexHolesPath.size(); i++)
    {
      updateTextureValue(listIndexVertexHolesPath[i], texvalue);
    }

    clearHoles();
  }

}

void SurfpaintTools::erase()
{
  popAllButtonPaintToolBar();
  eraseAction->setChecked(true);
  changeControl(5);
  clearAll();
}

void SurfpaintTools::clearAll()
{
  clearPath();
  clearRegion();
  clearHoles();
}

void SurfpaintTools::clearPath()
{
  clearUndoneGeodesicPath();
  listIndexVertexSelectSP.clear();
  listIndexVertexBrushPath.clear();
  listIndexVertexPathSP.clear();

  vector<rc_ptr<ATriangulated> >::iterator ite;
  ite = pathObject.begin();

  for (; ite != pathObject.end(); ++ite)
  {
    win3D->unregisterObject( ite->get() );
    theAnatomist->unregisterObject( ite->get() );
  }

  pathObject.clear();
}

void SurfpaintTools::clearHoles()
{
  clearUndoneHolesPaths();

  listIndexVertexBrushPath.clear();
  listIndexVertexHolesPath.clear();
  d->holeVertexIndices.clear();
  d->holeCount.clear();

  vector<rc_ptr<ATriangulated> >::iterator ite;
  ite = holesObject.begin();

  for (; ite != holesObject.end(); ++ite)
  {
    win3D->unregisterObject( ite->get() );
    theAnatomist->unregisterObject( ite->get() );
  }

  holesObject.clear();
}


void SurfpaintTools::clearUndoneSelectFill()
{
  d->undone_listIndexVertexFill.clear();
  d->undone_fillObject.clear();
}


void SurfpaintTools::clearRegion()
{
  clearUndoneSelectFill();

  listIndexVertexFill.clear();

  vector<rc_ptr<ATriangulated> >::iterator ite;

  ite = fillObject.begin();

  for (; ite != fillObject.end(); ++ite)
  {
    win3D->unregisterObject( ite->get() );
    theAnatomist->unregisterObject( ite->get() );
  }

  fillObject.clear();
}


void SurfpaintTools::save()
{
  rc_ptr<TimeTexture<float> > out = at->texture<float>( true );
  for (uint i = 0; i < at->size(); i++)
  {
    surfpaintTexInit[0].item(i) = (*out).item(i);
  }

  QString filt = ( ControlWindow::tr( "Textures" ).toStdString() + " ("
    + ObjectReader::supportedFileExtensions( "Texture" ) + ");;"
    + ControlWindow::tr( "All files" ).toStdString() + " (*)" ).c_str();
  QString capt = "Save Texture" ;

  QString filename = QFileDialog::getSaveFileName( QString::null, filt, 0, 0, capt );

  if( !filename.isNull() )
  {
    at->save( filename.utf8().data() );
  }

  clearAll();
}

bool SurfpaintTools::initSurfPaintModule(AWindow3D *w3)
{
  win3D = w3;

  stepToleranceValue = 0;

  //sélectionne l'objet positionné au milieu de la fenêtre (bof !)

  //récupère parmi tous les objets sélectionnés le dernier objet de type ATexSurface
  map< unsigned, set< AObject *> > sel = SelectFactory::factory()->selected ();
  map< unsigned, set< AObject *> >::iterator iter( sel.begin( ) ),last( sel.end( ) ) ;

  while( iter != last )
  {
    for( set<AObject*>::iterator it = iter->second.begin() ; it != iter->second.end() ; ++it )
    {
      if ((AObject::objectTypeName((*it)->type()) == "TEXTURED SURF."))
      {
        objtype = AObject::objectTypeName((*it)->type());
        objselect = (*it);
        cout << " " << (*it)->name() << "\n";
        break;
      }
    }

    ++iter ;
  }

  if( !objselect )
  {
    set<AObject *> objs = w3->Objects();
    for( set<AObject*>::iterator it=objs.begin(), et=objs.end(); it!=et; ++it )
    {
      if ((AObject::objectTypeName((*it)->type()) == "TEXTURED SURF."))
      {
        objtype = AObject::objectTypeName((*it)->type());
        objselect = (*it);
        cout << " " << (*it)->name() << "\n";
        break;
      }
    }
  }

  if( !objselect )
  {
    QMessageBox::warning(this, ControlledWindow::tr("No object selected"),
        ControlledWindow::tr("Cannot open surfpaint Toolbox"));
    return false;
  }

  GLComponent *glc = objselect->glAPI();

  if( !glc )
    return false;

  glc->glAPI()->glSetTexRGBInterpolation(true);

  if ( w3->hasObject(objselect))
  {
    objtype = objselect->objectTypeName(objselect->type());

    // cout << objtype << endl;

    if (objtype == "SURFACE")
    {
      QMessageBox::warning(this, ControlledWindow::tr("no associated texture"),
          ControlledWindow::tr("Cannot open surfpaint Toolbox"));
    }

    if (objtype == "TEXTURED SURF.")
    {
      ATexSurface *go = dynamic_cast<ATexSurface *> (objselect);

      if( !go )
      {
        cout << "not a ATexSurface\n";
        return false;
      }

      ATriangulated *as = static_cast<ATriangulated *> (go->surface());

      try
      {
        at = dynamic_cast<ATexture *>( go->texture() );
      }
      catch (const std::bad_cast& e)
      {
        std::cerr << e.what() << std::endl;
        std::cerr <<  objselect << " is not a Atexture" << std::endl;
      }
      if( !at )
        throw std::bad_cast();

      mesh = as->surface();

      AimsSurface<3,Void>   & surf= (*mesh)[0];
      vector<Point3df>    & vert = surf.vertex();
      vector<AimsVector<uint, 3> >  & tri = surf.polygon();

      options = Object::value(Dictionary());
      options->setProperty("scale", 0);

      at->attributed()->getProperty("data_type", textype);
      cout << "type texture :" << textype << endl;

      cout << "create Texture temp" << endl;

      float it = at->TimeStep();
      int tn = 0; // 1st texture
      GLComponent::TexExtrema & te = at->glTexExtrema(tn);
      int tx = 0; // 1st tex coord
      float scl = (te.maxquant[tx] - te.minquant[tx]);

      surfpaintTexInit = new Texture1d;
      surfpaintTexInit->reserve(at->size());

      rc_ptr<TimeTexture<float> > text ;
      text = ObjectConverter<TimeTexture<float> >::ana2aims(at, options);

      for (uint i = 0; i < at->size(); i++)
        surfpaintTexInit[0].item(i) = scl*(*text).item(i);

      setMaxPoly(tri.size());
      setMaxVertex(vert.size());

      int constraintType = w3->getConstraintType();
      AObject *constraintTex = w3->getConstraintTexture();

      if (w3->constraintEditorIsActive() && constraintType == 0)
      {
        setMinMaxTexture(-FLT_MAX, FLT_MAX);
      }
      else
      {
        setMinMaxTexture((float) (te.minquant[0]), (float) (te.maxquant[0]));
        textureValueMinSpinBox->setValue(te.minquant[0]);
        float cmax = 100;
        if( te.maxquant[0] != te.minquant[0] )
          cmax = te.maxquant[0];
        textureValueMaxSpinBox->setValue( cmax );
        setMinMaxTexture( te.minquant[0], cmax );
      }

      if (constraintTex == NULL)
      {
        cout << "compute texture curvature : ";
        texCurv = TimeTexture<float> (1, vert.size());
        texCurv = AimsMeshCurvature(surf);
        cout << "done" << endl;
      }
      else
      {
        cout << "load constraint texture  : ";
        texCurv = TimeTexture<float> (1, vert.size());

        try
        {
          ATexture *at=dynamic_cast<ATexture *> (constraintTex);

          text = ObjectConverter<TimeTexture<float> >::ana2aims(at, options);
          texCurv = *text;

          cout << "done" << endl;
        }
        catch (const std::bad_cast& e)
        {
          std::cerr << e.what() << std::endl;
          std::cerr <<  constraintTex << " is not a Atexture" << std::endl;
        }

      }

      sp = new GeodesicPath(*mesh,texCurv,0,0);
      sp_sulci = new GeodesicPath(*mesh,texCurv,1,3);
      sp_gyri = new GeodesicPath(*mesh,texCurv,2,3);

      cout << "compute surface neighbours : ";
      neighbours = SurfaceManip::surfaceNeighbours((*mesh));
      cout << "done" << endl;
    }
  }

  w3->Refresh();
  return true;
}

void SurfpaintTools::addToolBarControls(AWindow3D *w3)
{
  // cout << "addToolBarControls\n";

  if (w3)
  {
    const QPixmap *p;

    tbControls = new QToolBar(w3, ControlledWindow::tr(
        "surfpainttoolbarControls"));

    w3->addToolBar( Qt::TopToolBarArea,tbControls, ControlledWindow::tr( "surfpainttoolbarControls") );
    tbControls->setLabel( ControlledWindow::tr( "surfpainttoolbarControls") );
    tbControls->setIconSize( QSize( 20, 20 ) );

    string iconname;

    //brush
    iconname = Settings::findResourceFile( "icons/meshPaint/stylo.png" );
    paintBrushAction = new QToolButton();
    paintBrushAction->setIcon(QIcon(iconname.c_str()));
    paintBrushAction->setToolTip(ControlledWindow::tr("Brush"));
    paintBrushAction->setCheckable(true);
    paintBrushAction->setChecked(false);
    paintBrushAction->setIconSize(QSize(32, 32));
    paintBrushAction->setAutoRaise(true);
    connect(paintBrushAction, SIGNAL(clicked()), this, SLOT(brush()));

    //pipette
    iconname = Settings::findResourceFile( "icons/meshPaint/pipette.png" );
    colorPickerAction = new QToolButton();
    colorPickerAction->setIcon(QIcon(iconname.c_str()));
    //colorPickerAction->setStatusTip(tr("Texture value selection"));
    colorPickerAction->setToolTip(tr("Texture value selection"));
    colorPickerAction->setCheckable(true);
    colorPickerAction->setChecked(false);
    colorPickerAction->setIconSize(QSize(32, 32));
    colorPickerAction->setAutoRaise(true);
    connect(colorPickerAction, SIGNAL(clicked()), this, SLOT(colorPicker()));

    //erase
    iconname = Settings::findResourceFile( "icons/meshPaint/erase.png" );
    eraseAction = new QToolButton();
    eraseAction->setIcon(QIcon(iconname.c_str()));
    eraseAction->setToolTip(ControlledWindow::tr("Eraser"));
    eraseAction->setCheckable(true);
    eraseAction->setChecked(false);
    eraseAction->setIconSize(QSize(32, 32));
    eraseAction->setAutoRaise(true);
    connect(eraseAction, SIGNAL(clicked()), this, SLOT(erase()));

    //magic wand
    iconname = Settings::findResourceFile(
      "icons/meshPaint/magic_selection.png" );
    selectionAction = new QToolButton();
    selectionAction->setIcon(QIcon(iconname.c_str()));
    selectionAction->setToolTip(ControlledWindow::tr("Area magic selection"));
    selectionAction->setCheckable(true);
    selectionAction->setChecked(false);
    selectionAction->setIconSize(QSize(32, 32));
    selectionAction->setAutoRaise(true);
    connect(selectionAction, SIGNAL(clicked()), this, SLOT(magicSelection()));

    // shortest path
    iconname = Settings::findResourceFile( "icons/meshPaint/shortest.png" );
    pathAction = new QToolButton();
    pathAction->setIcon(QIcon(iconname.c_str()));
    pathAction->setPopupMode(QToolButton::MenuButtonPopup);
    pathAction->setToolTip(ControlledWindow::tr("GeodesicPath"));
    pathAction->setCheckable(true);
    pathAction->setChecked(false);
    pathAction->setAutoRaise(true);
    pathAction->setIconSize(QSize(32, 32));
    connect(pathAction, SIGNAL(clicked()), this, SLOT(path()));
    QMenu *menu = new QMenu(this);
    shortestPathAction = new QAction(QIcon(iconname.c_str()),
        ControlledWindow::tr("Unconstrained"), this);
    shortestPathAction->setToolTip(tr("Unconstrained"));
    connect(shortestPathAction, SIGNAL(triggered()), this, SLOT(shortestPath()));
    menu->addAction(shortestPathAction);
    iconname = Settings::findResourceFile( "icons/meshPaint/sulci.png" );
    sulciPathAction = new QAction(QIcon(iconname.c_str()),
        ControlledWindow::tr("sulci"), this);
    sulciPathAction->setToolTip(tr("sulci"));
    connect(sulciPathAction, SIGNAL(triggered()), this, SLOT(sulciPath()));
    menu->addAction(sulciPathAction);
    iconname = Settings::findResourceFile( "icons/meshPaint/gyri.png" );
    gyriPathAction = new QAction(QIcon(iconname.c_str()), ControlledWindow::tr(
        "gyri"), this);
    gyriPathAction->setToolTip(tr("gyri"));
    connect(gyriPathAction, SIGNAL(triggered()), this, SLOT(gyriPath()));
    menu->addAction(gyriPathAction);
    pathAction->setMenu(menu);

    //magic brush
    iconname = Settings::findResourceFile( "icons/meshPaint/magic_pencil.png" );
    magicBrushAction = new QToolButton();
    magicBrushAction->setIcon(QIcon(iconname.c_str()));
    magicBrushAction->setToolTip(ControlledWindow::tr("MagicBrush"));
    magicBrushAction->setCheckable(true);
    magicBrushAction->setChecked(false);
    magicBrushAction->setIconSize(QSize(32, 32));
    magicBrushAction->setAutoRaise(true);
    connect(magicBrushAction, SIGNAL(clicked()), this, SLOT(magicBrush()));

    //validate
    iconname = Settings::findResourceFile( "icons/meshPaint/valide.png" );
    validateEditAction = new QToolButton();
    validateEditAction->setIcon(QIcon(iconname.c_str()));
    validateEditAction->setToolTip(ControlledWindow::tr("Fill area or path selected"));
    validateEditAction->setIconSize(QSize(32, 32));
    connect(validateEditAction, SIGNAL(clicked()), this, SLOT(validateEdit()));

    //cancel
    iconname = Settings::findResourceFile( "icons/meshPaint/clear.png" );
    clearPathAction = new QToolButton();
    clearPathAction->setIcon(QIcon(iconname.c_str()));
    clearPathAction->setToolTip(ControlledWindow::tr(
        "Delete all selected objects"));
    clearPathAction->setIconSize(QSize(32, 32));
    connect(clearPathAction, SIGNAL(clicked()), this, SLOT(clearAll()));

    //distance
    iconname = Settings::findResourceFile( "icons/meshPaint/geodesic_distance.png" );
    distanceAction = new QToolButton();
    distanceAction->setIcon(QIcon(iconname.c_str()));
    distanceAction->setToolTip(tr("distance map"));
    distanceAction->setCheckable(true);
    distanceAction->setChecked(false);
    distanceAction->setIconSize(QSize(32, 32));
    distanceAction->setAutoRaise(true);
    connect(distanceAction, SIGNAL(clicked()), this, SLOT(distance()));

    // undo
    iconname = Settings::findResourceFile( "icons/undo.png" );
    QToolButton *undoAction = new QToolButton();
    undoAction->setIcon(QIcon(iconname.c_str()));
    undoAction->setToolTip(tr("undo"));
    undoAction->setCheckable(false);
    undoAction->setIconSize(QSize(32, 32));
    undoAction->setAutoRaise(true);
    connect(undoAction, SIGNAL(clicked()), this, SLOT(undo()));

    // redo
    iconname = Settings::findResourceFile( "icons/redo.png" );
    QToolButton *redoAction = new QToolButton();
    redoAction->setIcon(QIcon(iconname.c_str()));
    redoAction->setToolTip(tr("redo"));
    redoAction->setCheckable(false);
    redoAction->setIconSize(QSize(32, 32));
    redoAction->setAutoRaise(true);
    connect(redoAction, SIGNAL(clicked()), this, SLOT(redo()));

    //save
    iconname = Settings::findResourceFile( "icons/meshPaint/sauver.png" );
    saveAction = new QToolButton();
    saveAction->setIcon(QIcon(iconname.c_str()));
    saveAction->setToolTip(ControlledWindow::tr("Save texture"));
    saveAction->setIconSize(QSize(32, 32));
    connect(saveAction, SIGNAL(clicked()), this, SLOT(save()));

    tbControls->addWidget(paintBrushAction);
    tbControls->addWidget(colorPickerAction);
    tbControls->addWidget(eraseAction);
    tbControls->addWidget(selectionAction);
    tbControls->addWidget(pathAction);
    tbControls->addWidget(magicBrushAction);
    tbControls->addWidget(validateEditAction);
    tbControls->addWidget(clearPathAction);
    tbControls->addWidget(distanceAction);
    tbControls->addWidget(undoAction);
    tbControls->addWidget(redoAction);
    tbControls->addWidget(saveAction);

    switch( IDActiveControl )
    {
    case 1:
      colorPicker();
      break;
    case 2:
      magicSelection();
      break;
    case 3:
      // subcases (shortestPath etc)
      if( shortestPathSelectedType == "ShortestPath" )
        shortestPath();
      else if( shortestPathSelectedType == "SulciPath" )
        sulciPath();
      else if( shortestPathSelectedType == "GyriPath" )
        gyriPath();
      else
        path();
      break;
    case 4:
      brush();
      break;
    case 5:
      erase();
      break;
    case 6:
      magicBrush();
      break;
    case 7:
      distance();
      break;
    default:
      brush();
      break;
    }
  }
}

void SurfpaintTools::removeToolBarControls(AWindow3D *w3)
{
  // QAWindow  *aw = dynamic_cast<QAWindow *>( v->aWindow() );
  if (!w3)
    return;

  tbControls = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbarControls") );
  delete tbControls;
  tbControls = 0;
  paintBrushAction = 0;
  colorPickerAction = 0;
  eraseAction = 0;
  selectionAction = 0;
  pathAction = 0;
  sulciPathAction = 0;
  gyriPathAction = 0;
  magicBrushAction = 0;
  validateEditAction = 0;
  clearPathAction = 0;
  distanceAction = 0;
  saveAction = 0;
}

void SurfpaintTools::addToolBarInfosTexture(AWindow3D *w3)
{
  //cout << "addToolBar InfosTexture\n";

  if (w3)
  {
    const QPixmap *p;

    tbTextureValue = new QToolBar(w3, ControlledWindow::tr(
        "surfpainttoolbarTex"));

        w3->addToolBar( Qt::BottomToolBarArea,tbTextureValue, ControlledWindow::tr( "surfpainttoolbarTex" ) );
        tbTextureValue->setLabel( ControlledWindow::tr( "surfpainttoolbarTex" ) );
        tbTextureValue->setIconSize( QSize( 20, 20 ) );

    QLabel *SpinBoxLabel = new QLabel(ControlledWindow::tr("TextureValue"),
        tbTextureValue, "TextureValue");
    tbTextureValue->addWidget( SpinBoxLabel );

    textureFloatSpinBox = new QDoubleSpinBox(tbTextureValue);
    tbTextureValue->addWidget( textureFloatSpinBox );
    textureFloatSpinBox->setSingleStep(0.1);
    textureFloatSpinBox->setDecimals(2);
    //textureFloatSpinBox->setFixedHeight(30);
    textureFloatSpinBox->setFixedWidth(100);
    textureFloatSpinBox->setMinimum(-FLT_MAX);
    textureFloatSpinBox->setMaximum(FLT_MAX);
    textureFloatSpinBox->setValue(1.000);

    // si le module ConstraintEditor n'a pas été lancé ou l'intervalle des valeurs de contraintes est réglable min/max ?

    if (!w3->constraintEditorIsActive() || w3->getConstraintType()==1 )
    {
      textureValueMinSpinBox = new QDoubleSpinBox(tbTextureValue);
      tbTextureValue->addWidget( textureValueMinSpinBox );
      textureValueMinSpinBox->setSingleStep(0.1);
      textureValueMinSpinBox->setDecimals(2);
      //textureValueMinSpinBox->setFixedHeight(30);
      textureValueMinSpinBox->setFixedWidth(100);
      textureValueMinSpinBox->setMinimum(-FLT_MAX);
      textureValueMinSpinBox->setMaximum(FLT_MAX);

      connect( textureValueMinSpinBox, SIGNAL( valueChanged( double ) ),
               this, SLOT( changeMinValueSpinBox(double) ) );

      textureValueMaxSpinBox = new QDoubleSpinBox(tbTextureValue);
      tbTextureValue->addWidget( textureValueMaxSpinBox );
      textureValueMaxSpinBox->setSingleStep(0.1);
      textureValueMaxSpinBox->setDecimals(2);
      //textureValueMaxSpinBox->setFixedHeight(30);
      textureValueMaxSpinBox->setFixedWidth(100);
      textureValueMaxSpinBox->setMinimum(-FLT_MAX);
      textureValueMaxSpinBox->setMaximum(FLT_MAX);
      textureValueMaxSpinBox->setValue( 100 ); // arbitrary

      connect( textureValueMaxSpinBox, SIGNAL( valueChanged( double ) ),
               this, SLOT( changeMaxValueSpinBox(double) ) );

      if( at )
      {
        int tn = 0; // 1st texture
        GLComponent::TexExtrema & te = at->glTexExtrema(tn);
        int tx = 0; // 1st tex coord
        if( te.maxquant[tx] != te.minquant[tx] )
        {
          textureValueMinSpinBox->setValue( te.minquant[tx] );
          textureValueMaxSpinBox->setValue( te.maxquant[tx] );
        }
      }
    }

    // ARN on affiche la liste des contraintes seulement si le module
    // ConstraintEditor a été lancé en mode lat/lon?
    if (w3->constraintEditorIsActive() &&  w3->getConstraintType()==0)
    {
      //textureFloatSpinBox->setReadOnly(true);
      constraintList = new QComboBox(tbTextureValue);
      tbTextureValue->addWidget( constraintList );

      if (!w3->getConstraintList().empty())
        loadConstraintsList(w3->getConstraintList());

      connect( constraintList, SIGNAL( activated( int ) ),
               this, SLOT( updateConstraintList() ) );

      updateConstraintList();
    }

    tbTextureValue->show();

    tbInfos3D = new QToolBar( w3,ControlledWindow::tr( "surfpainttoolbar3D") );
      w3->addToolBar( Qt::BottomToolBarArea,tbInfos3D,
                      ControlledWindow::tr( "surfpainttoolbar3D" ) );
      tbInfos3D->setLabel( ControlledWindow::tr( "surfpainttoolbar3D" ) );
      tbInfos3D->setIconSize( QSize( 20, 20 ) );

    QLabel *IDPolygonSpinBoxLabel
      = new QLabel(ControlledWindow::tr("IDPolygon"), tbInfos3D);
    tbInfos3D->addWidget( IDPolygonSpinBoxLabel );

    IDPolygonSpinBox = new QSpinBox(tbInfos3D);
    tbInfos3D->addWidget( IDPolygonSpinBox );
    IDPolygonSpinBox->setSingleStep(1);
    //IDPolygonSpinBox->setFixedHeight(30);
    IDPolygonSpinBox->setFixedWidth(75);
    IDPolygonSpinBox->setValue(0);

    QLabel *IDVertexSpinBoxLabel
      = new QLabel(ControlledWindow::tr("IDVertex"),tbInfos3D);
    tbInfos3D->addWidget( IDVertexSpinBoxLabel );

    IDVertexSpinBox = new QSpinBox(tbInfos3D);
    tbInfos3D->addWidget( IDVertexSpinBox );
    IDVertexSpinBox->setSingleStep(1);
    //IDVertexSpinBox->setFixedHeight(30);
    IDVertexSpinBox->setFixedWidth(75);
    IDVertexSpinBox->setValue(0);

    toleranceSpinBoxLabel = new QLabel(tr("tolerance"),tbInfos3D);
    tbInfos3D->addWidget( toleranceSpinBoxLabel );
    toleranceSpinBox = new QSpinBox (tbInfos3D);
    tbInfos3D->addWidget( toleranceSpinBox );
    toleranceSpinBox->setSingleStep(1);
    //toleranceSpinBox->setFixedHeight(30);
    toleranceSpinBox->setFixedWidth(55);
    toleranceSpinBox->setValue(0);
    toleranceSpinBox->setRange(0,100);

    connect( toleranceSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeToleranceSpinBox(int) ) );

    constraintPathSpinBoxLabel
      = new QLabel(ControlledWindow::tr("constraint"), tbInfos3D);
    tbInfos3D->addWidget( constraintPathSpinBoxLabel );
    constraintPathSpinBox = new QSpinBox(tbInfos3D);
    tbInfos3D->addWidget( constraintPathSpinBox );
    constraintPathSpinBox->setSingleStep(1);
    //constraintPathSpinBox->setFixedHeight(30);
    constraintPathSpinBox->setFixedWidth(55);
    constraintPathSpinBox->setValue(3);
    constraintPathSpinBox->setRange(0,100);

    connect( constraintPathSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeConstraintPathSpinBox(int) ) );

    tbInfos3D->show();
  }
}

void SurfpaintTools::removeToolBarInfosTexture(AWindow3D *w3)
{
  // QAWindow  *aw = dynamic_cast<QAWindow *>( v->aWindow() );
  if (!w3)
    return;

    tbTextureValue = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbarTex") );
    tbInfos3D = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbar3D") );
    delete tbTextureValue;
    delete tbInfos3D;
}

void SurfpaintTools::restoreTextureValue(int indexVertex)
{
  float value;

  if (surfpaintTexInit)
  {
    value = surfpaintTexInit[0].item(indexVertex);
    updateTextureValue (indexVertex,value);
  }
}


void SurfpaintTools::updateTextureValue(int indexVertex, float value)
{
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect)
      && objtype == "TEXTURED SURF.")
  {
    float it = at->TimeStep();
    int tn = 0; // 1st texture
    GLComponent::TexExtrema & te = at->glTexExtrema(tn);
    int tx = 0; // 1st tex coord

    ViewState vs;
    GLfloat* texbuf = const_cast<GLfloat *>( at->glTexCoordArray( vs, tn ) );
    // hum, don't look at this const_cast...

    float scl = (te.maxquant[tx] - te.minquant[tx]);
    if( scl == 0 )
      scl = 1.;

    if( value < te.minquant[tx] || value > te.maxquant[tx] )
    {
      // extrema will need to change
      // value in internally rescaled texture
      float rval = ( value - te.minquant[tx] ) / scl;
      /* the whole tex needs rescaling because they need to fit in [0-1]
          for OpenGL */
      float offs = 0.;
      float nscl = 1. / ( std::max( rval, te.max[tx] )
        - std::min( rval, te.min[tx] ) );
      if( rval < 0 )
        offs = - rval * nscl;
      unsigned i, n = at->glTexCoordSize( vs, tn );
      GLfloat *tb = texbuf;
      for( unsigned i=0; i<n; ++i, ++tb )
        *tb = *tb * nscl + offs;
      // update internal extrema
      te.min[tx] = std::min( rval, te.min[tx] ) * nscl + offs;
      te.max[tx] = std::max( rval, te.max[tx] ) * nscl + offs;

      // update scaled extrema
      if( value < te.minquant[tx] )
        te.minquant[tx] = value;
      else
      {
        // update colormap bounds
        at->getOrCreatePalette();
        AObjectPalette *pal = at->palette();
        float cscale = 1.;
        if( textureValueMaxSpinBox )
          cscale = ( textureValueMaxSpinBox->value() - te.minquant[tx] )
            / ( value - te.minquant[tx] );
        else
        {
          cscale = 1.;
          float cmax = 0;
          for( int i=0; i<constraintList->count(); ++i )
          {
            string constraintLabel = string(constraintList->itemText(i));
            size_t position = constraintLabel.find_last_of(' ');
            if( position != string::npos )
            {
              std::istringstream strin(constraintLabel.substr(position + 1));
              int mvalue;
              strin >> mvalue;
              if( mvalue >= cmax )
                cmax = mvalue;
            }
          }
          if( cmax > 0 )
            cscale = ( cmax - te.minquant[tx] ) / ( value - te.minquant[tx] );
        }
        pal->setMax1( cscale );
        at->setPalette( *pal );

        te.maxquant[tx] = value;
      }
      scl = te.maxquant[tx] - te.minquant[tx];
    }

    float svalue;
    float oldTextureValue;

    svalue = ( value - te.minquant[tx] ) / scl;

    if( svalue < te.min[tx] )
      te.min[tx] = svalue;
    else if( svalue > te.max[tx] )
      te.max[tx] = svalue;

    oldTextureValue = texbuf[ indexVertex ] * scl + te.minquant[0];
    texbuf[ indexVertex ] = svalue;

    at->glSetChanged( GLComponent::glBODY );
    at->setChanged();
    at->setInternalsChanged();
    at->notifyObservers(this);


    if (IDActiveControl == 3 || IDActiveControl == 6)
      listIndexVertexBrushPath.push_back(indexVertex);

    if( d->recorded_modifs.back().find( indexVertex )
        == d->recorded_modifs.back().end() )
      d->recorded_modifs.back()[ indexVertex ] = oldTextureValue;
  }
}

void SurfpaintTools::updateTexture (vector<float> values)
{
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect))
  {
    if (objtype == "TEXTURED SURF.")
    {
      float it = at->TimeStep();
      int tn = 0; // 1st texture
      GLComponent::TexExtrema & te = at->glTexExtrema(tn);
      int tx = 0; // 1st tex coord

      ViewState vs;
      GLfloat* texbuf = const_cast<GLfloat *>( at->glTexCoordArray( vs, tn ) );
      // hum, don't look at this const_cast...

      float scl;

      float value,minv,maxv;

      vector<float>::iterator itemin = min_element (values.begin(), values.end());
      vector<float>::iterator itemax = max_element (values.begin(), values.end());

      minv = *itemin;
      maxv = *itemax;

      if (maxv > te.maxquant[tx] )
      {
        at->getOrCreatePalette();
        AObjectPalette *pal = at->palette();
        //pal->setMax1( maxv );
        at->setPalette( *pal );
        te.maxquant[tx] = maxv;
      }

      scl = (te.maxquant[tx] - te.minquant[tx]);

      float svalue;

      vector<float>::iterator ite = values.begin();
      int i = 0;

      for (; ite != values.end(); ite++)
      {
        svalue = ( *ite - te.minquant[tx] ) / scl;

        texbuf[ i ] = svalue;
      }

      at->glSetChanged( GLComponent::glBODY );
      at->setChanged();
      at->setInternalsChanged();
      at->notifyObservers(this);
    }
  }
}


float SurfpaintTools::currentTextureValue( unsigned vertexIndex ) const
{
  GLComponent::TexExtrema & te = at->glTexExtrema(0);
  float value = at->textureCoords()[ vertexIndex ] + te.minquant[0];
  if( te.minquant[0] != te.maxquant[0] )
    value *= te.maxquant[0] - te.minquant[0];
  return value;
}


void SurfpaintTools::floodFillStart(int indexVertex)
{
  listIndexVertexFill.clear();

  float texvalue = getTextureValueFloat();

  fastFillMove( indexVertex, texvalue, currentTextureValue( indexVertex ) );
}

void SurfpaintTools::floodFillStop(void)
{
  AimsSurface<3,Void>   & surf= (*mesh)[0];
  vector<Point3df>    & vert = surf.vertex();

  AimsSurfaceTriangle *MeshOut,*tmpMeshOut;
  MeshOut = new AimsSurfaceTriangle;

  std::set<int>::iterator ite = listIndexVertexFill.begin();

  for (; ite != listIndexVertexFill.end(); ite++)
  {
    //Point3df pt;
    tmpMeshOut = SurfaceGenerator::sphere(vert[*ite], 0.25, 50);

    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;
  }

  const AObjectPalette *pal = at->getOrCreatePalette();

  const AimsData<AimsRGBA> *col = pal->colors();

  unsigned  ncol0, ncol1;
  float   min1, max1;
  float   min2, max2;

  ncol0 = col->dimX();
  ncol1 = col->dimY();
  min1 = pal->min1();
  max1 = pal->max1();

  AimsRGBA empty;

  int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
  if( ind < 0 )
    ind = 0;
  else if( ind >= ncol0 )
    ind = ncol0 - 1;
  empty = (*col)( ind );

  Material mat2;
  mat2.setRenderProperty(Material::Ghost, 1);
  mat2.setRenderProperty(Material::RenderLighting, 1);
  mat2.SetDiffuse((float) empty.red() / 255, (float) empty.green() / 255,
      (float) empty.blue() / 255, 1.);

  ATriangulated *fillObjectTemp = new ATriangulated();
  fillObjectTemp->setName(theAnatomist->makeObjectName("fill"));
  fillObjectTemp->setSurface(MeshOut);
  fillObjectTemp->SetMaterial(mat2);
  fillObjectTemp->setReferentialInheritance( objselect );
  win3D->registerObject(fillObjectTemp, true, 1);
  theAnatomist->registerObject(fillObjectTemp, 0);
  fillObject.push_back(rc_ptr<ATriangulated>( fillObjectTemp ));
  theAnatomist->releaseObject( fillObjectTemp );
}


void SurfpaintTools::fastFillMove(int indexVertex, float newTextureValue,
    float oldTextureValue)
{
  /* Adds n items in listIndexVertexFill
  */

  std::queue<int> stack;
  stack.push(indexVertex);

  int indexCurr;
  GLComponent::TexExtrema & te = at->glTexExtrema(0);
  float maxq = te.maxquant[0], minq = te.minquant[0], scl = 1.;
  if( maxq != minq )
    scl = maxq - minq;
  float currentValue;
  const float* texdata = at->textureCoords();

  while (!stack.empty())
  {
    indexCurr = stack.front();
    stack.pop();

    std::set<uint> voisins = neighbours[indexCurr];
    std::set<uint>::iterator voisIt = voisins.begin();

    //on parcourt tous les voisins du sommet
    for (; voisIt != voisins.end(); voisIt++)
    {
      indexCurr = *voisIt;

      set<int>::const_iterator itef;
      itef = listIndexVertexFill.find(indexCurr);

      if (itef != listIndexVertexFill.end())
        continue;

      //on n'empile pas si le sommet appartient à un chemin en cours de tracé
      std::vector<unsigned>::iterator s1 = listIndexVertexPathSP.begin();
      std::vector<unsigned>::iterator s2 = listIndexVertexPathSP.end();
      std::vector<unsigned>::iterator ite = std::find(s1, s2, indexCurr);

      if (ite != listIndexVertexPathSP.end())
        continue;

      currentValue = minq + texdata[ indexCurr ] * scl;

      if( currentValue > (oldTextureValue + stepToleranceValue + 0.001 )
          || currentValue < (oldTextureValue - stepToleranceValue - 0.001 ) )
        continue;
      else
      {
        listIndexVertexFill.insert(indexCurr);
        stack.push(indexCurr);
      }
    }
  }
}


void SurfpaintTools::updateConstraintList(void)
{
  int item = constraintList->currentItem();
  int value = 1;
  if( item >= 0 )
  {
    string constraintLabel = string(constraintList->currentText());

    // cout << constraintLabel << " value " << item << endl;
    int position = constraintLabel.find_last_of(' ');

    if( position != string::npos )
    {
      // cout << "contrainte : " << constraintLabel.substr(0,position) << endl;
      std::istringstream strin(constraintLabel.substr(position + 1));
      strin >> value;
    }
  }
  setTextureValueFloat((float) value);
}

void SurfpaintTools::changeToleranceSpinBox(int v)
{
  toleranceValue = v;

  float it = at->TimeStep();
  int tn = 0; // 1st texture
  GLComponent::TexExtrema & te = at->glTexExtrema(tn);
  int tx = 0; // 1st tex coord
  float scl = (te.maxquant[tx] - te.minquant[tx]);

  stepToleranceValue = toleranceValue * (float)(scl)/100.;

  // cout << "stepToleranceValue " << stepToleranceValue << endl;
}

void SurfpaintTools::loadConstraintsList(vector<string> clist)
{
  char sep = carto::FileUtil::separator();

  vector<string>::iterator it = clist.begin();

  for (; it != clist.end(); ++it)
    {
    if ((*it).length() != 0)
    constraintList->addItem((*it).c_str());
    }
}

void SurfpaintTools::changeMinValueSpinBox(double v)
{
  setMinMaxTexture((float) v, (float) textureValueMaxSpinBox->value());

  textureValueMinSpinBox->setValue(v);
}

void SurfpaintTools::changeMaxValueSpinBox(double v)
{
  setMinMaxTexture((float) textureValueMinSpinBox->value(), (float) v);

  textureValueMaxSpinBox->setValue(v);

  if( at )
  {
    AObjectPalette *pal = at->palette();
    int tn = 0; // 1st texture
    GLComponent::TexExtrema & te = at->glTexExtrema(tn);
    int tx = 0; // 1st tex coord
    float cscale = 1.;
    if( te.minquant[tx] != te.maxquant[tx] )
      cscale = ( v - te.minquant[tx] ) / ( te.maxquant[tx] - te.minquant[tx] );
    pal->setMax1( cscale );
    at->setPalette( *pal );
    at->notifyObservers( this );
  }

}

void SurfpaintTools::changeConstraintPathSpinBox(int v)
{
  delete sp_sulci;
  delete sp_gyri;

  sp_sulci = new GeodesicPath(*mesh,texCurv,1,v);
  sp_gyri = new GeodesicPath(*mesh,texCurv,2,v);
}

void SurfpaintTools::fillHolesOnPath (void)
{
  /*
    Adds 1 item in holeVertexIndices
    calls addSimpleShortPath n times
    Adds 1 item in holeCount (n)
  */
  clearUndoneHolesPaths();

  unsigned last = 0, nholes = 0;
  if( !d->holeVertexIndices.empty() )
    last = d->holeVertexIndices.back();
  if( last > 0 )
    --last;
  vector<unsigned>::iterator it = listIndexVertexBrushPath.begin() + last;

  for (; it < listIndexVertexBrushPath.end()-1; ++it, ++last)
  {
    std::set<uint> voisins = neighbours[*it];

    std::set<uint>::iterator ite = std::find( voisins.begin(), voisins.end(),
                                              *(it+1) );

    if( ite == voisins.end() && ( (*it) != *(it+1) ) )
    {
      addSimpleShortPath( *it, *(it+1) );
      ++nholes;
    }
  }
  d->holeVertexIndices.push_back( listIndexVertexBrushPath.size() );
  d->holeCount.push_back( nholes ); // nholes paths added this step
}

void SurfpaintTools::addGeodesicPath(int indexNearestVertex,
    Point3df positionNearestVertex)
{
  /*
    Adds 1-2 AObject in pathObject: 1 if last (pathIsClosed() true)
    Adds 1 item in listIndexVertexSelectSP
    Adds a list: n in listIndexVertexPathSP if not 1st point
    Adds 1 item in numVerticesInPathElements: n if not 1st point
  */
  clearUndoneGeodesicPath();

  int i;

  AimsSurfaceTriangle *tmpMeshOut;
  tmpMeshOut = new AimsSurfaceTriangle;

  Material mat;
  mat.setRenderProperty(Material::Ghost, 1);
  //mat.setRenderProperty(Material::RenderLighting, 1);
  mat.SetDiffuse(1., 1.0, 1.0, 0.5);

  tmpMeshOut = SurfaceGenerator::sphere(positionNearestVertex, 0.50, 50);

  ATriangulated *sp = new ATriangulated();
  sp->setName(theAnatomist->makeObjectName("select"));
  sp->setSurface(tmpMeshOut);
  sp->SetMaterial(mat);
  sp->setReferentialInheritance( objselect );

  if (!pathIsClosed())
  {
    win3D->registerObject(sp, true, 1);
    theAnatomist->registerObject(sp, 0);
    pathObject.push_back(rc_ptr<ATriangulated>( sp ) );
    theAnatomist->releaseObject( sp );
  }

  std::vector<unsigned>::iterator ite;

  if (!pathIsClosed())
    listIndexVertexSelectSP.push_back(indexNearestVertex);
  else
    listIndexVertexSelectSP.push_back(*listIndexVertexSelectSP.begin());

  ite = listIndexVertexSelectSP.end();

  int nb_vertex;
  printf("nb vertex path = %lu\n", listIndexVertexSelectSP.size());

  const string ac = getPathType();

  GeodesicPath *sptemp;

  if (ac.compare("ShortestPath") == 0)
    sptemp = getMeshStructSP();
  else if (ac.compare("SulciPath") == 0)
    sptemp = getMeshStructSulciP();
  else if (ac.compare("GyriPath") == 0)
    sptemp = getMeshStructGyriP();

  if (listIndexVertexSelectSP.size() >= 2)
  {

    std::vector<Point3df> vertexList;

    unsigned target_vertex_index = (*(--ite));
    unsigned source_vertex_index = (*(--ite));

    std::vector<unsigned> listIndexVertexPathSPLast;

    sptemp->shortestPath_1_1_ind_xyz(
      source_vertex_index,target_vertex_index,
      listIndexVertexPathSPLast,vertexList );

    listIndexVertexPathSP.insert(listIndexVertexPathSP.end(),
        listIndexVertexPathSPLast.begin(), listIndexVertexPathSPLast.end());
    d->numVerticesInPathElements.push_back( listIndexVertexPathSPLast.size() );

    AimsSurfaceTriangle *MeshOut = new AimsSurfaceTriangle;

    for (i = 0; i < vertexList.size() - 1; ++i)
    {

      tmpMeshOut = SurfaceGenerator::sphere(vertexList[i], 0.25, 50);
      SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
      delete tmpMeshOut;

      tmpMeshOut = SurfaceGenerator::cylinder(vertexList[i], vertexList[i + 1],
          0.1, 0.1, 12, false, true);
      SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
      delete tmpMeshOut;
    }

    tmpMeshOut = SurfaceGenerator::sphere(vertexList[i], 0.2, 10);
    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;

    const AObjectPalette *pal = at->getOrCreatePalette();

    const AimsData<AimsRGBA> *col = pal->colors();

    unsigned  ncol0, ncol1;
    float   min1, max1;
    float   min2, max2;

    ncol0 = col->dimX();
    ncol1 = col->dimY();
    min1 = pal->min1();
    max1 = pal->max1();

    AimsRGBA empty;

    int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
    if( ind < 0 )
      ind = 0;
    else if( ind >= ncol0 )
      ind = ncol0 - 1;
    empty = (*col)( ind );

    Material mat2;
    mat2.setRenderProperty(Material::Ghost, 1);
    mat2.setRenderProperty(Material::RenderLighting, 1);
    mat2.SetDiffuse((float) (empty.red() / 255.),
                    (float) (empty.green() / 255.),
                    (float) (empty.blue() / 255.), 1.);

    ATriangulated *s3 = new ATriangulated();
    s3->setName(theAnatomist->makeObjectName("path"));
    s3->setSurface(MeshOut);
    s3->SetMaterial(mat2);
    s3->setReferentialInheritance( objselect );

    s3->setPalette( *pal );

    win3D->registerObject(s3, true, 1);
    theAnatomist->registerObject(s3, 0);
    pathObject.push_back( rc_ptr<ATriangulated>( s3 ) );
    theAnatomist->releaseObject( s3 );
  }

}


void SurfpaintTools::clearUndoneGeodesicPath()
{
  d->undone_listIndexVertexPathSP.clear();
  d->undone_listIndexVertexSelectSP.clear();
  d->undone_numVerticesInPathElements.clear();
  d->undone_pathObject.clear();
}


void SurfpaintTools::undoGeodesicPath()
{
  /*
    Removes 1-2 AObject in pathObject: 1 if last (pathIsClosed() true)
    Removes 1 item in listIndexVertexSelectSP
    Removes 1 item in numVerticesInPathElements: n if not 1st point
    Removes a list: n in listIndexVertexPathSP if not 1st point

    all trasfered to undone_* correspondig lists
  */

  if( pathObject.empty() )
    return; // nothing tio undo
  rc_ptr<ATriangulated> obj = pathObject.back();
  d->undone_pathObject.push_front( obj );
  getWindow3D()->unregisterObject( obj.get() );
  pathObject.pop_back();
  if( pathIsClosed() )
    setClosePath( false );
  else if( !pathObject.empty() )
  {
    obj = pathObject.back();
    d->undone_pathObject.push_front( obj );
    getWindow3D()->unregisterObject( obj.get() );
    pathObject.pop_back();
  }
  d->undone_listIndexVertexSelectSP.push_front(
    listIndexVertexSelectSP.back() );
  listIndexVertexSelectSP.pop_back();
  if( !d->numVerticesInPathElements.empty() )
  {
    unsigned i, n = d->numVerticesInPathElements.back();
    d->numVerticesInPathElements.pop_back();
    d->undone_numVerticesInPathElements.push_front( n );
    for( i=0; i!=n; ++i )
    {
      d->undone_listIndexVertexPathSP.push_front(
        listIndexVertexPathSP.back() );
      listIndexVertexPathSP.pop_back();
    }
  }

  getWindow3D()->refreshNow();
}


void SurfpaintTools::redoGeodesicPath()
{
  /*
    Adds 1-2 AObject in pathObject
    Adds 1 item in listIndexVertexSelectSP
    Adds a list: n in listIndexVertexPathSP if not 1st point
    Adds 1 item in numVerticesInPathElements: n if not 1st point
    all removed from undone_* correspondig lists
  */

  if( d->undone_pathObject.empty() )
    return; // nothing to redo
  rc_ptr<ATriangulated> obj = d->undone_pathObject.front();
  pathObject.push_back( obj );
  getWindow3D()->registerObject( obj.get(), true, 1 );
  d->undone_pathObject.pop_front();
  if( d->undone_pathObject.empty() && pathObject.size() > 1 )
    setClosePath( true );
  else if( pathObject.size() > 1 )
  {
    obj = d->undone_pathObject.front();
    pathObject.push_back( obj );
    getWindow3D()->registerObject( obj.get(), true, 1 );
    d->undone_pathObject.pop_front();
  }
  listIndexVertexSelectSP.push_back(
    d->undone_listIndexVertexSelectSP.front() );
  d->undone_listIndexVertexSelectSP.pop_front();
  if( !d->undone_numVerticesInPathElements.empty() )
  {
    unsigned i, n = d->undone_numVerticesInPathElements.front();
    d->undone_numVerticesInPathElements.pop_front();
    d->numVerticesInPathElements.push_back( n );
    for( i=0; i!=n; ++i )
    {
      listIndexVertexPathSP.push_back(
        d->undone_listIndexVertexPathSP.front() );
      d->undone_listIndexVertexPathSP.pop_front();
    }
  }

  getWindow3D()->refreshNow();
}


void SurfpaintTools::addSimpleShortPath(int indexSource,int indexDest)
{
  /* adds n in listIndexVertexHolesPath
     adds 1 item (n) in numIndexVertexHolesPath
     adds 1 item in holesObject
  */
  int i;

  AimsSurfaceTriangle *tmpMeshOut;
  tmpMeshOut = new AimsSurfaceTriangle;

  Material mat;
  mat.setRenderProperty(Material::Ghost, 1);
  mat.SetDiffuse(1., 1.0, 1.0, 0.5);

  unsigned target_vertex_index = indexSource;
  unsigned source_vertex_index = indexDest;

  vector<Point3df> vertexList;
  vector<unsigned> indexList;

  sp->shortestPath_1_1_ind_xyz (source_vertex_index, target_vertex_index,
                                indexList, vertexList );
  copy( indexList.begin(), indexList.end(),
        std::back_inserter(listIndexVertexHolesPath) );
  d->numIndexVertexHolesPath.push_back( indexList.size() );

  AimsSurfaceTriangle *MeshOut = new AimsSurfaceTriangle;

  for (i = 0; i < vertexList.size() - 1; ++i)
  {
    tmpMeshOut = SurfaceGenerator::sphere(vertexList[i], 0.25, 50);
    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;

    tmpMeshOut = SurfaceGenerator::cylinder(vertexList[i], vertexList[i + 1],
        0.1, 0.1, 12, false, true);
    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;
  }

  tmpMeshOut = SurfaceGenerator::sphere(vertexList[i], 0.2, 10);
  SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
  delete tmpMeshOut;

  const AObjectPalette *pal = at->getOrCreatePalette();

  const AimsData<AimsRGBA> *col = pal->colors();

  unsigned  ncol0, ncol1;
  float   min1, max1;
  float   min2, max2;

  ncol0 = col->dimX();
  ncol1 = col->dimY();
  min1 = pal->min1();
  max1 = pal->max1();

  AimsRGBA empty;

  int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
  if( ind < 0 )
    ind = 0;
  else if( ind >= ncol0 )
    ind = ncol0 - 1;
  empty = (*col)( ind );

  Material mat2;
  mat2.setRenderProperty(Material::Ghost, 1);
  mat2.setRenderProperty(Material::RenderLighting, 1);
  mat2.SetDiffuse((float) (empty.red() / 255.), (float) (empty.green() / 255.),
      (float) (empty.blue() / 255.), 1.);

  ATriangulated *s3 = new ATriangulated();
  s3->setName(theAnatomist->makeObjectName("path"));
  s3->setSurface(MeshOut);
  s3->SetMaterial(mat2);
  s3->setPalette( *pal );
  s3->setReferentialInheritance( objselect );

  win3D->registerObject(s3, true, 1);
  theAnatomist->registerObject(s3, 0);
  holesObject.push_back( rc_ptr<ATriangulated>( s3 ) );
  theAnatomist->releaseObject( s3 );
}


void SurfpaintTools::clearUndoneHolesPaths()
{
  d->undone_listIndexVertexHolesPath.clear();
  d->undone_numIndexVertexHolesPath.clear();
  d->undone_holesObject.clear();
  d->undone_holeVertexIndices.clear();
  d->undone_holeCount.clear();
}


void SurfpaintTools::undoSimpleShortPath()
{
  if( holesObject.empty() )
    return;
  rc_ptr<ATriangulated> obj = holesObject.back();
  getWindow3D()->unregisterObject( obj.get() );
  d->undone_holesObject.push_front( obj );
  holesObject.pop_back();
  unsigned i, n = d->numIndexVertexHolesPath.back();
  d->undone_numIndexVertexHolesPath.push_front( n );
  d->numIndexVertexHolesPath.pop_back();
  for( i=0; i<n; ++i )
  {
    d->undone_listIndexVertexHolesPath.push_front(
      listIndexVertexHolesPath.back() );
    listIndexVertexHolesPath.pop_back();
  }
}


void SurfpaintTools::redoSimpleShortPath()
{
  if( d->undone_holesObject.empty() )
    return;
  rc_ptr<ATriangulated> obj = d->undone_holesObject.front();
  getWindow3D()->registerObject( obj.get(), true, 1 );
  holesObject.push_back( obj );
  d->undone_holesObject.pop_front();
  unsigned i, n = d->undone_numIndexVertexHolesPath.front();
  d->numIndexVertexHolesPath.push_back( n );
  d->undone_numIndexVertexHolesPath.pop_front();
  for( i=0; i<n; ++i )
  {
    listIndexVertexHolesPath.push_back(
      d->undone_listIndexVertexHolesPath.front() );
    d->undone_listIndexVertexHolesPath.pop_front();
  }
}


void SurfpaintTools::undoHolesPaths()
{
  if( d->holeCount.empty() )
    return;
  unsigned i, n = d->holeCount.back();
  d->undone_holeCount.push_front( n );
  d->holeCount.pop_back();
  for( i=0; i<n; ++i )
    undoSimpleShortPath();
  n = d->holeVertexIndices.back();
  d->undone_holeVertexIndices.push_front( n );
  d->holeVertexIndices.pop_back();
  if( !d->holeVertexIndices.empty() )
    n -= d->holeVertexIndices.back();
  for( i=0; i<n; ++i )
  {
    d->undone_listIndexVertexBrushPath.push_front(
      listIndexVertexBrushPath.back() );
    listIndexVertexBrushPath.pop_back();
  }

  getWindow3D()->refreshNow();
}


void SurfpaintTools::redoHolesPaths()
{
  if( d->undone_holeCount.empty() )
    return;
  unsigned i, n = d->undone_holeCount.front(), m = 0;
  d->holeCount.push_back( n );
  d->undone_holeCount.pop_front();
  for( i=0; i<n; ++i )
    redoSimpleShortPath();
  n = d->undone_holeVertexIndices.front();
  if( !d->holeVertexIndices.empty() )
    m = d->holeVertexIndices.back();
  d->holeVertexIndices.push_back( n );
  d->undone_holeVertexIndices.pop_front();
  n -= m;
  for( i=0; i<n; ++i )
  {
    listIndexVertexBrushPath.push_back(
      d->undone_listIndexVertexBrushPath.front() );
    d->undone_listIndexVertexBrushPath.pop_front();
  }

  getWindow3D()->refreshNow();
}


void SurfpaintTools::undoSelectFill()
{
  if( listIndexVertexFill.empty() )
    return;
  d->undone_listIndexVertexFill = listIndexVertexFill;
  listIndexVertexFill.clear();
  rc_ptr<ATriangulated> obj;

  while( !fillObject.empty() )
  {
    obj = fillObject.back();
    getWindow3D()->unregisterObject( obj.get() );
    d->undone_fillObject.push_front( obj );
    fillObject.pop_back();
  }
}


void SurfpaintTools::redoSelectFill()
{
  if( d->undone_listIndexVertexFill.empty() )
    return;
  listIndexVertexFill = d->undone_listIndexVertexFill;
  d->undone_listIndexVertexFill.clear();
  rc_ptr<ATriangulated> obj;

  while( !d->undone_fillObject.empty() )
  {
    obj = d->undone_fillObject.front();
    getWindow3D()->registerObject( obj.get(), true, 1 );
    fillObject.push_back( obj );
    d->undone_fillObject.pop_front();
  }
}


void SurfpaintTools::computeDistanceMap(int indexNearestVertex)
{
  vector<float> distanceMap;
  double length;

  distanceMap.clear();

  const string ac = getPathType();

  GeodesicPath *sptemp;

  if (ac.compare("ShortestPath") == 0)
  sptemp = getMeshStructSP();
  else if (ac.compare("SulciPath") == 0)
  sptemp = getMeshStructSulciP();
  else if (ac.compare("GyriPath") == 0)
  sptemp = getMeshStructGyriP();

 sptemp->distanceMap_1_N_ind(indexNearestVertex, distanceMap,&length, 0);

  updateTexture(distanceMap);
}


void SurfpaintTools::newEditOperation()
{
  clearRedoBuffer();
  if( d->recorded_modifs.empty() || !d->recorded_modifs.back().empty() )
    d->recorded_modifs.push_back( map<unsigned, float>() );
}


void SurfpaintTools::undoTextureOperation()
{
  if( d->recorded_modifs.empty() )
    return;
  d->undone_modifs.push_front( map<unsigned, float>() );
  map<unsigned, float> & redobuf = d->undone_modifs.front();
  map<unsigned, float>::iterator ip, ep = d->recorded_modifs.back().end();
  GLComponent::TexExtrema & te = at->glTexExtrema(0);
  float *buffer = const_cast<float *>( at->textureCoords() );
  float minq = te.minquant[0], maxq = te.maxquant[0];
  float scl = 1., uscl = 1.;
  if( minq != maxq )
  {
    uscl = maxq - minq;
    scl = 1. / uscl;
  }

  for( ip=d->recorded_modifs.back().begin(); ip!=ep; ++ip )
  {
    redobuf[ ip->first ] = buffer[ ip->first ] * uscl + minq;
    buffer[ ip->first ] = ( ip->second - minq ) * scl;
  }

  d->recorded_modifs.pop_back();
  at->glSetChanged( GLComponent::glBODY );
  at->setChanged();
  at->setInternalsChanged();
  at->notifyObservers(this);
}


void SurfpaintTools::redoTextureOperation()
{
  if( d->undone_modifs.empty() )
    return;
  d->recorded_modifs.push_back( map<unsigned, float>() );
  map<unsigned, float> & undobuf = d->recorded_modifs.back();
  map<unsigned, float>::iterator ip, ep = d->undone_modifs.front().end();
  GLComponent::TexExtrema & te = at->glTexExtrema(0);
  float *buffer = const_cast<float *>( at->textureCoords() );
  float minq = te.minquant[0], maxq = te.maxquant[0];
  float scl = 1., uscl = 1.;
  if( minq != maxq )
  {
    uscl = maxq - minq;
    scl = 1. / uscl;
  }

  for( ip=d->undone_modifs.front().begin(); ip!=ep; ++ip )
  {
    undobuf[ ip->first ] = buffer[ ip->first ] * uscl + minq;
    buffer[ ip->first ] = ( ip->second - minq ) * scl;
  }

  d->undone_modifs.pop_front();
  at->glSetChanged( GLComponent::glBODY );
  at->setChanged();
  at->setInternalsChanged();
  at->notifyObservers(this);
}


void SurfpaintTools::clearRedoBuffer()
{
  d->undone_modifs.clear();
}


void SurfpaintTools::undo()
{
  if( !listIndexVertexSelectSP.empty() )
    undoGeodesicPath();
  else if( !listIndexVertexFill.empty() )
    undoSelectFill();
  else if( magicBrushStarted() )
    undoHolesPaths();
  else
    undoTextureOperation();
}


void SurfpaintTools::redo()
{
  if( !d->undone_modifs.empty() )
    redoTextureOperation();
  else if( !d->undone_listIndexVertexBrushPath.empty()
      || !d->undone_listIndexVertexHolesPath.empty()
      || !d->undone_holesObject.empty() )
    redoHolesPaths();
  else if( !d->undone_listIndexVertexFill.empty() )
    redoSelectFill();
  else // if( !d->undone_listIndexVertexSelectSP.empty() )
    redoGeodesicPath();
}



