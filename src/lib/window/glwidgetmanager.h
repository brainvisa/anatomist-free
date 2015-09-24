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


#ifndef ANATOMIST_WINDOW_GLWIDGETMANAGER_H
#define ANATOMIST_WINDOW_GLWIDGETMANAGER_H

#include <anatomist/controler/view.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/window/viewstate.h>

#include <qglobal.h>
#if QT_VERSION>=0x040000
#include <QtOpenGL/QGLWidget>
#else
#include <qgl.h>
#endif
#if QT_VERSION >= 0x040600
class QGestureEvent;
#endif


namespace aims
{
  class Quaternion;
}

class GLWidgetManager_Private_QObject;


namespace anatomist
{
  class AWindow;

  namespace internal
  {
    class AGraphicsView;
  }


  /** Base class for OpenGL-rendering widget. Actually this is *not* a
  QWidget (or a QGLWidget) since in some cases we need to separate this
  implementation from the actual Qt widget which can be an inherited one.
  */
  class GLWidgetManager : public anatomist::View
  {
  public:
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    virtual void updateGL();
    void renderBackBuffer( ViewState::glSelectRenderMode selectmode );
    void copyBackBuffer2Texture(void);
    /// basically calls paintGL()
    virtual void paintScene();

  protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();
#if QT_VERSION >= 0x040600
    virtual void gestureEvent( QGestureEvent *event );
#endif
    virtual void mousePressEvent( QMouseEvent* me );
    virtual void mouseReleaseEvent( QMouseEvent* me );
    virtual void mouseMoveEvent( QMouseEvent* me );
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent* ev );
    virtual void keyReleaseEvent( QKeyEvent* ev );
    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );
    virtual void wheelEvent( QWheelEvent * );

  public:
    // this private structure is public because it's used by internal functions
    struct Private;
    friend class ::GLWidgetManager_Private_QObject;

    GLWidgetManager( anatomist::AWindow* win, QGLWidget* widget );
    virtual ~GLWidgetManager();

    QGLWidget* qglWidget();
    /** this QObject is used for slots: updateZBuffer, saveContents,
    recordStart, recordStop */
    QObject* qobject();

    void setPrimitives( const anatomist::GLPrimitives & li );
    anatomist::GLPrimitives primitives() const;
    void setSelectionPrimitives( const anatomist::GLPrimitives & li );
    anatomist::GLPrimitives selectionPrimitives() const;
    void clearLists();
    /** set objects extrema, this also automatically sets the window bounding 
        box */
    void setExtrema( const Point3df & bmin, const Point3df & bmax );
    /// set window extrema (in the local orientation)
    void setWindowExtrema( const Point3df & bmin, const Point3df & bmax );
    Point3df boundingMin() const;
    Point3df boundingMax() const;
    Point3df windowBoundingMin() const;
    Point3df windowBoundingMax() const;
    void setLightGLList( GLuint l );
    GLuint lightGLList() const;
    void setPreferredSize( int, int );
    void setMinimumSizeHint( const QSize & );
    virtual bool positionFromCursor( int x, int y, Point3df & position );
    virtual bool cursorFromPosition( const Point3df & position, Point3df & cursor );
    virtual void readBackBuffer( int x, int y, GLubyte & red, GLubyte & green,
                                 GLubyte & blue );
    GLubyte* getTextureFromBackBuffer (void);
    //GLubyte* getTextureFromBackBuffer (void);
    virtual bool translateCursorPosition( float x, float y,
                                          Point3df & position );

    virtual std::string name() const;
    void setZoom( float z );
    float zoom() const;
    /** The quaternion rotates the initial eye direction (0, 0, 1) into the
        view coordinates, before Z axis inversion. */
    const aims::Quaternion & quaternion() const;
    void setQuaternion( const Point4df & q );
    void setQuaternion( const  aims::Quaternion & q );
    const float* rotation() const;
    void setXDirection( bool invert );
    void setYDirection( bool invert );
    void setZDirection( bool invert );
    bool invertedX() const;
    bool invertedY() const;
    bool invertedZ() const;
    void setRotationCenter( const Point3df & );
    Point3df rotationCenter() const;
    bool perspectiveEnabled() const;
    void enablePerspective( bool );
    void setAutoCentering( bool );
    bool autoCentering() const;
    virtual void recordStart( const QString & basename, 
                              const QString & format = QString::null );
    void saveContents( const QString & filename, const QString & format );
    void saveOtherBuffer( const QString & filename, 
                          const QString & format, int mode );
    void setOtherBuffersSaveMode( int mode );
    int otherBuffersSaveMode() const;

    bool hasTransparentObjects() const;
    void setTransparentObjects( bool );
    bool depthPeelingAllowed() const;
    bool depthPeelingEnabled() const;
    void enableDepthPeeling( bool );
    unsigned depthPeelingPasses() const;
    void setDepthPeelingPasses( unsigned n );
    unsigned numTextureUnits() const;
    bool recording() const;

    static QGLWidget* sharedWidget();
    void setBackgroundAlpha( float a );

    GLWidgetManager* rightEye();
    GLWidgetManager* leftEye();
    void setRightEye( GLWidgetManager* );
    void setLeftEye( GLWidgetManager* );

    AWindow * aWindow();
    const AWindow * aWindow() const;

    virtual int width() = 0;
    virtual int height() = 0;
  // public slots:

    virtual void saveContents();
    virtual void recordStart();
    virtual void recordStop();

    /// QGraphicsView needs to call event methods
    friend class anatomist::internal::AGraphicsView;

  protected:
    enum DrawMode
    {
      Normal,
      ZSelect,
      ObjectSelect,
      ObjectsSelect,
      PolygonSelect
    };

    virtual void project();
    virtual void setupView();
    void drawObjects( DrawMode m = Normal );
    void depthPeelingRender( DrawMode m = Normal );
    virtual void paintGL( DrawMode m );
    void record();

    anatomist::GLPrimitives _primitives;
    anatomist::GLPrimitives _selectprimitives;

    virtual void updateZBuffer();
    bool isZBufferUpToDate() const;
    void setZBufferUpdated( bool );
    void stopZBufferTimer();
    bool isRGBBufferUpToDate() const;
    void setRGBBufferUpdated( bool );

  private:
    Private *_pd;
  };

};


/// private internal class
class GLWidgetManager_Private_QObject : public QObject
{
  Q_OBJECT

public:
  GLWidgetManager_Private_QObject( QObject* parent,
                                    anatomist::GLWidgetManager* man );
  virtual ~GLWidgetManager_Private_QObject();

public slots:
  void saveContents();
  void recordStart();
  void recordStop();

protected slots:
  void updateZBuffer();

private:
  anatomist::GLWidgetManager *_manager;
};


#endif
