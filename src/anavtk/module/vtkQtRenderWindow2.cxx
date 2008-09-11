#include <anatomist/module/vtkQtRenderWindowInteractor2.h>

#include <vtkObjectFactory.h>
#include <anatomist/module/vtkQtRenderWindow2.h>


vtkCxxRevisionMacro(vtkQtRenderWindow2, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkQtRenderWindow2);


void vtkQtRenderWindow2::MakeCurrent()
{
  if( this->Interactor )
  {

    vtkQtRenderWindowInteractor2* iren = dynamic_cast<vtkQtRenderWindowInteractor2*>(this->Interactor);
    if( iren )
    {
      iren->makeCurrent();
    }
    else
    {
#ifdef Q_WS_X11
      vtkXOpenGLRenderWindow::MakeCurrent();
#endif
#ifdef Q_WS_WIN32
      vtkWin32OpenGLRenderWindow::MakeCurrent();
#endif
#ifdef Q_WS_MAC
      vtkCarbonRenderWindow::MakeCurrent();
#endif
    }
    
  }
}
