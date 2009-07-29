#ifndef _vtk_VectorAObject_h_
#define _vtk_VectorAObject_h_

#include "anatomist/vtkobject/vtkaobject.h"

#include <vtkVectorVisuManager.h>


namespace anatomist
{

  class vtkVectorAObject : public vtkAObject
  {

    Q_OBJECT

  public:
    static vtkVectorAObject* New();
    vtkTypeRevisionMacro(vtkVectorAObject, vtkAObject);

    //void setSlice (int);
	  
  protected:
    vtkVectorAObject();
    ~vtkVectorAObject();

    void addActors (vtkQAGLWidget*);
    void removeActors (vtkQAGLWidget*);
    
  private:

    std::map<vtkQAGLWidget*, vtkVectorVisuManager*> VectorManagerMap;

    //int SliceOrientation; // 0: sagittal, 1: coronal, 2: axial
    
  };
  
  
} // end of namespace


#endif
