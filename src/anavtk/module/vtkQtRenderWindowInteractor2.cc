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
#include <qapplication.h>
#include <qcursor.h>
#include "qevent.h"

#include "anatomist/module/vtkQtRenderWindowInteractor2.h"

#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleSwitch.h>

#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
#include <QX11Info>
#endif

#ifdef Q_WS_MAC
#include <vtkCarbonRenderWindow.h>
#endif


vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2() :
  vtkRenderWindowInteractor(), QGLWidget()
{
  this->InitRenderWindowInteractor();
}

vtkQtRenderWindowInteractor2 * vtkQtRenderWindowInteractor2::New()
{
  // we don't make use of the objectfactory, because we're not registered
  return new vtkQtRenderWindowInteractor2((QWidget*)NULL, 0, 0, 0);
}



vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2(
  QWidget* parent, const char* name, const QGLWidget* shareWidget,
  Qt::WindowFlags f) :
  vtkRenderWindowInteractor(), QGLWidget ( parent, shareWidget, f )
{
  setObjectName( name );
  this->InitRenderWindowInteractor();
}



vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2(
  const QGLFormat& format, QWidget* parent, const char* name,
  const QGLWidget* shareWidget, Qt::WindowFlags f) :
  vtkRenderWindowInteractor(), QGLWidget( format, parent, shareWidget, f )
{
  setObjectName( name );
  this->InitRenderWindowInteractor();
}



void vtkQtRenderWindowInteractor2::InitRenderWindowInteractor()
{
  this->RenderWindow = NULL;

  vtkRenderWindow* rwin = vtkRenderWindow::New();
  this->SetRenderWindow ( rwin );
  rwin->Delete();

  this->Handle = 0;
  this->UpdateRenderWindow = 1;

#if QT_VERSION < 0x040000
  setFocusPolicy(QWidget::WheelFocus);
#else
  setFocusPolicy(Qt::WheelFocus);
#endif

  vtkInteractorStyleSwitch* tb = vtkInteractorStyleSwitch::New();
  tb->SetCurrentStyleToTrackballCamera ();
  this->SetInteractorStyle (tb);
  tb->Delete();

}



void vtkQtRenderWindowInteractor2::SetRenderWindow (vtkRenderWindow* w)
{
  // do nothing if we don't have to
  if(w == this->RenderWindow )
  {
    return;
  }

  // unregister previous window
  if(this->RenderWindow)
  {
    //clean up window as one could remap it
    if(this->RenderWindow->GetMapped())
    {
      this->RenderWindow->Finalize();
    }
    this->RenderWindow->SetDisplayId(NULL);
    this->RenderWindow->SetParentId(NULL);
    this->RenderWindow->SetWindowId(NULL);
    this->RenderWindow->UnRegister(this);
  }
  
  // now set the window
  this->RenderWindow = w;
  
  if(this->RenderWindow)
  {
    // register new window
    this->RenderWindow->Register(this);
    
    // if it is mapped somewhere else, unmap it
    if(this->RenderWindow->GetMapped())
    {
      this->RenderWindow->Finalize();
    }

    /*    
#ifdef Q_WS_WIN32
    this->Handle = QWidget::winId();
    this->RenderWindow->SetWindowId ( this->Handle );
    this->RenderWindow->SetDeviceContext (GetDC(WindowId));
#endif
    
#ifdef Q_WS_X11
    this->Handle = this->winId(); // QPaintDevice::handle();
    this->RenderWindow->SetDisplayId ( this->x11Display() ); // buggy?
    this->RenderWindow->SetWindowId  ( reinterpret_cast<void*>( this->Handle ) );
#endif

#ifdef Q_WS_MAC
  this->Handle = (long)this->handle();
  vtkCarbonRenderWindow* rwin = vtkCarbonRenderWindow::SafeDownCast(this->RenderWindow);
  if( rwin )
  {
  rwin->SetWindowId( (void*)NULL );
  rwin->SetRootWindow(reinterpret_cast<WindowPtr>( this->Handle ));
  }
#endif
    */
    
    // tell the vtk window what the size of this window is
    this->RenderWindow->vtkRenderWindow::SetSize(this->width(), this->height());
    this->RenderWindow->vtkRenderWindow::SetPosition(this->x(), this->y());
    
    
    // if an interactor wasn't provided, we'll make one by default
    if( !this->RenderWindow->GetInteractor() )
    {
      this->RenderWindow->SetInteractor(this);
    }
    
    // tell the interactor the size of this window
    this->RenderWindow->GetInteractor()->SetSize(this->width(), this->height());

  }
  
}


vtkQtRenderWindowInteractor2::~vtkQtRenderWindowInteractor2()
{
  this->SetRenderWindow(NULL);

  // to circumvent VTK warnings assuming our destructor is called correctly
  if (this->ReferenceCount > 0) {
    this->SetReferenceCount(0);
  }
}



void vtkQtRenderWindowInteractor2::PrintSelf(ostream&os, vtkIndent indent)
{
  vtkRenderWindowInteractor::PrintSelf(os, indent);
}


void vtkQtRenderWindowInteractor2::Initialize()
{
  int *size = this->RenderWindow->GetSize();
  // enable everything and start rendering
  this->Enable();
  if ( this->UpdateRenderWindow )
    this->RenderWindow->Start();
  
  // set the size in the render window interactor
  Size[0] = size[0];
  Size[1] = size[1];

  // this is initialized
  Initialized = 1;
}


void vtkQtRenderWindowInteractor2::Enable()
{
  // if already enabled then done
  if (Enabled)
    return;
  
  // that's it
  this->Enabled = 1;
  this->makeCurrent();
  this->Modified();
}



void vtkQtRenderWindowInteractor2::Disable()
{
  // if already disabled then done
  if (!Enabled)
    return;
  
  // that's it (we can't remove the event handler like it should be...)
  this->Enabled = 0;
  this->Modified();
}


void vtkQtRenderWindowInteractor2::Start()
{
  // the interactor cannot control the event loop
  vtkErrorMacro(<<"vtkQtRenderWindowInteractor2::Start() \
    interactor cannot control event loop.");
}


void vtkQtRenderWindowInteractor2::Render()
{
  long handle = 0;
#if defined( Q_WS_WIN32 ) || defined( Q_OS_WIN32 )
  handle = QWidget::winId();
#endif
  
#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
  handle = this->winId(); //QPaintDevice::handle();  
#endif

#if defined( Q_WS_MAC ) || defined( Q_OS_MAC )
  handle = (long)this->handle();
#endif

  if( this->Handle && this->Handle==handle )
  {
    updateGL();
  }
  else if ( handle )
  {
    this->Handle = handle;
    this->RenderWindow->SetNextWindowId(reinterpret_cast<void *>(handle));
    this->RenderWindow->WindowRemap();
    updateGL();
  }
  
}


void vtkQtRenderWindowInteractor2::UpdateSize(int x, int y)
{
  if( this->RenderWindow )
  {
    // if the size changed tell render window
    if ( x != Size[0] || y != Size[1] )
    {
      // adjust our (vtkRenderWindowInteractor size)
      Size[0] = x;
      Size[1] = y;
      // and our RenderWindow's size
      this->RenderWindow->SetSize(x, y);
    }
  }
}

void vtkQtRenderWindowInteractor2::mousePressEvent(QMouseEvent *me) {

  if (!Enabled)
    return;

  this->SetSize ( this->RenderWindow->GetSize() );
  
  int ctrl = 0, shift = 0;
  if (me->modifiers() & Qt::ControlModifier)
    ctrl = 1;
  if (me->modifiers() & Qt::ShiftModifier)
    shift = 1;
  int xp = me->x();
  int yp = me->y();
  SetEventInformationFlipY(xp, yp, ctrl, shift);

  switch (me->button())
  {
      case Qt::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
        break;
      case Qt::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
        break;
      case Qt::RightButton:
        InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
        break;
      default:
        return;
  }
}

void vtkQtRenderWindowInteractor2::mouseReleaseEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    this->SetSize ( this->RenderWindow->GetSize() );

    int ctrl = 0, shift = 0;
    if (me->modifiers() & Qt::ControlModifier)
        ctrl = 1;
    if (me->modifiers() & Qt::ShiftModifier)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->button())
    {
    case Qt::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
        break;
    case Qt::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
        break;
    case Qt::RightButton:
        InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
        break;
    default:
        return;
    }

}


void vtkQtRenderWindowInteractor2::timer()
{
  if (!Enabled){
    return;
  }
  InvokeEvent(vtkCommand::TimerEvent,NULL);
}


void vtkQtRenderWindowInteractor2::SetTimerDuration (unsigned long duration)
{
  vtkRenderWindowInteractor::SetTimerDuration (duration);
  qTimer.setInterval (duration);
}

int vtkQtRenderWindowInteractor2::CreateTimer(int timertype)
{
  if (timertype == VTKI_TIMER_FIRST)
  {
    QObject::connect(&qTimer, SIGNAL(timeout()), SLOT(timer()));
    qTimer.start( this->GetTimerDuration() );
  }
  return 1;
}


int vtkQtRenderWindowInteractor2::DestroyTimer()
{
  qTimer.stop();
  QObject::disconnect(&qTimer, SIGNAL(timeout()), this, 0);
  return 1;
}




void vtkQtRenderWindowInteractor2::paintGL()
{
  if( !this->Handle )
  {
    
#if defined( Q_WS_WIN32 ) || defined( Q_OS_WIN32 )
    this->Handle = QWidget::winId();
    this->RenderWindow->SetWindowId ( this->Handle );
    this->RenderWindow->SetDeviceContext (GetDC(WindowId));
#endif

#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
    this->Handle = this->winId(); // QPaintDevice::handle();
    this->RenderWindow->SetDisplayId ( QX11Info::display() ); // buggy?
    this->RenderWindow->SetWindowId  ( reinterpret_cast<void*>( this->Handle ) );
#endif

#if defined( Q_WS_MAC ) || defined( Q_OS_MAC )
  this->Handle = (long)this->handle();
  vtkCarbonRenderWindow* rwin = vtkCarbonRenderWindow::SafeDownCast(this->RenderWindow);
  if( rwin )
  {
  rwin->SetWindowId( (void*)NULL );
  rwin->SetRootWindow(reinterpret_cast<WindowPtr>(this->handle()));
  }
#endif

  }

  if( this->UpdateRenderWindow )
    this->RenderWindow->Render();
  
}


void vtkQtRenderWindowInteractor2::resizeGL(int x,int y) {

  if ((this->Size[0] != x)||(this->Size[1] != y))
  {
    this->UpdateSize (x, y);
  }

  if (!Enabled) 
  {
    return;
  }

  InvokeEvent(vtkCommand::ConfigureEvent, NULL);

  //updateGL();

}



void vtkQtRenderWindowInteractor2::mouseMoveEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    this->SetSize ( this->RenderWindow->GetSize() );

    int ctrl = 0, shift = 0;
    if (me->modifiers() & Qt::ControlModifier)
        ctrl = 1;
    if (me->modifiers() & Qt::ShiftModifier)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);
    InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
}



void vtkQtRenderWindowInteractor2::keyPressEvent(QKeyEvent *ke)
{

  if (!Enabled)
    return;
  
  
  this->SetSize ( this->RenderWindow->GetSize() );
  int ctrl = 0, shift = 0;
  if (ke->modifiers() & Qt::ControlModifier)
    ctrl = 1;
  if (ke->modifiers() & Qt::ShiftModifier)
    shift = 1;
  QPoint cp = this->mapFromGlobal(QCursor::pos());
  int xp = cp.x();
  int yp = cp.y();
  SetEventInformationFlipY(xp, yp, ctrl, shift,
                           tolower(ke->text().toStdString().c_str()[0]), 1,
                           ke->text().toStdString().c_str());
  InvokeEvent(vtkCommand::KeyPressEvent, NULL);
  InvokeEvent(vtkCommand::CharEvent, NULL);
 
}




void vtkQtRenderWindowInteractor2::wheelEvent(QWheelEvent *e)
{
  if (!Enabled)
    return;

  this->SetEventInformationFlipY(
    e->x(), e->y(), (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
    (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

  if(e->delta() > 0)
  {
      this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e);
  }
  else
  {
    this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e);
  }
}


#if defined( Q_WS_X11 ) && QT_VERSION < 0x050000 // || defined( Q_OS_LINUX )

int vtkQtRenderWindowInteractor2::GetDesiredDepth()
{
  return QApplication::primaryScreen()->depth();
}


Colormap vtkQtRenderWindowInteractor2::GetDesiredColormap()
{
  return (Colormap) QX11Info::appColormap();
}


Visual *vtkQtRenderWindowInteractor2::GetDesiredVisual()
{
  return (Visual*) QX11Info::appVisual();
}

#endif
