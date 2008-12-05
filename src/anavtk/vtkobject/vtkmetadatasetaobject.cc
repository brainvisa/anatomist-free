#include "anatomist/vtkobject/vtkmetadatasetaobject.h"

#include <vtkObjectFactory.h>
#include <vtkMetaDataSetSequence.h>
#include <vtkActor.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkCallbackCommand.h>

namespace anatomist
{
  vtkCxxRevisionMacro(vtkMetaDataSetAObject, "$Revision: 1.0 $");
  vtkStandardNewMacro (vtkMetaDataSetAObject);


  int                                  vtkMetaDataSetAObject::CurrentIndex = 0;
  std::vector<vtkMetaDataSetSequence*> vtkMetaDataSetAObject::MetaDataSetSequenceList;
  vtkCallbackCommand*                  vtkMetaDataSetAObject::CallbackCommand = 0;

  
  vtkMetaDataSetAObject::vtkMetaDataSetAObject()
  {
    this->TimerID = -1;
    this->MetaDataSet = 0;
    this->Actor = vtkActor::New();
  }

  
  vtkMetaDataSetAObject::~vtkMetaDataSetAObject()
  {
    this->Actor->Delete();
    if( this->MetaDataSet )
    {
      this->MetaDataSet->Delete();
    }
  }



  bool vtkMetaDataSetAObject::boundingBox ( Point3df & bmin, Point3df & bmax) const
  {
    if( !this->MetaDataSet )
    {
      return false;
    }
    
    double* bounds = this->MetaDataSet->GetDataSet()->GetBounds();
    
    bmin[0] = bounds[0];
    bmin[1] = bounds[2];
    bmin[2] = bounds[4];
    
    bmax[0] = bounds[1];
    bmax[1] = bounds[3];
    bmax[2] = bounds[5];
    
    return true;  
  }



  void vtkMetaDataSetAObject::RemoveMetaDataSetSequence (vtkMetaDataSetSequence* sequence)
  {
    std::vector<vtkMetaDataSetSequence*> list = vtkMetaDataSetAObject::GetMetaDataSetSequenceList();
    std::vector<vtkMetaDataSetSequence*>::iterator it = list.begin();

    while( it!=list.end() )
    {
      if( (*it)==sequence )
      {
	list.erase (it);
	break;
      }
      ++it;
    }
  }
  

  
  void vtkMetaDataSetAObject::addActors (vtkQAGLWidget* widget)
  {
    vtkMetaDataSetSequence* dataset = vtkMetaDataSetSequence::SafeDownCast ( this->MetaDataSet );

    if( !dataset )
    {
      return;
    }


    vtkMetaDataSetAObject::AddMetaDataSetSequence ( dataset );
  
    

    // rendering pipeline
    vtkDataSetSurfaceFilter* geometryextractor = vtkDataSetSurfaceFilter::New();
    vtkPolyDataNormals*      normalextractor = vtkPolyDataNormals::New();
    vtkPolyDataMapper*       mapper = vtkPolyDataMapper::New();
    
    normalextractor->SetFeatureAngle (90);
    
    ///\todo try to skip the normal extraction filter in order to enhance the visualization speed when the data is time sequence.
    geometryextractor->SetInput ( dataset->GetDataSet() );
    normalextractor->SetInput ( geometryextractor->GetOutput() );
    mapper->SetInput ( normalextractor->GetOutput() );

    
    this->Actor->SetMapper ( mapper );
    widget->AddActor ( this->Actor );


    /*
      vtkCallbackCommand* callback = vtkCallbackCommand::New();
      callback->SetClientData( this );
      callback->SetCallback( vtkMetaDataSetAObject::HandleTimer );
    */

    //widget->AddObserver(vtkCommand::TimerEvent, callback, 0.0);
    if( !widget->HasObserver (vtkCommand::TimerEvent, vtkMetaDataSetAObject::GetCallbackCommand()) )
    {
      widget->AddObserver(vtkCommand::TimerEvent, vtkMetaDataSetAObject::GetCallbackCommand(), 0.0);
      widget->SetTimerDuration (200);
      this->TimerID = widget->CreateTimer ( 0 );
    }
    
    geometryextractor->Delete();
    normalextractor->Delete();
    mapper->Delete();
    //callback->Delete();
    
  }



  void vtkMetaDataSetAObject::removeActors (vtkQAGLWidget* widget)
  {
    vtkMetaDataSetSequence* dataset = vtkMetaDataSetSequence::SafeDownCast ( this->MetaDataSet );
    vtkMetaDataSetAObject::RemoveMetaDataSetSequence ( dataset );
    widget->RemoveActor ( this->Actor );
  }
  


  void vtkMetaDataSetAObject::HandleTimer (vtkObject* caller, unsigned long id, void* clientdata, void* calldata)
  {
    vtkRenderWindowInteractor* iren   = vtkRenderWindowInteractor::SafeDownCast( caller );
    
    if( !iren )
    {
      std::cerr << "Error while casting" << std::endl;
      return;
    }
    
    std::vector<vtkMetaDataSetSequence*> seq = vtkMetaDataSetAObject::GetMetaDataSetSequenceList();
    int currentIndex = vtkMetaDataSetAObject::GetCurrentIndex();

    for (unsigned int i=0; i<seq.size(); i++)
    {
      seq[i]->UpdateToIndex ( currentIndex );
      
      if( currentIndex >= seq[i]->GetNumberOfMetaDataSets() ) {
	vtkMetaDataSetAObject::SetCurrentIndex ( 0 );
      } else {
	vtkMetaDataSetAObject::SetCurrentIndex ( currentIndex+1 );
      }
    }
    iren->Render(); 
  }
  
}
