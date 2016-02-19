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
#ifndef _vtkQtRenderWindowInteractor2_h
#define _vtkQtRenderWindowInteractor2_h

#include <qglobal.h>
#include <QtOpenGL/QGLWidget>
#include <qpaintdevice.h>
#include <qtimer.h>

#ifdef Q_WS_X11
#include "vtkXOpenGLRenderWindow.h"
#endif

#include "vtkRenderWindowInteractor.h"

#ifdef WIN32
  #define VTK_QT_EXPORT __declspec (dllexport)
#else
  #define VTK_QT_EXPORT
#endif

#ifndef VTK_QT_EXPORT
#ifdef _WIN_32
  #define VTK_QT_EXPORT __declspec (dllexport)
#else
  #define VTK_QT_EXPORT
#endif
#endif

class vtkRenderWindow;

class VTK_QT_EXPORT vtkQtRenderWindowInteractor2 : public QGLWidget, virtual public vtkRenderWindowInteractor {
  Q_OBJECT
    
    public:

  vtkQtRenderWindowInteractor2();
  vtkQtRenderWindowInteractor2(QWidget* parent=0, const char* name=0,
			       const QGLWidget* shareWidget = 0, Qt::WFlags f=0 );
  vtkQtRenderWindowInteractor2(const QGLFormat& format, QWidget* parent=0, const char* name=0,
			       const QGLWidget* shareWidget = 0, Qt::WFlags f=0 );
  
  static int IsTypeOf(const char *type)
  {
    if ( !strcmp("vtkQtRenderWindowInteractor2",type) )
      return 1;
    return vtkRenderWindowInteractor::IsTypeOf(type);
  }
  virtual int IsA(const char *type)
  {
    return this->IsTypeOf(type);
  }
  
  virtual ~vtkQtRenderWindowInteractor2();
  
  static vtkQtRenderWindowInteractor2 *New(); // inline

  void PrintSelf(ostream& os, vtkIndent indent);

  
  // vtkRenderWindowInteractor overrides
  void Initialize();
  void Enable();
  void Disable();
  void Start();
  void UpdateSize(int x, int y);
  void TerminateApp() {};
  void Render();

  void SetRenderWindow (vtkRenderWindow*);
  
  // timer methods
  virtual void SetTimerDuration (unsigned long);
  virtual int CreateTimer(int timertype);
  virtual int DestroyTimer(void);


  virtual QSizePolicy sizePolicy() const { return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding); }
  virtual QSize sizeHint()         const { return QSize(500, 500); }
  virtual QSize minimumSizeHint()  const { return QSize(500, 500); }
  
  
  //virtual void initializeGL();
  virtual void paintGL();
  virtual void resizeGL(int, int);
  virtual void focusInEvent(QFocusEvent*){};
  virtual void focusOutEvent(QFocusEvent*){};

#ifdef Q_WS_X11
  int GetDesiredDepth();
  Colormap GetDesiredColormap();
  Visual *GetDesiredVisual();
#endif


  int    UpdateRenderWindow;
  
    
  protected slots:
  void timer();
    
    
 protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void keyPressEvent(QKeyEvent*);
  virtual void wheelEvent (QWheelEvent*);

 private:
  virtual const char* GetClassNameInternal() const
    { return "vtkQtRenderWindowInteractor2"; }

  void InitRenderWindowInteractor (void);
  
  QTimer qTimer;
  long   Handle;

#if defined (Q_WS_MAC) && QT_VERSION < 0x040000
  void macFixRect(void);
#endif
  
};

#endif


