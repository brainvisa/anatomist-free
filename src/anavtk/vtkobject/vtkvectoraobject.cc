/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */
#ifndef ANATOMIST_NO_VTKINRIA3D

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

#endif // #ifndef ANATOMIST_NO_VTKINRIA3D

