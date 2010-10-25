#include "meshpaint.h"

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

  resize(800, 600);

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
  _mode = 4;

  char sep = FileUtil::separator();

  #ifdef USE_SHARE_CONFIG
    string talref = carto::Paths::globalShared() + sep + BRAINVISA_SHARE_DIRECTORY + sep + "nomenclature" + sep + "surfaceanalysis" + sep
    + "constraint_correspondance.txt";
  #else
    string talref = carto::Paths::shfjShared() + sep + "nomenclature" + sep
        + "surfaceanalysis" + sep + "constraint_correspondance.txt";
  #endif

    cout << "File contraints loaded : " << talref << endl;

  constraintList = new QComboBox( );

  string line;
  ifstream myfile (talref.c_str());
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

  else cout << "Unable to open file " << talref << endl;

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
}

template<typename T>
void myMeshPaint<T>::saveTexture(void)
{
  //cout << "mode = " << mode << endl;
  glWidget->saveTexture();
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
  constraintPathSpinBox->setValue(5);
  constraintPathSpinBox->setRange(0,100);

  //Brush
  iconname = Settings::globalPath() + "/icons/meshPaint/stylo.png";
  paintBrushAction = new QAction(QIcon(iconname.c_str()), tr("&PaintBrush"),this);
  paintBrushAction->setStatusTip(tr("PaintBrush"));
  paintBrushAction->setCheckable(true);
  connect(paintBrushAction, SIGNAL(triggered()), this, SLOT(paintBrush()));

  //remplissage
  iconname = Settings::globalPath() + "/icons/meshPaint/fill.png";
  fillAction = new QAction(QIcon(iconname.c_str()), tr("&filling"), this);
  fillAction->setStatusTip(tr("fill"));
  connect(fillAction, SIGNAL(triggered()), this, SLOT(filling()));

  //gomme
  iconname = Settings::globalPath() + "/icons/meshPaint/clear.png";
  clearAction = new QAction(QIcon(iconname.c_str()), tr("&clear"), this);
  clearAction->setStatusTip(tr("clear"));
  clearAction->setCheckable(true);
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
  paintBrushAction->setChecked(false);
  clearAction->setChecked(false);
  selectionAction->setChecked(false);
  fillAction->setChecked(false);
  pathButton->setChecked(false);
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
  paintToolBar->addAction(paintBrushAction);
  // Remplissage
  paintToolBar->addAction(fillAction);
  //Gomme
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

