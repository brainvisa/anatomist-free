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
