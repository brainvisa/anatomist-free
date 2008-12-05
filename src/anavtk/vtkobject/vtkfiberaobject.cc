#include "anatomist/vtkobject/vtkfiberaobject.h"

#include <vtkObjectFactory.h>

namespace anatomist
{

  vtkCxxRevisionMacro(vtkFiberAObject, "$Revision: 1.0 $");
  vtkStandardNewMacro (vtkFiberAObject);

  
  vtkFiberAObject::vtkFiberAObject()
  {
  }
  
  
  vtkFiberAObject::~vtkFiberAObject()
  {

    std::map<vtkQAGLWidget*, vtkFibersManager*>::iterator it = this->FiberManagerMap.begin();
    while ( it!=this->FiberManagerMap.end() )
    {
      (*it).second->Initialize();
      (*it).second->SetRenderWindowInteractor (0);
      (*it).second->Delete();
      
      ++it;
    }

  }
  
  
  void vtkFiberAObject::addActors (vtkQAGLWidget* widget)
  {
    if( !widget )
    {
      std::cerr << "Error: widget is null." << std::endl;
      return;
    }
    
    vtkFibersManager* manager = vtkFibersManager::New();    
    manager->SetRenderWindowInteractor ( widget );

    vtkPolyData* polydata = vtkPolyData::SafeDownCast (this->DataSet);

    if( !polydata )
    {
      std::cerr << "Error: cannot cast dataset to polydata." << std::endl;
      return;
    }

    manager->SetInput ( polydata );
    manager->BoxWidgetOn();

    
    this->FiberManagerMap.insert( std::pair<vtkQAGLWidget*, vtkFibersManager*>(widget, manager) );
  }
  
  
  void vtkFiberAObject::removeActors (vtkQAGLWidget* widget)
  {

    vtkFibersManager* toremove = this->FiberManagerMap[widget];

    if( toremove )
    {
      toremove->Initialize();
      toremove->SetRenderWindowInteractor (0);
      toremove->Delete();

      this->FiberManagerMap.erase (widget);
    }

  }

}
