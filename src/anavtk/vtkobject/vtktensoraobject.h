#ifndef _vtk_tensoraobject_h_
#define _vtk_tensoraobject_h_

#include "anatomist/vtkobject/vtkaobject.h"

#include <vtkTensorManager.h>

namespace anatomist
{

  class vtkTensorAObject : public vtkAObject
  {

    Q_OBJECT

  public:
    static vtkTensorAObject* New();
    vtkTypeRevisionMacro(vtkTensorAObject, vtkAObject);

    void setSlice (int);
	  
  protected:
    vtkTensorAObject();
    ~vtkTensorAObject();

    void addActors (vtkQAGLWidget*);
    void removeActors (vtkQAGLWidget*);
    
  private:

    std::map<vtkQAGLWidget*, vtkTensorManager*> TensorManagerMap;

    int SliceOrientation; // 0: sagittal, 1: coronal, 2: axial
    
  };
  
  
} // end of namespace


#endif
