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

#include <aims/geodesicpath/geodesic_algorithm_dijkstra.h>
#include <aims/geodesicpath/geodesic_algorithm_subdivision.h>
#include <aims/geodesicpath/geodesic_algorithm_exact.h>

SurfpaintTools* SurfpaintTools::my_instance = 0;

SurfpaintTools* SurfpaintTools::instance()
{
  if (my_instance == 0)
    my_instance = new SurfpaintTools;
  return my_instance;
}

SurfpaintTools::SurfpaintTools()/* : Observer()*/
{
  changeControl(0);
  shortestPathSelectedType = "ShortestPath";
}

SurfpaintTools::~SurfpaintTools()
{
  //win3D->deleteObserver( this );
}

//void SurfpaintTools::update( const Observable*, void* arg )
//{
//  cout << "Tools3DWindow::update\n";
//
//  if( arg == 0 )
//    {
//      delete this;
//      return;
//    }
//}
//
//
//void SurfpaintTools::unregisterObservable( Observable* o )
//{
//  Observer::unregisterObservable( o );
//}

void SurfpaintTools::popAllButtonPaintToolBar()
{
  colorPickerAction->setChecked(false);
  selectionAction->setChecked(false);
  pathAction->setChecked(false);
  shortestPathAction->setChecked(false);
  sulciPathAction->setChecked(false);
  gyriPathAction->setChecked(false);
  paintBrushAction->setChecked(false);
  eraseAction->setChecked(false);
}

void SurfpaintTools::colorPicker()
{
  popAllButtonPaintToolBar();
  colorPickerAction->setChecked(true);
  //cout << "colorPickerAction\n";
  changeControl(1);
}

void SurfpaintTools::magicSelection()
{
  popAllButtonPaintToolBar();
  selectionAction->setChecked(true);
  //cout << "magicSelectionAction\n";
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
  string iconname = Settings::globalPath() + "/icons/meshPaint/shortest.png";
  pathAction->setIcon(QIcon(iconname.c_str()));
  pathAction->setChecked(true);
  shortestPathSelectedType = "ShortestPath";
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::sulciPath()
{
  popAllButtonPaintToolBar();
  string iconname = Settings::globalPath() + "/icons/meshPaint/sulci.png";
  pathAction->setIcon(QIcon(iconname.c_str()));
  pathAction->setChecked(true);
  shortestPathSelectedType = "SulciPath";
  changeControl(3);
  setClosePath(false);
}

void SurfpaintTools::gyriPath()
{
  popAllButtonPaintToolBar();
  string iconname = Settings::globalPath() + "/icons/meshPaint/gyri.png";
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

void SurfpaintTools::fill()
{
  int i;

  float texvalue = getTextureValueFloat();

  //remplissage de région fermée
  if (IDActiveControl == 2)
  {
    for (unsigned i = 0; i < listIndexVertexSelectFill.size(); i++)
    {
      updateTextureValue(listIndexVertexSelectFill[i], texvalue);
      listVertexChanged[listIndexVertexSelectFill[i]] = texvalue;
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


  //win3D->refreshNow();
}

void SurfpaintTools::erase()
{
  popAllButtonPaintToolBar();
  eraseAction->setChecked(true);
  changeControl(5);
}

void SurfpaintTools::clearAll()
{
  clearPath();
  clearRegion();
}

void SurfpaintTools::clearPath()
{
  listIndexVertexSelectSP.clear();
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
  //cout << "save Texture on disk " << endl;

  //  typename std::map<int, T>::const_iterator mit(_listVertexChanged.begin()),
  //      mend(_listVertexChanged.end());
  //
  //  const float* t = _aTex->textureCoords();
  //
  //
  //  TimeTexture<T> out(1, _mesh.vertex().size());
  //  for (uint i = 0; i < _mesh.vertex().size(); i++)
  //  {
  //    if (_dataType == "FLOAT")
  //    {
  //      out[0].item(i) = (float) (_tex[0].item(i));
  //      //out[0].item(i) = 0;
  //    }
  //
  //    if (_dataType == "S16")
  //    {
  //      out[0].item(i) = (int) (_tex[0].item(i));
  //      //out[0].item(i) = 0;
  //    }
  //  }
  //
  //  for (; mit != mend; ++mit)
  //  {
  //    //cout << (int)mit->first << " " << mit->second << endl;
  //
  //    _tex[0].item(mit->first) = mit->second;
  //    out[0].item(mit->first) = mit->second;
  //    //out[0].item(mit->first) = 1;
  //  }
  //
  //  rc_ptr<Texture1d> tex(new Texture1d);
  //  Converter<TimeTexture<T> , Texture1d> c;
  //  c.convert(_tex, *tex);
  //  _aTex->setTexture(tex);
  //
  //  _listVertexChanged.clear();
  //
  //  Writer<TimeTexture<T> > wt(_adressTexOut);
  //  wt.write(out);
}

void SurfpaintTools::initSurfPaintModule(AWindow3D *w3)
{
  //cout << "initSurfPaintModule\n";

  //w3->addObserver(this);
  stepToleranceValue = 0;

  const string ac = w3->view()->controlSwitch()->activeControl();
  //cout << "active control: " << ac << endl;

  GLWidgetManager * glw = dynamic_cast<GLWidgetManager *> (w3->view());

  if (glw)
  {
    glw->copyBackBuffer2Texture();

    //sélectionne l'objet positionné au milieu de la fenêtre (bof !)
    QSize s = glw->qglWidget()->size();
    AObject *o = w3->objectAtCursorPosition(s.width() / 2, s.height() / 2);

    cout << "size : " << s.width() << " " <<  s.height() << endl;

    objselect = o;

    cout << objselect << endl;

    GLComponent *glc = o->glAPI();
    glc->glAPI()->glSetTexRGBInterpolation(true);

    if (o != NULL && w3->hasObject(o))
    {
      objtype = o->objectTypeName(o->type());

      cout << objtype << endl;

      if (objtype == "SURFACE")
      {
        QMessageBox::warning(this, ControlledWindow::tr(
            "not texture associated"), ControlledWindow::tr(
            "Cannot open surfpaint Toolbox"));
      }

      if (objtype == "TEXTURED SURF.")
      {
        cout << "coucou" << endl;

        go = dynamic_cast<ATexSurface *> (o);
        surf = go->surface();
        tex = go->texture();
        at = dynamic_cast<ATexture *> (tex);
        int t = (int) w3->GetTime();

        cout << "t " << t << endl;

        as = dynamic_cast<ATriangulated *> (surf);

        cout << "as " << endl;

        rc_ptr<AimsSurfaceTriangle> mesh(new AimsSurfaceTriangle);

        cout << "mesh " << endl;

        Object options = Object::value(Dictionary());
        options->setProperty("scale", 0);

        mesh = ObjectConverter<AimsSurfaceTriangle>::ana2aims(as, options);
        //mesh = as->surface();

        //AimsSurfaceTriangle mesh;
        //AimsSurface<3, Void> *s = as->surfaceOfTime(t);

        cout << "AimsSurface " << endl;

        vector<AimsVector<uint, 3> > & tri = mesh->polygon();
        const vector<Point3df> & vert = mesh->vertex();

        at->attributed()->getProperty("data_type", textype);

        cout << "type texture :" << textype << endl;
        cout << "create Texture temp" << endl;

        surfpaintTexInit = new Texture1d;
        surfpaintTexInit->reserve(at->size());



        rc_ptr<TimeTexture<float> > text;
        text = ObjectConverter<TimeTexture<float> >::ana2aims(tex, options);

        for (uint i = 0; i < at->size(); i++)
          surfpaintTexInit[0].item(i) = (*text).item(i);

        float it = at->TimeStep();
        const GLComponent::TexExtrema & te = at->glTexExtrema(0);

        cout << "minmax tex : " << te.min[0] << " " << te.max[0] << endl;
        cout << "minmax quant tex : " << te.minquant[0] << " "
            << te.maxquant[0] << endl;

        setMaxPoly(tri.size());
        setMaxVertex(vert.size());

        setMinMaxTexture(0, 360);

//        if (w3->constraintEditorIsActive())
//          setMinMaxTexture(0, 360);
//        else
//          setMinMaxTexture((float) (te.minquant[0]), (float) (te.maxquant[0]));

        TimeTexture<float> texCurv;
        cout << "compute texture curvature : ";
        texCurv = TimeTexture<float> (1, vert.size());

        CurvatureFactory CF;
        Curvature *curvat = CF.createCurvature(*mesh,"barycenter");
        texCurv[0] = curvat->doIt();
        curvat->regularize(texCurv[0],1);
        curvat->getTextureProperties(texCurv[0]);
        delete curvat;

//        texCurv = AimsMeshCurvature(mesh);
        cout << "done" << endl;

        texCurvature = (float*) malloc(texCurv[0].nItem() * sizeof(float));
        for (uint i = 0; i < texCurv[0].nItem(); i++)
        {
          texCurvature[i] = (float) (texCurv[0].item(i));
          //cout << texCurvature[i] << endl;
        }

        // copy vertex and faces vector
        std::vector<double> pointsSP;
        std::vector<unsigned> facesSP;
        pointsSP.resize(3 * vert.size());
        facesSP.resize(3 * tri.size());

        for (uint j = 0; j < (int) vert.size(); j++)
        {
          pointsSP[3 * j] = vert[j][0];
          pointsSP[3 * j + 1] = vert[j][1];
          pointsSP[3 * j + 2] = vert[j][2];
        }
        for (uint j = 0; j < (int) tri.size(); j++)
        {
          facesSP[3 * j] = tri[j][0];
          facesSP[3 * j + 1] = tri[j][1];
          facesSP[3 * j + 2] = tri[j][2];
        }

        // compute adjacence graph
        cout << "compute adjacences graphs : ";

        meshSP.initialize_mesh_data(pointsSP, facesSP, NULL, 0, 0);
        meshSulciCurvSP.initialize_mesh_data(pointsSP, facesSP, texCurvature,
            1, 5);
        meshGyriCurvSP.initialize_mesh_data(pointsSP, facesSP, texCurvature, 2,
            5);

        cout << "done" << endl;




        cout << "compute surface neighbours : ";
        neighbours = SurfaceManip::surfaceNeighbours(*mesh);
        cout << "done" << endl;

      }
    }
  }

  w3->Refresh();
}

void SurfpaintTools::addToolBarControls(AWindow3D *w3)
{
  //cout << "addToolBar InfosTexture\n";

  if (w3)
  {
    win3D = w3;

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

    //pipette
    iconname = Settings::globalPath() + "/icons/meshPaint/pipette.png";
    colorPickerAction = new QToolButton();
    colorPickerAction->setIcon(QIcon(iconname.c_str()));
    //colorPickerAction->setStatusTip(tr("Texture value selection"));
    colorPickerAction->setToolTip(tr("Texture value selection"));
    colorPickerAction->setCheckable(true);
    colorPickerAction->setChecked(false);
    colorPickerAction->setIconSize(QSize(32, 32));
    colorPickerAction->setAutoRaise(true);
    connect(colorPickerAction, SIGNAL(clicked()), this, SLOT(colorPicker()));

    //baguette magique
    iconname = Settings::globalPath() + "/icons/meshPaint/magic_selection.png";
    selectionAction = new QToolButton();
    selectionAction->setIcon(QIcon(iconname.c_str()));
    selectionAction->setToolTip(ControlledWindow::tr("Area magic selection"));
    selectionAction->setCheckable(true);
    selectionAction->setChecked(false);
    selectionAction->setIconSize(QSize(32, 32));
    selectionAction->setAutoRaise(true);
    connect(selectionAction, SIGNAL(clicked()), this, SLOT(magicSelection()));

    //plus court chemin
    iconname = Settings::globalPath() + "/icons/meshPaint/shortest.png";
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
    iconname = Settings::globalPath() + "/icons/meshPaint/sulci.png";
    sulciPathAction = new QAction(QIcon(iconname.c_str()),
        ControlledWindow::tr("sulci"), this);
    sulciPathAction->setToolTip(tr("sulci"));
    connect(sulciPathAction, SIGNAL(triggered()), this, SLOT(sulciPath()));
    menu->addAction(sulciPathAction);
    iconname = Settings::globalPath() + "/icons/meshPaint/gyri.png";
    gyriPathAction = new QAction(QIcon(iconname.c_str()), ControlledWindow::tr(
        "gyri"), this);
    gyriPathAction->setToolTip(tr("gyri"));
    connect(gyriPathAction, SIGNAL(triggered()), this, SLOT(gyriPath()));
    menu->addAction(gyriPathAction);
    pathAction->setMenu(menu);

    //clear
    iconname = Settings::globalPath() + "/icons/meshPaint/clear.png";
    clearPathAction = new QToolButton();
    clearPathAction->setIcon(QIcon(iconname.c_str()));
    clearPathAction->setToolTip(ControlledWindow::tr(
        "Delete all selected objects"));
    clearPathAction->setIconSize(QSize(32, 32));
    connect(clearPathAction, SIGNAL(clicked()), this, SLOT(clearAll()));

    //brush
    iconname = Settings::globalPath() + "/icons/meshPaint/stylo.png";
    paintBrushAction = new QToolButton();
    paintBrushAction->setIcon(QIcon(iconname.c_str()));
    paintBrushAction->setToolTip(ControlledWindow::tr("Brush"));
    paintBrushAction->setCheckable(true);
    paintBrushAction->setChecked(false);
    paintBrushAction->setIconSize(QSize(32, 32));
    paintBrushAction->setAutoRaise(true);
    connect(paintBrushAction, SIGNAL(clicked()), this, SLOT(brush()));

    //fill
    //iconname = Settings::globalPath() + "/icons/meshPaint/fill.png";
    iconname = Settings::globalPath() + "/icons/meshPaint/valide.png";
    fillAction = new QToolButton();
    fillAction->setIcon(QIcon(iconname.c_str()));
    fillAction->setToolTip(ControlledWindow::tr("Fill area or path selected"));
    fillAction->setIconSize(QSize(32, 32));
    connect(fillAction, SIGNAL(clicked()), this, SLOT(fill()));

    //erase
    iconname = Settings::globalPath() + "/icons/meshPaint/erase.png";
    eraseAction = new QToolButton();
    eraseAction->setIcon(QIcon(iconname.c_str()));
    eraseAction->setToolTip(ControlledWindow::tr("Eraser"));
    eraseAction->setCheckable(true);
    eraseAction->setChecked(false);
    eraseAction->setIconSize(QSize(32, 32));
    eraseAction->setAutoRaise(true);
    connect(eraseAction, SIGNAL(clicked()), this, SLOT(erase()));

    //save
    iconname = Settings::globalPath() + "/icons/meshPaint/sauver.png";
    saveAction = new QToolButton();
    saveAction->setIcon(QIcon(iconname.c_str()));
    saveAction->setToolTip(ControlledWindow::tr("Save texture"));
    saveAction->setIconSize(QSize(32, 32));
    connect(saveAction, SIGNAL(clicked()), this, SLOT(save()));

    tbControls->addWidget(colorPickerAction);
    tbControls->addWidget(selectionAction);
    tbControls->addWidget(pathAction);
    tbControls->addWidget(clearPathAction);
    tbControls->addWidget(paintBrushAction);
    tbControls->addWidget(fillAction);
    tbControls->addWidget(eraseAction);
    tbControls->addWidget(saveAction);
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
    textureFloatSpinBox->setFixedHeight(30);
    textureFloatSpinBox->setFixedWidth(100);
    textureFloatSpinBox->setValue(0.000);

    // ARN on affiche la liste des contraintes seulement si le module ConstraintEditor a été lancé ?
    if (w3->constraintEditorIsActive())
    {
      constraintList = new QComboBox(infosTextureValue);
      loadConstraintsList();
connect    ( constraintList, SIGNAL( activated( int ) ), this, SLOT( updateConstraintList() ) );
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
  IDPolygonSpinBox->setFixedHeight(30);
  IDPolygonSpinBox->setFixedWidth(75);
  IDPolygonSpinBox->setValue(0);

  QLabel *IDVertexSpinBoxLabel = new QLabel(ControlledWindow::tr("IDVertex"),infos3D);

  IDVertexSpinBox = new QSpinBox(infos3D);
  IDVertexSpinBox->setSingleStep(1);
  IDVertexSpinBox->setFixedHeight(30);
  IDVertexSpinBox->setFixedWidth(75);
  IDVertexSpinBox->setValue(0);

  toleranceSpinBoxLabel = new QLabel(tr("tolerance"),infos3D);
  toleranceSpinBox = new QSpinBox (infos3D);
  toleranceSpinBox->setSingleStep(1);
  toleranceSpinBox->setFixedHeight(30);
  toleranceSpinBox->setFixedWidth(55);
  toleranceSpinBox->setValue(0);
  toleranceSpinBox->setRange(0,100);

  constraintPathSpinBoxLabel = new QLabel(ControlledWindow::tr("constraint"),infos3D);
  constraintPathSpinBox = new QSpinBox(infos3D);
  constraintPathSpinBox->setSingleStep(1);
  constraintPathSpinBox->setFixedHeight(30);
  constraintPathSpinBox->setFixedWidth(55);
  constraintPathSpinBox->setValue(5);
  constraintPathSpinBox->setRange(0,100);

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
  tbTextureValue = w3->removeToolBar( "surfpaint_toolbar_Tex" );
  tbInfos3D = w3->removeToolBar( "surfpaint_toolbar_3D" );
  delete tbTextureValue;
  delete tbInfos3D;
#else
  tbTextureValue
      = dynamic_cast<QToolBar *> (w3->child("surfpaint_toolbar_Tex"));
  tbInfos3D = dynamic_cast<QToolBar *> (w3->child("surfpaint_toolbar_3D"));
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
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect))
  {
    if (objtype == "TEXTURED SURF.")
    {
      ATexSurface *go;
      go = dynamic_cast<ATexSurface *> (objselect);

      AObject *tex = go->texture();

      Object options = Object::value(Dictionary());
      options->setProperty("scale", 0);

      float it = at->TimeStep();
      const GLComponent::TexExtrema & te = at->glTexExtrema(0);

      float scl = (te.maxquant[0] - te.minquant[0]);

      rc_ptr<TimeTexture<float> > t;
      t = ObjectConverter<TimeTexture<float> >::ana2aims(tex, options);

      float value;
      value = surfpaintTexInit[0].item(indexVertex);
      (*t).item(indexVertex) = (float) value;

      tex->setChanged();
      tex->notifyObservers();
      tex->setInternalsChanged();
      //tex->internalUpdate();
      win3D->Refresh();
    }
  }
}

void SurfpaintTools::updateTextureValue(int indexVertex, float value)
{
  if (win3D != NULL && objselect != NULL && win3D->hasObject(objselect))
  {
    if (objtype == "TEXTURED SURF.")
    {
      ATexSurface *go;
      go = dynamic_cast<ATexSurface *> (objselect);

      AObject *tex = go->texture();

      Object options = Object::value(Dictionary());
      options->setProperty("scale", 0);

      float it = at->TimeStep();
      const GLComponent::TexExtrema & te = at->glTexExtrema(0);

      float scl = (te.maxquant[0] - te.minquant[0]);

      rc_ptr<TimeTexture<float> > t;
      t = ObjectConverter<TimeTexture<float> >::ana2aims(tex, options);

      (*t).item(indexVertex) = (float) value / scl;

      tex->setChanged();
      tex->notifyObservers(this);
      tex->setInternalsChanged();
      //      win3D->setChanged();
      //      win3D->notifyObservers( this );
      //win3D->Refresh();
      //win3D->refreshNow();
      listVertexChanged[indexVertex] = value;
    }
  }
}

void SurfpaintTools::floodFillStart(int indexVertex)
{
  listIndexVertexSelectFill.clear();

  std::map<int, float>::iterator itef;
  itef = listVertexChanged.find(indexVertex);

  float texvalue = getTextureValueFloat();

  if (itef != listVertexChanged.end())
    floodFillMove (indexVertex, texvalue,itef->second);
  else
    floodFillMove (indexVertex, texvalue,surfpaintTexInit[0].item(indexVertex));
}

void SurfpaintTools::floodFillStop(void)
{
  AimsSurface<3, Void> *s = as->surfaceOfTime(0);
  const vector<Point3df> & vert = s->vertex();

  AimsSurfaceTriangle *MeshOut,*tmpMeshOut;
  MeshOut = new AimsSurfaceTriangle;

  for (unsigned i = 0; i < listIndexVertexSelectFill.size(); i++)
  {
    //Point3df pt;
    tmpMeshOut = SurfaceGenerator::sphere(vert[listIndexVertexSelectFill[i]], 0.25, 50);

    SurfaceManip::meshMerge(*MeshOut, *tmpMeshOut);
    delete tmpMeshOut;
  }

  const anatomist::AObjectPalette *pal;
  GLComponent *glc = objselect->glAPI();
  pal = glc->glPalette(0);

  const AimsData<AimsRGBA> *col = pal->colors();
  const GLComponent::TexExtrema & te = glc->glTexExtrema(0);

  AimsRGBA empty;
  empty = (*col)((int) 255
      * (float) (getTextureValueFloat() / 360));

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
  //win3D->registerObject(fillObject, true, 0);
  win3D->registerObject(fillObjectTemp, true, 1);
  theAnatomist->registerObject(fillObjectTemp, 0);
  //theAnatomist->registerObject(fillObject, 1);
  fillObject.push_back(fillObjectTemp);
}


void SurfpaintTools::floodFillMove(int indexVertex, float newTextureValue,
    float oldTextureValue)
{
  bool go;
  bool stop;
  bool pip;

  std::set<uint> voisins = neighbours[indexVertex];
  std::set<uint>::iterator voisIt = voisins.begin();

  voisIt = voisins.begin();

  std::vector<int>::iterator s1 = listIndexVertexPathSP.begin();
  std::vector<int>::iterator s2 = listIndexVertexPathSP.end();
  std::vector<int>::iterator ite = std::find(s1, s2, indexVertex);

  std::vector<int>::iterator f1 = listIndexVertexSelectFill.begin();
  std::vector<int>::iterator f2 = listIndexVertexSelectFill.end();
  std::vector<int>::iterator itef = std::find(f1, f2, indexVertex);

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
    listIndexVertexSelectFill.push_back(indexVertex);
    for (; voisIt != voisins.end(); voisIt++)
      floodFillMove(*voisIt, newTextureValue, oldTextureValue);
  }

}

void SurfpaintTools::updateConstraintList(void)
{
  int item = constraintList->currentItem();
  string constraintLabel = string(constraintList->currentText());

  //cout << constraintLabel << " value " << item << endl;
  int position = constraintLabel.find_last_of(' ');

  //cout << "contrainte : " << constraintLabel.substr(0,position) << endl;
  std::istringstream strin(constraintLabel.substr(position + 1));
  int value;
  strin >> value;
  //cout << value << endl;

  setTextureValueFloat((float) value);
  //  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32"))
  //    setTextureValueInt(value);
  //
  //  if (_textype == "FLOAT")
  //    setTextureValueFloat(value);
}

void SurfpaintTools::changeToleranceSpinBox(int v)
{
  toleranceValue = v;
  stepToleranceValue = toleranceValue * (float)(360 - 0)/100.;
  //cout << "ToleranceValue " << stepToleranceValue << endl;
}

void SurfpaintTools::loadConstraintsList()
{
  char sep = carto::FileUtil::separator();

#if 0 // def USE_SHARE_CONFIG
  string talref = carto::Paths::globalShared() + sep + BRAINVISA_SHARE_DIRECTORY + sep + "nomenclature" + sep + "surfaceanalysis" + sep
  + "constraint_correspondance.txt";
#else
  string talref = carto::Paths::shfjShared() + sep + "nomenclature" + sep
      + "surfaceanalysis" + sep + "constraint_correspondance.txt";
#endif

  cout << "File contraints loaded : " << talref << endl;

  string line;
  ifstream myfile(talref.c_str());
  if (myfile.is_open())
  {
    while (myfile.good())
    {
      getline(myfile, line);
      if (line.length() != 0)
        constraintList->addItem(line.c_str());
    }
    myfile.close();
  }

  else
    cout << "Unable to open file " << talref << endl;
}

void SurfpaintTools::changeConstraintPathSpinBox(int v)
{

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

  if (!pathIsClosed())
  {
    win3D->registerObject(sp, true, 1);
    //win3D->registerObject(sp, true, 0);
    //theAnatomist->registerObject(sp, 1);
    theAnatomist->registerObject(sp, 0);
    pathObject.push_back(sp);
  }

  std::vector<int>::iterator ite;

  if (!pathIsClosed())
    listIndexVertexSelectSP.push_back(indexNearestVertex);
  else
    listIndexVertexSelectSP.push_back(*listIndexVertexSelectSP.begin());

  ite = listIndexVertexSelectSP.end();

  int nb_vertex;
  printf("nb vertex path = %d\n", listIndexVertexSelectSP.size());

  std::vector<geodesic::SurfacePoint> sources;
  std::vector<geodesic::SurfacePoint> targets;

  geodesic::GeodesicAlgorithmDijkstra *dijkstra_algorithm;
  geodesic::Mesh meshSP;

  const string ac = getPathType();

  if (ac.compare("ShortestPath") == 0)
    meshSP = getMeshStructSP();
  else if (ac.compare("SulciPath") == 0)
    meshSP = getMeshStructSulciP();
  else if (ac.compare("GyriPath") == 0)
    meshSP = getMeshStructGyriP();

  dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&meshSP);

  if (listIndexVertexSelectSP.size() >= 2)
  {
    unsigned target_vertex_index = (*(--ite));
    unsigned source_vertex_index = (*(--ite));

    printf("indice source = %d target = %d \n", source_vertex_index,
        target_vertex_index);

    std::vector<geodesic::SurfacePoint> SPath;
    SPath.clear();

    listIndexVertexPathSPLast.clear();

    geodesic::SurfacePoint short_sources(
        &meshSP.vertices()[source_vertex_index]);
    geodesic::SurfacePoint short_targets(
        &meshSP.vertices()[target_vertex_index]);

    dijkstra_algorithm->geodesic(short_sources, short_targets, SPath,
        listIndexVertexPathSPLast);

    ite = listIndexVertexPathSPLast.end();

    reverse(listIndexVertexPathSPLast.begin(), listIndexVertexPathSPLast.end());
    listIndexVertexPathSPLast.push_back((int) target_vertex_index);

    listIndexVertexPathSP.insert(listIndexVertexPathSP.end(),
        listIndexVertexPathSPLast.begin(), listIndexVertexPathSPLast.end());

    cout << "path dijkstra = ";

    for ( i = 0; i < listIndexVertexPathSP.size(); i++)
    {
      cout << listIndexVertexPathSP[i] << " ";
    }
    cout << endl;

    std::vector<Point3df> vertexList;
    Point3df newVertex;

    AimsSurfaceTriangle *MeshOut = new AimsSurfaceTriangle;

    for (i = 0; i < SPath.size(); ++i)
    {
      newVertex[0] = SPath[i].x();
      newVertex[1] = SPath[i].y();
      newVertex[2] = SPath[i].z();
      vertexList.push_back(newVertex);
    }

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

    const anatomist::AObjectPalette *pal;

    //GLComponent *glc = objselect->glAPI();

    GLComponent *glc = at->glAPI();

    pal = at->glAPI()->glPalette(0);

    at->setTexExtrema(0,360);

    const AimsData<AimsRGBA> *col = pal->colors();

    const GLComponent::TexExtrema & te = glc->glTexExtrema(0);

    AimsRGBA empty;

    empty = (*col)((int) 255
        * (float) (getTextureValueFloat() / 360));

//    cout << "minq = " << te.min[0] << "maxq = " << te.maxquant[0] << endl;
//    cout << "min = " << te.minquant[0] << "max = " << te.max[0] << endl;

    cout << "texture value RGB " << (int) empty.red() << " "
        << (int) empty.green() << " " << (int) empty.blue() << " " << endl;
    Material mat2;
    mat2.setRenderProperty(Material::Ghost, 1);
    mat2.setRenderProperty(Material::RenderLighting, 1);
    mat2.SetDiffuse((float) empty.red() / 255, (float) empty.green() / 255,
        (float) empty.blue() / 255, 1.);

    ATriangulated *s3 = new ATriangulated();
    s3->setName(theAnatomist->makeObjectName("path"));
    s3->setSurface(MeshOut);
    s3->SetMaterial(mat2);
    //win3D->registerObject(s3, true, 0);
    win3D->registerObject(s3, true, 1);
    theAnatomist->registerObject(s3, 0);
    //theAnatomist->registerObject(s3, 1);
    pathObject.push_back(s3);
  }
}

/////////////////////////////

//
//View* SurfpaintToolsWindow::view()
//{
//  return (_window->view());
//}
//
//const View* SurfpaintToolsWindow::view() const
//{
//  return (_window->view());
//}
//
//const string & SurfpaintToolsWindow::baseTitle() const
//{
//  return (_baseTitle);
//}
//
//
//void SurfpaintToolsWindow::colorPicker()
//{
//  popAllButtonPaintToolBar();
//  colorPickerAction->setChecked(true);
//  cout << "colorPickerAction\n";
//
//  _window->view()->controlSwitch()->notifyAvailableControlChange();
//  _window->view()->controlSwitch()->notifyActivableControlChange();
//
//  _window->view()->controlSwitch()->setActiveControl("SurfpaintColorPickerControl");
//  _window->view()->controlSwitch()->notifyActiveControlChange();
//  _window->view()->controlSwitch()->notifyActionChange();
//
//SurfpaintToolsWindow::SurfpaintToolsWindow(AWindow3D *win, string textype, AObject *surf, AObject *tex) :
//    QWidget(0, "SurfpaintControl", Qt::WDestructiveClose),
////    ControlledWindow(win, "paintcontrol3D", NULL, 0),
//    Observer(),
//    _window(win),
//    _surf(surf),
//    _textype(textype), destroying(false)
//{
//  shortestPathSelectedType = "ShortestPath";
//
//  //SurfpaintModule     *sp = SurfpaintModule::instance();
//
//  // read triangulation
////  cout << "reading triangulation   : " << flush;
////  AimsSurfaceTriangle surface;
////  Reader<AimsSurfaceTriangle> triR( meshFileIn );
////  triR >> surface;
////  cout << "done" << endl;
//
// // AimsSurfaceTriangle surface;
//  ATriangulated *as;
//  as = dynamic_cast<ATriangulated *> (surf);
//  AimsSurface<3, Void> *s = as->surfaceOfTime(0);
//
//  // compute and copy curvature

//
//  win->addObserver(this);
//
//  setCaption(tr("SurfpaintControl") + _window->Title().c_str());
//  const QPixmap *p;
//  p = IconDictionary::instance()->getIconInstance("SurfpaintControl");
//  this->setWindowIcon(*p);
//
//  QVBoxLayout *mainlay = new QVBoxLayout( this, 10, 5 );
//
//  QHGroupBox  *actions = new QHGroupBox( tr( "Controlers :" ), this );
//
//
//  //QHBox *ctrl = new QHBox();
//
////

//
//  string iconname;
//
//  //pipette
//  iconname = Settings::globalPath() + "/icons/meshPaint/pipette.png";
//  colorPickerAction = new QToolButton (actions);
//  colorPickerAction->setIcon(QIcon(iconname.c_str()));
//  //colorPickerAction->setStatusTip(tr("Texture value selection"));
//  colorPickerAction->setToolTip(tr("Texture value selection"));
//  colorPickerAction->setCheckable(true);
//  colorPickerAction->setChecked(false);
//  colorPickerAction->setIconSize(QSize(32, 32));
//  //colorPickerAction->setFocusPolicy(Qt::StrongFocus);
//  colorPickerAction->setAutoRaise(true);
//  connect(colorPickerAction, SIGNAL(clicked()), this, SLOT(colorPicker()));
//
//  //baguette magique
//  iconname = Settings::globalPath() + "/icons/meshPaint/magic_selection.png";
//  selectionAction = new QToolButton (actions);
//  selectionAction->setIcon(QIcon(iconname.c_str()));
//  selectionAction->setToolTip(tr("Area magic selection"));
//  selectionAction->setCheckable(true);
//  selectionAction->setChecked(false);
//  //selectionAction->setFocusPolicy(Qt::StrongFocus);
//  selectionAction->setIconSize(QSize(32, 32));
//  selectionAction->setAutoRaise(true);
//
//  connect(selectionAction, SIGNAL(clicked()), this, SLOT(magicSelection()));

//  mainlay->addWidget(actions);
//
//  QVGroupBox  *infos = new QVGroupBox( tr( "Parameters :" ), this );
//
//  QHBox *infosTextureValue = new QHBox(infos);
//
//  QLabel *SpinBoxLabel = new QLabel(tr("TextureValue"), infosTextureValue);
//
//  if (_textype == "FLOAT")
//  {
//    QDoubleSpinBox *textureFloatSpinBox = new QDoubleSpinBox(infosTextureValue);
//
//    textureFloatSpinBox->setSingleStep(0.1);
//    textureFloatSpinBox->setDecimals(2);
//    textureFloatSpinBox->setFixedHeight(30);
//    textureFloatSpinBox->setFixedWidth(100);
//    textureFloatSpinBox->setValue(0.000);
//    textureSpinBox = static_cast<QDoubleSpinBox*> (textureFloatSpinBox);
//    connect  (textureFloatSpinBox,SIGNAL(valueChanged(double)),this,SLOT(setTextureValueFloat(double)));
//  }
//
//  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32"))
//  {
//    QSpinBox *textureIntSpinBox = new QSpinBox(infosTextureValue);
//    textureIntSpinBox->setSingleStep(1);
//    textureIntSpinBox->setFixedHeight(30);
//    textureIntSpinBox->setFixedWidth(75);
//    textureIntSpinBox->setValue(0);
//    textureSpinBox = static_cast<QSpinBox*>(textureIntSpinBox);
//    connect(textureIntSpinBox,SIGNAL(valueChanged(int)),this,SLOT(setTextureValueInt(int)));
//  }
//

//
//  QHBox *infos3D = new QHBox(infos);
//
//  QLabel *IDPolygonSpinBoxLabel = new QLabel(tr("ID Polygon : "),infos3D);
//
//  IDPolygonSpinBox = new QSpinBox(infos3D);
//  IDPolygonSpinBox->setSingleStep(1);
//  IDPolygonSpinBox->setFixedHeight(30);
//  IDPolygonSpinBox->setFixedWidth(75);
//  IDPolygonSpinBox->setValue(0);
//
//  QLabel *IDVertexSpinBoxLabel = new QLabel(tr("ID Vertex : "),infos3D);
//
//  IDVertexSpinBox = new QSpinBox(infos3D);
//  IDVertexSpinBox->setSingleStep(1);
//  IDVertexSpinBox->setFixedHeight(30);
//  IDVertexSpinBox->setFixedWidth(75);
//  IDVertexSpinBox->setValue(0);
//
//  mainlay->addWidget(infos);
//
////  QHBox *parameters = new QHBox(infos);
////
//  connect(toleranceSpinBox ,SIGNAL(valueChanged(int)),this,SLOT(changeToleranceSpinBox(int)));
////  connect(constraintPathSpinBox ,SIGNAL(valueChanged(int)),this,SLOT(changeConstraintPathSpinBox(int)));
//
//
//  setLayout(mainlay);
//}
//
//SurfpaintToolsWindow::~SurfpaintToolsWindow()
//{
//  destroying = true;
//  //_window->deleteObserver( this );
//  //cleanupObserver();
//  //_window->painttoolsWinDestroyed();
//}
//
//void SurfpaintToolsWindow::update(const Observable*, void* arg)
//{
//  cout << "SurfpaintToolsWindow::update\n";
//
//}
//
//void SurfpaintToolsWindow::unregisterObservable(Observable* o)
//{
////  Observer::unregisterObservable(o);
////  if (!destroying) delete this;
//}
//
//void SurfpaintToolsWindow::setPolygon(int p)
//{
//  IDPolygonSpinBox->setValue(p);
//
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setVertex(int v)
//{
//  IDVertexSpinBox->setValue(v);
//
//  _window->Refresh();
//}
//float SurfpaintToolsWindow::getTextureValue(void)
//{
//  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32")) return (float) (dynamic_cast<QSpinBox*> (textureSpinBox)->value());
//
//  if (_textype == "FLOAT") return (float) (dynamic_cast<QDoubleSpinBox*> (textureSpinBox)->value());
//}
//
//void SurfpaintToolsWindow::setTextureValue(float v)
//{
//  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32"))
//    setTextureValueInt((int) v);
//
//  if (_textype == "FLOAT") setTextureValueFloat((double) v);
//
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setTextureValueInt(int v)
//{
//  dynamic_cast<QSpinBox*> (textureSpinBox)->setValue(v);
//
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setTextureValueFloat(double v)
//{
//  dynamic_cast<QDoubleSpinBox*> (textureSpinBox)->setValue(v);
//
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setMaxPoly(int max)
//{
//  IDPolygonSpinBox->setRange(0, max);
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setMaxVertex(int max)
//{
//  IDVertexSpinBox->setRange(0, max);
//  _window->Refresh();
//}
//
//void SurfpaintToolsWindow::setMinMaxTexture(float min, float max)
//{
//  if (_textype == "S16" || (_textype == "U32") || (_textype == "S32")) dynamic_cast<QSpinBox*> (textureSpinBox)->setRange(
//      (int) min, (int) max);
//
//  if (_textype == "FLOAT") dynamic_cast<QDoubleSpinBox*> (textureSpinBox)->setRange( min, max);
//
//  _window->Refresh();
//}
