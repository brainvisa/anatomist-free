#include "anatomist/window3D/window3D.h"
#include "anatomist/window/vtkglwidget.h"
#include "anatomist/vtkobject/vtkvectoraobject.h"

#include <vtkObjectFactory.h>

namespace anatomist
{

  vtkCxxRevisionMacro (vtkVectorAObject, "$Revision: 1.0 $");
  vtkStandardNewMacro (vtkVectorAObject);

  
  vtkVectorAObject::vtkVectorAObject()
  {
    //this->SliceOrientation = 2;
  }
  
  
  vtkVectorAObject::~vtkVectorAObject()
  {
    std::map<vtkQAGLWidget*, vtkVectorVisuManager*>::iterator it = this->VectorManagerMap.begin();
    while ( it!=this->VectorManagerMap.end() )
    {
      (*it).second->Initialize();
      (*it).second->SetRenderWindowInteractor (0);
      (*it).second->Delete();      
      ++it;
    }
  }
  
  
  void vtkVectorAObject::addActors (vtkQAGLWidget* widget)
  {
    if( !widget )
    {
      std::cerr << "Error: widget is null." << std::endl;
      return;
    }

    /*
    AWindow3D* window = dynamic_cast<AWindow3D*>( widget->window() );

    const aims::Quaternion & quaternion = window->sliceQuaternion();
    
    if( quaternion.axis()[0] > 0.99 ) {
      this->SliceOrientation = 1;
    }
    else if (quaternion.axis()[2] > 0.99 ) {
      this->SliceOrientation = 2;
    }
    else {
      this->SliceOrientation = 0;
    }
    */

    vtkVectorVisuManager* manager = vtkVectorVisuManager::New();    
    manager->SetRenderWindowInteractor ( widget );

    /*
    if( this->SliceOrientation == 2 )
    {
      manager->SetAxialSliceVisibility (1);
      manager->SetSagittalSliceVisibility (0);
      manager->SetCoronalSliceVisibility (0);
    }
    else if (this->SliceOrientation == 1 )
    {
      manager->SetAxialSliceVisibility (0);
      manager->SetSagittalSliceVisibility (0);
      manager->SetCoronalSliceVisibility (1);
    }
    else // (this->SliceOrientation == 0 )
    {
      manager->SetAxialSliceVisibility (0);
      manager->SetSagittalSliceVisibility (1);
      manager->SetCoronalSliceVisibility (0);
    }
    */

    
    manager->SetInput ( this->DataSet );
    manager->BoxWidgetOn();
    
    this->VectorManagerMap.insert( std::pair<vtkQAGLWidget*, vtkVectorVisuManager*>(widget, manager) ); 
    //this->setSlice ( window->getSliceSliderPosition() );
    
  }

  
  
  void vtkVectorAObject::removeActors (vtkQAGLWidget* widget)
  {
    vtkVectorVisuManager* toremove = this->VectorManagerMap[widget];
    if( toremove )
    {
      toremove->Delete();
      this->VectorManagerMap.erase (widget);
    }
  }


  /*
  void vtkVectorAObject::setSlice (int slice)
  {
    std::map<vtkQAGLWidget*, vtkVectorManager*>::iterator it = this->VectorManagerMap.begin();
    while ( it!=this->VectorManagerMap.end() )
    {
      int position [3];
      (*it).second->GetCurrentPosition( position );
      
      position[this->SliceOrientation] = slice;
      
      (*it).second->SetCurrentPosition ( position );
      ++it;
    }
  }
  */
}

  
