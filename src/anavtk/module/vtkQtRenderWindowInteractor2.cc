#include <qapplication.h>
#include <qcursor.h>

#include "anatomist/module/vtkQtRenderWindowInteractor2.h"
//#include "anatomist/module/vtkQtRenderWindow2.h"

#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleSwitch.h>

#ifdef Q_WS_MAC
#include <vtkCarbonRenderWindow.h>
#endif




vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2() :
  vtkRenderWindowInteractor(), QGLWidget ( (QWidget*)NULL, 0, 0, 0)
{
  this->InitRenderWindowInteractor();
}

vtkQtRenderWindowInteractor2 * vtkQtRenderWindowInteractor2::New()
{
  // we don't make use of the objectfactory, because we're not registered
  return new vtkQtRenderWindowInteractor2 (NULL, 0, 0, 0, 0);
}



vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2(QWidget* parent, const char* name,
							   const QGLWidget* shareWidget, Qt::WFlags f) :
  vtkRenderWindowInteractor(), QGLWidget ( parent, name, shareWidget, f)
{
  this->InitRenderWindowInteractor();
}



vtkQtRenderWindowInteractor2::vtkQtRenderWindowInteractor2(const QGLFormat& format, QWidget* parent, const char* name,
							   const QGLWidget* shareWidget, Qt::WFlags f) :
  vtkRenderWindowInteractor(), QGLWidget ( format, parent, name, shareWidget, f)
{
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

  setFocusPolicy(QWidget::WheelFocus);

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
#ifdef Q_WS_WIN32
  handle = QWidget::winId();
#endif
  
#ifdef Q_WS_X11
  handle = this->winId(); //QPaintDevice::handle();  
#endif

#ifdef Q_WS_MAC
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
  if (me->state() & Qt::ControlButton)
    ctrl = 1;
  if (me->state() & Qt::ShiftButton)
    shift = 1;
  int xp = me->x();
  int yp = me->y();
  SetEventInformationFlipY(xp, yp, ctrl, shift);
  
  switch (me->button()) {
      case QEvent::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
        break;
      case QEvent::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
        break;
      case QEvent::RightButton:
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
    if (me->state() & Qt::ControlButton)
        ctrl = 1;
    if (me->state() & Qt::ShiftButton)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->button()) {
    case QEvent::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
        break;
    case QEvent::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
        break;
    case QEvent::RightButton:
        InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
        break;
    default:
        return;
    }

}


void vtkQtRenderWindowInteractor2::timer()
{
  if (!Enabled)
    return;
  InvokeEvent(vtkCommand::TimerEvent,NULL);
}


int vtkQtRenderWindowInteractor2::CreateTimer(int timertype)
{
  if (timertype == VTKI_TIMER_FIRST)
  {
    QObject::connect(&qTimer, SIGNAL(timeout()), SLOT(timer()));
    qTimer.start(10);
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
  
#if defined (Q_WS_MAC) && (QT_VERSION < 0x040000)
  macFixRect();
#endif

  InvokeEvent(vtkCommand::ConfigureEvent, NULL);
  
  //updateGL();

}



void vtkQtRenderWindowInteractor2::mouseMoveEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    this->SetSize ( this->RenderWindow->GetSize() );

    int ctrl = 0, shift = 0;
    if (me->state() & Qt::ControlButton)
        ctrl = 1;
    if (me->state() & Qt::ShiftButton)
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
  if (ke->state() & Qt::ControlButton)
    ctrl = 1;
  if (ke->state() & Qt::ShiftButton)
    shift = 1;
  QPoint cp = this->mapFromGlobal(QCursor::pos());
  int xp = cp.x();
  int yp = cp.y();
  SetEventInformationFlipY(xp, yp, ctrl, shift, (char) tolower(ke->ascii()), 1, (const char *) ke->text());
  InvokeEvent(vtkCommand::KeyPressEvent, NULL);
  InvokeEvent(vtkCommand::CharEvent, NULL);
 
}




void vtkQtRenderWindowInteractor2::wheelEvent(QWheelEvent *e)
{
  if (!Enabled)
    return;
  
#if QT_VERSION < 0x040000
  this->SetEventInformationFlipY(e->x(), e->y(), 
				 (e->state() & Qt::ControlButton) > 0 ? 1 : 0, 
				 (e->state() & Qt::ShiftButton ) > 0 ? 1 : 0);
#else
  this->SetEventInformationFlipY(e->x(), e->y(), 
				 (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0, 
				 (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);
#endif

 if(e->delta() > 0)
 {
    this->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e);
 }
 else
  {
    this->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e);
  }
}


#if defined (Q_WS_MAC) && QT_VERSION < 0x040000

// gotta do some special stuff on the MAC to make it work right
// this stuff will need changing when using Qt4 with HIViews

#include <AGL/agl.h>

void vtkQtRenderWindowInteractor2::macFixRect()
{
  AGLContext context = static_cast<vtkCarbonRenderWindow*>(this->GetRenderWindow())->GetContextId();
  
  if(!this->isTopLevel())
    {
    GLint bufRect[4];

    // always do AGL_BUFFER_RECT if we have a parent
    if(!aglIsEnabled(context, AGL_BUFFER_RECT))
      aglEnable(context, AGL_BUFFER_RECT);

    // get the clip region
    QRegion clip = this->clippedRegion();
    QRect clip_rect = clip.boundingRect();
    
    // get the position of this widget with respect to the top level widget
    QPoint mp(posInWindow(this));
    int win_height = this->topLevelWidget()->height();
    win_height -= win_height - this->topLevelWidget()->clippedRegion(FALSE).boundingRect().height();

    // give the position and size to agl
    bufRect[0] = mp.x();
    bufRect[1] = win_height -(mp.y() + this->height());
    bufRect[2] = this->width();
    bufRect[3] = this->height();
    aglSetInteger(context, AGL_BUFFER_RECT, bufRect);

    if(clip_rect.isEmpty())
      {
      // no clipping, disable it
      if(!aglIsEnabled(context, AGL_CLIP_REGION))
        aglDisable(context, AGL_CLIP_REGION);
      
      bufRect[0] = 0;
      bufRect[1] = 0;
      bufRect[2] = 0;
      bufRect[3] = 0;
      aglSetInteger(context, AGL_BUFFER_RECT, bufRect);
      }
    else
      {
      // we are clipping, so lets enable it
      if(!aglIsEnabled(context, AGL_CLIP_REGION))
        aglEnable(context, AGL_CLIP_REGION);

      // give agl the clip region
      aglSetInteger(context, AGL_CLIP_REGION, (const GLint*)clip.handle(TRUE));
      }
    }
  
  // update the context
  aglUpdateContext(context);
}

#endif
