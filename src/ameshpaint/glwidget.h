#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QBrush>
#include <QFont>
#include <QImage>
#include <QPen>
#include <QGLWidget>
#include <QTime>
#include <QWheelEvent>
#include <iostream>
#include <QtGui>
#include <QDrag>
#include <QtOpenGL>
#include <stdlib.h>
#include <math.h>
#include <aims/getopt/getopt2.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/process.h>
#include <aims/io/finder.h>
#include <aims/mesh/surface.h>
#include <aims/mesh/texture.h>
#include <aims/def/path.h>
#include <aims/def/general.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>
#include <cartobase/config/paths.h>
#include <cartobase/config/version.h>
#include <aims/config/aimsdata_config.h>
#include <cmath>
#include <anatomist/surface/texture.h>
#include <aims/utility/converter_texture.h>
#include <float.h>
#include "trackball.h"

#include <aims/geodesicpath/geodesic_mesh.h>

#include <map>

#include <aims/mesh/curv.h>
#include <aims/data/data.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/mesh/geometric.h>
#include <aims/data/data_g.h>
#include <aims/io/io_g.h>
#include <aims/math/math_g.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/distancemap/meshdistance_d.h>
#include <aims/distancemap/distancemap_g.h>
#include <aims/morphology/morphology_g.h>

#if defined(__APPLE__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# ifdef HAS_GLEXT
#  include <OpenGL/glext.h>
# endif
#else
# include <GL/gl.h>
# include <GL/glu.h>
# ifdef HAS_GLEXT
#  include <GL/glext.h>
# endif
#endif

using namespace carto;
using namespace aims;
using namespace std;
using namespace anatomist;
using namespace aims::meshdistance;

//QT_BEGIN_NAMESPACE
class QPaintEvent;
class QWidget;
class QDragEnterEvent;
class QDropEvent;
//QT_END_NAMESPACE

class GLWidget : public QGLWidget
{
  Q_OBJECT

protected :
  virtual void changeTextureValueFloat(double){}
  virtual void changeTextureValueInt(int){}
  virtual void changeIDPolygonValue(int){}
  virtual void changeIDVertexValue(int){}
  virtual void changeToleranceValue(int){}
  virtual void changeConstraintPathValue(int){}

private slots:

  void changeTextureSpinBoxFloat(double v) {changeTextureValueFloat(v);}
  void changeTextureSpinBoxInt(int v) {changeTextureValueInt(v);}
  void changeIDPolygonSpinBox(int v) {changeIDPolygonValue(v);}
  void changeIDVertexSpinBox(int v) {changeIDVertexValue(v);}

  void changeToleranceSpinBox(int v) {changeToleranceValue(v);}
  void changeConstraintPathSpinBox(int v) {changeConstraintPathValue(v);}


public:
  GLWidget (QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent){};
  ~GLWidget (){};
};

template<typename T>
class myGLWidget : public GLWidget
{
public:
  myGLWidget (QWidget *parent, string adressTexIn,string adressMeshIn,string adressTexCurvIn,string adressTexOut,string colorMap,string dataType);
  ~myGLWidget ();

public :
  void changeTextureValueFloat(double );
  void changeTextureValueInt(int );
  void changeIDPolygonValue(int );
  void changeIDVertexValue(int );

  void changeToleranceValue(int );
  void changeConstraintPathValue(int );

public:
  int getMode () const { return _mode; }
  float getZoom () const { return _zoom; }
  float getTranslate () const { return _trans; }

  void fill(void);
  void changeMode (int mode);
  void changeTextureValue(T value);
  void updateInfosPicking(int idp, int idv);
  void saveTexture (void);
  void clearAll (void);

  void unitize (AimsSurfaceTriangle as, Point3df *meshCenter, float *meshScale);
  int buildDisplayList (AimsSurfaceTriangle as,int mode);
  void buildDataArray(void);
  int computeNearestVertexFromPolygonPoint (Point3df position, int poly, AimsSurfaceTriangle as);
  void computeIndexColor (AimsSurfaceTriangle as);
  void copyBackBuffer2Texture (void);

  void fillHolesOnPath (void);
  void addSimpleShortPath(int indexSource,int indexDest);
  void computeIsoline(int indexSource);
  GLuint loadColorMap (const char * filename);
  void drawColorMap (void);
  void keyPressEvent (QKeyEvent *event);

  void setZoom(float z);
  void setTranslate(float t);

protected:
  void initializeGL ();
  void paintGL ();
  void resizeGL (int width, int height);
  void mousePressEvent (QMouseEvent *event);
  void mouseMoveEvent (QMouseEvent *event);
  void mouseReleaseEvent (QMouseEvent *event);
  void wheelEvent (QWheelEvent *event);
  void dragEnterEvent (QDragEnterEvent *event);
  void dragMoveEvent (QDragMoveEvent *event);
  void dropEvent (QDropEvent *event);
  QPointF pixelPosToViewPos (const QPointF& p);
  void drawScenetoBackBuffer (void);

  GLuint checkIDpolygonPicked (int x, int y);
  Point3df check3DpointPicked (int x, int y);
  void projectionPerspective (void);
  void projectionOrtho (void);
  void trackBallTransformation(void);
  void drawPrimitivePicked (void);
  void drawHoles (void);
  void floodFill(int indexVertex, T newTextureValue, T oldTextureValue);

private:

  QWidget *_parent;
  void setupViewport (int width, int height);

  int _mode;
  float _zoom ;
  float _trans;
  bool _resized;
  bool _showInfos;

  GLUquadricObj *_qobj_cursor;

  int _frames;
  QTime _time;

  string _dataType;
  AimsSurfaceTriangle _mesh;
  TimeTexture<T> _tex;
  TimeTexture<float> _texCurv;

  string _adressTexIn;
  string _adressMeshIn;
  string _adressTexCurvIn;
  string _adressTexOut;
  string _colorMap;

  TrackBall _trackBall;

  GLuint _listMeshPicking;

  Point3df _meshCenter;
  float _meshScale;

  std::vector<int> _indexTexture;

  vector<GLubyte> backBufferTexture;

  ATexture	*_aTex;
  ATexture  *_aTexCurv;
  T _minT;
  T _maxT;

  unsigned char* dataColorMap;

  Point3df _point3Dpicked;
  Point3df _vertexNearestpicked;
  Point3d _colorpicked;

  GLuint _indexPolygon;
  GLuint _indexVertex;

  GLuint _constraintPathValue;
  GLuint _toleranceValue;
  float _stepToleranceValue;

  T _textureValue;
  bool _wireframe;
  //bool _parcelation;
  bool _texCurvDisplay;

  GLuint _IDcolorMap;

  map<int,T> _listVertexChanged;

  GLfloat *_vertices;
  GLfloat *_normals;
  GLubyte *_colors;
  GLubyte *_colorsCurv;
  GLubyte *_colorsDist;
  GLuint *_indices;

  std::vector<double> _pointsSP;
  std::vector<unsigned> _facesSP;

  geodesic::Mesh _meshSP;
  geodesic::Mesh _meshGyriCurvSP;
  geodesic::Mesh _meshSulciCurvSP;
  std::vector<std::set<uint> >  neighbours;

  std::vector<geodesic::SurfacePoint> _pathSP;
  //std::vector<geodesic::SurfacePoint> _pathCurvSP;
  //std::vector<geodesic::SurfacePoint> _pathExactSP;

  std::vector<int> _listIndexVertexPathSP;
  std::vector<int> _listIndexVertexPathSPLast;

  //std::vector<int> _listIndexVertexPathCurvSP;
  std::vector<int> _listIndexVertexSelectSP;
  std::vector<int> _listIndexVertexSelectFill;

  std::vector<int> _listIndexVertexBrushPath;
  std::vector<std::vector<int> >  _listIndexVertexHolesPath;

};

#endif
