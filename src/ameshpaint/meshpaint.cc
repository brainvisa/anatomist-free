#include "meshpaint.h"
#ifdef USE_SHARE_CONFIG
#include <brainvisa-share/config.h>
#endif

template<typename T>
myMeshPaint<T>::myMeshPaint(string adressTexIn, string adressMeshIn,
    string adressTexCurvIn, string adressTexOut, string colorMap,
    string dataType) :
  _adressTexIn(adressTexIn), _adressMeshIn(adressMeshIn), _adressTexCurvIn(
      adressTexCurvIn), _adressTexOut(adressTexOut), _colorMap(colorMap),
      _dataType(dataType)
{
  QRect r = geometry();
  r.moveCenter(QApplication::desktop()->availableGeometry().center());
  setGeometry(r);

  resize(880, 600);

  glWidget = new myGLWidget<T> (this, adressTexIn, adressMeshIn,
      adressTexCurvIn, adressTexOut, colorMap, dataType);

  setCentralWidget( glWidget);
  glWidget->setFocusPolicy(Qt::StrongFocus);


  QLabel *SpinBoxLabel = new QLabel(tr("Texture Value : "));

  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox = new QDoubleSpinBox;

    textureFloatSpinBox->setSingleStep(0.01);
    textureFloatSpinBox->setFixedHeight(30);
    textureFloatSpinBox->setFixedWidth(75);
    textureFloatSpinBox->setValue(0.0);

    textureSpinBox = static_cast<QDoubleSpinBox*> (textureFloatSpinBox);
    connect  (textureFloatSpinBox,SIGNAL(valueChanged(double)),glWidget,SLOT(changeTextureSpinBoxFloat(double)));
  }

  if (_dataType == "S16")
  {
    QSpinBox *textureIntSpinBox = new QSpinBox;
    textureIntSpinBox->setSingleStep(1);
    textureIntSpinBox->setFixedHeight(30);
    textureIntSpinBox->setFixedWidth(75);
    textureIntSpinBox->setValue(0);
    textureSpinBox = static_cast<QSpinBox*>(textureIntSpinBox);
    connect(textureIntSpinBox,SIGNAL(valueChanged(int)),glWidget,SLOT(changeTextureSpinBoxInt(int)));
  }

  QLabel *IDPolygonSpinBoxLabel = new QLabel(tr("ID Polygon : "));

  IDPolygonSpinBox = new QSpinBox;
  IDPolygonSpinBox->setSingleStep(1);
  IDPolygonSpinBox->setFixedHeight(30);
  IDPolygonSpinBox->setFixedWidth(75);
  IDPolygonSpinBox->setValue(0);

  QLabel *IDVertexSpinBoxLabel = new QLabel(tr("ID Vertex : "));

  IDVertexSpinBox = new QSpinBox;
  IDVertexSpinBox->setSingleStep(1);
  IDVertexSpinBox->setFixedHeight(30);
  IDVertexSpinBox->setFixedWidth(75);
  IDVertexSpinBox->setValue(0);

  // liste des contraintes
  infosToolBar->addWidget(constraintList);

  infosToolBar->addWidget(SpinBoxLabel);
  infosToolBar->addWidget(textureSpinBox);
  infosToolBar->addSeparator();

  infosToolBar->addWidget(IDPolygonSpinBoxLabel);
  infosToolBar->addWidget(IDPolygonSpinBox);
  infosToolBar->addSeparator();

  infosToolBar->addWidget(IDVertexSpinBoxLabel);
  infosToolBar->addWidget(IDVertexSpinBox);

  connect(IDPolygonSpinBox ,SIGNAL(valueChanged(int)),glWidget,SLOT(changeIDPolygonSpinBox(int)));
  connect(IDVertexSpinBox ,SIGNAL(valueChanged(int)),glWidget,SLOT(changeIDVertexSpinBox(int)));

  connect(toleranceSpinBox ,SIGNAL(valueChanged(int)),glWidget,SLOT(changeToleranceSpinBox(int)));
  connect(constraintPathSpinBox ,SIGNAL(valueChanged(int)),glWidget,SLOT(changeConstraintPathSpinBox(int)));
}

template<typename T>
myMeshPaint<T>::~myMeshPaint()
{
}

template<typename T>
void myMeshPaint<T>::DisplayConstraintList (void)
{
  int  item = constraintList->currentItem();
  string  constraintLabel = string(constraintList->currentText());

  //cout << constraintLabel << " value " << item << endl;

  int position = constraintLabel.find_last_of (' ');

  //cout << "contrainte : " << constraintLabel.substr(0,position) << endl;

  std::istringstream strin(constraintLabel.substr(position+1));
  int value;
  strin >> value;
  cout << value << endl;

  glWidget->changeTextureValue(value);
}

MeshPaint::MeshPaint()
{
  _modePath = 4;
  _modeBrush = 9;

  char sep = FileUtil::separator();

  string consfile = Paths::findResourceFile( string( "nomenclature" ) + sep
    + "surfaceanalysis" + sep + "constraint_correspondance.txt" );

    cout << "Loading constraints file : " << consfile << endl;

  constraintList = new QComboBox( );

  string line;
  ifstream myfile (consfile.c_str());
  if (myfile.is_open())
  {
    while ( myfile.good() )
    {
      getline (myfile,line);
      if (line.length() != 0)
        constraintList->addItem(line.c_str());
    }
    myfile.close();
  }

  else cout << "Unable to open file " << consfile << endl;

  connect( constraintList, SIGNAL( activated( int ) ), this,  SLOT( updateConstraintList() ) );

  createActions();
  createToolBars();
  show();
}

MeshPaint::~MeshPaint()
{
}

template<typename T>
void myMeshPaint<T>::fillRegionOrPath(void)
{
  glWidget->fill();
}

template<typename T>
void myMeshPaint<T>::changeMode(int mode)
{
  //cout << "mode = " << mode << endl;
  glWidget->changeMode(mode);

  if (mode == 4)
    glWidget->changeModePath(1);
  if (mode == 5)
    glWidget->changeModePath(2);
  if (mode == 6)
    glWidget->changeModePath(3);
}

template<typename T>
void myMeshPaint<T>::saveTexture(void)
{
  //cout << "mode = " << mode << endl;
  glWidget->saveTexture();
}

template<typename T>
void myMeshPaint<T>::clearAll(void)
{
  //cout << "mode = " << mode << endl;
  glWidget->clearAll();
}

template<typename T>
void myMeshPaint<T>::keyPressEvent(QKeyEvent* event)
{
  glWidget->keyPressEvent(event);
}

void MeshPaint::createActions()
{
  //zoom
  string iconname = Settings::globalPath() + "/icons/meshPaint/zoom.png";
  trackballAction = new QAction(QIcon(iconname.c_str()), tr("&trackball"), this);
  trackballAction->setStatusTip(tr("trackball"));
  trackballAction->setCheckable(true);
  trackballAction->setChecked(true);
  connect(trackballAction, SIGNAL(triggered()), this, SLOT(trackball()));

  //pipette
  iconname = Settings::globalPath() + "/icons/meshPaint/pipette.png";
  colorPickerAction = new QAction(QIcon(iconname.c_str()), tr("&ColorPicker"),this);
  colorPickerAction->setStatusTip(tr("ColorPicker"));
  colorPickerAction->setCheckable(true);
  connect(colorPickerAction, SIGNAL(triggered()), this, SLOT(colorPicker()));

  //geodesic Distance
  iconname = Settings::globalPath() + "/icons/meshPaint/geodesic_distance.png";
  geodesicDistanceAction = new QAction(QIcon(iconname.c_str()), tr("&GeodesicDistance"),this);
  geodesicDistanceAction->setStatusTip(tr("GeodesicDistance"));
  geodesicDistanceAction->setCheckable(true);
  connect(geodesicDistanceAction, SIGNAL(triggered()), this, SLOT(geodesicDistance()));

  distanceSpinBoxLabel = new QLabel(tr("distance : "));
  distanceSpinBox = new QSpinBox;
  distanceSpinBox->setSingleStep(0.5);
  distanceSpinBox->setFixedHeight(30);
  distanceSpinBox->setFixedWidth(55);
  distanceSpinBox->setValue(0);
  distanceSpinBox->setRange(0,1000);

  //baguette magique
  iconname = Settings::globalPath() + "/icons/meshPaint/magic_selection.png";
  selectionAction = new QAction(QIcon(iconname.c_str()),tr("&magic_selection"), this);
  selectionAction->setStatusTip(tr("magic_selection"));
  selectionAction->setCheckable(true);
  connect(selectionAction, SIGNAL(triggered()), this, SLOT(selection()));
  toleranceSpinBoxLabel = new QLabel(tr("tolerance : "));
  toleranceSpinBox = new QSpinBox;
  toleranceSpinBox->setSingleStep(1);
  toleranceSpinBox->setFixedHeight(30);
  toleranceSpinBox->setFixedWidth(55);
  toleranceSpinBox->setValue(0);
  toleranceSpinBox->setRange(0,100);

  //plus court chemin
  iconname = Settings::globalPath() + "/icons/meshPaint/shortest.png";
  pathButton = new QToolButton;
  pathButton->setIcon(QIcon(iconname.c_str()));
  pathButton->setPopupMode(QToolButton::MenuButtonPopup);
  pathButton->setCheckable(true);
  pathButton->setChecked(false);
  pathButton->setIconSize(QSize(32, 32));
  pathButton->setToolTip(tr("Shortest Path"));
  connect(pathButton, SIGNAL(clicked()), this, SLOT(path()));
  QMenu *menu = new QMenu(this);
  shortPathAction = new QAction(QIcon(iconname.c_str()), tr("&Unconstrained"),this);
  shortPathAction->setToolTip(tr("Unconstrained"));
  connect(shortPathAction, SIGNAL(triggered()), this, SLOT(shortPath()));
  menu->addAction(shortPathAction);
  iconname = Settings::globalPath() + "/icons/meshPaint/sulci.png";
  sulciPathAction = new QAction(QIcon(iconname.c_str()), tr("&sulci"), this);
  sulciPathAction->setToolTip(tr("sulci"));
  connect(sulciPathAction, SIGNAL(triggered()), this, SLOT(sulciPath()));
  menu->addAction(sulciPathAction);
  iconname = Settings::globalPath() + "/icons/meshPaint/gyri.png";
  gyriPathAction = new QAction(QIcon(iconname.c_str()), tr("&gyri"), this);
  gyriPathAction->setToolTip(tr("gyri"));
  connect(gyriPathAction, SIGNAL(triggered()), this, SLOT(gyriPath()));
  menu->addAction(gyriPathAction);
  pathButton->setMenu(menu);
  constraintPathSpinBoxLabel = new QLabel(tr("constraint : "));
  constraintPathSpinBox = new QSpinBox;
  constraintPathSpinBox->setSingleStep(1);
  constraintPathSpinBox->setFixedHeight(30);
  constraintPathSpinBox->setFixedWidth(55);
  constraintPathSpinBox->setValue(3);
  constraintPathSpinBox->setRange(0,100);

  //Brush
  iconname = Settings::globalPath() + "/icons/meshPaint/stylo.png";
  brushButton = new QToolButton;
  brushButton->setIcon(QIcon(iconname.c_str()));
  brushButton->setPopupMode(QToolButton::MenuButtonPopup);
  brushButton->setCheckable(true);
  brushButton->setChecked(false);
  brushButton->setIconSize(QSize(32, 32));
  brushButton->setToolTip(tr("Brush"));
  connect(brushButton, SIGNAL(clicked()), this, SLOT(brush()));
  QMenu *menuBrush = new QMenu(this);
  paintBrushAction = new QAction(QIcon(iconname.c_str()), tr("&paintBrush"),this);
  paintBrushAction->setToolTip(tr("paintBrush"));
  connect(paintBrushAction, SIGNAL(triggered()), this, SLOT(paintBrush()));
  menuBrush->addAction(paintBrushAction);
  iconname = Settings::globalPath() + "/icons/meshPaint/magic_pencil.png";
  magicBrushAction = new QAction(QIcon(iconname.c_str()), tr("&magicBrush"), this);
  magicBrushAction->setToolTip(tr("magicBrush"));
  connect(magicBrushAction, SIGNAL(triggered()), this, SLOT(magicBrush()));
  menuBrush->addAction(magicBrushAction);
  //gomme
  iconname = Settings::globalPath() + "/icons/meshPaint/erase.png";
  eraseAction = new QAction(QIcon(iconname.c_str()), tr("&erase"), this);
  eraseAction->setStatusTip(tr("erase"));
  connect(eraseAction, SIGNAL(triggered()), this, SLOT(erase()));
  menuBrush->addAction(eraseAction);

  brushButton->setMenu(menuBrush);

//
//  iconname = Settings::globalPath() + "/icons/meshPaint/stylo.png";
//  paintBrushAction = new QAction(QIcon(iconname.c_str()), tr("&PaintBrush"),this);
//  paintBrushAction->setStatusTip(tr("PaintBrush"));
//  paintBrushAction->setCheckable(true);
//  connect(paintBrushAction, SIGNAL(triggered()), this, SLOT(paintBrush()));

  //remplissage
  //iconname = Settings::globalPath() + "/icons/meshPaint/fill.png";
  iconname = Settings::globalPath() + "/icons/meshPaint/valide.png";
  fillAction = new QAction(QIcon(iconname.c_str()), tr("&filling"), this);
  fillAction->setStatusTip(tr("fill"));
  connect(fillAction, SIGNAL(triggered()), this, SLOT(filling()));

  //balai
  iconname = Settings::globalPath() + "/icons/meshPaint/clear.png";
  clearAction = new QAction(QIcon(iconname.c_str()), tr("&clear"), this);
  clearAction->setStatusTip(tr("clear"));
  connect(clearAction, SIGNAL(triggered()), this, SLOT(clear()));

  //Sauvegarde
  iconname = Settings::globalPath() + "/icons/meshPaint/sauver.png";
  saveAction = new QAction(QIcon(iconname.c_str()), tr("&Save Texture"), this);
  saveAction->setStatusTip(tr("save"));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

}



void MeshPaint::popAllButtonPaintToolBar()
{
  trackballAction->setChecked(false);
  colorPickerAction->setChecked(false);
  geodesicDistanceAction->setChecked(false);
  eraseAction->setChecked(false);
  selectionAction->setChecked(false);
  pathButton->setChecked(false);
  brushButton->setChecked(false);
}

void MeshPaint::createToolBars()
{
  paintToolBar = addToolBar(tr("PaintToolBar"));
  paintToolBar->setIconSize(QSize(32, 32));

  // zoom
  paintToolBar->addAction(trackballAction);
  paintToolBar->addSeparator();

  //pipette
  paintToolBar->addAction(colorPickerAction);

  paintToolBar->addSeparator();

  //geodesic distance
  paintToolBar->addAction(geodesicDistanceAction);
  //paintToolBar->addWidget(distanceSpinBoxLabel);
  paintToolBar->addWidget(distanceSpinBox);

  paintToolBar->addSeparator();
  //baguette magique
  paintToolBar->addAction(selectionAction);
  //paintToolBar->addWidget(toleranceSpinBoxLabel);
  paintToolBar->addWidget(toleranceSpinBox);

  paintToolBar->addSeparator();
  //plus court chemin
  paintToolBar->addWidget(pathButton);
  //paintToolBar->addWidget(constraintPathSpinBoxLabel);
  paintToolBar->addWidget(constraintPathSpinBox);

  paintToolBar->addSeparator();

  //Brush
  paintToolBar->addWidget(brushButton);
  //paintToolBar->addAction(paintBrushAction);

  // Remplissage
  paintToolBar->addAction(fillAction);

  //Balai
  paintToolBar->addAction(clearAction);
  paintToolBar->addSeparator();

  //Sauvegarde
  paintToolBar->addAction(saveAction);

  infosToolBar = new QToolBar(tr("InfosToolBar"), this);
  infosToolBar->setIconSize(QSize(32, 32));
  addToolBar(Qt::BottomToolBarArea, infosToolBar);
}

template class myMeshPaint<float> ;
template class myMeshPaint<short> ;

