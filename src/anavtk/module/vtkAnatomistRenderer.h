#ifndef _vtk_AnatomistRenderer_h_
#define _vtk_AnatomistRenderer_h_

#include <vtkOpenGLRenderer.h>

class vtkQAGLWidget;

class vtkAnatomistRenderer : public vtkOpenGLRenderer
{

 public:
  static vtkAnatomistRenderer *New();
  vtkTypeRevisionMacro (vtkAnatomistRenderer, vtkOpenGLRenderer);
  

  virtual void DeviceRender();


  vtkSetObjectMacro (QAGLWidget, vtkQAGLWidget);
  vtkGetObjectMacro (QAGLWidget, vtkQAGLWidget);
  
    
 protected:
  vtkAnatomistRenderer();
  ~vtkAnatomistRenderer(){};


  //virtual int UpdateGeometry (void);
  

 private:
  vtkAnatomistRenderer (const vtkAnatomistRenderer&);
  void operator=(const vtkAnatomistRenderer&);

  vtkQAGLWidget* QAGLWidget;
  
};


#endif
