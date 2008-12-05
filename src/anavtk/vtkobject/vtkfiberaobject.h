#ifndef _vtk_fiberaobject_h_
#define _vtk_fiberaobject_h_

#include "anatomist/vtkobject/vtkaobject.h"

#include <vtkFibersManager.h>

namespace anatomist
{

  class vtkFiberAObject : public vtkAObject
  {

  public:
    static vtkFiberAObject* New();
    vtkTypeRevisionMacro(vtkFiberAObject, vtkAObject);
    
    
  protected:
    vtkFiberAObject();
    ~vtkFiberAObject();

    void addActors (vtkQAGLWidget*);
    void removeActors (vtkQAGLWidget*);

  private:

    std::map<vtkQAGLWidget*, vtkFibersManager*> FiberManagerMap;
    
  };
  
  
} // end of namespace


#endif
