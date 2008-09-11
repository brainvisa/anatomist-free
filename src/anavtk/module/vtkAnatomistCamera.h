#ifndef _vtk_AnatomistCamera_h_
#define _vtk_AnatomistCamera_h_

#include "vtkOpenGLCamera.h"
#include "vtkPerspectiveTransform.h"
#include "vtkTransform.h"

class AnaPerspectiveTransform;
class AnaViewTransform;


class vtkAnatomistCamera : public vtkOpenGLCamera
{
public:
  static vtkAnatomistCamera *New();
  vtkTypeRevisionMacro(vtkAnatomistCamera,vtkOpenGLCamera);

  // Description:
  // Implement base class method.
  void Render(vtkRenderer *ren);

  void SetAnaPerspectiveTransform (const double[16]);
  void SetAnaPerspectiveTransform (vtkMatrix4x4*);
  vtkGetObjectMacro (AnaPerspectiveTransform, vtkMatrix4x4);
  
  void SetAnaViewTransform (const double[16]);
  void SetAnaViewTransform (vtkMatrix4x4*);
  vtkGetObjectMacro (AnaViewTransform, vtkMatrix4x4);

  
  virtual vtkMatrix4x4 *GetPerspectiveTransformMatrix(double,
                                                      double,
                                                      double);
  
  void SetPerspectiveBounds (const double&, const double&, const double&, const double&, const double&, const double&);
  
protected:  
  vtkAnatomistCamera();
  ~vtkAnatomistCamera();

  vtkMatrix4x4* AnaPerspectiveTransform;
  vtkMatrix4x4* AnaViewTransform;
  double PerspectiveBounds[6];
   
  
private:
  vtkAnatomistCamera(const vtkAnatomistCamera&);  // Not implemented.
  void operator=(const vtkAnatomistCamera&);  // Not implemented.
};


#endif
