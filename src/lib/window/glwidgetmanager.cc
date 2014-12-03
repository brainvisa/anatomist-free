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


#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/window/glcaps.h>
#include <anatomist/application/fileDialog.h>
#include <qimage.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <map>
#include <anatomist/controler/controlswitch.h>
#include <anatomist/window/glcontext.h>
#include <anatomist/reference/Transformation.h>
#include <aims/resampling/quaternion.h>
#include <cartobase/type/string_conversion.h>
#include <cartobase/stream/fileutil.h>
#include <qapplication.h>
#include <QImageWriter>
#include <qfiledialog.h>
#include <iostream>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/viewstate.h>
#include <qdesktopwidget.h>
#include <QGraphicsView>
#include <QSysInfo>

namespace Qt
{
}
using namespace Qt;
using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

// this macro is #undef'ed if GL extensions are not compiled
#define COMPILE_DEPTH_PEELING


QGLWidget* GLWidgetManager::sharedWidget()
{
  static QGLWidget *w = 0;
  return  w ? w : w = new GLWidget( 0, "ref GLWidget" );
}


GLWidgetManager_Private_QObject::GLWidgetManager_Private_QObject(
    QObject* parent, GLWidgetManager* man )
  : QObject( parent ), _manager( man )
{
}


GLWidgetManager_Private_QObject::~GLWidgetManager_Private_QObject()
{
}


void GLWidgetManager_Private_QObject::updateZBuffer()
{
  _manager->updateZBuffer();
}


void GLWidgetManager_Private_QObject::saveContents()
{
  _manager->saveContents();
}


void GLWidgetManager_Private_QObject::recordStart()
{
  _manager->recordStart();
}


void GLWidgetManager_Private_QObject::recordStop()
{
  _manager->recordStop();
}


// ---------

struct GLWidgetManager::Private
{
  Private();
  void buildRotationMatrix();
  void setAutoCenter();
  void setWindowExtrema();

  QGLWidget             *glwidget;
  Point3df		bmino;
  Point3df		bmaxo;
  Point3df		bminw;
  Point3df		bmaxw;
  Point3df		center;
  GLuint		light;
  QString		recordBaseName;
  QString		recordFormat;
  QString		recordSuffix;
  bool			record;
  unsigned		recIndex;
  QSize			prefSize;
  QSize			minSizeHint;
  float			zoom;
  Quaternion		quaternion;
  AimsVector<float,16>	rotation;
  bool			invertX;
  bool			invertY;
  bool			invertZ;
  bool			perspective;
  bool			autocenter;
  int			otherbuffers;
  // depth peeling stuff
  bool			hastransparent;
  bool			depthpeeling;
  bool			depthpeelallowed;
  unsigned		depthpasses;
  //
  bool			texunits;
  bool			zbufready;
  QTimer		*zbuftimer;
  bool			rgbbufready;
  int			lastkeypress_for_qt_bug;
  GLWidgetManager       *righteye;
  GLWidgetManager       *lefteye;
  GLWidgetManager_Private_QObject *qobject;
  bool                  transparentBackground;
  unsigned char         backgroundAlpha;

  vector<GLubyte> backBufferTexture;
  int mouseX;
  int mouseY;
  bool resized;
  bool saveInProgress;
};


GLWidgetManager::Private::Private()
  : glwidget( 0 ), bmino( 0, 0, 0 ), bmaxo( 1, 1, 1 ), bminw( 0, 0, 0 ),
    bmaxw( 1, 1, 1 ),
    center( 0, 0, 0 ), light( 0 ), record( false ), 
    recIndex( 0 ), prefSize( 384, 384 ), zoom( 1 ), 
    quaternion( 1./::sqrt(2), 0, 0, 1./::sqrt(2) ), 
    invertX( false ), 
    invertY( false ), invertZ( true ), perspective( false ), 
    autocenter( true ), otherbuffers( 1 ), 
    hastransparent( false ), depthpeeling( false ), depthpeelallowed( false ), 
    depthpasses( 2 ), texunits( 1 ), zbufready( false ), zbuftimer( 0 ), 
    rgbbufready( false ), 
    lastkeypress_for_qt_bug( 0 ), righteye( 0 ), lefteye( 0 ),
    qobject( 0 ),
    transparentBackground( true ), backgroundAlpha( 128 ),
    mouseX( 0 ), mouseY( 0 ), resized(false), saveInProgress( false )
{
  buildRotationMatrix();
}


void GLWidgetManager::Private::buildRotationMatrix()
{
  rotation = quaternion.inverseRotationMatrix();
}


void GLWidgetManager::Private::setAutoCenter()
{
  center = ( bmino + bmaxo ) / 2.F;
}


void GLWidgetManager::Private::setWindowExtrema()
{
  // determine extrema after transformations

  Transformation	t( 0, 0 );

  t.setQuaternion( quaternion );
  t.invert();
  t.transformBoundingBox( bmino - center, bmaxo - center, bminw, bmaxw );
}

// --------


GLWidgetManager::GLWidgetManager( anatomist::AWindow* win, QGLWidget * glw )
  : View( win ), _pd( new Private )
{
  _pd->glwidget = glw;
  glw->setFocusPolicy( StrongFocus );
  glw->setSizePolicy( QSizePolicy( QSizePolicy::Preferred,
                      QSizePolicy::Preferred ) );
}


GLWidgetManager::~GLWidgetManager()
{
  clearLists();
  delete _pd;
}


QObject* GLWidgetManager::qobject()
{
  if( !_pd->qobject )
    _pd->qobject = new GLWidgetManager_Private_QObject( _pd->glwidget, this );
  return _pd->qobject;
}


QGLWidget* GLWidgetManager::qglWidget()
{
  return _pd->glwidget;
}


namespace
{

  void checkDepthPeeling( GLWidgetManager::Private* _pd )
  {
    _pd->depthpeelallowed = false;
    _pd->depthpeeling = false;

#if !defined( GL_SGIX_shadow ) || \
( !defined( GL_ARB_shadow ) && !defined( GL_SGIX_shadow ) ) || \
( !defined( GL_SGIX_depth_texture ) && !defined( GL_ARB_depth_texture ) )
#ifdef COMPILE_DEPTH_PEELING
#undef COMPILE_DEPTH_PEELING
#endif

#else

    if( GLCaps::numTextureUnits() == 1 
        || ( !GLCaps::ext_ARB_shadow() && !GLCaps::ext_SGIX_shadow() ) 
        || ( !GLCaps::ext_SGIX_depth_texture() 
             && !GLCaps::ext_ARB_depth_texture() ) )
      _pd->depthpeelallowed = false;

#endif // depth peeling extensions
  }

}


void GLWidgetManager::initializeGL()
{
  // depth peeling checks and initialization
  checkDepthPeeling( _pd );

  // "regular" stuff
  glEnable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHT0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  glFrontFace( GL_CW );
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
  glEnable( GL_NORMALIZE );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );

  glClearColor( 1, 1, 1, 1 );

  _pd->backBufferTexture.resize( _pd->glwidget->width()* _pd->glwidget->height() * 3 );
}


void GLWidgetManager::resizeGL( int w, int h )
{
  glViewport( 0, 0, (GLint)w, (GLint)h );
  _pd->resized = true;
}


bool GLWidgetManager::depthPeelingAllowed() const
{
  return _pd->depthpeelallowed;
}


bool GLWidgetManager::depthPeelingEnabled() const
{
  return _pd->depthpeelallowed && _pd->depthpeeling;
}


void GLWidgetManager::enableDepthPeeling( bool x )
{
  if( _pd->depthpeelallowed )
    _pd->depthpeeling = x;
  else
    _pd->depthpeeling = false;
}


unsigned GLWidgetManager::depthPeelingPasses() const
{
  return _pd->depthpasses;
}


void GLWidgetManager::setDepthPeelingPasses( unsigned n )
{
  _pd->depthpasses = n;
}


unsigned GLWidgetManager::numTextureUnits() const
{
  return _pd->texunits;
}


bool GLWidgetManager::hasTransparentObjects() const
{
  return _pd->hastransparent;
}


void GLWidgetManager::setTransparentObjects( bool x )
{
  _pd->hastransparent = x;
}


void GLWidgetManager::paintScene()
{
  paintGL();
}

void GLWidgetManager::paintGL()
{
  _pd->zbufready = true;
  if( _pd->zbuftimer )
    _pd->zbuftimer->stop();
  paintGL( Normal );
  _pd->rgbbufready = true;
  if( !_pd->zbufready )
  {
    if( !_pd->zbuftimer )
    {
      _pd->zbuftimer = new QTimer( _pd->glwidget );
      if( !_pd->qobject )
        _pd->qobject = new GLWidgetManager_Private_QObject( _pd->glwidget,
          this );
      _pd->qobject->connect( _pd->zbuftimer, SIGNAL( timeout() ), _pd->qobject,
                           SLOT( updateZBuffer() ) );
    }
    _pd->zbuftimer->setSingleShot( true );
    _pd->zbuftimer->start( 300 );
  }
}


void GLWidgetManager::renderBackBuffer( ViewState::glSelectRenderMode
    selectmode )
{
  _pd->glwidget->makeCurrent();
  DrawMode mode = Normal;
  switch( selectmode )
  {
  case ViewState::glSELECTRENDER_OBJECT:
    mode = ObjectSelect;
    _pd->zbufready = false;
    break;
  case ViewState::glSELECTRENDER_OBJECTS:
    mode = ObjectsSelect;
    _pd->zbufready = false;
    break;
  case ViewState::glSELECTRENDER_POLYGON:
    mode = PolygonSelect;
    _pd->zbufready = false;
    break;
  default:
    break;
  }
  paintGL( mode );
}


void GLWidgetManager::updateZBuffer()
{
  stopZBufferTimer();
  if( isZBufferUpToDate() )
    return;
  // render the Z buffer without ghost objects
  _pd->glwidget->makeCurrent();
  paintGL( ZSelect );
  setZBufferUpdated( true );
  setRGBBufferUpdated( false );
}


bool GLWidgetManager::isZBufferUpToDate() const
{
  return _pd->zbufready;
}


void GLWidgetManager::setZBufferUpdated( bool x )
{
  _pd->zbufready = x;
}


void GLWidgetManager::stopZBufferTimer()
{
  if( _pd->zbuftimer )
    _pd->zbuftimer->stop();
}

bool GLWidgetManager::isRGBBufferUpToDate() const
{
  return _pd->rgbbufready;
}


void GLWidgetManager::setRGBBufferUpdated( bool x )
{
  _pd->rgbbufready = x;
}


void GLWidgetManager::paintGL( DrawMode m )
{
//   _pd->glwidget->makeCurrent();
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  glPushAttrib( GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT );
  glEnable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glFrontFace( GL_CW );
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
  glEnable( GL_NORMALIZE );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  glClearColor( 1, 1, 1, 1 );

  project();

  if( m != ObjectSelect && m != ObjectsSelect && m != PolygonSelect )
  {
    // Lighting is described in the viewport coordinate system
    if( glIsList( _pd->light ) )
      glCallList( _pd->light );
  }

  // Clear the viewport
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Modelview matrix: we only apply rotation for now!
  //glTranslatef( _pd->campos[0], _pd->campos[1], _pd->campos[2] );
  glLoadMatrixf( &_pd->rotation[0] );

  /*  if (_compassOn)
    {
      // Projection matrix: save before redefining locally for compass
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();

      // Viewport to draw compass into
      compassWinDim = (_dimx/4 < 70) ? _dimx/4 : 70;
      glViewport(0, 0, compassWinDim, compassWinDim);

      // Projection matrix: compass needs this orthographic projection
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      orthoMinX = - 1.0;
      orthoMinY = - 1.0;
      orthoMinZ = - 1.0;
      orthoMaxX =   1.0;
      orthoMaxY =   1.0;
      orthoMaxZ =   1.0;
      glOrtho(orthoMinX, orthoMaxX, 
	      orthoMinY, orthoMaxY, 
	      orthoMinZ, orthoMaxZ);

      // Draw compass
      glClear(GL_DEPTH_BUFFER_BIT);
      glCallList(_3DGuide->GetCompassGLList());

      // Projection matrix: restore
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      }*/

  // Viewport to draw objects into
  glViewport( 0, 0, _pd->glwidget->width(), _pd->glwidget->height() );

  // Modelview matrix: we can now apply translation and left-right mirroring
  glMatrixMode( GL_MODELVIEW );
  glScalef( _pd->invertX ? -1 : 1, _pd->invertY ? -1 : 1, _pd->invertZ ? -1 : 1 );
  glTranslatef( -_pd->center[0], -_pd->center[1], -_pd->center[2] );

  // Draw frame
  /*if (_frameOn)
    glCallList(_3DGuide->GetFrameGLList());*/

  if( _pd->hastransparent && _pd->depthpeelallowed && _pd->depthpeeling )
    depthPeelingRender( m );
  else
    drawObjects( m );

  glPopAttrib();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}


void GLWidgetManager::drawObjects( DrawMode m )
{
  // Draw objects
  // cout << "GLWidgetManager::drawObjects " << m << endl;
  GLPrimitives::const_iterator	il = _primitives.begin(),
      el = _primitives.end();
  if( m == ObjectSelect || m == ObjectsSelect || m == PolygonSelect )
  {
    il = _selectprimitives.begin();
    el = _selectprimitives.end();
  }

  //cout << "paintGL, prim : " << _primitives.size() << endl;
  for( ; il!=el; ++il )
  {
    if( (*il)->ghost() )
      switch( m )
      {
      case Normal:
        _pd->zbufready = false;
        (*il)->callList();
        break;
      case ZSelect:
        break;
      default:
        break;
      }
    else
      (*il)->callList();
  }
}


void GLWidgetManager::depthPeelingRender( DrawMode m )
{
  /* #ifndef COMPILE_DEPTH_PEELING
  drawObjects();
  #else */

  /* Algorithm from "Interactive order-independent transparency", 
     Cass Everitt, NVIDIA OpenGL applications engineering */
  // cout << "depthPeelingRender: not working yet\n";

  glDisable( GL_BLEND );
  glClearDepth( 0 );
  glClear( GL_DEPTH_BUFFER_BIT );
  glDepthFunc( GL_GREATER );

  drawObjects( m );

  unsigned		n = _pd->glwidget->width() * _pd->glwidget->height();
  vector<GLfloat>	buffer( n );
  GLfloat		*b = &buffer[0], *be = b + n;

  // make depth texture
  // could be optimized by SGIX_depth_texture extension
  glReadPixels( 0, 0, (GLint) _pd->glwidget->width(),
                (GLint) _pd->glwidget->height(),
		GL_DEPTH_COMPONENT, GL_FLOAT, b );

  cout << "corner: " << buffer[0] << endl;
  cout << "center: " << buffer[ _pd->glwidget->width()*_pd->glwidget->height()/2
      + _pd->glwidget->width()/2 ] << endl;

  // generate binary alpha texture from buffer

  for( b=&buffer[0]; b<be; ++b )
    if( *b == 1 )
      *b = 1;
    else
      *b = 0;

  // map texture onto polygons

  // glTexGen( EYE_LINEAR ) : projective texture mapping & multitexture
  // 3D coords in eye coords, (s,t)=2 Dcoords, r=depth

  // test: lookup(s,t): depth of nearest surface, r=depth of fragment
  // must compare r to lookup(s,t) : SGIX_shadow extension...

  glClearDepth( 1 );
  glDepthFunc( GL_LESS );
  glEnable( GL_BLEND );

  //#endif
}


void GLWidgetManager::clearLists()
{
  _primitives.clear();
  _selectprimitives.clear();
}


void GLWidgetManager::setPrimitives( const GLPrimitives & pl )
{
  clearLists();
  _primitives = pl;
}


GLPrimitives GLWidgetManager::primitives() const
{
  return( _primitives );
}


void GLWidgetManager::setSelectionPrimitives( const GLPrimitives & pl )
{
  _selectprimitives = pl;
}


GLPrimitives GLWidgetManager::selectionPrimitives() const
{
  return( _selectprimitives );
}


void GLWidgetManager::setExtrema( const Point3df & bmin,
                                  const Point3df & bmax )
{
  _pd->bmino = bmin;
  _pd->bmaxo = bmax;

  if( _pd->autocenter )
    _pd->setAutoCenter();

  _pd->setWindowExtrema();
}


void GLWidgetManager::setWindowExtrema( const Point3df & bmin,
                                        const Point3df & bmax )
{
  if( _pd->lefteye )
  {
    _pd->bminw = bmin + _pd->lefteye->windowBoundingMin() - _pd->bminw;
    _pd->bmaxw = bmax + _pd->lefteye->windowBoundingMax() - _pd->bmaxw;
  }
  else
  {
    _pd->bminw = bmin;
    _pd->bmaxw = bmax;
  }
}


Point3df GLWidgetManager::boundingMin() const
{
  return( _pd->bmino );
}


Point3df GLWidgetManager::boundingMax() const
{
  return( _pd->bmaxo );
}


Point3df GLWidgetManager::windowBoundingMin() const
{
  return( _pd->bminw );
}


Point3df GLWidgetManager::windowBoundingMax() const
{
  return( _pd->bmaxw );
}


void GLWidgetManager::setLightGLList( GLuint l )
{
  _pd->light = l;
}


GLuint GLWidgetManager::lightGLList() const
{
  return _pd->light;
}


QSize GLWidgetManager::sizeHint() const
{
  return( _pd->prefSize );
}


void GLWidgetManager::record()
{
  if( _pd->saveInProgress )
    return;
  // flush buffered events, without saving pictures
  _pd->saveInProgress = true;
  aWindow()->show();
  qApp->processEvents();
  _pd->saveInProgress = false;

  QString	num = QString::number( _pd->recIndex );
  while( num.length() < 4 )
    num.insert( 0, '0' );
  QString	filename = _pd->recordBaseName + num + _pd->recordSuffix;

  cout << "writing " << filename.toStdString() << endl;
  saveContents( filename, _pd->recordFormat );
  ++_pd->recIndex;
}


void GLWidgetManager::saveContents( const QString & filename,
                                    const QString & format )
{
  if( _pd->saveInProgress )
    return;
  _pd->saveInProgress = true;
  aWindow()->show();
  qApp->processEvents();

  /* TODO: try using FrameBuffer objects
     See Matthieu's trials: /home/mp210984/fbo in NS
  */
  /* // renderPixmap seems not to work with shared contexts:
  // when using the existing context, the rendering is just crap.
  // When using its own context, OpenGL fails with errors, either because
  // the context is not shared, or because there are some makeCurrent() calls
  // within our routines (and we may have to find a way to disable them)
  int x, y;
  unsigned w, h;
  aWindow()->geometry( &x, &y, &w, &h );
  QPixmap pix = qglWidget()->renderPixmap( w, h, false ); // doesn't work....
  if( pix.isNull() )
  {
    cout << "Pixmap rendering failed\n";
  }
  else
  {
    pix.save( filename, format, 100 );
    return;
  }
  */

  QString	f;
  if( format.isNull() )
    {
      f = stringUpper( FileUtil::extension( filename.toStdString() ) ).c_str();
      if( f == "JPG" )
        f = "JPEG";
    }
  else
    f = format;

  if( _pd->otherbuffers )
  {
    unsigned	i;
    int	mode;
    for( i=0; i<31; ++i )
    {
      mode = _pd->otherbuffers & (1<<i);
      if( mode )
        saveOtherBuffer( filename, f, mode );
    }
  }
  _pd->saveInProgress = false;
}


void GLWidgetManager::saveOtherBuffer( const QString & filename,
                                       const QString & format, int bufmode )
{
  //setupView();
  if( !_pd->rgbbufready )
    {
      paintGL( Normal );
      _pd->rgbbufready = true;
    }

  int		depth;
  GLenum	mode;
  QString	ext;
  bool		alpha;
  QImage::Format iformat;

  switch( bufmode )
    {
    case 2:	// alpha buffer
      depth = 8;
      mode = GL_ALPHA;
      alpha = false;
      ext = "-alpha";
      iformat = QImage::Format_Indexed8;
      break;
    case 4:	// RGBA
      depth = 32;
      mode = GL_BGRA;
      alpha = true;
      ext = "-rgba";
      iformat = QImage::Format_ARGB32;
      break;
    case 8:	// depth
      depth = 8;
      mode = GL_DEPTH_COMPONENT;
      alpha = false;
      ext = "-depth";
      iformat = QImage::Format_Indexed8;
      break;
    case 16:	// luminance
      depth = 8;
      mode = GL_LUMINANCE;
      alpha = false;
      ext = "-luminance";
      iformat = QImage::Format_Indexed8;
      break;
    default:	// RGB buffer
      depth = 32;
      mode = GL_BGRA;
      alpha = false;
      iformat = QImage::Format_RGB32;
      break;
    }

  int	ncol = 0;
  if( depth == 8 )
    ncol = 256;
  QImage pix( _pd->glwidget->width(), _pd->glwidget->height(), iformat );
  int	i;
  for( i=0; i<ncol; ++i )
    pix.setColor( i, qRgb(i,i,i) );
  // read the GL buffer
  _pd->glwidget->makeCurrent();
//   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
//   glPixelStorei( GL_UNPACK_SKIP_PIXELS, 0 );
  glReadBuffer( GL_FRONT );
  glReadPixels( 0, 0, (GLint) _pd->glwidget->width(), (GLint)
      _pd->glwidget->height(), mode, GL_UNSIGNED_BYTE, pix.bits() );

  pix = pix.mirrored( false, true );
  if( depth == 32 && QSysInfo::ByteOrder != QSysInfo::LittleEndian )
  {
    cout << "change bit order\n";
    int n = _pd->glwidget->width()*_pd->glwidget->height();
    unsigned char *buf = pix.bits(), c;
    for( i=0; i<n; ++i )
    {
      c = *buf;
      *buf = *(buf+3);
      ++buf;
      *(buf+2) = c;
      c = *buf;
      *buf = *(buf+1);
      ++buf;
      *buf = c;
      ++buf;
      ++buf;
    }
  }
  if( alpha && _pd->transparentBackground && bufmode == 4
      && _pd->backgroundAlpha != 255 && depth == 32 )
  {
    glReadBuffer( GL_FRONT );
    int n = width()*height(), y, w = width();
    vector<GLfloat> buffer( n, 2. );
    // read Z buffer
    glReadPixels( 0, 0, (GLint) width(), (GLint) height(),
                  GL_DEPTH_COMPONENT, GL_FLOAT, &buffer[0] );
    unsigned char *buf = pix.bits();
    // TODO: WHY THIS y-inversion ???
    for( y=height()-1; y>=0; --y )
      for( i=0; i<w; ++i )
      {
        buf += 3;
        if( buffer[i+y*w] >= 1. )
          *buf = _pd->backgroundAlpha;
        ++buf;
      }
  }
  QString	alphaname = filename;
  int pos = alphaname.lastIndexOf( '.' );
  if( pos == -1 )
    pos = alphaname.length();
  alphaname = alphaname.insert( pos, ext );
  cout << "saving " << alphaname.toStdString() << endl;
  pix.save( alphaname, format.toStdString().c_str(), 100 );
}


void GLWidgetManager::setBackgroundAlpha( float a )
{
  _pd->backgroundAlpha = a * 255.9;
}


void GLWidgetManager::updateGL()
{
  bool done = false;
  if( _pd->glwidget
    && dynamic_cast<QGraphicsView *>( _pd->glwidget->parent() ) )
  {
    // cout << "updateGL in a QGraphicsView\n";
    QGraphicsView *gv
      = dynamic_cast<QGraphicsView *>( _pd->glwidget->parent() );
    if( gv->scene() )
      gv->scene()->update();
    done = true;
  }

  if( !done )
  {
    if( dynamic_cast<QGLWidget *>( this ) == _pd->glwidget )
      _pd->glwidget->QGLWidget::updateGL();
    else
      _pd->glwidget->updateGL();
  }

  if( _pd->record )
    record();
  if( _pd->righteye )
    _pd->righteye->updateGL();
}


namespace
{

  QString formatFromName( const QString & name )
  {
    int x = name.lastIndexOf( '.' );
    if( x < 0 )
      return QString();
    QString suf = name.right( name.length() - x );
    if( suf == ".jpg" || suf == ".JPG" )
      return "JPEG";
    else if( suf == ".png" || suf == ".png" )
      return "PNG";
    else if( suf == ".bmp" || suf == ".BMP" )
      return "BMP";
    else if( suf == ".pbm" || suf == ".PBM" )
      return "PBM";
    else if( suf == ".ppm" || suf == ".PPM" )
      return "PPM";
    else if( suf == ".pgm" || suf == ".PGM" )
      return "PGM";
    else if( suf == ".xpm" || suf == ".XPM" )
      return "XPM";
    else
      return "PNG";
    return QString();
  }


QStringList fileAndFormat( const QString & caption )
{
  QStringList	res;

  QList<QByteArray>	formats = QImageWriter::supportedImageFormats();
  QList<QByteArray>::iterator	fi, fe = formats.end();
  QStringList		filter;
  char		*c;
  bool		def = false;
  unsigned	i = 0;
  list<unsigned>	flist;
  QString genformat = "All image formats (";

  for( fi=formats.begin(); fi!=fe; ++fi, ++i )
  {
    c = fi->data();
    c = (char *)stringUpper(c).c_str();
    if( !strcmp( c, "JPEG" ) )
    {
      filter.prepend( "JPEG (*.jpg)" );
      flist.push_front( i );
      def = true;
      genformat += " *.jpg";
    }
    else if( !strcmp( c, "PNG" ) )
    {
      if( !def )
      {
        filter.prepend( "PNG (*.png)" );
        def = true;
        flist.push_front( i );
      }
      else
      {
        filter.append( "PNG (*.png)" );
        flist.push_back( i );
      }
      genformat += " *.png";
    }
    else if( !strcmp( c, "BMP" ) )
    {
      if( !def )
      {
        filter.prepend( "BMP (*.bmp)" );
        def = true;
        flist.push_front( i );
      }
      else
      {
        filter.append( "BMP (*.bmp)" );
        flist.push_back( i );
      }
      genformat += " *.bmp";
    }
    else if( !strcmp( c, "PBM" ) )
    {
      filter.append( "PBM (*.pbm)" );
      flist.push_back( i );
      genformat += " *.pbm";
    }
    else if( !strcmp( c, "PPM" ) )
    {
      filter.append( "PPM (*.ppm)" );
      flist.push_back( i );
      genformat += " *.ppm";
    }
    else if( !strcmp( c, "PGM" ) )
    {
      filter.append( "PGM (*.pgm)" );
      flist.push_back( i );
      genformat += " *.pgm";
    }
    else if( !strcmp( c, "XPM" ) )
    {
      filter.append( "XPM (*.xpm)" );
      flist.push_back( i );
      genformat += " *.xpm";
    }
    else if( !strcmp( c, "XBM" ) )
    {
      filter.append( "XBM (*.xbm)" );
      flist.push_back( i );
      genformat += " *.xbm";
    }
    else
    {
      filter.append( c );
      flist.push_back( i );
    }
  }
  filter.prepend( genformat + " )" );

  QFileDialog	& fdiag = fileDialog();
  fdiag.setNameFilter( filter.join( ";;" ) );
  fdiag.setWindowTitle( caption );
  fdiag.setFileMode( QFileDialog::AnyFile );
  if( fdiag.exec() )
  {
    QStringList	filenames = fdiag.selectedFiles();
    if( !filenames.isEmpty() )
    {
      QString filename = filenames[0];
      QString format = fdiag.selectedFilter();
      QString format2 = formatFromName( filename );
      if( format2.isEmpty() )
      {
        int				nf = filter.indexOf( format );
        list<unsigned>::iterator	il;

        if( nf > 0 && nf <= (int) formats.count() )
          {
            --nf; // skip the generic one
            for( i=0, il=flist.begin(); i<(unsigned)nf; ++i, ++il ) {}
            format = formats.at( *il );
          }
        else
          format = "JPEG";
      }
      else
        format = format2;
      res.append( filename );
      res.append( format );
      //cout << "format : " << format << endl;
    }
  }

  return( res );
}

} // namespace {}


void GLWidgetManager::saveContents()
{
  QStringList	names = fileAndFormat( "Save window image" );
  if( names.count() != 2 )
    {
      cout << "save aborted\n";
      return;
    }

  saveContents( names.first(), names.last() );
}


void GLWidgetManager::recordStart( const QString & basename,
                                   const QString & format )
{
  _pd->recordBaseName = basename;
  _pd->recordFormat = format;
  int	p = _pd->recordBaseName.lastIndexOf( '.' );
  if( p >= 0 )
  {
    _pd->recordSuffix
	    = _pd->recordBaseName.right( _pd->recordBaseName.length() - p );
    _pd->recordBaseName = _pd->recordBaseName.left( p );
    if( format.isEmpty() )
      _pd->recordFormat = formatFromName( _pd->recordBaseName );
  }
  else
    _pd->recordSuffix = "";
  cout << "record ON\n";
  _pd->record = true;
  _pd->recIndex = 0;
}


void GLWidgetManager::recordStart()
{
  QStringList	names = fileAndFormat( "Record window in images" );
  if( names.count() != 2 )
    {
      cout << "save aborted\n";
      return;
    }
  recordStart( names.first(), names.last() );
}


void GLWidgetManager::recordStop()
{
  _pd->record = false;
}


void GLWidgetManager::setPreferredSize( int w, int h )
{
  _pd->prefSize = QSize( w, h );
}


void GLWidgetManager::project()
{
  // Make our OpenGL context current
  _pd->glwidget->makeCurrent();

  // Projection matrix: should be defined only at init time and when resizing
  float	w = _pd->glwidget->width(), h = _pd->glwidget->height();
  float ratio = w / h;

  float	sizex = ( _pd->bmaxw[0] - _pd->bminw[0] ) / 2;
  float	sizey = ( _pd->bmaxw[1] - _pd->bminw[1] ) / 2;
  float oratio = ratio / sizex * sizey;
  if( oratio <= 1.0 )
    sizey /= oratio;
  else
    sizex *= oratio;
  sizex /= _pd->zoom;
  sizey /= _pd->zoom;

  Point3df		bmin, bmax;
  Transformation	t( 0, 0 );

  t.setQuaternion( quaternion() );
  t.invert();

  Point3df	bmino = _pd->bmino - _pd->center;
  Point3df	bmaxo = _pd->bmaxo - _pd->center;
  bmino[2] *= -1;
  bmaxo[2] *= -1;
  t.transformBoundingBox( bmino, bmaxo, bmin, bmax );

  //	viewport setup

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  if( _pd->perspective )
    {
      float pnear = -bmax[2], pfar = -bmin[2];
      if( pnear <= 0 )
      {
        if( pfar <= 0 )
          pnear = 1;
        else
          pnear = pfar * 0.05;
      }
      if( pfar < pnear )
        pfar = pnear * 10;
      gluPerspective( 45, ratio, pnear, pfar );
    }
  else
    {
      //cout << "clip [ " << bmin[2] << ", " << bmax[2] << "]\n";
      //cout << "glWidget ortho : " << sizex*2 << " x " << sizey*2 << endl;
      glOrtho( -sizex, sizex, -sizey, sizey, -bmax[2], -bmin[2] );
    }

  // Modelview matrix: we now use the viewport coordinate system
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


void GLWidgetManager::setupView()
{
  project();
  // Modelview matrix: we only apply rotation for now!
  glLoadMatrixf( &_pd->rotation[0] );

  // Viewport to draw objects into
  glViewport( 0, 0, _pd->glwidget->width(), _pd->glwidget->height() );

  // Modelview matrix: we can now apply translation and left-right mirroring
  glMatrixMode( GL_MODELVIEW );
  glScalef( _pd->invertX ? -1 : 1, _pd->invertY ? -1 : 1, _pd->invertZ ? -1 : 1 );
  glTranslatef( -_pd->center[0], -_pd->center[1], -_pd->center[2] );
}


bool GLWidgetManager::positionFromCursor( int x, int y, Point3df & position )
{
  _pd->glwidget->makeCurrent();
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  updateZBuffer();

  setupView();
  y = _pd->glwidget->height() - 1 - y;
  // get z coordinate in the depth buffer
  GLfloat z = 2.;
  glReadPixels( (GLint)x, (GLint) y, 1, 1, 
                GL_DEPTH_COMPONENT, GL_FLOAT, &z );

  // if this z-buffer pixel still has its initial value,
  // we interpret it as being `background'
  // => take no action
  // not perfect, but should do the job...
  if (z >= 1.0) // initial value of the z-buffer
  {
    // hit background!
    //cout << "background\n";
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    return( false );
  }
  else
  {
    // from window coordinates to object coordinates
    // see "OpenGL programming Guide, Second Edition", p. 149
    GLint viewport[4];
    GLdouble mmatrix[16];
    GLdouble pmatrix[16];
    GLdouble wx, wy, wz;
    glGetIntegerv( GL_VIEWPORT, viewport );
    glGetDoublev( GL_MODELVIEW_MATRIX, mmatrix );
    glGetDoublev( GL_PROJECTION_MATRIX, pmatrix );
    gluUnProject( (GLdouble) x, (GLdouble) y, z,
                  mmatrix, pmatrix, viewport,
                  &wx, &wy, &wz);

    position = Point3df( wx, wy, wz );
    //cout << "readpixel position : " << position << endl;

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    return( true );
  }
}

bool GLWidgetManager::cursorFromPosition( const Point3df & position, Point3df & cursor )
{
	_pd->glwidget->makeCurrent();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

	updateZBuffer();
	setupView();

	GLint viewport[4];
	GLdouble mmatrix[16];
	GLdouble pmatrix[16];
	GLdouble ox, oy, oz;
	glGetIntegerv( GL_VIEWPORT, viewport );
	glGetDoublev( GL_MODELVIEW_MATRIX, mmatrix );
	glGetDoublev( GL_PROJECTION_MATRIX, pmatrix );
	gluProject( (GLdouble) position[0], (GLdouble) position[1], (GLdouble) position[2],
				  mmatrix, pmatrix, viewport,
				  &ox, &oy, &oz);

	oy = _pd->glwidget->height() - 1 - oy;

	cursor[0] = ox;
	cursor[1] = oy;

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	return true;
}

void GLWidgetManager::copyBackBuffer2Texture(void)
{
  // cout << "copyBackBuffer2Texture\n";
  _pd->glwidget->makeCurrent();
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  setupView();

  AWindow3D *w3 = dynamic_cast<AWindow3D *> (aWindow());

  if (w3 && w3->surfpaintIsVisible())
  {
  // TODO : le rendu est fait sur l'object situé au centre de l'image (à améliorer)
    //AObject *obj = w3->objectAtCursorPosition(_pd->glwidget->width()/2,_pd->glwidget->height()/2);
    //AObject *obj = w3->objectAtCursorPosition(_pd->mouseX,_pd->mouseY);

    //le rendu est fait sur le dernier objet sélectionné de type ATexSurface

    AObject *obj = 0;
    string objtype;

    map< unsigned, set< AObject *> > sel = SelectFactory::factory()->selected ();
    map< unsigned, set< AObject *> >::iterator iter( sel.begin( ) ),last( sel.end( ) ) ;
    int num_obj = 0;

    while( iter != last )
    {
      for( set<AObject*>::iterator it = iter->second.begin() ;
        it != iter->second.end() ; ++it )
      {
        if ((AObject::objectTypeName((*it)->type()) == "TEXTURED SURF."))
        {
          objtype = AObject::objectTypeName((*it)->type());
          obj = (*it);
          cout << obj << " " << (*it)->name() << "\n";
          break;
        }
      }
      ++iter ;
    }
    if( !obj )
    {
      set<AObject *> objs = w3->Objects();
      for( set<AObject*>::iterator it=objs.begin(), et=objs.end(); it!=et;
        ++it )
      {
        if ((AObject::objectTypeName((*it)->type()) == "TEXTURED SURF."))
        {
          objtype = AObject::objectTypeName((*it)->type());
          obj = (*it);
          cout << " " << (*it)->name() << "\n";
          break;
        }
      }
    }

    if (theAnatomist->userLevel() >= 3)
    {
      cout << "obj " << obj << endl;
      if( obj )
        cout << obj->name() << endl;
      cout << "mouseX " << _pd->mouseX << " mouseY " << _pd->mouseY << endl;
    }

    cout << "renderSelectionBuffer\n";
    w3->renderSelectionBuffer(ViewState::glSELECTRENDER_POLYGON, obj);

    //glFlush(); // or glFinish() ?
    glFinish();
    glReadBuffer( GL_BACK);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned long bufsz = _pd->glwidget->width() * _pd->glwidget->height() * 3;

    //if (theAnatomist->userLevel() >= 3)
    cout << "back buffer size: " << _pd->backBufferTexture.size() << ", needs: "<< _pd->glwidget->width() << " x " << _pd->glwidget->height() << " x 4 = " << bufsz << endl;

    if( bufsz != _pd->backBufferTexture.size() )
      _pd->backBufferTexture.resize( _pd->glwidget->width() * _pd->glwidget->height() * 3 );

    glReadPixels(0, 0, _pd->glwidget->width(), _pd->glwidget->height(),GL_RGB,
        GL_UNSIGNED_BYTE, &_pd->backBufferTexture[0] );

    //glFinish();
  }

  _pd->resized = false;

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}

void GLWidgetManager::readBackBuffer( int x, int y, GLubyte & red,
                                      GLubyte & green, GLubyte & blue )
{
  _pd->glwidget->makeCurrent();
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  setupView();
  glFlush(); // or glFinish() ?
  glReadBuffer( GL_BACK );
  GLubyte rgba[4];
  glReadPixels( x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba );
  red = rgba[0];
  green = rgba[1];
  blue = rgba[2];

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}

GLubyte* GLWidgetManager::getTextureFromBackBuffer(void)
{
  return &_pd->backBufferTexture[0];
}

bool GLWidgetManager::translateCursorPosition( float x, float y,
                                               Point3df & position )
{
  Point3df	sz = windowBoundingMax() - windowBoundingMin();
  float oratio = float(_pd->glwidget->width()) / _pd->glwidget->height() / sz[0]
      * sz[1];
  if( oratio <= 1 )
    {
      x *= sz[0] / _pd->glwidget->width();
      y *= sz[1] / _pd->glwidget->height() / oratio;
    }
  else
    {
      x *= sz[0] / _pd->glwidget->width() * oratio;
      y *= sz[1] / _pd->glwidget->height();
    }
  x /= zoom();
  y /= zoom();
  position = quaternion().inverse().apply( Point3df( x, y, 0 ) );
  if( invertedZ() )
    position[2] *= -1;	// indirect referential

  return true;
}


#if QT_VERSION >= 0x040600
void GLWidgetManager::gestureEvent( QGestureEvent * event )
{
  controlSwitch()->gestureEvent( event );
}
#endif


void GLWidgetManager::mousePressEvent( QMouseEvent* ev )
{
  //cout << "GLWidgetManager::mousePressEvent\n";
  _pd->mouseX = ev->x();
  _pd->mouseY = ev->y();

  if (ev->buttons() == Qt::LeftButton && _pd->resized)
    copyBackBuffer2Texture();

  controlSwitch()->mousePressEvent( ev );
}


void GLWidgetManager::mouseReleaseEvent( QMouseEvent* ev )
{
//  cout << "GLWidgetManager::mouseReleaseEvent\n";
//  cout << "button : " << (int) ev->button() << endl;
//  cout << "state  : " << (int) ev->state() << endl;

  // WARNING what is that button 4 / modifiers 4 ??
  if ((ev->button() == 4) && (ev->modifiers() == 4))
    copyBackBuffer2Texture();

  controlSwitch()->mouseReleaseEvent( ev );
}


void GLWidgetManager::mouseDoubleClickEvent( QMouseEvent* ev )
{
  _pd->mouseX = ev->x();
  _pd->mouseY = ev->y();

  if (ev->buttons() == Qt::LeftButton && _pd->resized)
    copyBackBuffer2Texture();

  controlSwitch()->mouseDoubleClickEvent( ev );
}


void GLWidgetManager::mouseMoveEvent( QMouseEvent* ev )
{
//  cout << "GLWidgetManager::mouseMoveEvent\n";
//  cout << "button : " << (int) ev->button() << endl;
//  cout << "state  : " << (int) ev->state() << endl;
  controlSwitch()->mouseMoveEvent( ev );
}


void GLWidgetManager::keyPressEvent( QKeyEvent* ev )
{
  /* cout << "GLWidgetManager::keyPressEvent\n";
  cout << "key   : " << ev->key() << endl;
  cout << "state : " << (int) ev->state() << endl; */
  _pd->lastkeypress_for_qt_bug = ev->key();
  controlSwitch()->keyPressEvent( ev );
}


void GLWidgetManager::keyReleaseEvent( QKeyEvent* ev )
{
  /* cout << "GLWidgetManager::keyReleaseEvent\n";
  cout << "key   : " << ev->key() << endl;
  cout << "state : " << (int) ev->state() << endl; */
  if( ev->key() == 0 )
  {
    // cout << "qtbug. taking " << _pd->lastkeypress_for_qt_bug << endl;
    QKeyEvent	e( ev->type(), _pd->lastkeypress_for_qt_bug, ev->modifiers(),
                  ev->text(), ev->isAutoRepeat(), ev->count() );
    controlSwitch()->keyReleaseEvent( &e );
  }
  else
    controlSwitch()->keyReleaseEvent( ev );
}


void GLWidgetManager::wheelEvent( QWheelEvent* ev )
{
//  cout << "wheelEvent glwidget \n" ;
  controlSwitch()->wheelEvent( ev );
}


string GLWidgetManager::name() const
{
  return( "QAGLWidget" );
}


void GLWidgetManager::setZoom( float z )
{
  _pd->zoom = z;
  _pd->resized = true;
  if( _pd->righteye )
    _pd->righteye->setZoom( z );
}


float GLWidgetManager::zoom() const
{
  return( _pd->zoom );
}


const Quaternion & GLWidgetManager::quaternion() const
{
  return( _pd->quaternion );
}


void GLWidgetManager::setQuaternion( const Point4df & q )
{
  if( _pd->righteye )
  {
    Quaternion qr( q );
    qr *= _pd->quaternion.inverse();
    _pd->righteye->setQuaternion( qr * _pd->righteye->quaternion() );
  }
  _pd->quaternion = q;
  _pd->buildRotationMatrix();
  _pd->resized = true;
}


void GLWidgetManager::setQuaternion( const Quaternion & q )
{
  if( _pd->righteye )
  {
    _pd->righteye->setQuaternion( q * _pd->quaternion.inverse()
                                * _pd->righteye->quaternion() );
  }
  _pd->quaternion = q;
  _pd->buildRotationMatrix();
  _pd->resized = true;
}


const float* GLWidgetManager::rotation() const
{
  return( &_pd->rotation[0] );
}


bool GLWidgetManager::invertedX() const
{
  return( _pd->invertX );
}


bool GLWidgetManager::invertedY() const
{
  return( _pd->invertY );
}


bool GLWidgetManager::invertedZ() const
{
  return( _pd->invertZ );
}


void GLWidgetManager::setXDirection( bool inv )
{
  _pd->invertX = inv;
  _pd->resized = true;
}


void GLWidgetManager::setYDirection( bool inv )
{
  _pd->invertY = inv;
  _pd->resized = true;
}


void GLWidgetManager::setZDirection( bool inv )
{
  _pd->invertZ = inv;
  _pd->resized = true;
}


void GLWidgetManager::setRotationCenter( const Point3df & c )
{
  _pd->autocenter = false;
  if( _pd->righteye )
  {
    _pd->righteye->setRotationCenter( c + _pd->righteye->rotationCenter()
                                    - _pd->center );
    _pd->center = c;
  }
  else
    _pd->center = c;
  _pd->resized = true;
}


Point3df GLWidgetManager::rotationCenter() const
{
  return( _pd->center );
}


bool GLWidgetManager::perspectiveEnabled() const
{
  return( _pd->perspective );
}


void GLWidgetManager::enablePerspective( bool p )
{
  _pd->perspective = p;
  _pd->resized = true;
}


void GLWidgetManager::setAutoCentering( bool x )
{
  _pd->autocenter = x;
  if( x )
    _pd->setAutoCenter();
  if( _pd->righteye )
    _pd->righteye->setAutoCentering( x );
}


QSize GLWidgetManager::minimumSizeHint() const
{
  if( _pd->minSizeHint == QSize( 0, 0 ) )
  {
    if( dynamic_cast<const QGLWidget *>( this ) == _pd->glwidget )
      return _pd->glwidget->QGLWidget::minimumSizeHint();
    else
      return _pd->glwidget->minimumSizeHint();
  }
  return( _pd->minSizeHint );
}


void GLWidgetManager::setMinimumSizeHint( const QSize & sz )
{
  _pd->minSizeHint = sz;
}


void GLWidgetManager::setOtherBuffersSaveMode( int mode )
{
  _pd->otherbuffers = mode;
}


int GLWidgetManager::otherBuffersSaveMode() const
{
  return _pd->otherbuffers;
}


void GLWidgetManager::focusInEvent( QFocusEvent * )
{
}


void GLWidgetManager::focusOutEvent( QFocusEvent * )
{
}


GLWidgetManager* GLWidgetManager::rightEye()
{
  return _pd->righteye;
}


GLWidgetManager* GLWidgetManager::leftEye()
{
  return _pd->lefteye;
}


void GLWidgetManager::setRightEye( GLWidgetManager* w )
{
  if( _pd->righteye == w )
    return;
  if( _pd->righteye )
    _pd->righteye->setLeftEye( 0 );
  _pd->righteye = w;
  if( w )
    w->setLeftEye( this );
}


void GLWidgetManager::setLeftEye( GLWidgetManager* w )
{
  if( _pd->lefteye == w )
    return;
  if( _pd->lefteye && w )
    _pd->lefteye->setRightEye( 0 );
  _pd->lefteye = w;
}


const AWindow* GLWidgetManager::aWindow() const
{
  return View::aWindow();
}

AWindow* GLWidgetManager::aWindow()
{
  return View::aWindow();
}


bool GLWidgetManager::recording() const
{
  return _pd->record;
}


