#ifndef _vtkQtRenderWindow2_h_
#define _vtkQtRenderWindow2_h_

#ifdef Q_WS_X11
#include <vtkXOpenGLRenderWindow.h>
#endif

#ifdef Q_WS_WIN32
#include <vtkWin32OpenGLRenderWindow.h>
#endif

#ifdef Q_WS_MAC
#include <vtkCarbonRenderWindow.h>
#endif

class vtkQtRenderWindow2 : 
#ifdef Q_WS_X11
  public vtkXOpenGLRenderWindow
#endif
#ifdef Q_WS_WIN32
  public vtkWin32OpenGLRenderWindow
#endif
#ifdef Q_WS_MAC
  public vtkCarbonRenderWindow
#endif
{

 public:
  static vtkQtRenderWindow2 *New();
#ifdef Q_WS_X11
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkXOpenGLRenderWindow);
#endif
#ifdef Q_WS_WIN32
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkWin32OpenGLRenderWindow);
#endif
#ifdef Q_WS_MAC
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkCarbonRenderWindow);
#endif

  void MakeCurrent (void);
  
 protected:
  vtkQtRenderWindow2(){};
  ~vtkQtRenderWindow2(){};

 private:
  vtkQtRenderWindow2 (const vtkQtRenderWindow2&);
  void operator=(const vtkQtRenderWindow2&);
  
};



#endif
