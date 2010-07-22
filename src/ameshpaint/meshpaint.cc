#include "meshpaint.h"

template<typename T>
myMeshPaint<T>::myMeshPaint(string adressTexIn,string adressMeshIn,string adressTexOut,string colorMap, string dataType)
	:_adressTexIn(adressTexIn),_adressMeshIn(adressMeshIn),_adressTexOut(adressTexOut),_colorMap(colorMap),_dataType(dataType)
{
  QRect r = geometry();
  r.moveCenter(QApplication::desktop()->availableGeometry().center());
  setGeometry(r);

  resize(640,480);

  glWidget = new myGLWidget<T> (this,adressTexIn,adressMeshIn,adressTexOut,colorMap,dataType);

  setCentralWidget(glWidget);
  glWidget->setFocusPolicy(Qt::StrongFocus);

  QLabel *SpinBoxLabel = new QLabel(tr("Texture Value : "));

  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox = new QDoubleSpinBox;

    textureFloatSpinBox->setSingleStep(0.01);
    textureFloatSpinBox->setFixedHeight(30);
    textureFloatSpinBox->setFixedWidth(75);
    textureFloatSpinBox->setValue(0.1);

    textureSpinBox = static_cast<QDoubleSpinBox*>(textureFloatSpinBox);
    connect(textureFloatSpinBox,SIGNAL(valueChanged(double)),glWidget,SLOT(changeTextureSpinBoxFloat(double)));
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
}

template<typename T>
myMeshPaint<T>::~myMeshPaint()
{
}

MeshPaint::MeshPaint()
{
  createActions();
  createToolBars();
  show();
}

MeshPaint::~MeshPaint()
{
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
void myMeshPaint<T>::keyPressEvent( QKeyEvent* event )
{
  glWidget->keyPressEvent( event );
}

void MeshPaint::createActions()
{
  string iconname = Settings::globalPath() + "/icons/meshPaint/colorpicker.png";

  colorPickerAction = new QAction(QIcon(iconname.c_str()), tr("&ColorPicker"), this);
  colorPickerAction->setShortcut(tr("c"));
  colorPickerAction->setStatusTip(tr("ColorPicker"));
  colorPickerAction->setCheckable(true);
  connect(colorPickerAction, SIGNAL(triggered()), this, SLOT(colorPicker()));

  iconname = Settings::globalPath() + "/icons/meshPaint/paintbrush.png";

  paintBrushAction = new QAction(QIcon(iconname.c_str()), tr("&PaintBrush"), this);
  paintBrushAction->setShortcut(tr("b"));
  paintBrushAction->setStatusTip(tr("PaintBrush"));
  paintBrushAction->setCheckable(true);
  connect(paintBrushAction, SIGNAL(triggered()), this, SLOT(paintBrush()));

  iconname = Settings::globalPath() + "/icons/meshPaint/zoom.png";

  trackballAction = new QAction(QIcon(iconname.c_str()), tr("&trackballAction"), this);
  trackballAction->setShortcut(tr("t"));
  trackballAction->setStatusTip(tr("trackball"));
  trackballAction->setCheckable(true);
  trackballAction->setChecked(true);
  connect(trackballAction, SIGNAL(triggered()), this, SLOT(trackball()));

  iconname = Settings::globalPath() + "/icons/meshPaint/save.png";

  saveAction = new QAction(QIcon(iconname.c_str()), tr("&Save Texture"), this);
  saveAction->setShortcut(tr("s"));
  saveAction->setStatusTip(tr("save"));
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
}

void MeshPaint::createToolBars()
{
  paintToolBar = addToolBar(tr("PaintToolBar"));
  paintToolBar->setIconSize(QSize(32, 32));

  paintToolBar->addAction(trackballAction);
  paintToolBar->addAction(colorPickerAction);
  paintToolBar->addAction(paintBrushAction);
  paintToolBar->addAction(saveAction);

  infosToolBar = new QToolBar( tr( "InfosToolBar" ), this );
  infosToolBar->setIconSize(QSize(32, 32));

  addToolBar(Qt::BottomToolBarArea,infosToolBar);


}

template class myMeshPaint<float>;
template class myMeshPaint<short>;

