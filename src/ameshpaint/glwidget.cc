#include "glwidget.h"
#include "meshpaint.h"
#include <aims/geodesicpath/geodesic_algorithm_dijkstra.h>
#include <aims/geodesicpath/geodesic_algorithm_subdivision.h>
#include <aims/geodesicpath/geodesic_algorithm_exact.h>

/* enums */
enum
{
  X, Y, Z, W
};

template<typename T>
myGLWidget<T>::myGLWidget(QWidget *parent, string adressTexIn,
    string adressMeshIn, string adressTexCurvIn, string adressTexOut, string colorMap, string dataType) :
  GLWidget(parent), _adressTexIn(adressTexIn),_adressTexCurvIn(adressTexCurvIn), _adressMeshIn(adressMeshIn),
      _adressTexOut(adressTexOut), _colorMap(colorMap), _dataType(dataType)
{
  _zoom = -2.0;
  _trans = 0.0;
  _mode = 1;
  _listMeshPicking = 0;
  _indexVertex = 0;
  _indexPolygon = 0;
  _textureValue = 0;
  _texCurvDisplay = false;
  _wireframe = false;
  _resized = false;
  _parent = parent;
  _showInfos = true;
  _constraintPathValue = 3;

  backBufferTexture.resize( parent->width() * parent->height() * 3 );

  cout << "load mesh : " << adressMeshIn;
  Reader<AimsSurfaceTriangle> rmeshIn(adressMeshIn);
  rmeshIn.read(_mesh);
  cout << " OK" << endl;

  cout << "compute surface neighbours : " << endl;
  neighbours = SurfaceManip::surfaceNeighbours(_mesh);
  cout << " OK" << endl;

  if (adressTexIn.length()!=0)
  {
    cout << "load texture : " << adressTexIn;
    Reader<TimeTexture<T> > rtexIn(adressTexIn);
    rtexIn.read(_tex);
    std::cout << " OK" << endl;
  }
  else
  {
    cout << "create texture : " << adressTexIn;
    _tex = TimeTexture<T> (1,_mesh.vertex().size() );
    for (uint i = 0; i < _mesh.vertex().size(); i++)
      _tex[0].item(i) = 0;
    cout << " OK" << endl;
  }

  if (adressTexCurvIn.length()!=0)
  {
    cout << "load texture curvature : " << adressTexCurvIn;
    Reader<TimeTexture<float> > rtexCurvIn(adressTexCurvIn);
    rtexCurvIn.read(_texCurv);
    cout << " OK" << endl;
  }
  else
  {
    //calcul de la courbure
    cout << "compute texture curvature : ";

    //TimeTexture<float> AimsMeshCurvature( const AimsSurface<3,Void> & mesh);

    _texCurv = TimeTexture<float>(1, _mesh.vertex().size());

    //const AimsSurface<3,Void>   & surface = _mesh[0];
    _texCurv = AimsMeshCurvature(_mesh[0]);
//
//    CurvatureFactory CF;
//    Curvature *curvat = CF.createCurvature(_mesh,"barycenter");
//    _texCurv[0] = curvat->doIt();
//    curvat->regularize(_texCurv[0],1);
//    curvat->getTex  tureProperties(_texCurv[0]);
//    delete curvat;

//    CurvatureFactory CF;
//    Curvature * curv = CF.createCurvature(surface,"fem");
//    cout << "processing..." << flush;
//

    //_texCurv = AimsMeshCurvature (surface);

//    float nx, ny, nz, ix, iy, iz, vx, vy , vz;
//    int Ni;
//    for (uint i = 0; i < _mesh.vertex().size(); i++)
//    {
//      std::set<uint> voisins=neighbours[i];
//      std::set<uint>::iterator voisIt=voisins.begin();
//      voisIt=voisins.begin();
//
//      Ni = voisins.size();
//
//      float nix, niy, niz;
//      float vix, viy, viz;
//      float nvix, nviy, nviz;
//      float ci;
//
//      ci = 0;
//      nix = 0; niy = 0 ; niz = 0;
//
//      ix = ((_mesh.vertex())[i])[0];
//      iy = ((_mesh.vertex())[i])[1];
//      iz = ((_mesh.vertex())[i])[2];
//
//      for ( ; voisIt != voisins.end(); voisIt++)
//      {
//        nix += ((_mesh.normal())[*voisIt])[0];
//        niy += ((_mesh.normal())[*voisIt])[1];
//        niz += ((_mesh.normal())[*voisIt])[2];
//      }
//
//      nix = (float)nix/Ni;
//      niy = (float)niy/Ni;
//      niz = (float)niz/Ni;
//
//      voisIt=voisins.begin();
//
//      for ( ; voisIt != voisins.end(); voisIt++)
//      {
//        vx=((_mesh.vertex())[*voisIt])[0];
//        vy=((_mesh.vertex())[*voisIt])[1];
//        vz=((_mesh.vertex())[*voisIt])[2];
//        vix = ix - vx;
//        viy = iy - vy;
//        viz = iz - vz;
//        nvix = sqrt(vix*vix);
//        nviy = sqrt(viy*viy);
//        nviz = sqrt(viz*viz);
//        ci += (float)(nix*vix + niy*viy + niz*viz)/(nvix + nviy + nviz);
//      }
//    _texCurv[0].item(i) = (float)ci/Ni;
//    }

    //fin calcul de la courbure
    cout << " OK" << endl;
  }

  _trackBall = TrackBall(0.5f, gfx::Vector3f::vector(0, 1, 0),
      TrackBall::Sphere);
  //_trackBall = TrackBall(0.0f, gfx::Vector3f::vector(0, 1, 0),TrackBall::Plane);
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
void myGLWidget<T>::unitize(AimsSurfaceTriangle as, Point3df *meshCenter,
    float *meshScale)
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
    if (maxx < vert[j][X]) maxx = vert[j][X];
    if (minx > vert[j][X]) minx = vert[j][X];
    if (maxy < vert[j][Y]) maxy = vert[j][Y];
    if (miny > vert[j][Y]) miny = vert[j][Y];
    if (maxz < vert[j][Z]) maxz = vert[j][Z];
    if (minz > vert[j][Z]) minz = vert[j][Z];
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

  const float* t = _aTex->textureCoords();

  for (j = 0; j < (int) vert.size(); ++j)
  {
    vert[j][X] -= (_meshCenter)[X];
    vert[j][Y] -= (_meshCenter)[Y];
    vert[j][Z] -= (_meshCenter)[Z];
    vert[j][X] *= (_meshScale);
    vert[j][Y] *= (_meshScale);
    vert[j][Z] *= (_meshScale);
    vert[j][Z] *= -1;
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

    if (mode == 1) glColor3ub(_indexTexture[3 * j], _indexTexture[3 * j + 1],
        _indexTexture[3 * j + 2]);

    if (mode == 0) glColor3ub(255, 255, 255);

    if (mode == 0) glTexCoord2d(t[tri[j][0]], 0);
    glNormal3f(norm[tri[j][0]][0], norm[tri[j][0]][1], norm[tri[j][0]][2]);
    glVertex3f(vert[tri[j][0]][0], vert[tri[j][0]][1], vert[tri[j][0]][2]);

    if (mode == 0) glTexCoord2d(t[tri[j][1]], 0);
    glNormal3f(norm[tri[j][1]][0], norm[tri[j][1]][1], norm[tri[j][1]][2]);
    glVertex3f(vert[tri[j][1]][0], vert[tri[j][1]][1], vert[tri[j][1]][2]);

    if (mode == 0) glTexCoord2d(t[tri[j][2]], 0);
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

  const float* t = _aTex->textureCoords();
  const float* tcurv = _aTexCurv->textureCoords();

  _vertices = (GLfloat*) malloc(3 * vert.size() * sizeof(GLfloat));
  //_textures = (GLfloat*) malloc(vert.size() * sizeof(GLfloat));
  _normals = (GLfloat*) malloc(3 * vert.size() * sizeof(GLfloat));
  _colors = (GLubyte*) malloc(3 * vert.size() * sizeof(GLubyte));

  _colorsCurv = (GLubyte*) malloc(3 * vert.size() * sizeof(GLubyte));

  for (j = 0; j < (int) vert.size(); j++)
  {
    _vertices[3 * j] = (GLfloat)(_meshScale * (vert[j][X] - _meshCenter[X]));
    _vertices[3 * j + 1]
        = (GLfloat)(_meshScale * (vert[j][Y] - _meshCenter[Y]));
    _vertices[3 * j + 2] = -(GLfloat)(_meshScale
        * (vert[j][Z] - _meshCenter[Z]));

    _normals[3 * j] = norm[j][X];
    _normals[3 * j + 1] = norm[j][Y];
    _normals[3 * j + 2] = -norm[j][Z];

    _colors[3 * j] = (int) dataColorMap[3 * (int) (256 * t[j])];
    _colors[3 * j + 1] = (int) dataColorMap[3 * (int) (256 * t[j]) + 1];
    _colors[3 * j + 2] = (int) dataColorMap[3 * (int) (256 * t[j]) + 2];

    _colorsCurv[3 * j] = (int) dataColorMap[3 * (int) (256 * tcurv[j])];
    _colorsCurv[3 * j + 1] = (int) dataColorMap[3 * (int) (256 * tcurv[j]) + 1];
    _colorsCurv[3 * j + 2] = (int) dataColorMap[3 * (int) (256 * tcurv[j]) + 2];
  }

  _indices = (GLuint*) malloc(3 * tri.size() * sizeof(GLuint));

  for (j = 0; j < (int) tri.size(); ++j)
  {
    _indices[3 * j] = (GLuint) tri[j][0];
    _indices[3 * j + 1] = (GLuint) tri[j][1];
    _indices[3 * j + 2] = (GLuint) tri[j][2];
  }

  _pointsSP.resize(3*vert.size());
  _facesSP.resize(3*tri.size());

  for (j = 0; j < (int) vert.size(); j++)
  {
    _pointsSP[3*j] = vert[j][0];
    _pointsSP[3*j+1] = vert[j][1];
    _pointsSP[3*j+2] = vert[j][2];
    //cout <<   _pointsSP[3*j]  << ' ' <<  _pointsSP[3*j +1 ] << ' ' <<   _pointsSP[3*j+2] << endl;
  }

  for (j = 0; j < (int) tri.size(); j++)
  {
    _facesSP[3*j] = tri[j][0];
    _facesSP[3*j+1] = tri[j][1];
    _facesSP[3*j+2] = tri[j][2];
  }

  //const float *f = _aTexCurv->textureCoords();

  float *f = (float*) malloc (_texCurv[0].nItem() * sizeof(float));
  for( uint i = 0; i < _texCurv[0].nItem(); i++)
  {
  f[i] = (float)(_texCurv[0].item(i));
  }

  _meshSP.initialize_mesh_data(_pointsSP,_facesSP, NULL,0,0);
  _meshSulciCurvSP.initialize_mesh_data(_pointsSP,_facesSP, f,1,_constraintPathValue);
  _meshGyriCurvSP.initialize_mesh_data(_pointsSP,_facesSP, f,2,_constraintPathValue);
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
  float v;
  if ((_maxT - _minT)!=0)
  {
    v = (float) (value - _minT) / (float) (_maxT - _minT);
    _colorpicked[0] = (int) dataColorMap[3 * (int) (255 * v)];
    _colorpicked[1] = (int) dataColorMap[3 * (int) (255 * v) + 1];
    _colorpicked[2] = (int) dataColorMap[3 * (int) (255 * v) + 2];
    _textureValue = value;
  }
  else
  {
    _colorpicked[0] = 0;
    _colorpicked[1] = 0;
    _colorpicked[2] = 0;
    _textureValue = 0;
  }

  //cout << "changeTextureValueInt " << (int) dataColorMap[3 * (int) (255 * v)] << endl;
  updateGL();
}

template<typename T>
void myGLWidget<T>::changeTextureValueFloat(double value)
{
  float v;
  if ((_maxT - _minT)!=0)
  {
    v = (float) (value - _minT) / (float) (_maxT - _minT);
    _colorpicked[0] = (int) dataColorMap[3 * (int) (255 * v)];
    _colorpicked[1] = (int) dataColorMap[3 * (int) (255 * v) + 1];
    _colorpicked[2] = (int) dataColorMap[3 * (int) (255 * v) + 2];
    _textureValue = value;
  }
  else
  {
    _colorpicked[0] = 0;
    _colorpicked[1] = 0;
    _colorpicked[2] = 0;
    _textureValue = 0;
  }

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
void myGLWidget<T>::changeToleranceValue(int value)
{
  _toleranceValue = value;

  _stepToleranceValue = _toleranceValue * (float)(_maxT -_minT)/100.;

  updateGL();
  cout << "ToleranceValue " << _stepToleranceValue << endl;
}

template<typename T>
void myGLWidget<T>::changeConstraintPathValue(int value)
{
  _constraintPathValue = value;

  //const float *f = _aTexCurv->textureCoords();
  float *f = (float*) malloc (_texCurv[0].nItem() * sizeof(float));
  for( uint i = 0; i < _texCurv[0].nItem(); i++)
  {
  f[i] = (float)(_texCurv[0].item(i));
  }

  //_meshSP.initialize_mesh_data(_pointsSP,_facesSP, NULL,0);
  _meshSulciCurvSP.update_weight(f,1, (int)_constraintPathValue);
  _meshGyriCurvSP.update_weight(f,2, (int)_constraintPathValue);

  if ( (_mode == 4 || _mode == 5 ||_mode == 6 ) )
  {
    for (unsigned i = 0; i < _listIndexVertexPathSPLast.size(); ++i)
    _listIndexVertexPathSP.pop_back();

   // _listIndexVertexPathSP.clear();

    std::vector<int>::iterator ite;
    ite = _listIndexVertexSelectSP.end();

    std::vector<geodesic::SurfacePoint> sources;
    std::vector<geodesic::SurfacePoint> targets;

    geodesic::GeodesicAlgorithmDijkstra *dijkstra_algorithm;

    if (_mode == 4)
    dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshSP);

    if (_mode == 5)
    dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshSulciCurvSP);

    if (_mode == 6)
    dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshGyriCurvSP);

    if (_listIndexVertexSelectSP.size() >= 2)
    {
      unsigned target_vertex_index = (*(--ite) );
      unsigned source_vertex_index = (*(--ite) );

      printf("indice source = %d target = %d \n",
          source_vertex_index, target_vertex_index);

      std::vector<geodesic::SurfacePoint> SPath;
      SPath.clear();

      //std::vector<int> _listIndexVertexPathSPLast;

      _listIndexVertexPathSPLast.clear();

      geodesic::SurfacePoint short_sources(
          &_meshSP.vertices()[source_vertex_index]);
      geodesic::SurfacePoint short_targets(
          &_meshSP.vertices()[target_vertex_index]);

      dijkstra_algorithm->geodesic(short_sources,short_targets, SPath, _listIndexVertexPathSPLast);

      ite = _listIndexVertexPathSPLast.end();

      reverse(_listIndexVertexPathSPLast.begin(),_listIndexVertexPathSPLast.end());
      _listIndexVertexPathSPLast.push_back((int)target_vertex_index);

      _listIndexVertexPathSP.insert(_listIndexVertexPathSP.end(), _listIndexVertexPathSPLast.begin(), _listIndexVertexPathSPLast.end());

      cout << "path dijkstra = ";

      for (unsigned i = 0; i < _listIndexVertexPathSP.size(); i++)
        cout << _listIndexVertexPathSP[i] << " " ;

      cout << endl;
      cout << "path dijkstra temp= ";
      cout << endl;
      for (unsigned i = 0; i < _listIndexVertexPathSPLast.size(); i++)
        cout << _listIndexVertexPathSPLast[i] << " " ;

      cout << endl;
      for (unsigned i = 0; i < SPath.size(); ++i)
      {
        geodesic::SurfacePoint ss ;
        ss.x() = (-_meshCenter[0] + (float) SPath[SPath.size() - i - 1].x()) * _meshScale;
        ss.y() = (-_meshCenter[1] + (float) SPath[SPath.size() - i - 1].y()) * _meshScale;
        ss.z() = (_meshCenter[2] - (float) SPath[SPath.size() - i - 1].z()) * _meshScale;
        _pathSP.push_back(ss);
        //cout << i << " " << ss.x() << ' ' << ss.y() << ' ' <<  ss.z() << endl;
      }
    }
  }

  updateGL();
  cout << "ConstraintPathValue " << value << endl;
}

template<typename T>
void myGLWidget<T>::changeIDVertexValue(int value)
{
  _indexVertex = value;

  _vertexNearestpicked[0] = _vertices[3 * _indexVertex];
  _vertexNearestpicked[1] = _vertices[3 * _indexVertex + 1];
  _vertexNearestpicked[2] = _vertices[3 * _indexVertex + 2];

  updateGL();
  //cout << "changeIDVertexValue " << value << endl;
}

template<typename T>
void myGLWidget<T>::saveTexture(void)
{
  cout << "save Texture on disk " << endl;

  typename std::map<int, T>::const_iterator mit(_listVertexChanged.begin()),
      mend(_listVertexChanged.end());

  const float* t = _aTex->textureCoords();


  TimeTexture<T> out(1, _mesh.vertex().size());
  for (uint i = 0; i < _mesh.vertex().size(); i++)
  {
    if (_dataType == "FLOAT")
    {
      out[0].item(i) = (float) (_tex[0].item(i));
      //out[0].item(i) = 0;
    }

    if (_dataType == "S16")
    {
      out[0].item(i) = (int) (_tex[0].item(i));
      //out[0].item(i) = 0;
    }
  }

  for (; mit != mend; ++mit)
  {
    //cout << (int)mit->first << " " << mit->second << endl;

    _tex[0].item(mit->first) = mit->second;
    out[0].item(mit->first) = mit->second;
    //out[0].item(mit->first) = 1;
  }

  rc_ptr<Texture1d> tex(new Texture1d);
  Converter<TimeTexture<T> , Texture1d> c;
  c.convert(_tex, *tex);
  _aTex->setTexture(tex);

  _listVertexChanged.clear();

  Writer<TimeTexture<T> > wt(_adressTexOut);
  wt.write(out);
}

template<typename T>
void myGLWidget<T>::changeMode(int mode)
{
  _mode = mode;
  //cout << "mode = " << mode << endl;
  //if (mode != 1) copyBackBuffer2Texture();
  updateGL();

}

template<typename T>
void myGLWidget<T>::fill(void)
{
//cout << "fill " << endl;

  //remplissage de région fermée
  if (_mode == 7)
  {
    for (unsigned i = 0; i < _listIndexVertexSelectFill.size(); i++)
    {
      //cout << i << " " << _colorpicked[0] << " " << _colorpicked << " " << _colorpicked[2] << endl;
      _listVertexChanged[_listIndexVertexSelectFill[i]] = _textureValue;
      _colors[3 * _listIndexVertexSelectFill[i]] = _colorpicked[0];
      _colors[3 * _listIndexVertexSelectFill[i] + 1] = _colorpicked[1];
      _colors[3 * _listIndexVertexSelectFill[i] + 2] = _colorpicked[2];
    }

    _listIndexVertexSelectFill.clear();
  }

  if (_mode == 4 || _mode == 5 || _mode == 6)
  {
    for (unsigned i = 0; i < _listIndexVertexPathSP.size(); i++)
    {
    //cout << i << " " << _colorpicked[0] << " " << _colorpicked << " " << _colorpicked[2] << endl;
    _listVertexChanged[_listIndexVertexPathSP[i]] = _textureValue;
    _colors[3 * _listIndexVertexPathSP[i]] = _colorpicked[0];
    _colors[3 * _listIndexVertexPathSP[i] + 1] = _colorpicked[1];
    _colors[3 * _listIndexVertexPathSP[i] + 2] = _colorpicked[2];
    }

  _listIndexVertexSelectSP.clear();
  _listIndexVertexPathSP.clear();
  }

  updateGL();
}

template<typename T>
void myGLWidget<T>::initializeGL()
{
  glEnable( GL_LIGHTING);
  glEnable( GL_LIGHT0);
  glEnable( GL_DEPTH_TEST);
  glShadeModel( GL_SMOOTH);
  static GLfloat lightPosition[4] =
  { 0.5, 5.0, 7.0, 1.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glClearColor(1, 1, 1, 1);

  unitize(_mesh, &_meshCenter, &_meshScale);
  cout << "mesh scale = " << _meshScale << "\n";
  cout << "mesh center = (" << _meshCenter[0] << "," << _meshCenter[1] << ","
      << _meshCenter[2] << ")\n";

  computeIndexColor( _mesh);

  loadColorMap(_colorMap.c_str());

  cout << "nb triangle = " << _mesh.polygon().size() << endl;
  cout << "nb vertex = " << _mesh.vertex().size() << endl;

  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *> (_parent);

  toolbar->IDPolygonSpinBox->setRange(0, _mesh.polygon().size());
  toolbar->IDVertexSpinBox->setRange(0, _mesh.vertex().size());

  //  _listMeshSmooth = buildDisplayList(_mesh, 0);

  // compute display list for index color rendering
  _listMeshPicking = buildDisplayList(_mesh, 1);

  //  _listMeshParcelation = buildDisplayList(_mesh, 2);
  //  _listMeshRender = _listMeshSmooth;

  buildDataArray();
  QPointF p(width()/2,height()/2);

  _trackBall.start();
  _trackBall.push(pixelPosToViewPos(p),gfx::Quaternionf::identity());
  trackBallTransformation();
  //paintGL();
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
  return QPointF(2.0 * float(p.x()) / width() - 1.0, 1.0 - 2.0 * float(p.y())
      / height());
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

  //cout << "mousePressEvent\n" <<  event->buttons() << endl;
  if (_resized) copyBackBuffer2Texture();

  if (event->buttons() == Qt::MidButton /*&& _mode == 1*/)
  {
    _trackBall.start();
    _trackBall.push(pixelPosToViewPos(event->pos()),
        gfx::Quaternionf::identity());
    event->accept();
    updateGL();
  }

  if (event->buttons() == Qt::LeftButton && _mode == 2)
  {
    //_trackBall.stop();
      _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
      //cout << "ID polygon : " << _indexPolygon << endl;
      _point3Dpicked = check3DpointPicked(event->x(), event->y());
      //cout << "3D : " << _point3Dpicked[0] << " " << _point3Dpicked[1] << " " << _point3Dpicked[2] << " " << endl;

      Point3df p;
      p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
      p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
      p[2] = _meshCenter[2] + (float) -_point3Dpicked[2] / _meshScale;

      _indexVertex
          = computeNearestVertexFromPolygonPoint(p, _indexPolygon, _mesh);

      //cout << "3D coord vertex value = " << _vertexNearestpicked[X] << " " << _vertexNearestpicked[Y] << " " << _vertexNearestpicked[Z] << "\n" ;

      if (_indexVertex >= 0 && _indexVertex < _mesh.vertex().size())
      {
        _textureValue = _tex[0].item(_indexVertex);

        typename std::map<int, T>::const_iterator it(_listVertexChanged.find(
            _indexVertex));

        if (it != _listVertexChanged.end()) _textureValue
            = _listVertexChanged[_indexVertex];

        _colorpicked[0] = _colors[3 * _indexVertex];
        _colorpicked[1] = _colors[3 * _indexVertex + 1];
        _colorpicked[2] = _colors[3 * _indexVertex + 2];

        changeTextureValue( _textureValue);
        updateInfosPicking(_indexPolygon, _indexVertex);
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

  if (event->buttons() == Qt::LeftButton && (_mode == 3 || _mode == 8 || _mode == 7) )
  {
    //_trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    _point3Dpicked = check3DpointPicked(event->x(), event->y());

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) -_point3Dpicked[2] / _meshScale;

    _indexVertex
        = computeNearestVertexFromPolygonPoint(p, _indexPolygon, _mesh);

    if (_indexVertex >= 0 && _indexVertex < 3 * _mesh.vertex().size())
    {
      if (_mode == 7)
      {
        //_listIndexVertexSelectSP.clear();
        _listIndexVertexSelectFill.clear();

        typename std::map<int, T>::iterator itef;
        itef = _listVertexChanged.find(_indexVertex);

        //cout << "new tex = " << _textureValue << " old tex = " << _tex[0].item(_indexVertex) << endl;
        //_listIndexVertexSelectSP.insert(_listIndexVertexSelectSP.end(), _listIndexVertexPathSP.begin(), _listIndexVertexPathSP.end());

        if (itef != _listVertexChanged.end())
          floodFill (_indexVertex, _textureValue,itef->second);
        else
          floodFill (_indexVertex, _textureValue,_tex[0].item(_indexVertex));

      }

      if (_mode == 3)
      {
        _colors[3 * _indexVertex] = _colorpicked[0];
        _colors[3 * _indexVertex + 1] = _colorpicked[1];
        _colors[3 * _indexVertex + 2] = _colorpicked[2];
        _listVertexChanged[_indexVertex] = _textureValue;
      }

      if (_mode == 8)
      {
        const float* t = _aTex->textureCoords();

        _colors[3 * _indexVertex] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex])];
        _colors[3 * _indexVertex + 1] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex]) + 1];
        _colors[3 * _indexVertex + 2] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex]) + 2];
        _listVertexChanged[_indexVertex] = _tex[0].item(_indexVertex);
      }


      updateInfosPicking(_indexPolygon, _indexVertex);
    }

    updateGL();
  }

  if ( (_mode == 4 || _mode == 5 ||_mode == 6 ) )
    {
      //_trackBall.stop();
      if (event->buttons() == Qt::LeftButton)
      {
        _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
        _point3Dpicked = check3DpointPicked(event->x(), event->y());
        Point3df p;
        p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
        p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
        p[2] = _meshCenter[2] + (float) -_point3Dpicked[2] / _meshScale;
        _indexVertex
            = computeNearestVertexFromPolygonPoint(p, _indexPolygon, _mesh);
      }

      if (event->buttons() == Qt::RightButton && _listIndexVertexSelectSP.size() > 1)
        _listIndexVertexSelectSP.push_back(*_listIndexVertexSelectSP.begin());

      if (event->buttons() != Qt::MidButton &&_indexVertex >= 0 && _indexVertex < 3 * _mesh.vertex().size())
      {
        if (event->buttons() == Qt::LeftButton)
          _listIndexVertexSelectSP.push_back(_indexVertex);

        std::vector<int>::iterator ite;
        ite = _listIndexVertexSelectSP.end();

        int nb_vertex;
        printf("nb vertex path = %d\n", _listIndexVertexSelectSP.size());

        std::vector<geodesic::SurfacePoint> sources;
        std::vector<geodesic::SurfacePoint> targets;

        geodesic::GeodesicAlgorithmDijkstra *dijkstra_algorithm;

        if (_mode == 4)
          dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshSP);

        if (_mode == 5)
          dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshSulciCurvSP);

        if (_mode == 6)
          dijkstra_algorithm = new geodesic::GeodesicAlgorithmDijkstra(&_meshGyriCurvSP);

        if (_listIndexVertexSelectSP.size() >= 2)
        {
          unsigned target_vertex_index = (*(--ite) );
          unsigned source_vertex_index = (*(--ite) );

          printf("indice source = %d target = %d \n",
              source_vertex_index, target_vertex_index);

          std::vector<geodesic::SurfacePoint> SPath;
          SPath.clear();

          //std::vector<int> _listIndexVertexPathSPLast;

          _listIndexVertexPathSPLast.clear();

          geodesic::SurfacePoint short_sources(
              &_meshSP.vertices()[source_vertex_index]);
          geodesic::SurfacePoint short_targets(
              &_meshSP.vertices()[target_vertex_index]);

          dijkstra_algorithm->geodesic(short_sources,short_targets, SPath, _listIndexVertexPathSPLast);

          ite = _listIndexVertexPathSPLast.end();

          reverse(_listIndexVertexPathSPLast.begin(),_listIndexVertexPathSPLast.end());
          _listIndexVertexPathSPLast.push_back((int)target_vertex_index);

          _listIndexVertexPathSP.insert(_listIndexVertexPathSP.end(), _listIndexVertexPathSPLast.begin(), _listIndexVertexPathSPLast.end());

          cout << "path dijkstra = ";

          for (unsigned i = 0; i < _listIndexVertexPathSP.size(); i++)
            cout << _listIndexVertexPathSP[i] << " " ;

          cout << endl;
          cout << "path dijkstra temp= ";
          cout << endl;
          for (unsigned i = 0; i < _listIndexVertexPathSPLast.size(); i++)
            cout << _listIndexVertexPathSPLast[i] << " " ;

          cout << endl;
          for (unsigned i = 0; i < SPath.size(); ++i)
          {
            geodesic::SurfacePoint ss ;
            ss.x() = (-_meshCenter[0] + (float) SPath[SPath.size() - i - 1].x()) * _meshScale;
            ss.y() = (-_meshCenter[1] + (float) SPath[SPath.size() - i - 1].y()) * _meshScale;
            ss.z() = (_meshCenter[2] - (float) SPath[SPath.size() - i - 1].z()) * _meshScale;
            _pathSP.push_back(ss);
            //cout << i << " " << ss.x() << ' ' << ss.y() << ' ' <<  ss.z() << endl;
          }

        }
        updateInfosPicking(_indexPolygon, _indexVertex);
      }

      updateGL();
    }

}

template<typename T>
void myGLWidget<T>::mouseReleaseEvent(QMouseEvent *event)
{
  //cout << "mouseReleaseEvent\n" <<  event->buttons() << endl;
  //if (event->isAccepted()) return;
  if (_resized)
  {
    _trackBall.release(pixelPosToViewPos(event->pos()),
        gfx::Quaternionf::identity());
    event->accept();
    _trackBall.stop();

    //cout << "copyBackBuffer2Texture\n";
    copyBackBuffer2Texture();
    updateGL();
  }

}

template<typename T>
void myGLWidget<T>::mouseMoveEvent(QMouseEvent *event)
{
  if (event->buttons() == Qt::MidButton/* && _mode == 1*/)
  {
    _trackBall.move(pixelPosToViewPos(event->pos()),
        gfx::Quaternionf::identity());
    event->accept();

    _resized = true;

    //cout << "BackBuffer change\n";
    updateGL();
  }

  if (event->buttons() == Qt::LeftButton && _mode == 2)
  {
    //_trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    //cout << "ID polygon : " << _indexPolygon << endl;
    _point3Dpicked = check3DpointPicked(event->x(), event->y());
    //cout << "3D : " << _point3Dpicked[0] << " " << _point3Dpicked[1] << " " << _point3Dpicked[2] << " " << endl;

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) -_point3Dpicked[2] / _meshScale;

    _indexVertex
        = computeNearestVertexFromPolygonPoint(p, _indexPolygon, _mesh);

    //cout << "3D coord vertex value = " << _vertexNearestpicked[X] << " " << _vertexNearestpicked[Y] << " " << _vertexNearestpicked[Z] << "\n" ;
    const float* t = _aTex->textureCoords();
    if (_indexVertex >= 0 && _indexVertex < _mesh.vertex().size())
    {
      _textureValue = _tex[0].item(_indexVertex);

      typename std::map<int, T>::const_iterator it(_listVertexChanged.find(
          _indexVertex));

      if (it != _listVertexChanged.end()) _textureValue
          = _listVertexChanged[_indexVertex];

      _colorpicked[0] = _colors[3 * _indexVertex];
      _colorpicked[1] = _colors[3 * _indexVertex + 1];
      _colorpicked[2] = _colors[3 * _indexVertex + 2];

      changeTextureValue( _textureValue);
      updateInfosPicking(_indexPolygon, _indexVertex);
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

  if (event->buttons() == Qt::LeftButton && (_mode == 3 || _mode == 8))
  {
    //_trackBall.stop();
    _indexPolygon = checkIDpolygonPicked(event->x(), event->y());
    _point3Dpicked = check3DpointPicked(event->x(), event->y());

    Point3df p;
    p[0] = _meshCenter[0] + (float) _point3Dpicked[0] / _meshScale;
    p[1] = _meshCenter[1] + (float) _point3Dpicked[1] / _meshScale;
    p[2] = _meshCenter[2] + (float) -_point3Dpicked[2] / _meshScale;

    _indexVertex
        = computeNearestVertexFromPolygonPoint(p, _indexPolygon, _mesh);

    if (_mode == 3 && _indexVertex >= 0 && _indexVertex < 3 * _mesh.vertex().size())
    {
      _colors[3 * _indexVertex] = _colorpicked[0];
      _colors[3 * _indexVertex + 1] = _colorpicked[1];
      _colors[3 * _indexVertex + 2] = _colorpicked[2];

      _listVertexChanged[_indexVertex] = _textureValue;
      updateInfosPicking(_indexPolygon, _indexVertex);
    }

    if (_mode == 8 && _indexVertex >= 0 && _indexVertex < 3 * _mesh.vertex().size())
    {
      const float* t = _aTex->textureCoords();

      _colors[3 * _indexVertex] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex])];
      _colors[3 * _indexVertex + 1] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex]) + 1];
      _colors[3 * _indexVertex + 2] = (int) dataColorMap[3 * (int) (256 * t[_indexVertex]) + 2];
      _listVertexChanged[_indexVertex] = _tex[0].item(_indexVertex);
    }
    updateGL();
  }
}

template<typename T>
void myGLWidget<T>::keyPressEvent(QKeyEvent *event)
{
  typename std::map<int, T>::const_iterator mit(_listVertexChanged.begin()),
      mend(_listVertexChanged.end());
  const float* t = _aTex->textureCoords();

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
    case Qt::Key_T:
      _texCurvDisplay = !_texCurvDisplay;
      break;
    case Qt::Key_Space:
      cout << "clear all vertex painted\n";

      //_listVertexChanged.clear();
      _listIndexVertexPathSP.clear();
      _listIndexVertexSelectSP.clear();
      _listIndexVertexSelectFill.clear();
      _pathSP.clear();

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

  //if (_mode == 1)
  {
    _trackBall.push(pixelPosToViewPos(event->pos()),
        gfx::Quaternionf::identity());

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

  _resized = true;
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

  glVertex2d(10, 25);
  glVertex2d(10, 35);
  glVertex2d(100, 35);
  glVertex2d(100, 25);
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

//Recursive 4-way floodfill, crashes if recursion stack is full

template<typename T>
void  myGLWidget<T>::floodFill(int indexVertex, T newTextureValue, T oldTextureValue)
{
  T textureValue;
  bool go;
  bool stop;
  bool pip;

  std::set<uint> voisins=neighbours[indexVertex];
  std::set<uint>::iterator voisIt=voisins.begin();

  voisIt=voisins.begin();

  std::vector<int>::iterator s1=_listIndexVertexPathSP.begin();
  std::vector<int>::iterator s2=_listIndexVertexPathSP.end();
  std::vector<int>::iterator ite = std::find(s1,s2, indexVertex);

  std::vector<int>::iterator f1=_listIndexVertexSelectFill.begin();
  std::vector<int>::iterator f2=_listIndexVertexSelectFill.end();
  std::vector<int>::iterator itef = std::find(f1,f2, indexVertex);


  typename std::map<int, T>::iterator itemap;
  itemap = _listVertexChanged.begin();
  itemap = _listVertexChanged.find(indexVertex);

  go = false;
  stop = false;
  pip = false;

  if (itemap != _listVertexChanged.end() )
    {
    if ((*itemap).second > (oldTextureValue + _stepToleranceValue) || (*itemap).second < (oldTextureValue - _stepToleranceValue) )
      stop = true;

    if ((*itemap).second <= (oldTextureValue + _stepToleranceValue) && (*itemap).second >= (oldTextureValue - _stepToleranceValue) )
     go = true;
    }

  if ( (_tex[0].item(indexVertex) <= (oldTextureValue + _stepToleranceValue)) && (_tex[0].item(indexVertex) >= (oldTextureValue - _stepToleranceValue)) )
    pip = true;

//  cout << "i " << indexVertex << " oldTextureValue " << _tex[0].item(indexVertex) << endl <<  " max " <<
//      oldTextureValue + _stepToleranceValue << "min " << oldTextureValue - _stepToleranceValue<< " stop " << stop << " go " << go << " pip " << pip << endl;

  if ( (go  || (pip && !stop)) && (ite == _listIndexVertexPathSP.end())
      && (itef == _listIndexVertexSelectFill.end() ) )
  {
    //_listIndexVertexSelectSP.push_back(indexVertex);
    _listIndexVertexSelectFill.push_back(indexVertex);
    for ( ; voisIt != voisins.end(); voisIt++)
      floodFill(*voisIt, newTextureValue, oldTextureValue);
  }

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

  std::vector<int>::iterator ite;
  ite = _listIndexVertexSelectSP.begin();

  if (_mode == 7)
    glColor3ub(_colorpicked[0],_colorpicked[1],_colorpicked[2]);
  else
    glColor3d(1, 1, 1);

  for (; ite != _listIndexVertexSelectSP.end(); ++ite)
  {
    glTranslatef(_vertices[*ite * 3], _vertices[*ite * 3 + 1 ],_vertices[*ite * 3 + 2]);
    gluSphere(quadric, 0.003, 10, 10);
    glTranslatef(-_vertices[*ite * 3], -_vertices[*ite * 3 + 1 ],-_vertices[*ite * 3 + 2]);
  }

  if (_mode == 7)
    glColor3ub(_colorpicked[0],_colorpicked[1],_colorpicked[2]);
  else
    glColor3d(1, 1, 1);

  ite = _listIndexVertexSelectFill.begin();
  for (; ite != _listIndexVertexSelectFill.end(); ++ite)
  {
    glTranslatef(_vertices[*ite * 3], _vertices[*ite * 3 + 1 ],_vertices[*ite * 3 + 2]);
    gluSphere(quadric, 0.002, 10, 10);
    glTranslatef(-_vertices[*ite * 3], -_vertices[*ite * 3 + 1 ],-_vertices[*ite * 3 + 2]);
  }

  if (_mode == 4 || _mode == 5 || _mode == 6 )
    glColor3ub(_colorpicked[0],_colorpicked[1],_colorpicked[2]);
  else
    glColor3d(1, 1, 1);

  ite = _listIndexVertexPathSP.begin();

  for (; ite != _listIndexVertexPathSP.end(); ++ite)
  {
    glTranslatef(_vertices[*ite * 3], _vertices[*ite * 3 + 1 ],_vertices[*ite * 3 + 2]);
    gluSphere(quadric, 0.002, 36, 18);
    glTranslatef(-_vertices[*ite * 3], -_vertices[*ite * 3 + 1 ],-_vertices[*ite * 3 + 2]);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable( GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LINE_SMOOTH);
  glLineWidth(2.0);
  glDisable(GL_LIGHTING);

  glColor3d(1, 1, 1);

  glBegin( GL_LINE_STRIP);
  ite = _listIndexVertexPathSP.begin();
  for (; ite != _listIndexVertexPathSP.end(); ++ite)
    glVertex3f(_vertices[*ite * 3], _vertices[*ite * 3 + 1 ],_vertices[*ite * 3 + 2]);

  glEnd();

//  glColor3d(0, 1, 0);
//  glBegin( GL_LINE_STRIP);
//  ite = _listIndexVertexPathCurvSP.begin();
//  for (; ite != _listIndexVertexPathCurvSP.end(); ++ite)
//    glVertex3f(_vertices[*ite * 3], _vertices[*ite * 3 + 1 ],_vertices[*ite * 3 + 2]);
//
//  glEnd();

  gluDeleteQuadric(quadric);

  glLineWidth(1.0);

  if (_indexPolygon >= 0 && _indexPolygon < 3 * _mesh.polygon().size())
  {
    glBegin( GL_TRIANGLES);

    glColor3ub(255 - _colorpicked[0], 255 - _colorpicked[1], 255
        - _colorpicked[2]);

    glVertex3f((_vertices[3 * _indices[3 * _indexPolygon]]), (_vertices[3
        * _indices[3 * _indexPolygon] + 1]), (_vertices[3 * _indices[3
        * _indexPolygon] + 2]));

    glVertex3f((_vertices[3 * _indices[3 * _indexPolygon + 1]]), (_vertices[3
        * _indices[3 * _indexPolygon + 1] + 1]), (_vertices[3 * _indices[3
        * _indexPolygon + 1] + 2]));

    glVertex3f((_vertices[3 * _indices[3 * _indexPolygon + 2]]), (_vertices[3
        * _indices[3 * _indexPolygon + 2] + 1]), (_vertices[3 * _indices[3
        * _indexPolygon + 2] + 2]));
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


  if (_texCurvDisplay)
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, _colorsCurv);
  else
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

  glDrawElements(GL_TRIANGLES, 3 * _mesh.polygon().size(), GL_UNSIGNED_INT,
      _indices);

  glDisable(GL_POLYGON_OFFSET_FILL);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable( GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LINE_SMOOTH);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);

  if (_wireframe) glDrawElements(GL_TRIANGLES, 3 * _mesh.polygon().size(),
      GL_UNSIGNED_INT, _indices);

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
  backBufferTexture.resize( width * height * 3);

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
int myGLWidget<T>::computeNearestVertexFromPolygonPoint(Point3df position,
    int poly, AimsSurfaceTriangle as)
{
  int index_nearest_vertex, index_min = 0;
  Point3df pt[3];
  uint v[3];

  const vector<Point3df> & vert = as.vertex();
  vector<AimsVector<uint, 3> > & tri = as.polygon();

  //cout << "poly= " << poly << endl;

  if (poly < (int) tri.size() && poly > 0)
  {
    v[0] = tri[poly][0];
    v[1] = tri[poly][1];
    v[2] = tri[poly][2];

    pt[0] = vert[v[0]];
    pt[1] = vert[v[1]];
    pt[2] = vert[v[2]];

    //compute the nearest polygon vertex
    float min, dist_min = FLT_MAX;

    for (int i = 0; i < 3; i++)
    {
      min = (float) sqrt((position[0] - pt[i][0]) * (position[0] - pt[i][0])
          + (position[1] - pt[i][1]) * (position[1] - pt[i][1]) + (position[2]
          - pt[i][2]) * (position[2] - pt[i][2]));

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
  _vertexNearestpicked[Z] *= -1;

//  cout << "position 3D = " << position[X] << " " << position[Y] << " " << position[Z] << "\n";
//  cout << "3D 1 = " << pt[0][X] << " " << pt[0][Y] << " " << pt[0][Z] << "\n";
//  cout << "3D 2 = " << pt[1][X] << " " << pt[1][Y] << " " << pt[1][Z] << "\n";
//  cout << "3D 3 = " << pt[2][X] << " " << pt[2][Y] << " " << pt[2][Z] << "\n";

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

  if (_listMeshPicking != 0) glCallList( _listMeshPicking);

  glPopAttrib();
}

template<typename T>
void myGLWidget<T>::copyBackBuffer2Texture(void)
{
  drawScenetoBackBuffer();
  glFinish();
  //glFlush();
  glReadBuffer( GL_BACK);
  glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE,
      &backBufferTexture[0]);
  _resized = false;
}

template<typename T>
void myGLWidget<T>::changeTextureValue(T value)
{
  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *> (_parent);
  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox =
        dynamic_cast<QDoubleSpinBox *> (toolbar->textureSpinBox);
    textureFloatSpinBox->setValue(value);
  }

  if (_dataType == "S16")
  {
    QSpinBox *textureIntSpinBox =
        dynamic_cast<QSpinBox *> (toolbar->textureSpinBox);
    textureIntSpinBox->setValue(value);
  }
}

template<typename T>
void myGLWidget<T>::updateInfosPicking(int idp, int idv)
{
  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *> (_parent);
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

  string path = p.globalShared() + s + "aims-" + carto::cartobaseShortVersion()
      + s + "Rgb" + s + filename;

  cout << "colorMap : " << path << endl;

  file = fopen(path.c_str(), "rb");

  if (file == NULL) return 0;

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
      dataColorMap);

  rc_ptr<Texture1d> tex(new Texture1d);
  Converter<TimeTexture<T> , Texture1d> c;
  c.convert(_tex, *tex);
  _aTex = new ATexture;
  _aTex->setTexture(tex);

  rc_ptr<Texture1d> texCurv(new Texture1d);
  Converter<TimeTexture<float> , Texture1d> cTexCurv;
  cTexCurv.convert(_texCurv, *texCurv);
  _aTexCurv = new ATexture;
  _aTexCurv->setTexture(texCurv);

//  _aTexCurv->normalize();
//  _aTex->normalize();

  const GLComponent::TexExtrema & te = _aTex->glTexExtrema();
  T min = te.minquant[0];
  T max = te.maxquant[0];

  _minT = min;
  _maxT = max;

  cout << "minquant " << min << " maxquant " << max << endl;

  myMeshPaint<T> *toolbar = dynamic_cast<myMeshPaint<T> *> (_parent);

  if (_dataType == "FLOAT")
  {
    QDoubleSpinBox *textureFloatSpinBox =
        dynamic_cast<QDoubleSpinBox *> (toolbar->textureSpinBox);

    textureFloatSpinBox->setRange(min, max);
  }

  if (_dataType == "S16")
  {
    QSpinBox *textureIntSpinBox =
        dynamic_cast<QSpinBox *> (toolbar->textureSpinBox);

    _minT = 0;
    _maxT = 360;

    if (_adressTexIn.length()!=0)
      textureIntSpinBox->setRange(_minT, _maxT);
    else
      {
      //_maxT = toolbar->constraintList->count() - 1;
      textureIntSpinBox->setRange(_minT,_maxT);
      }
  }

  _colorpicked[0] = dataColorMap[0];
  _colorpicked[1] = dataColorMap[1];
  _colorpicked[2] = dataColorMap[2];

  return 1;
}

template class myGLWidget<float> ;
template class myGLWidget<short> ;
