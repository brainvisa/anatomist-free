namespace anatomist
{

  class AWindow : carto::SharedObject /Abstract/
  {
%TypeHeaderCode
#include <anatomist/window/Window.h>
%End

%ConvertToSubClassCode
  sipClass = 0;
  if( dynamic_cast<QAWindow *>(sipCpp) )
  {
    *sipCppRet = static_cast<QAWindow *>( sipCpp );
    if( dynamic_cast<ControlledWindow *>( sipCpp ) )
      if( dynamic_cast<AWindow3D *>( sipCpp ) )
        sipClass = sipClass_AWindow3D;
      else
        sipClass = sipClass_ControlledWindow;
      else
        sipClass = sipClass_QAWindow;
  }
%End

  public:
    enum Type { CTRL_WINDOW = 100, WINDOW_2D = 101, WINDOW_3D = 102 };
    enum SubType { AXIAL_WINDOW = 200,
	SAGITTAL_WINDOW = 201,
	CORONAL_WINDOW = 202,
	OBLIQUE_WINDOW = 203,
	OTHER = 299
    };

    virtual ~AWindow();
    virtual std::string Title() const;

    virtual Type type() const = 0;
    virtual SubType subtype() const /AutoGen/;
    virtual void registerObject( anatomist::AObject *, bool = false,
      int=-1 ) /AutoGen/;
    virtual void unregisterObject( anatomist::AObject * ) /AutoGen/;
    virtual void Refresh();
    int Group() const;
    bool hasObject(anatomist::AObject *) const;
    float GetTime() const;
    void setHasCursor(int);
    virtual void showToolBars(int = 2 );
    void setFullScreen(int);
    set_AObjectPtr Objects() const;
    virtual anatomist::AObject *objectAt( float x, float y, float z, float t );
    bool isTemporary( anatomist::AObject* o ) const;

static anatomist::AWindow* fromObject( carto::GenericObject );
%MethodCode
  try
  {
    sipRes = a0->value<anatomist::AWindow *>();
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End


  protected:
    AWindow();

  private:
    AWindow( const anatomist::AWindow & );

    virtual void displayClickPoint() const = 0;
  };


/*
  class QAWindowDrag : QStoredDrag
  {
%TypeHeaderCode
#include <anatomist/control/windowdrag.h>
%End

  public:
    QAWindowDrag( const set_AWindowPtr &, QWidget * dragSource = 0,
		  const char * name = 0 );
    virtual ~QAWindowDrag();

    static bool canDecode( const QMimeSource * e );
    static bool decode( const QMimeSource * e, set_AWindowPtr & o );
  };
*/


class View
{
%TypeHeaderCode
#include <anatomist/controler/view.h>
#include <anatomist/window/glwidget.h>

%End

%ConvertToSubClassCode
  if( dynamic_cast<QAGLWidget *>( sipCpp ) )
  {
    *sipCppRet = static_cast<QAGLWidget *>( sipCpp );
    sipClass = sipClass_QAGLWidget;
  }
  else
    sipClass = NULL;
%End

  public:
  View( anatomist::AWindow * );
  virtual ~View();
  
  QAGLWidget* QAGLWidget();
%MethodCode
    sipRes = dynamic_cast<QAGLWidget *>( sipCpp );
%End

  ControlSwitch * controlSwitch();
  virtual std::string name() const =0;
  anatomist::AWindow * window();
  // const anatomist::AWindow * window() const;
};

};


class QAWindow : QMainWindow, anatomist::AWindow /Abstract/
{
%TypeHeaderCode
#include <qmainwindow.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/qwindow.h>
#include <stdio.h>
%End

public:
  QAWindow( QWidget * /TransferThis/ = 0, const char *=0,
    carto::Object =carto::none(), Qt::WFlags=Qt::WType_TopLevel );
  virtual ~QAWindow();
  virtual void refreshNow() /AutoGen/;
  static QAWindow* fromObject( carto::GenericObject );
%MethodCode
  try
  {
    anatomist::AWindow *aw = a0->value<anatomist::AWindow *>();
    sipRes = dynamic_cast<QAWindow *>(aw);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End
};



class QAGLWidget : QGLWidget, anatomist::View
{
%TypeHeaderCode

#include <anatomist/window/glwidget.h>
#include <pyaims/vector/vector.h>

%End

  public:

    QAGLWidget( anatomist::AWindow*, QWidget* = 0, 
                const char* = 0, 
                const QGLWidget * = 0, Qt::WFlags = 0 );
    virtual ~QAGLWidget();

    void setPrimitives( const anatomist::GLPrimitives & );
    anatomist::GLPrimitives primitives() const;

    void clearLists();
    void setExtrema( const Point3df &, const Point3df & );
    void setWindowExtrema( const Point3df &, const Point3df & );
    Point3df boundingMin() const;
    Point3df boundingMax() const;
    Point3df windowBoundingMin() const;
    Point3df windowBoundingMax() const;
    void setLightGLList( unsigned );
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    void setPreferredSize( int, int );
    void setMinimumSizeHint( const QSize & );
    virtual bool positionFromCursor( int, int, Point3df & );
    virtual bool translateCursorPosition( float, float, Point3df & );

    virtual std::string name() const;
    void setZoom( float );
    float zoom() const;
    const aims::Quaternion & quaternion() const;
    void setQuaternion( const Point4df & );
    void setQuaternion( const  aims::Quaternion & );
    /* const float* rotation() const; */
    void setXDirection( bool );
    void setYDirection( bool );
    void setZDirection( bool );
    bool invertedX() const;
    bool invertedY() const;
    bool invertedZ() const;
    void setRotationCenter( const Point3df & );
    Point3df rotationCenter() const;
    bool perspectiveEnabled() const;
    void enablePerspective( bool );
    void setAutoCentering( bool );
    virtual void recordStart( const QString &, 
                              const QString & = QString::null );
    void saveContents( const QString &, const QString & );
    void saveOtherBuffer( const QString &, 
                          const QString &, int );
    void setOtherBuffersSaveMode( int );
    int otherBuffersSaveMode() const;
  
    bool hasTransparentObjects() const;
    void setTransparentObjects( bool );
    bool depthPeelingAllowed() const;
    bool depthPeelingEnabled() const;
    void enableDepthPeeling( bool );
    unsigned depthPeelingPasses() const;
    void setDepthPeelingPasses( unsigned );
    unsigned numTextureUnits() const;

  public slots:
    virtual void updateGL();
    virtual void saveContents();
    virtual void recordStart();
    virtual void recordStop();

  private :
    QAGLWidget( const QAGLWidget& );
};


class AWindow3D : ControlledWindow, anatomist::Observable
{
%TypeHeaderCode
#include <anatomist/window3D/window3D.h>
%End
public:
  virtual ~AWindow3D();

  void focusView();
  void refreshTemp();
  void refreshTempNow();
  void refreshLightView();
  void refreshLightViewNow();
};


class FixedPointOfViewWindow : QWidget, anatomist::Observer
{
%TypeHeaderCode
#include <anatomist/window3D/wFixedPointOfView.h>
#include <anatomist/window3D/window3D.h>
%End
public:
  FixedPointOfViewWindow( AWindow3D* win, QWidget* parent = 0,
                          const char* name = 0 );
  virtual ~FixedPointOfViewWindow();

  void straightPOV(int);
  void edgePOV(int);
  void cornerPOV(int);
};
