#include "anatomist/window3D/window3D.h"
#include "anatomist/window/vtkglwidget.h"
#include "anatomist/vtkobject/vtktensoraobject.h"
#include <aims/resampling/quaternion.h>

#include <vtkObjectFactory.h>

namespace anatomist
{

  vtkCxxRevisionMacro(vtkTensorAObject, "$Revision: 1.0 $");
  vtkStandardNewMacro (vtkTensorAObject);

  
  vtkTensorAObject::vtkTensorAObject()
  {
    this->SliceOrientation = 2;
  }
  
  
  vtkTensorAObject::~vtkTensorAObject()
  {
    std::map<vtkQAGLWidget*, vtkTensorManager*>::iterator it = this->TensorManagerMap.begin();
    while ( it!=this->TensorManagerMap.end() )
    {
      (*it).second->Initialize();
      (*it).second->SetRenderWindowInteractor (0);
      (*it).second->Delete();      
      ++it;
    }
  }
  
  
  void vtkTensorAObject::addActors (vtkQAGLWidget* widget)
  {
    if( !widget )
    {
      std::cerr << "Error: widget is null." << std::endl;
      return;
    }

    anatomist::View* view = (anatomist::View*)widget; // needed to disambiguate between:
    // AWindow* View::window()
    // and
    // QWidget* QWidget::window() inherited from vtkQAGLWidget

    
    AWindow3D* window = dynamic_cast<AWindow3D*>( view->window() );
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
    
    vtkRenderer* renderer = widget->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	
    vtkTensorManager* manager = vtkTensorManager::New();    
    if (renderer )
      manager->SetRenderWindowInteractor ( widget, renderer );
    else
      manager->SetRenderWindowInteractor ( widget );
    manager->SetGlyphResolution (8);
    
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
    

    vtkStructuredPoints* tensors = vtkStructuredPoints::SafeDownCast (this->DataSet);
    if( !tensors )
    {
      std::cerr << "Error: cannot cast dataset to vtkStructuredPoints." << std::endl;
      return;
    }
    
    manager->SetInput ( tensors );
    this->TensorManagerMap.insert( std::pair<vtkQAGLWidget*, vtkTensorManager*>(widget, manager) ); 
    this->setSlice ( window->getSliceSliderPosition() );
    
    manager->Update();
  }

  
  
  void vtkTensorAObject::removeActors (vtkQAGLWidget* widget)
  {
    vtkTensorManager* toremove = this->TensorManagerMap[widget];
    if( toremove )
    {
      toremove->Delete();
      this->TensorManagerMap.erase (widget);
    }
  }


  
  void vtkTensorAObject::setSlice (int slice)
  {
    std::map<vtkQAGLWidget*, vtkTensorManager*>::iterator it = this->TensorManagerMap.begin();
    while ( it!=this->TensorManagerMap.end() )
    {
      int position [3];
      (*it).second->GetCurrentPosition( position );
      
      position[this->SliceOrientation] = slice;
      
      (*it).second->SetCurrentPosition ( position );
      ++it;
    }
  }
  
}
