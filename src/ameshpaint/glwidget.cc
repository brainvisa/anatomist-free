#include "glwidget.h"
#include "meshpaint.h"

/* enums */
enum
{
  X, Y, Z, W
};

template<typename T>
myGLWidget<T>::myGLWidget(QWidget *parent, string adressTexIn,string adressMeshIn, string adressTexOut, string colorMap,string dataType) :
GLWidget(parent), _adressTexIn(adressTexIn), _adressMeshIn(adressMeshIn),
_adressTexOut(adressTexOut), _colorMap(colorMap), _dataType(dataType)
{
  _zoom = -2.0;
  _trans = 0.0;
  _mode = 1;
  _listMeshPicking = 0;
  _indexVertex = 0;
  _indexPolygon = 0;
  _parcelation = false;
  _wireframe = false;
  _resized = false;
  _parent = parent;
  _showInfos = true;

  backBufferTexture = NULL;

  QDesktopWidget *desktop = QApplication::desktop();

  int screenWidth = desktop->width();
  int screenHeight = desktop->height();

  cout << screenWidth << " " << screenHeight << endl;

  backBufferTexture = (GLubyte*) malloc((screenWidth * screenHeight) * 3
      * sizeof(GLubyte));

  std::cout << "Reading mesh and texture" << endl;

  Reader<AimsSurfaceTriangle> rmeshIn(adressMeshIn);
  rmeshIn.read(_mesh);

  Reader<TimeTexture<T> > rtexIn(adressTexIn);
  rtexIn.read(_tex);

  _trackBall = TrackBall(0.05f, gfx::Vector3f::vector(0, 1, 0),TrackBall::Sphere);
  _trackBall = TrackBall(0.0f, gfx::Vector3f::vector(0, 1, 0),TrackBall::Plane);
  setAutoFillBackground(false);
  setMinimumSize(320, 240);
  setWindowTitle(tr("painting on the mesh"));
  setAutoBufferSwap(false);
  setAcceptDrops(true);
}

template<typename T>
myGLWidget<T>::~myGLWidget()
{
}

template<typename T>
void myGLWidget<T>::unitize(AimsSurfaceTriangle as, Point3df *meshCenter,float *meshScale)
{
  GLfloat maxx, minx, maxy, miny, maxz, minz;
  GLfloat w, h, d;

  const vector<Point3df> & vert = as.vertex();
  /* get the max/mins */
  maxx = minx = vert[0][X];
  maxy = miny = vert[0][Y];
  maxz = minz = vert[0][Z];

  for (int j = 0; j < (int) vert.size(); ++j)
  {
    if (maxx < vert[j][X])
      maxx = vert[j][X];
    if (minx > vert[j][X])
      minx = vert[j][X];
    if (maxy < vert[j][Y])
      maxy = vert[j][Y];
    if (miny > vert[j][Y])
      miny = vert[j][Y];
    if (maxz < vert[j][Z])
      maxz = vert[j][Z];
    if (minz > vert[j][Z])
      minz = vert[j][Z];
  }
  /* calculate mesh width, height, and depth */
  w = fabs(maxx) + fabs(minx);
  h = fabs(maxy) + fabs(miny);
  d = fabs(maxz) + fabs(minz);
  /* calculate center of the mesh */
  (*meshCenter)[X] = (maxx + minx) / 2.0;
  (*meshCenter)[Y] = (maxy + miny) / 2.0;
  (*meshCenter)[Z] = (maxz + minz) / 2.0;
  /* calculate unitizing scale factor */
  *meshScale = 2.0 / max(max(w, h), d);
  /* translate around center then scale */
}

template<typename T>
int myGLWidget<T>::buildDisplayList(AimsSurfaceTriangle as, int mode)
{
  int j;
  GLuint list = 0;
  list = glGenLists(1);
  glNewList(list, GL_COMPILE);

  vector<Point3df> & vert = as.vertex();
  const vector<Point3df> & norm = as.normal();

  vector<AimsVector<uint, 3> > & tri = as.polygon();

  const float* t = _ao->textureCoords();

  for (j = 0; j < (int) vert.size(); ++j)
  {
    vert[j][X] -= (_meshCenter)[X];
    vert[j][Y] -= (_meshCenter)[Y];
    vert[j][Z] -= (_meshCenter)[Z];
    vert[j][X] *= (_meshScale);
    vert[j][Y] *= (_meshScale);
    vert[j][Z] *= (_meshScale);
    //vert[j][Z] *= -1 ;
  }

  glBegin( GL_TRIANGLES);
  for (j = 0; j < (int) tri.size(); ++j)
  {
    if (mode == 2)
    {
      if (t[tri[j][0]] == t[tri[j][1]] && t[tri[j][0]] == t[tri[j][2]])
      {
        glColor3ub((int) dataColorMap[3 * (int) (256 * t[tri[j][0]])],
            (int) dataColorMap[3 * (int) (256 * t[tri[j][0]]) + 1],
            (int) dataColorMap[3 * (int) (256 * t[tri[j][0]]) + 2]);
      }
      else
        glColor3ub(255, 255, 255);
    }

    if (mode == 1)
      glColor3ub(_indexTexture[3 * j], _indexTexture[3 * j + 1],_indexTexture[3 * j + 2]);

    if (mode == 0)
      glColor3ub(255, 255, 255);

    if (mode == 0)
      glTexCoord2d(t[tri[j][0]], 0);
    glNormal3f(norm[tri[j][0]][0], norm[tri[j][0]][1], norm[tri[j][0]][2]);
    glVertex3f(vert[tri[j][0]][0], vert[tri[j][0]][1], vert[tri[j][0]][2]);

    if (mode == 0)
      glTexCoord2d(t[tri[j][1]], 0);
    glNormal3f(norm[tri[j][1]][0], norm[tri[j][1]][1], norm[tri[j][1]][2]);
    glVertex3f(vert[tri[j][1]][0], vert[tri[j][1]][1], vert[tri[j][1]][2]);

    if (mode == 0)
      glTexCoord2d(t[tri[j][2]], 0);
    glNormal3f(norm[tri[j][2]][0], norm[tri[j][2]][1], norm[tri[j][2]][2]);
    glVertex3f(vert[tri[j][2]][0], vert[tri[j][2]][1], vert[tri[j][2]][2]);
  }

  glEnd();
  glEndList();

  cout << "ID list created = " << list << endl;

  //  _qobj_cursor = gluNewQuadric();
  //  gluQuadricDrawStyle(_qobj_cursor, GLU_FILL);
  //  gluQuadricNormals(_qobj_cursor, GLU_SMOOTH);

  return list;
}

template<typename T>
void myGLWidget<T>::buildDataArray(void)
{
  int j;

  vector<Point3df> & vert = _mesh.vertex();
  const vector<Point3df> & norm = _mesh.normal();

  vector<AimsVector<uint, 3> > & tri = _mesh.polygon();

  const float* t = _ao->textureCoords();

  _vertices = (GLfloat*) malloc(3 * vert.size() * sizeof(GLfloat));
  //_textures = (GLfloat*) malloc(vert.size() * sizeof(GLfloat));
  _normals = (GLfloat*) malloc(3 * vert.size() * sizeof(GLfloat));
  _colors = (GLubyte*) malloc(3 * vert.size() * sizeof(GLubyte));

  for (j = 0; j < (int) vert.size(); j++)
  {
    _vertices[3 * j] = (GLfloat)(_meshScale * (vert[j][X] - _meshCenter[X]));
    _vertices[3 * j + 1] = (GLfloat)(_meshScale * (vert[j][Y] - _meshCenter[Y]));
    _vertices[3 * j + 2] = (GLfloat)(_meshScale * (vert[j][Z] - _meshCenter[Z]));

    _normals[3 * j] =  norm[j][X];
    _normals[3 * j + 1] = norm[j][Y];
    _normals[3 * j + 2] = norm[j][Z];

    //_textures[j] = t[j];

    _colors[3 * j] = (int) dataColorMap[3 * (int) (256 * t[j])];
    _colors[3 * j + 1] = (int) dataColorMap[3 * (int) (256 * t[j]) + 1];
    _colors[3 * j + 2] = (int) dataColorMap[3 * (int) (256 * t[j]) + 2];
  }

  _indices = (GLuint*) malloc(3 * tri.size() * sizeof(GLuint));

  for (j = 0; j < (int) tri.size(); ++j)
  {
    _indices[3*j] = (GLuint)tri[j][0];
    _indices[3*j+1] = (GLuint)tri[j][1];
    _indices[3*j+2] = (GLuint)tri[j][2];
  }

//  for (j = 0; j < 20; j++) {
//    cout << "v " << vert[j][0] << "v " << vert[j][1] << "v " << vert[j][2]
//        << endl;
//    cout << "vg " << _vertices[3 * j] << "vg " << _vertices[3 * j + 1]
//        << "vg " << _vertices[3 * j + 2] << endl;
//  }
//
//  for (j = 0; j < 20; j++){
//    cout << j << endl;
//    cout << " " << tri[j][0] << " " << tri[j][1] << " " << tri[j][2] << endl;
//    cout << " " << (int)_indices[3*j]  << " " << (int)_indices[3*j+1]  << " " << (int)_indices[3*j+2] << endl;
//  }
}

template<typename T>
void myGLWidget<T>::setZoom(float z)
{
  _zoom = z;
  updateGL();
}

template<typename T>
void myGLWidget<T>::setTranslate(float t)
{
  _trans = t;
  updateGL();
}

template<typename T>
void myGLWidget<T>::changeTextureValueInt(int value)
{
  float v = (float)(value - _minT)/(float)(_maxT - _minT);
  _colorpicked[0] = (int) dataColorMap[3 * (int) (255 * v)];
  _colorpicked[1] = (int) dataColorMap[3 * (int) (255 * v) + 1];
  _colorpicked[2] = (int) dataColorMap[3 * (int) (255 * v) + 2];

  _textureValue = value;
  //cout << "changeTextureValueInt " << (int) dataColorMap[3 * (int) (255 * v)] << endl;
  updateGL();
}

template<typename T>
void myGLWidget<T>::changeTextureValueFloat(double value)
{
  float v = (float)(value - _minT)/(float)(_maxT - _minT);
  _colorpicked[0] = (int) dataColorMap[3 * (int) (255 * v)];
  _colorpicked[1] = (int) dataColorMap[3 * (int) (255 * v) + 1];
  _colorpicked[2] = (int) dataColorMap[3 * (int) (255 * v) + 2];

  _textureValue = value;

  updateGL();
  //cout << "changeTextureValueFloat " << value << endl;
}

template<typename T>
void myGLWidget<T>::changeIDPolygonValue(int value)
{
  _indexPolygon = value;
  updateGL();
  //cout << "changeIDPolygonValue " << value << endl;
}

template<typename T>
void myGLWidget<T>::changeIDVertexValue(int value)
{
  _indexVertex = value;

 _point3Dpicked[0] = _vertices[3 * _indexVertex];
 _point3Dpicked[1] =  _vertices[3 * _indexVertex + 1];
 _point3Dpicked[2] = _vertices[3 * _indexVertex + 2];

 _vertexNearestpicked[0] = _point3Dpicked[0];
 _vertexNearestpicked[1] = _point3Dpicked[1];
 _vertexNearestpicked[2] = _point3Dpicked[2];

  updateGL();
  //cout << "changeIDVertexValue " << value << endl;
}


template<typename T>
void myGLWidget<T>::saveTexture(void)
{
  cout << "save Texture on disk " << endl;

  typename std::map<int,T>::const_iterator mit(_listVertexSelect.begin()),mend(_listVertexSelect.end());

  TimeTexture<T> out(1,_mesh.vertex().size() );
  for (uint i=0;  i<_mesh.vertex().size(); i++)
  {
    if (_dataType == "FLOAT")
    {
      out[0].item(i)= (float)(_tex[0].item(i));
    }

    if (_dataType == "S16")
    {
      out[0].item(i)=  (int)(_tex[0].item(i));
    }
  }

  for (; mit != mend; ++mit)
  {
  //cout << (int)mit->first << " " << mit->second << endl;
    out[0].item(mit->first) = mit->second;
  }

  Writer< TimeTexture<T> > wt(_adressTexOut);
  wt.write( out );
}

template<typename T>
void myGLWidget<T>::changeMode(int mode)
{
  _mode = mode;
  //cout << "mode = " << mode << endl;

  if (mode == 2 || _mode == 3)
    copyBackBuffer2Texture();
}

template<typename T>
void myGLWidget<T>::initializeGL()
{
  glEnable( GL_LIGHTING);
  glEnable( GL_LIGHT0);
  glEnable( GL_DEPTH_TEST);
  glShadeModel( GL_SMOOTH);
  static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glClearColor(1, 1, 1, 1);

  unitize(_mesh, &_meshCenter, &_meshScale);
  cout << "mesh scale = " << _meshScale << "\n";
  cout << "mesh center = (" << _meshCenter[0] << "," << _meshCenter[1] << "," << _meshCenter[2] << ")\n";

  computeIndexColor( _mesh);

  loadColorMap(_colorMap.c_str());

  cout << "nb triangle = " << _mesh.polygon().size() << endl;
  cout << "nb vertex = " << _mesh.vertex().size() << endl;

  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *>( _parent );

  toolbar->IDPolygonSpinBox->setRange(0, _mesh.polygon().size());
  toolbar->IDVertexSpinBox->setRange(0,_mesh.vertex().size());

  //  _listMeshSmooth = buildDisplayList(_mesh, 0);

  // compute display list for index color rendering
  _listMeshPicking = buildDisplayList(_mesh, 1);

//  _listMeshParcelation = buildDisplayList(_mesh, 2);
//  _listMeshRender = _listMeshSmooth;

  buildDataArray();
}

template<typename T>
void myGLWidget<T>::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasUrls())
  {
    if (event->mimeData()->urls()[0].toString().startsWith("file:///"))
    {
      // Check extension
      QString path = event->mimeData()->urls()[0].toString();
      QString ext = path.mid(path.lastIndexOf(".") + 1);
      //std::cout << ext.toStdString() << ext.compare("gii") << std::endl;
      if (ext.compare("gii") == 0)
      {
        event->acceptProposedAction();
      }
    }
  }
}

template<typename T>
void myGLWidget<T>::dragMoveEvent(QDragMoveEvent *event)
{
  if (event->mimeData()->hasText())
  {
    std::cout << "drag move" << std::endl;
  }
  event->accept();
}

template<typename T>
void myGLWidget<T>::dropEvent(QDropEvent *event)
{
  QString fName;

  if (event->mimeData()->hasUrls())
  {
    if (event->mimeData()->urls()[0].toString().startsWith("file:///"))
    {
      // Check extension
      QString path = event->mimeData()->urls()[0].toString();
      QString ext = path.mid(path.lastIndexOf(".") + 1);
      if (ext.compare("gii") == 0)
      {
        fName = event->mimeData()->urls()[0].toLocalFile();
        std::cout << fName.toStdString() << std::endl;
      }
    }
  }
}

template<typename T>
QPointF myGLWidget<T>::pixelPosToViewPos(const QPointF& p)
{
  return QPointF(2.0 * float(p.x()) / width() - 1.0, 1.0 - 2.0 * float(p.y()) / height());
}

template<typename T>
Point3df myGLWidget<T>::check3DpointPicked(int x, int y)
{
  Point3df p;

  drawScenetoBackBuffer();
  glReadBuffer( GL_BACK);
  glFinish();

  projectionPerspective();
  trackBallTransformation();

  GLdouble modelview[4 * 4];
  GLdouble projection[4 * 4];
  GLint viewport[4];
  GLdouble meshx = 0.0;
  GLdouble meshy = 0.0;
  GLdouble meshz = 0.0;
  GLfloat wz = 0.0;
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  glReadPixels(x, viewport[3] - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &wz);

  gluUnProject((GLdouble) x, (GLdouble)(viewport[3] - y), (GLdouble) wz,
    modelview, projection, viewport, &meshx, &meshy, &meshz);

  p[0] = meshx;
  p[1] = meshy;
  p[2] = meshz;

  //  _point3Dpicked[0] = _meshCenter[0] + (float)meshx/_meshScale;
  //  _point3Dpicked[1] = _meshCenter[1] + (float)meshy/_meshScale;
  //  _point3Dpicked[2] = _meshCenter[2] + (float)meshz/_meshScale;

  return p;
}

template<typename T>
GLuint myGLWidget<T>::checkIDpolygonPicked(int x, int y)
{
  GLuint r, g, b;
  r = backBufferTexture[3 * (height() - y) * width() + 3 * x];
  g = backBufferTexture[3 * (height() - y) * width() + 3 * x + 1];
  b = backBufferTexture[3 * (height() - y) * width() + 3 * x + 2];

  return (GLuint)(b + 256 * g + 256 * 256 * r);
}

template<typename T>
void myGLWidget<T>::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::LeftButton & _mode == 1)
  {
    _trackBall.start();
    _trackBall.push(pixelPosToViewPos(event->pos()),
      gfx::Quaternionf::identity());
    event->accept();
    updateGL();
  }

  if (event->buttons() == Qt::LeftButton & _mode == 2)
  {
    if (_resized)
      copyBackBuffer2Texture();

    int indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    _point3Dpicked = check3DpointPicked(event->x(), event->y());
    _trackBall.stop();
  }
  if (_mode == 3)
  {
    _trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    _point3Dpicked = check3DpointPicked(event->x(), event->y());

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) _point3Dpicked[2] / _meshScale;

    _indexVertex = computeNearestVertexFromPolygonPoint(p,_indexPolygon, _mesh);

    if (_indexVertex >= 0 && _indexVertex < 3*_mesh.vertex().size())
    {
      _colors[3 * _indexVertex] = _colorpicked[0];
      _colors[3 * _indexVertex + 1] = _colorpicked[1];
      _colors[3 * _indexVertex + 2] = _colorpicked[2];

      _listVertexSelect[_indexVertex] = _textureValue;

      updateInfosPicking(_indexPolygon,_indexVertex);
    }

    updateGL();
  }
}

template<typename T>
void myGLWidget<T>::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->isAccepted())
    return;

  if (event->buttons() & Qt::LeftButton & _mode == 1)
  {
    _trackBall.release(pixelPosToViewPos(event->pos()),gfx::Quaternionf::identity());
    event->accept();
    updateGL();
  }

  if (event->buttons() & Qt::LeftButton & _mode == 2)
  {
    event->accept();
    updateGL();
  }
}

template<typename T>
void myGLWidget<T>::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::LeftButton && _mode == 1)
  {
    _trackBall.move(pixelPosToViewPos(event->pos()),gfx::Quaternionf::identity());
    event->accept();
    updateGL();
  }

  if (_mode == 2)
  {
    _trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    //cout << "ID polygon : " << _indexPolygon << endl;
    _point3Dpicked = check3DpointPicked(event->x(), event->y());
    //cout << "3D : " << _point3Dpicked[0] << " " << _point3Dpicked[1] << " " << _point3Dpicked[2] << " " << endl;

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) _point3Dpicked[2] / _meshScale;

    _indexVertex = computeNearestVertexFromPolygonPoint(p, _indexPolygon,_mesh);

    //cout << "3D coord vertex value = " << _vertexNearestpicked[X] << " " << _vertexNearestpicked[Y] << " " << _vertexNearestpicked[Z] << "\n" ;
    const float* t = _ao->textureCoords();
    if (_indexVertex < _mesh.vertex().size())
    {
//      _colorpicked[0] = (int) dataColorMap[3 * (int) (256
//          * t[_indexVertex])];
//      _colorpicked[1] = (int) dataColorMap[3 * (int) (256
//          * t[_indexVertex]) + 1];
//      _colorpicked[2] = (int) dataColorMap[3 * (int) (256
//          * t[_indexVertex]) + 2];
      _textureValue = _tex[0].item(_indexVertex);

      typename std::map<int,T>::const_iterator it(_listVertexSelect.find(_indexVertex));

      if (it != _listVertexSelect.end())
        _textureValue = _listVertexSelect[_indexVertex];

      _colorpicked[0] = _colors[3 * _indexVertex];
      _colorpicked[1] = _colors[3 * _indexVertex + 1];
      _colorpicked[2] = _colors[3 * _indexVertex + 2];

      changeTextureValue(_textureValue);
      updateInfosPicking(_indexPolygon,_indexVertex);
      //cout << "texture value " << _textureValue << endl;

    }
    else
    {
      _textureValue = 0;
      _colorpicked[0] = dataColorMap[0];
      _colorpicked[1] = dataColorMap[1];
      _colorpicked[2] = dataColorMap[2];
    }

    updateGL();
  }

  if (_mode == 3)
  {
    _trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    _point3Dpicked = check3DpointPicked(event->x(), event->y());

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) _point3Dpicked[2] / _meshScale;

    _indexVertex = computeNearestVertexFromPolygonPoint(p,_indexPolygon, _mesh);

    if (_indexVertex >= 0 && _indexVertex < 3*_mesh.vertex().size())
    {
      _colors[3 * _indexVertex] = _colorpicked[0];
      _colors[3 * _indexVertex + 1] = _colorpicked[1];
      _colors[3 * _indexVertex + 2] = _colorpicked[2];

      _listVertexSelect[_indexVertex] = _textureValue;
      updateInfosPicking(_indexPolygon,_indexVertex);
    }

    updateGL();
  }
}

template<typename T>
void myGLWidget<T>::keyPressEvent(QKeyEvent *event)
{
typename std::map<int,T>::const_iterator mit(_listVertexSelect.begin()),mend(_listVertexSelect.end());
const float* t = _ao->textureCoords();

switch (event->key())
{
  case Qt::Key_Plus:
    break;
  case Qt::Key_Minus:
    break;
  case Qt::Key_Left:
    break;
  case Qt::Key_Right:
    break;
  case Qt::Key_Down:
    break;
  case Qt::Key_Up:
    break;
  case Qt::Key_I:
    _showInfos = !_showInfos;
    cout << "show infos\n";
    break;
  case Qt::Key_W:
    _wireframe = !_wireframe;
    break;
  case Qt::Key_P:
    _parcelation = !_parcelation;
    break;
  case Qt::Key_Space:
    cout << "clear all vertex painted\n";
    //_listTriangleSelect.clear();
    for (; mit != mend; ++mit)
    {
      _colors[3 * mit->first] = (int) dataColorMap[3 * (int) (255 * t[mit->first])];
      _colors[3 * mit->first+1] = (int) dataColorMap[3 * (int) (255 * t[mit->first]) + 1];
      _colors[3 * mit->first+2] = (int) dataColorMap[3 * (int) (255 * t[mit->first]) + 2];
    }

    _listVertexSelect.clear();
    break;
  default:
    QWidget::keyPressEvent(event);
  }
  updateGL();
}

template<typename T>
void myGLWidget<T>::wheelEvent(QWheelEvent *event)
{
  int numDegrees = event->delta() / 8;
  float numSteps = (float) (numDegrees / 300.);

  if (_mode == 1)
  {
    _trackBall.push(pixelPosToViewPos(event->pos()),gfx::Quaternionf::identity());

    if (event->orientation() == Qt::Horizontal)
    {
      setTranslate(_trans + numSteps);
    }
    else
    {
      setZoom(_zoom + numSteps);
    }
    event->accept();
  }
}

template<typename T>
void myGLWidget<T>::drawColorMap(void)
{
  glPushAttrib( GL_ALL_ATTRIB_BITS);

  glDisable( GL_LIGHTING);
  glEnable( GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel( GL_FLAT);

  glDisable( GL_DEPTH_TEST);

  glBegin( GL_QUADS);
  glColor4f(1, 1, 1, 0.75);
  glVertex2d(5, 5);
  glVertex2d(105, 5);
  glVertex2d(105, 50);
  glVertex2d(5, 50);
  glEnd();

  glBegin(GL_QUADS);

  //  glColor3ub((int)dataColorMap[3*(int)(256*t[_indexVertex])],
  //                 (int)dataColorMap[3*(int)(256*t[_indexVertex])+1],
  //                 (int)dataColorMap[3*(int)(256*t[_indexVertex])+2]);
  //
  glColor3ub(_colorpicked[0], _colorpicked[1], _colorpicked[2]);

  glVertex2d(10,25);
  glVertex2d(10,35);
  glVertex2d(100,35);
  glVertex2d(100,25);
  glEnd();

  glColor3f(1, 1, 1);
  glEnable( GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, _IDcolorMap);

  glBegin(GL_QUADS);
  glColor4f(1, 1, 1, 0.75);
  glTexCoord2d(0, 0);
  glVertex2d(10, 10);
  glTexCoord2d(0, 0);
  glVertex2d(10, 20);
  glTexCoord2d(1, 0);
  glVertex2d(100, 20);
  glTexCoord2d(1, 0);
  glVertex2d(100, 10);
  glEnd();

  glPopAttrib();
}

template<typename T>
void myGLWidget<T>::drawTexturePaint(void)
{
  glPushAttrib( GL_ALL_ATTRIB_BITS);

//  std::map<int, int>::const_iterator mit(_listVertexSelect.begin()),
//      mend(_listVertexSelect.end());

//  vector<Point3df> & vert = _mesh.vertex();
//  const vector<Point3df> & norm = _mesh.normal();
//  vector<AimsVector<uint, 3> > & tri = _mesh.polygon();

//  int v1, v2, v3;
//
//  glBegin( GL_TRIANGLES);
//
//  for (; mit != mend; ++mit) {
//    glColor3ub(mit->second[0], mit->second[1], mit->second[2]);
//
//    glNormal3f(norm[tri[mit->first][0]][0], norm[tri[mit->first][0]][1],
//        norm[tri[mit->first][0]][2]);
//    glVertex3f(_meshScale * (vert[tri[mit->first][0]][0] - _meshCenter[0]),
//        _meshScale * (vert[tri[mit->first][0]][1] - _meshCenter[1]),
//        _meshScale * (vert[tri[mit->first][0]][2] - _meshCenter[2]));
//
//    //vert[tri[mit->first][0]][0], vert[tri[mit->first][0]][1], vert[tri[mit->first][0]][2]);
//    glNormal3f(norm[tri[mit->first][1]][0], norm[tri[mit->first][1]][1],
//        norm[tri[mit->first][1]][2]);
//    glVertex3f(_meshScale * (vert[tri[mit->first][1]][0] - _meshCenter[0]),
//        _meshScale * (vert[tri[mit->first][1]][1] - _meshCenter[1]),
//        _meshScale * (vert[tri[mit->first][1]][2] - _meshCenter[2]));
//
//    //glVertex3f(vert[tri[mit->first][1]][0], vert[tri[mit->first][1]][1], vert[tri[mit->first][1]][2]);
//    glNormal3f(norm[tri[mit->first][2]][0], norm[tri[mit->first][2]][1],
//        norm[tri[mit->first][2]][2]);
//    glVertex3f(_meshScale * (vert[tri[mit->first][2]][0] - _meshCenter[0]),
//        _meshScale * (vert[tri[mit->first][2]][1] - _meshCenter[1]),
//        _meshScale * (vert[tri[mit->first][2]][2] - _meshCenter[2]));
//  }
//  glEnd();

  glPopAttrib();
}

template<typename T>
void myGLWidget<T>::drawPrimitivePicked(void)
{
  glPushAttrib( GL_ALL_ATTRIB_BITS);

  glDisable( GL_TEXTURE_2D);
  glEnable( GL_COLOR_MATERIAL);
  glEnable( GL_LIGHTING);

  GLUquadricObj *quadric;
  quadric = gluNewQuadric();
  gluQuadricDrawStyle(quadric, GLU_FILL);

  glColor3d(1, 0, 0);
  glTranslatef(_point3Dpicked[0], _point3Dpicked[1], _point3Dpicked[2]);
  gluSphere(quadric, 0.001, 36, 18);
  glTranslatef(-_point3Dpicked[0], -_point3Dpicked[1], -_point3Dpicked[2]);

  glColor3d(0, 0, 1);
  glTranslatef(_vertexNearestpicked[0], _vertexNearestpicked[1],
      _vertexNearestpicked[2]);
  gluSphere(quadric, 0.001, 36, 18);
  glTranslatef(-_vertexNearestpicked[0], -_vertexNearestpicked[1],
      -_vertexNearestpicked[2]);

  gluDeleteQuadric(quadric);
  //printf("angle %f ",(float)(180.*acos(model->facetnorms[3*triangle->findex + 2]))/M_PI);
  //glRotatef(acos(model->facetnorms[3*triangle->findex + 2]),1,0,0);
  //glutSolidSphere(0.001, 15, 15);

  //     glMultMatrixf(matrice_rotation);
  //
  //     glColor3d(1,1,0);
  //     gluCylinder(qobj_cone,0,0.002,0.004,20,1);
  //     glColor3d(0,1,1);
  //     gluCylinder(qobj_cone,0,0.001,0.01,20,1);
  //
  //
  //  glPushMatrix();
  //
  //
  //  glPopMatrix();

  if (_indexPolygon >= 0 && _indexPolygon < 3*_mesh.polygon().size())
  {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable( GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LINE_SMOOTH);
  //glDisable(GL_COLOR_MATERIAL);
  glDisable( GL_LIGHTING);

  glBegin( GL_TRIANGLES);

  glColor3ub(255 - _colorpicked[0],255 - _colorpicked[1],255 - _colorpicked[2]);

  glVertex3f( (_vertices[3 * _indices[3 * _indexPolygon]]), (_vertices[3 * _indices[3
      * _indexPolygon] + 1]), (_vertices[3
      * _indices[3 * _indexPolygon] + 2]));


  glVertex3f((_vertices[3 * _indices[3 * _indexPolygon + 1]]), (_vertices[3 * _indices[3
      * _indexPolygon + 1] + 1]), (_vertices[3 * _indices[3 * _indexPolygon + 1] + 2]));


  glVertex3f( (_vertices[3 * _indices[3 * _indexPolygon + 2]]), (_vertices[3 * _indices[3
      * _indexPolygon + 2] + 1] ), (_vertices[3 * _indices[3 * _indexPolygon + 2] + 2]));
  glEnd();

  }

  glPopAttrib();
}

template<typename T>
void myGLWidget<T>::projectionOrtho()
{
  glMatrixMode( GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, width(), 0, height(), -1, 1);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

template<typename T>
void myGLWidget<T>::projectionPerspective()
{
  float ratio = (float) width() / (float) height();

  //cout << "ratio = " << ratio << endl;

  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, ratio, 0.01, 10);
  glMatrixMode( GL_MODELVIEW);
  glLoadIdentity();
}

template<typename T>
void myGLWidget<T>::trackBallTransformation(void)
{
  glTranslatef(_trans, 0.0, _zoom);
  gfx::Matrix4x4f m;
  _trackBall.rotation().matrix(m);
  glMultMatrixf(m.bits());
}

template<typename T>
void myGLWidget<T>::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  projectionPerspective();

  glEnable( GL_DEPTH_TEST);
  glDisable( GL_TEXTURE_2D);

  trackBallTransformation();

  glEnableClientState( GL_NORMAL_ARRAY);
  glEnableClientState( GL_COLOR_ARRAY);
  glEnableClientState( GL_VERTEX_ARRAY);
  glNormalPointer(GL_FLOAT, 0, _normals);
  glColorPointer(3, GL_UNSIGNED_BYTE, 0, _colors);
  glVertexPointer(3, GL_FLOAT, 0, _vertices);

  //  if (_listMeshRender == _listMeshSmooth) {
  //    glEnable( GL_TEXTURE_2D);
  //  }
  //
  //  if (_listMeshRender == _listMeshParcelation) {
  //    glDisable( GL_TEXTURE_2D);
  //  }

  glEnable ( GL_POLYGON_OFFSET_FILL);
  glEnable( GL_COLOR_MATERIAL);
  glPolygonOffset(1., 1.);

  glDrawElements(GL_TRIANGLES, 3 * _mesh.polygon().size(), GL_UNSIGNED_INT,_indices);

  glDisable(GL_POLYGON_OFFSET_FILL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable( GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LINE_SMOOTH);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);

  if (_wireframe)
    glDrawElements(GL_TRIANGLES, 3 * _mesh.polygon().size(),GL_UNSIGNED_INT, _indices);

  //  glCallList( _listMeshRender);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);

  glDisableClientState(GL_VERTEX_ARRAY); // disable vertex arrays
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  drawPrimitivePicked();

  if (_showInfos)
  {
    //glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    projectionOrtho();
    drawColorMap();
    glPopMatrix();

    QPainter painter;
    painter.begin(this);
    QString framesPerSecond;
    framesPerSecond.setNum(_frames / (_time.elapsed() / 1000.0), 'f', 2);
    painter.setPen(Qt::black);
    painter.drawText(10, height() - 40, framesPerSecond + " fps");

    //    QString nbPolygon;
    //    nbPolygon.setNum((int) _indexPolygon, 10);
    //
    //    QString indexVertex;
    //    indexVertex.setNum((int) _indexVertex, 10);
    //
    //    QString textureValue;
    //    if (_indexVertex < _mesh.vertex().size()) {
    //      if (_dataType == "FLOAT")
    //        textureValue.setNum((float) _tex[0].item((int) _indexVertex), 'f',
    //            4);
    //      if (_dataType == "S16")
    //        textureValue.setNum((int) _tex[0].item((int) _indexVertex), 10);
    //    } else {
    //      indexVertex.setNum(0, 1);
    //      indexPolygon.setNum(0, 1);
    //      textureValue.setNum(0, 1);
    //    }
    //
    //    painter.drawText(30, height() - 80, "polygon = " + indexPolygon);
    //    painter.drawText(30, height() - 60, "vertex = " + indexVertex);
    //    painter.drawText(30, height() - 40, "texture = " + textureValue);

    painter.end();

    if (!(_frames % 100))
    {
      _time.start();
      _frames = 0;
    }

    _frames++;
  }

  swapBuffers();
}

template<typename T>
void myGLWidget<T>::resizeGL(int width, int height)
{
  //cout << "w " << width << "h " << height << endl;
  _resized = true;
  _mode = 1;
  setupViewport(width, height);
}

template<typename T>
void myGLWidget<T>::setupViewport(int width, int height)
{
  glViewport(0, 0, width, height);
}

template<typename T>
int myGLWidget<T>::computeNearestVertexFromPolygonPoint(Point3df position,int poly, AimsSurfaceTriangle as)
{
  int index_nearest_vertex, index_min = 0;
  Point3df pt[3];
  uint v[3];

  const vector<Point3df> & vert = as.vertex();
  vector<AimsVector<uint, 3> > & tri = as.polygon();

  if (poly < (int) tri.size() && poly > 0)
  {
    v[0] = tri[poly][0];
    v[1] = tri[poly][1];
    v[2] = tri[poly][2];

    pt[0] = vert[v[0]];
    pt[1] = vert[v[1]];
    pt[2] = vert[v[2]];

    //cout << "nb poly= " << tri.size() << endl;

    //compute the nearest polygon vertex
    float min, dist_min = FLT_MAX;

    for (int i = 0; i < 3; i++)
    {
      min = (float) sqrt((position[0] - pt[i][0]) * (position[0]
          - pt[i][0]) + (position[1] - pt[i][1]) * (position[1]
          - pt[i][1]) + (position[2] - pt[i][2]) * (position[2]
          - pt[i][2]));

      if (min < dist_min)
      {
        dist_min = min;
        index_min = i;
      }
    }
  }

  index_nearest_vertex = v[index_min];
  _vertexNearestpicked = pt[index_min];
  _vertexNearestpicked[X] -= (_meshCenter)[X];
  _vertexNearestpicked[Y] -= (_meshCenter)[Y];
  _vertexNearestpicked[Z] -= (_meshCenter)[Z];
  _vertexNearestpicked[X] *= (_meshScale);
  _vertexNearestpicked[Y] *= (_meshScale);
  _vertexNearestpicked[Z] *= (_meshScale);

  //cout << "3D coord vertex value = " << _vertexNearestpicked[X] << " " << _vertexNearestpicked[Y] << " " << _vertexNearestpicked[Z] << "\n" ;

  return index_nearest_vertex;
}

template<typename T>
void myGLWidget<T>::computeIndexColor(AimsSurfaceTriangle as)
{
  int i;
  int r, g, b;

  _indexTexture.clear();

  for (i = 0; i < (int) (as.polygon().size()); i++)
  {
    r = (i / 256) / 256;
    g = (i / 256) % 256;
    b = i % 256;
    _indexTexture.push_back(r);
    _indexTexture.push_back(g);
    _indexTexture.push_back(b);
  }
}

template<typename T>
void myGLWidget<T>::drawScenetoBackBuffer(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  projectionPerspective();
  trackBallTransformation();

  glPushAttrib( GL_ALL_ATTRIB_BITS);

  glEnable( GL_DEPTH_TEST);
  glDisable( GL_LIGHTING);
  glEnable( GL_COLOR_MATERIAL);
  glDisable( GL_TEXTURE_2D);

  if (_listMeshPicking != 0)
    glCallList( _listMeshPicking);

  glPopAttrib();
}

template<typename T>
void myGLWidget<T>::copyBackBuffer2Texture(void)
{
  drawScenetoBackBuffer();
  glFinish();
  //glFlush();
  glReadBuffer( GL_BACK);
  glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE,backBufferTexture);
  _resized = false;
}

template<typename T>
void myGLWidget<T>::changeTextureValue(T value)
{
  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *>( _parent );
  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox = dynamic_cast<QDoubleSpinBox *>( toolbar->textureSpinBox );
    textureFloatSpinBox->setValue(value);
  }

  if (_dataType == "S16")
  {
    QSpinBox *textureIntSpinBox = dynamic_cast<QSpinBox *>( toolbar->textureSpinBox );
    textureIntSpinBox->setValue(value);
  }
}

template<typename T>
void myGLWidget<T>::updateInfosPicking(int idp, int idv)
{
  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *>( _parent );
  toolbar->IDPolygonSpinBox->setValue(idp);
  toolbar->IDVertexSpinBox->setValue(idv);
}

template<typename T>
GLuint myGLWidget<T>::loadColorMap(const char * filename)
{
  int i;
  int width, height;
  unsigned char* data;

  FILE * file;
  char textureName[256];

  const Path & p = Path::singleton();
  char s = FileUtil::separator();

  string path = p.globalShared() + s + "aims-"
      + carto::cartobaseShortVersion() + s + "Rgb" + s + filename;

  cout << "colorMap : " << path << endl;

  file = fopen(path.c_str(), "rb");

  if (file == NULL)
    return 0;

  // read texture data
  fscanf(file, "%s\n%d\nRed\n", &textureName, &width);

  height = 1;
  data = (unsigned char *) malloc(width * height * 3 * sizeof(unsigned char));
  dataColorMap = (unsigned char *) malloc(width * height * 3
      * sizeof(unsigned char));

  for (i = 0; i < width; i++)
    fscanf(file, "%d ", &data[i]);

  fscanf(file, "\nGreen\n");

  for (i = 0; i < width; i++)
    fscanf(file, "%d ", &data[256 + i]);

  fscanf(file, "\nBlue\n");
  for (i = 0; i < width; i++)
    fscanf(file, "%d ", &data[256 + 256 + i]);

  for (i = 0; i < width; i++)
  {
    dataColorMap[3 * i] = data[i];
    dataColorMap[3 * i + 1] = data[256 + i];
    dataColorMap[3 * i + 2] = data[512 + i];
  }

  fclose(file);

  // allocate a texture name
  glGenTextures(1, &_IDcolorMap);

  // select our current texture
  glBindTexture(GL_TEXTURE_2D, _IDcolorMap);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // select modulate to mix texture with color for shading
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, 1, 0, GL_RGB,
      GL_UNSIGNED_BYTE, dataColorMap);

  rc_ptr<Texture1d> tex(new Texture1d);
  Converter<TimeTexture<T> , Texture1d> c;
  c.convert(_tex, *tex);
  _ao = new ATexture;
  _ao->setTexture(tex);

  const GLComponent::TexExtrema  & te = _ao->glTexExtrema();
  T min = te.minquant[0];
  T max = te.maxquant[0];

  _minT = min;
  _maxT = max;

  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *>( _parent );

  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox = dynamic_cast<QDoubleSpinBox *>( toolbar->textureSpinBox );
    textureFloatSpinBox->setRange(min,max);
  }

  if (_dataType == "S16")
  {
    QSpinBox *textureIntSpinBox = dynamic_cast<QSpinBox *>( toolbar->textureSpinBox );
    textureIntSpinBox->setRange(min,max);
  }

  _ao->normalize();

  _colorpicked[0] = dataColorMap[0];
  _colorpicked[1] = dataColorMap[1];
  _colorpicked[2] = dataColorMap[2];

  return 1;
}

template class myGLWidget<float> ;
template class myGLWidget<short> ;
