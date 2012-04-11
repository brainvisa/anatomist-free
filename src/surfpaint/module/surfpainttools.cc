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

SurfpaintTools* & SurfpaintTools::my_instance()
{
  static SurfpaintTools* instance = 0;
  return instance;
}

SurfpaintTools* SurfpaintTools::instance()
{
  if (my_instance() == 0)
    {
    my_instance() = new SurfpaintTools;
    }
  return my_instance();
}

SurfpaintTools::SurfpaintTools()/* : Observer()*/
    : QWidget(theAnatomist->getQWidgetAncestor(), Qt::Window),
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
  fillAction( 0 ),
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
  IDActiveControl( -1 ),
  pathClosed( false ),
  go(0),
  as(0),
  mesh(0),
  tex(0),
  at(0),
  options(0)
{
  changeControl(0);
  shortestPathSelectedType = "ShortestPath";
}

SurfpaintTools::~SurfpaintTools()
{
  delete my_instance();
  delete sp;
  delete sp_sulci;
  delete sp_gyri;
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

void SurfpaintTools::magicbrush()
{
  popAllButtonPaintToolBar();
  magicBrushAction->setChecked(true);
  changeControl(6);
}

void SurfpaintTools::fill()
{
  int i;

  float texvalue = getTextureValueFloat();

  //remplissage de région fermée
  if (IDActiveControl == 2)
  {
    std::set<int>::iterator ite = listIndexVertexFill.begin();
    for (; ite != listIndexVertexFill.end(); ite++)
    {
      updateTextureValue(*ite, texvalue);
      listVertexChanged[*ite] = texvalue;
    }

    clearRegion();
  }

  //remplissage path
  if (IDActiveControl == 3)
  {
    for (unsigned i = 0; i < listIndexVertexPathSP.size(); i++)
    {
      updateTextureValue(listIndexVertexPathSP[i], texvalue);
      listVertexChanged[listIndexVertexPathSP[i]] = texvalue;
    }

    clearPath();
  }

  //remplissage trous
  if (IDActiveControl == 6)
  {
    for (unsigned i = 0; i < listIndexVertexHolesPath.size(); i++)
    {
      updateTextureValue(listIndexVertexHolesPath[i], texvalue);
      listVertexChanged[listIndexVertexHolesPath[i]] = texvalue;
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
  listIndexVertexSelectSP.clear();
  listIndexVertexBrushPath.clear();
  listIndexVertexPathSP.clear();
  listIndexVertexPathSPLast.clear();
  pathSP.clear();

  std::vector<ATriangulated*>::iterator ite;
  ite = pathObject.begin();

  for (; ite != pathObject.end(); ++ite)
  {
    win3D->unregisterObject((*ite));
    theAnatomist->unregisterObject((*ite));
  }

  pathObject.clear();
}

void SurfpaintTools::clearHoles()
{
  listIndexVertexBrushPath.clear();
  listIndexVertexHolesPath.clear();

  std::vector<ATriangulated*>::iterator ite;
  ite = holesObject.begin();

  for (; ite != holesObject.end(); ++ite)
  {
    win3D->unregisterObject((*ite));
    theAnatomist->unregisterObject((*ite));
  }

  holesObject.clear();
}

void SurfpaintTools::clearRegion()
{
  listIndexVertexSelectFill.clear();

  std::vector<ATriangulated*>::iterator ite;

  ite = fillObject.begin();

  for (; ite != fillObject.end(); ++ite)
    {
      //cout <<  (*ite) << endl;
      win3D->unregisterObject((*ite));
      theAnatomist->unregisterObject((*ite));
    }

  fillObject.clear();
}

void SurfpaintTools::save()
{
  cout << "save Texture on disk " << endl;

  std::map<int, float>::const_iterator mit(listVertexChanged.begin()),mend(listVertexChanged.end());

  rc_ptr<TimeTexture<float> > out( new TimeTexture<float>(1, at->size()) );

//  rc_ptr<TimeTexture<float> > text ;
//  text = ObjectConverter<TimeTexture<float> >::ana2aims(at, options);

  float it = at->TimeStep();

  int tn = 0; // 1st texture
  GLComponent::TexExtrema & te = at->glTexExtrema(tn);
  int tx = 0; // 1st tex coord
  float scl = (te.maxquant[tx] - te.minquant[tx]);

  for (uint i = 0; i < at->size(); i++)
    {
    (*out).item(i) = surfpaintTexInit[0].item(i);
//    if ((*out).item(i)>0)
//      cout << (*out).item(i) << " " ;
    }
//
//  for (uint i = 0; i < at->size(); i++)
//    {
//    (*out).item(i) = (*text).item(i)*scl;
//    if ((*text).item(i) > 0)
//      cout << (*out).item(i) << " ";
//    }

  for (; mit != mend; ++mit)
    {
    (*out).item(mit->first) = mit->second;
//    if ((*out).item(mit->first) > 0)
//      cout << (*out).item(mit->first) << " ";
    }

  for (uint i = 0; i < at->size(); i++)
    {
    surfpaintTexInit[0].item(i) = (*out).item(i);
    }

//  cout << "scale = " <<  scl << endl;
//  rc_ptr<Texture1d> textemp(new Texture1d);
//  Converter<TimeTexture<float> , Texture1d> c;
//  c.convert(*out, *textemp);
//  at->setTexture(textemp);

  listVertexChanged.clear();

//  set<AObject*> toSave ;
//  toSave.insert( at ) ;

//  string fileName =
//      ObjectActions::specificSaveStatic( ana,
//                 string( ( const char * )(ControlledWindow::tr( "Texture" )+ " (*.tex)" ) ),
//                 string( "Save Texture" ) ) ;
//
//  cout << "fileName " << fileName << endl;

  QString filt = ControlledWindow::tr( "Texture" ) + " (*.tex)" ;
  QString capt = "Save Texture" ;

  QString filename = QFileDialog::getSaveFileName( QString::null, filt, 0, 0, capt );

  if( !filename.isNull() )
  {
    Writer<TimeTexture<float> > wt(filename.latin1());
    wt.write((*out));
  }

  clearAll();
}

bool SurfpaintTools::initSurfPaintModule(AWindow3D *w3)
{
  win3D = w3;

  stepToleranceValue = 0;

  //sélectionne l'objet positionné au milieu de la fenêtre (bof !)
  //QSize s = glw->qglWidget()->size();
  //objselect = w3->objectAtCursorPosition(s.width() / 2, s.height() / 2);
  //cout << objselect << " " << objselect->name() << endl;

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
      QMessageBox::warning(this, ControlledWindow::tr("not texture associated"),
          ControlledWindow::tr("Cannot open surfpaint Toolbox"));
    }

    if (objtype == "TEXTURED SURF.")
    {
      go = dynamic_cast<ATexSurface *> (objselect);

      if( !go )
      {
        cout << "not a ATexSurface\n";
        return false;
      }

      as = static_cast<ATriangulated *> (go->surface());

      tex = go->texture();

      try
      {
        at=dynamic_cast<ATexture *> (go->texture());
      }
      catch (const std::bad_cast& e)
      {
        std::cerr << e.what() << std::endl;
        std::cerr <<  objselect << " is not a Atexture" << std::endl;
      }

      //int t = (int) w3->GetTime();

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
        textureValueMaxSpinBox->setValue(te.maxquant[0]);
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
  //cout << "addToolBar InfosTexture\n";

  if (w3)
  {
    const QPixmap *p;

    tbControls = new QToolBar(w3, ControlledWindow::tr(
        "surfpainttoolbarControls"));

#if QT_VERSION >= 0x040000
    w3->addToolBar( Qt::TopToolBarArea,tbControls, ControlledWindow::tr( "surfpainttoolbarControls") );
    tbControls->setLabel( ControlledWindow::tr( "surfpainttoolbarControls") );
    tbControls->setIconSize( QSize( 20, 20 ) );
#else
    tbControls = new QToolBar(w3, ControlledWindow::tr(
        "surfpainttoolbarControls"));
    tbControls->setLabel(ControlledWindow::tr("surfpainttoolbarControls"));
#endif

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

    //plus court chemin
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
    connect(magicBrushAction, SIGNAL(clicked()), this, SLOT(magicbrush()));

    //validate
    iconname = Settings::findResourceFile( "icons/meshPaint/valide.png" );
    fillAction = new QToolButton();
    fillAction->setIcon(QIcon(iconname.c_str()));
    fillAction->setToolTip(ControlledWindow::tr("Fill area or path selected"));
    fillAction->setIconSize(QSize(32, 32));
    connect(fillAction, SIGNAL(clicked()), this, SLOT(fill()));

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
    tbControls->addWidget(fillAction);
    tbControls->addWidget(clearPathAction);
    tbControls->addWidget(distanceAction);
    tbControls->addWidget(saveAction);
    brush();
  }
}

void SurfpaintTools::removeToolBarControls(AWindow3D *w3)
{
  // QAWindow  *aw = dynamic_cast<QAWindow *>( v->window() );
  if (!w3)
    return;

#if QT_VERSION >= 0x040000
  tbControls = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbarControls") );
  delete tbControls;
#else
  tbControls = dynamic_cast<QToolBar *> (w3->child(ControlledWindow::tr(
      "surfpainttoolbarControls")));
  if (tbControls)
  {
    delete tbControls;
    return;
  }
#endif
}

void SurfpaintTools::addToolBarInfosTexture(AWindow3D *w3)
{
  //cout << "addToolBar InfosTexture\n";

  if (w3)
  {
    const QPixmap *p;

    tbTextureValue = new QToolBar(w3, ControlledWindow::tr(
        "surfpainttoolbarTex"));

    #if QT_VERSION >= 0x040000
        w3->addToolBar( Qt::BottomToolBarArea,tbTextureValue, ControlledWindow::tr( "surfpainttoolbarTex" ) );
        tbTextureValue->setLabel( ControlledWindow::tr( "surfpainttoolbarTex" ) );
        tbTextureValue->setIconSize( QSize( 20, 20 ) );
    #else
        tbTextureValue = new QToolBar(w3, ControlledWindow::tr(
            "surfpainttoolbarTex"));
        tbTextureValue->setLabel(ControlledWindow::tr("surfpainttoolbarTex"));
    #endif

    QHBox *infosTextureValue = new QHBox();

    QLabel *SpinBoxLabel = new QLabel(ControlledWindow::tr("TextureValue"),
        infosTextureValue, "TextureValue");

    textureFloatSpinBox = new QDoubleSpinBox(infosTextureValue);
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
      textureValueMinSpinBox = new QDoubleSpinBox(infosTextureValue);
      textureValueMinSpinBox->setSingleStep(0.1);
      textureValueMinSpinBox->setDecimals(2);
      //textureValueMinSpinBox->setFixedHeight(30);
      textureValueMinSpinBox->setFixedWidth(100);
      textureValueMinSpinBox->setMinimum(-FLT_MAX);
      textureValueMinSpinBox->setMaximum(FLT_MAX);

      connect( textureValueMinSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( changeMinValueSpinBox(double) ) );

      textureValueMaxSpinBox = new QDoubleSpinBox(infosTextureValue);
      textureValueMaxSpinBox->setSingleStep(0.1);
      textureValueMaxSpinBox->setDecimals(2);
      //textureValueMaxSpinBox->setFixedHeight(30);
      textureValueMaxSpinBox->setFixedWidth(100);
      textureValueMaxSpinBox->setMinimum(-FLT_MAX);
      textureValueMaxSpinBox->setMaximum(FLT_MAX);

      connect( textureValueMaxSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( changeMaxValueSpinBox(double) ) );
    }

    // ARN on affiche la liste des contraintes seulement si le module ConstraintEditor a été lancé en mode lat/lon?
    if (w3->constraintEditorIsActive() &&  w3->getConstraintType()==0)
    {
      //textureFloatSpinBox->setReadOnly(true);
      constraintList = new QComboBox(infosTextureValue);

      if (!w3->getConstraintList().empty())
        loadConstraintsList(w3->getConstraintList());

      connect( constraintList, SIGNAL( activated( int ) ), this, SLOT( updateConstraintList() ) );

      updateConstraintList();
    }

    tbTextureValue->addWidget(infosTextureValue);
    tbTextureValue->show();

    tbInfos3D = new QToolBar( w3,ControlledWindow::tr( "surfpainttoolbar3D") );
    #if QT_VERSION >= 0x040000
      w3->addToolBar( Qt::BottomToolBarArea,tbInfos3D, ControlledWindow::tr( "surfpainttoolbar3D" ) );
      tbInfos3D->setLabel( ControlledWindow::tr( "surfpainttoolbar3D" ) );
      tbInfos3D->setIconSize( QSize( 20, 20 ) );
    #else
      tbInfos3D = new QToolBar( w3, ControlledWindow::tr( "surfpainttoolbar3D" ) );
      tbInfos3D->setLabel( ControlledWindow::tr( "surfpainttoolbar3D" ) );
    #endif

    QHBox *infos3D = new QHBox();

    QLabel *IDPolygonSpinBoxLabel = new QLabel(ControlledWindow::tr("IDPolygon"),infos3D);

    IDPolygonSpinBox = new QSpinBox(infos3D);
    IDPolygonSpinBox->setSingleStep(1);
    //IDPolygonSpinBox->setFixedHeight(30);
    IDPolygonSpinBox->setFixedWidth(75);
    IDPolygonSpinBox->setValue(0);

    QLabel *IDVertexSpinBoxLabel = new QLabel(ControlledWindow::tr("IDVertex"),infos3D);

    IDVertexSpinBox = new QSpinBox(infos3D);
    IDVertexSpinBox->setSingleStep(1);
    //IDVertexSpinBox->setFixedHeight(30);
    IDVertexSpinBox->setFixedWidth(75);
    IDVertexSpinBox->setValue(0);

    toleranceSpinBoxLabel = new QLabel(tr("tolerance"),infos3D);
    toleranceSpinBox = new QSpinBox (infos3D);
    toleranceSpinBox->setSingleStep(1);
    //toleranceSpinBox->setFixedHeight(30);
    toleranceSpinBox->setFixedWidth(55);
    toleranceSpinBox->setValue(0);
    toleranceSpinBox->setRange(0,100);

    connect( toleranceSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeToleranceSpinBox(int) ) );

    constraintPathSpinBoxLabel = new QLabel(ControlledWindow::tr("constraint"),infos3D);
    constraintPathSpinBox = new QSpinBox(infos3D);
    constraintPathSpinBox->setSingleStep(1);
    //constraintPathSpinBox->setFixedHeight(30);
    constraintPathSpinBox->setFixedWidth(55);
    constraintPathSpinBox->setValue(3);
    constraintPathSpinBox->setRange(0,100);

    connect( constraintPathSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeConstraintPathSpinBox(int) ) );

    tbInfos3D->addWidget(infos3D);

    tbInfos3D->show();
  }
}

void SurfpaintTools::removeToolBarInfosTexture(AWindow3D *w3)
{
  // QAWindow  *aw = dynamic_cast<QAWindow *>( v->window() );
  if (!w3)
    return;

  #if QT_VERSION >= 0x040000
    tbTextureValue = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbarTex") );
    tbInfos3D = w3->removeToolBar( ControlledWindow::tr("surfpainttoolbar3D") );
    delete tbTextureValue;
    delete tbInfos3D;
  #else
    tbTextureValue
        = dynamic_cast<QToolBar *> (w3->child(ControlledWindow::tr("surfpainttoolbarTex")));
    tbInfos3D = dynamic_cast<QToolBar *> (w3->child(ControlledWindow::tr("surfpainttoolbar3D")));
    if (tbTextureValue)
    {
      delete tbTextureValue;
      return;
    }
    if (tbInfos3D)
    {
      delete tbInfos3D;
      return;
    }

  #endif
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
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect))
  {
    if (objtype == "TEXTURED SURF.")
    {
      Object options = Object::value(Dictionary());
      options->setProperty("scale", 0);

      float it = at->TimeStep();
      int tn = 0; // 1st texture
      GLComponent::TexExtrema & te = at->glTexExtrema(tn);
      int tx = 0; // 1st tex coord

      ViewState vs;
      GLfloat* texbuf = const_cast<GLfloat *>( at->glTexCoordArray( vs, tn ) );
      // hum, don't look at this const_cast...

      float scl = (te.maxquant[tx] - te.minquant[tx]);


      if( value < te.minquant[tx] || value > te.maxquant[tx] )
      {
        // extrema will need to change
        // value in internally rescaled texture
        float rval = value - te.minquant[tx];
        if( scl == 0 )
          scl = 1.;
        else
          rval /= scl;
        if( rval < 0 || rval > 1. )
        {
          /* the whole tex needs rescaling because they need to fit in [0-1]
             for OpenGL */
          float offs = 0.;
          float nscl = 1. / ( std::max( rval, te.max[tx] )
            - std::min( rval, te.min[tx] ) );
          if( rval < 0 )
            offs = - rval * nscl;
          unsigned i, n = at->glTexCoordSize( vs, tn );
          GLfloat *tb = texbuf;
          for( unsigned i=0; i<n; ++i )
            *tb++ = *tb * nscl + offs;
          // update internal extrema
          te.min[tx] = std::min( rval, te.min[tx] ) * nscl + offs;
          te.max[tx] = std::max( rval, te.max[tx] ) * nscl + offs;
        }

        // update scaled extrema
        if( value < te.minquant[tx] )
          te.minquant[tx] = value;
        else if( value > te.maxquant[tx] )
          te.maxquant[tx] = value;
        else
        {
          // update colormap bounds
          tex->getOrCreatePalette();
          AObjectPalette *pal = tex->palette();
          pal->setMax1( scl * pal->max1() / ( value - te.minquant[tx] ) );
          tex->setPalette( *pal );

          te.maxquant[tx] = value;
        }
        scl = te.maxquant[tx] - te.minquant[tx];
      }

      float svalue;

      svalue = ( value - te.minquant[tx] ) / scl;

      if( svalue < te.min[tx] )
        te.min[tx] = svalue;
      else if( svalue > te.max[tx] )
        te.max[tx] = svalue;

      texbuf[ indexVertex ] = svalue;
      listVertexChanged[indexVertex] = value;

      at->glSetChanged( GLComponent::glBODY );
      tex->setChanged();
      tex->setInternalsChanged();
      tex->notifyObservers(this);


      if (IDActiveControl == 3 || IDActiveControl == 6)
        listIndexVertexBrushPath.push_back(indexVertex);
    }
  }
}

void SurfpaintTools::updateTexture (vector<float> values)
{
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect))
  {
    if (objtype == "TEXTURED SURF.")
    {
      Object options = Object::value(Dictionary());
      options->setProperty("scale", 0);

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
      tex->getOrCreatePalette();
      AObjectPalette *pal = tex->palette();
      //pal->setMax1( maxv );
      tex->setPalette( *pal );
      te.maxquant[tx] = maxv;
      }

      scl = (te.maxquant[tx] - te.minquant[tx]);

      //


//      if( minv < te.minquant[tx])
//        value = minv;
//      if (maxv > te.maxquant[tx] )
//        value = maxv;
//
//        // extrema will need to change
//        // value in internally rescaled texture
//        float rval = value - te.minquant[tx];
//        if( scl == 0 )
//          scl = 1.;
//        else
//          rval /= scl;
//        if( rval < 0 || rval > 1. )
//        {
//          /* the whole tex needs rescaling because they need to fit in [0-1]
//             for OpenGL */
//          float offs = 0.;
//          float nscl = 1. / ( std::max( rval, te.max[tx] )
//            - std::min( rval, te.min[tx] ) );
//          if( rval < 0 )
//            offs = - rval * nscl;
//          unsigned i, n = at->glTexCoordSize( vs, tn );
//          GLfloat *tb = texbuf;
//          for( unsigned i=0; i<n; ++i )
//            *tb++ = *tb * nscl + offs;
//          // update internal extrema
//          te.min[tx] = std::min( rval, te.min[tx] ) * nscl + offs;
//          te.max[tx] = std::max( rval, te.max[tx] ) * nscl + offs;
//        }
//
//        // update scaled extrema
//        if( value < te.minquant[tx] )
//          te.minquant[tx] = value;
//        else if( value > te.maxquant[tx] )
//          te.maxquant[tx] = value;
//        else
//        {
//          // update colormap bounds
//          tex->getOrCreatePalette();
//          AObjectPalette *pal = tex->palette();
//          pal->setMax1( scl * pal->max1() / ( value - te.minquant[tx] ) );
//          tex->setPalette( *pal );
//
//          te.maxquant[tx] = value;
//        }
//        scl = te.maxquant[tx] - te.minquant[tx];

      float svalue;

//

      vector<float>::iterator ite = values.begin();
      int i = 0;

      for (; ite != values.end(); ite++)
      {
        svalue = ( *ite - te.minquant[tx] ) / scl;
//        if( svalue < te.min[tx] )
//          te.min[tx] = svalue;
//        else if( svalue > te.max[tx] )
//          te.max[tx] = svalue;

        texbuf[ i ] = svalue;
        listVertexChanged[i++] = *ite;
      }

      at->glSetChanged( GLComponent::glBODY );
      tex->setChanged();
      tex->setInternalsChanged();
      tex->notifyObservers(this);

      //win3D->Refresh();
      //win3D->refreshNow();
      //

//      if (IDActiveControl == 3 || IDActiveControl == 6)
//        listIndexVertexBrushPath.push_back(indexVertex);
    }
  }
}

void SurfpaintTools::floodFillStart(int indexVertex)
{
  listIndexVertexSelectFill.clear();
  listIndexVertexFill.clear();

  std::map<int, float>::iterator itef;
  itef = listVertexChanged.find(indexVertex);

  float texvalue = getTextureValueFloat();

//  if (itef != listVertexChanged.end())
//    floodFillMove (indexVertex, texvalue,itef->second);
//  else
//    floodFillMove (indexVertex, texvalue,surfpaintTexInit[0].item(indexVertex));
//
  if (itef != listVertexChanged.end())
    fastFillMove (indexVertex, texvalue,itef->second);
  else
    fastFillMove (indexVertex, texvalue,surfpaintTexInit[0].item(indexVertex));
}

void SurfpaintTools::floodFillStop(void)
{
  AimsSurface<3,Void>   & surf= (*mesh)[0];
  vector<Point3df>    & vert = surf.vertex();

  AimsSurfaceTriangle *MeshOut,*tmpMeshOut;
  MeshOut = new AimsSurfaceTriangle;

  std::set<int>::iterator ite = listIndexVertexFill.begin();

  for (; ite != listIndexVertexFill.end(); ite++)
  //for (unsigned i = 0; i < listIndexVertexSelectFill.size(); i++)
  //for (unsigned i = 0; i < listIndexVertexFill.size(); i++)
  {
    //Point3df pt;
    tmpMeshOut = SurfaceGenerator::sphere(vert[*ite], 0.25, 50);

    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;
  }

  cout << "vertex number = " << listIndexVertexFill.size() << endl;

  const AObjectPalette *pal = at->getOrCreatePalette();

  const AimsData<AimsRGBA> *col = pal->colors();

  unsigned  ncol0, ncol1;
  float   min1, max1;
  float   min2, max2;

  ncol0 = col->dimX();
  ncol1 = col->dimY();
  min1 = pal->min1();
  max1 = pal->max1();

  cout << "ncol0 = " << ncol0 << " ncol1 = " << ncol1 << endl;
  cout << "min1 = " << min1 << " max1 = " << max1 << endl;

  AimsRGBA empty;

  int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
  if( ind < 0 )
    ind = 0;
  else if( ind >= ncol0 )
    ind = ncol0 - 1;
  empty = (*col)( ind );

  cout << "texture value RGB " << (int) empty.red() << " "
      << (int) empty.green() << " " << (int) empty.blue() << " " << endl;

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
  //win3D->registerObject(fillObject, true, 0);
  win3D->registerObject(fillObjectTemp, true, 1);
  theAnatomist->registerObject(fillObjectTemp, 0);
  //theAnatomist->registerObject(fillObject, 1);
  fillObject.push_back(fillObjectTemp);
}


void SurfpaintTools::fastFillMove(int indexVertex, float newTextureValue,
    float oldTextureValue)
{
  cout << indexVertex << " " << newTextureValue << endl;

  std::queue<int> stack;
  stack.push(indexVertex);

  int indexCurr;

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

      std::map<int, float>::iterator itemap;
      itemap = listVertexChanged.begin();
      itemap = listVertexChanged.find(indexCurr);

      if (itemap != listVertexChanged.end())
      {
        if ((*itemap).second > (oldTextureValue + stepToleranceValue)
            || (*itemap).second < (oldTextureValue - stepToleranceValue))
          continue;
        else
          {
          listIndexVertexFill.insert(indexCurr);
          cout << indexCurr << endl;
          stack.push(indexCurr);
          continue;
          }
      }

      if ((surfpaintTexInit[0].item(indexCurr) <= (oldTextureValue + 0.001 + stepToleranceValue))
          && (surfpaintTexInit[0].item(indexCurr) >= (oldTextureValue - 0.001 - stepToleranceValue)))
      {
      listIndexVertexFill.insert(indexCurr);
      stack.push(indexCurr);
      }
    }
  }
}

void SurfpaintTools::floodFillMove(int indexVertex, float newTextureValue,
    float oldTextureValue)
{
  bool go;
  bool stop;
  bool pip;


  std::set<uint> voisins = neighbours[indexVertex];
  std::set<uint>::iterator voisIt = voisins.begin();

  std::vector<unsigned>::iterator s1 = listIndexVertexPathSP.begin();
  std::vector<unsigned>::iterator s2 = listIndexVertexPathSP.end();
  std::vector<unsigned>::iterator ite = std::find(s1, s2, indexVertex);

  std::vector<unsigned>::iterator f1 = listIndexVertexSelectFill.begin();
  std::vector<unsigned>::iterator f2 = listIndexVertexSelectFill.end();
  std::vector<unsigned>::iterator itef = std::find(f1, f2, indexVertex);

  std::map<int, float>::iterator itemap;

  itemap = listVertexChanged.begin();
  itemap = listVertexChanged.find(indexVertex);

  go = false;
  stop = false;
  pip = false;

  if (itemap != listVertexChanged.end())
  {
    if ((*itemap).second > (oldTextureValue + stepToleranceValue)
        || (*itemap).second < (oldTextureValue - stepToleranceValue))
      stop = true;

    if ((*itemap).second <= (oldTextureValue + stepToleranceValue)
        && (*itemap).second >= (oldTextureValue - stepToleranceValue))
      go = true;
  }

  if ((surfpaintTexInit[0].item(indexVertex) <= (oldTextureValue + stepToleranceValue))
      && (surfpaintTexInit[0].item(indexVertex) >= (oldTextureValue - stepToleranceValue)))
    pip = true;

  //  cout << "i " << indexVertex << " oldTextureValue " << _tex[0].item(indexVertex) << endl <<  " max " <<
  //      oldTextureValue + _stepToleranceValue << "min " << oldTextureValue - _stepToleranceValue<< " stop " << stop << " go " << go << " pip " << pip << endl;

  if ((go || (pip && !stop)) && (ite == listIndexVertexPathSP.end()) && (itef
      == listIndexVertexSelectFill.end()))
  {
    //cout << indexVertex << " " ;

    listIndexVertexSelectFill.push_back(indexVertex);
    voisIt= voisins.begin();
    for (; voisIt != voisins.end(); voisIt++)
      floodFillMove(*voisIt, newTextureValue, oldTextureValue);
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
  vector<unsigned>::iterator it = listIndexVertexBrushPath.begin();

  for (; it < listIndexVertexBrushPath.end()-1; ++it)
  {
    std::set<uint> voisins = neighbours[*it];

    std::set<uint>::iterator ite = std::find(voisins.begin(),voisins.end(), *(it+1) );

    if (ite == voisins.end() && ((*it)!=*(it+1)) )
      addSimpleShortPath(*it,*(it+1));
  }
}

void SurfpaintTools::addGeodesicPath(int indexNearestVertex,
    Point3df positionNearestVertex)
{
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
    //win3D->registerObject(sp, true, 0);
    //theAnatomist->registerObject(sp, 1);
    theAnatomist->registerObject(sp, 0);
    pathObject.push_back(sp);
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

    listIndexVertexPathSPLast.clear();

    sptemp->shortestPath_1_1_ind_xyz(source_vertex_index,target_vertex_index,listIndexVertexPathSPLast,vertexList);

    listIndexVertexPathSP.insert(listIndexVertexPathSP.end(),
        listIndexVertexPathSPLast.begin(), listIndexVertexPathSPLast.end());

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

    cout << "ncol0 = " << ncol0 << " ncol1 = " << ncol1 << endl;
    cout << "min1 = " << min1 << " max1 = " << max1 << endl;

    AimsRGBA empty;

    int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
    if( ind < 0 )
      ind = 0;
    else if( ind >= ncol0 )
      ind = ncol0 - 1;
    empty = (*col)( ind );

    cout << "texture value RGB " << (int) empty.red() << " "
        << (int) empty.green() << " " << (int) empty.blue() << " " << endl;
    Material mat2;
    mat2.setRenderProperty(Material::Ghost, 1);
    mat2.setRenderProperty(Material::RenderLighting, 1);
    mat2.SetDiffuse((float) (empty.red() / 255.), (float) (empty.green() / 255.),
        (float) (empty.blue() / 255.), 1.);

    ATriangulated *s3 = new ATriangulated();
    s3->setName(theAnatomist->makeObjectName("path"));
    s3->setSurface(MeshOut);
    s3->SetMaterial(mat2);
    s3->setReferentialInheritance( objselect );

    s3->setPalette( *pal );

    //win3D->registerObject(s3, true, 0);
    win3D->registerObject(s3, true, 1);
    theAnatomist->registerObject(s3, 0);
    //theAnatomist->registerObject(s3, 1);
    pathObject.push_back(s3);
  }

}

void SurfpaintTools::addSimpleShortPath(int indexSource,int indexDest)
{
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

  //vertexList = sp->shortestPath_1_1_xyz(source_vertex_index,target_vertex_index);
  sp->shortestPath_1_1_ind_xyz(source_vertex_index,target_vertex_index, indexList,vertexList);
  copy(indexList.begin(), indexList.end(), std::back_inserter(listIndexVertexHolesPath));

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

  cout << "ncol0 = " << ncol0 << " ncol1 = " << ncol1 << endl;
  cout << "min1 = " << min1 << " max1 = " << max1 << endl;

  AimsRGBA empty;

  int ind = int( (ncol0 - 1) * (float) (getTextureValueFloat() / 360.) );
  if( ind < 0 )
    ind = 0;
  else if( ind >= ncol0 )
    ind = ncol0 - 1;
  empty = (*col)( ind );

  cout << "texture value RGB " << (int) empty.red() << " "
      << (int) empty.green() << " " << (int) empty.blue() << " " << endl;
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

  //win3D->registerObject(s3, true, 0);
  win3D->registerObject(s3, true, 1);
  theAnatomist->registerObject(s3, 0);
  //theAnatomist->registerObject(s3, 1);
  holesObject.push_back(s3);
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

//  vector<float>::iterator it = distanceMap.begin();
//  int i = 0;
//  for (; it != distanceMap.end(); ++it)
//    {
//    //cout << *it << " ";
//    //updateTextureValue(i++, *it);
//    }

  updateTexture(distanceMap);
}
