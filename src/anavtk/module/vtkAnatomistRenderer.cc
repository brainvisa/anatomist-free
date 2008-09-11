#include "anatomist/window/vtkglwidget.h"

#include "anatomist/module/vtkAnatomistRenderer.h"

#include "anatomist/module/vtkAnatomistCamera.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"

#include "vtkgl.h"

#ifndef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkAnatomistRenderer, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkAnatomistRenderer);
#endif

vtkAnatomistRenderer::vtkAnatomistRenderer()
{
  vtkCamera* cam = vtkAnatomistCamera::New();
  cam->ParallelProjectionOn();
  this->SetActiveCamera (cam);
  cam->Delete();
}


void vtkAnatomistRenderer::DeviceRender()
{

  this->RenderWindow->MakeCurrent();

  
  // standard render method 
  //this->ClearLights(); // -> Lights are controlled by anatomist, not vtk
  
  this->UpdateCamera();
  //this->GetActiveCamera()->Render (this);
  //this->UpdateLightGeometry(); // -> Lights are controlled by anatomist, not vtk
  //this->UpdateLights();  // -> Lights are controlled by anatomist, not vtk
  
  
  // set matrix mode for actors 
  glMatrixMode(GL_MODELVIEW);
  
  
  this->UpdateGeometry();

  // clean up the model view matrix set up by the camera

  
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}


/*
int vtkAnatomistRenderer::UpdateGeometry()
{
  
  if( this->QAGLWidget )
  {

    //this->QAGLWidget->qglWidget()->makeCurrent();
    
    if( this->QAGLWidget->hasTransparentObjects() && this->QAGLWidget->depthPeelingEnabled() )
    {
      this->QAGLWidget->depthPeelingRender( this->QAGLWidget->GetDrawMode() );
    }
    else
    {
      this->QAGLWidget->drawObjects( this->QAGLWidget->GetDrawMode() );
    }
  }
  
  //this->RenderWindow->MakeCurrent();
  int nactors = vtkOpenGLRenderer::UpdateGeometry();


  return nactors;
  
}
*/
