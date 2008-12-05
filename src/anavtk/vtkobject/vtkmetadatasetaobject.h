#ifndef _vtk_MetaDataSetAObject_h_
#define _vtk_MetaDataSetAObject_h_

#include "anatomist/vtkobject/vtkaobject.h"
#include <vtkMetaDataSet.h>
#include <vtkCallbackCommand.h>

class vtkDataManager;
class vtkActor;
class vtkMetaDataSetSequence;

namespace anatomist
{

  class vtkMetaDataSetAObject : public vtkAObject
  {

  public:
    static vtkMetaDataSetAObject* New();
    vtkTypeRevisionMacro(vtkMetaDataSetAObject, vtkAObject);

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;
    
    vtkGetMacro (TimerID, int);
    vtkSetObjectMacro (MetaDataSet, vtkMetaDataSet);
    vtkGetObjectMacro (MetaDataSet, vtkMetaDataSet);


    static void HandleTimer (vtkObject*, unsigned long, void*, void*);

    static void HandleKeyboard (vtkObject*, unsigned long, void*, void*);
    

    static void SetCurrentIndex (int index)
    { CurrentIndex = index; }

    
    static int& GetCurrentIndex (void)
    { return CurrentIndex; }

    
    static std::vector<vtkMetaDataSetSequence*>& GetMetaDataSetSequenceList (void)
    { return MetaDataSetSequenceList; }


    static void AddMetaDataSetSequence (vtkMetaDataSetSequence* seq)
    { MetaDataSetSequenceList.push_back ( seq ); }

    
    static void RemoveMetaDataSetSequence (vtkMetaDataSetSequence*);
    
    
    static vtkCallbackCommand* GetTimerCallbackCommand (void)
    {
      if( !TimerCallbackCommand )
      {
	TimerCallbackCommand = vtkCallbackCommand::New();
	TimerCallbackCommand->SetCallback( vtkMetaDataSetAObject::HandleTimer );
      }
      return TimerCallbackCommand;
    }

    static vtkCallbackCommand* GetKeyboardCallbackCommand (void)
    {
      if( !KeyboardCallbackCommand )
      {
	KeyboardCallbackCommand = vtkCallbackCommand::New();
	KeyboardCallbackCommand->SetCallback( vtkMetaDataSetAObject::HandleKeyboard );
      }
      return KeyboardCallbackCommand;
    }

    
  protected:
    vtkMetaDataSetAObject();
    ~vtkMetaDataSetAObject();

    
    void addActors (vtkQAGLWidget*);
    void removeActors (vtkQAGLWidget*);

    
  private:

    int TimerID;
    static int CurrentIndex;

    vtkActor*       Actor;

    vtkMetaDataSet* MetaDataSet;

    
    static std::vector<vtkMetaDataSetSequence*> MetaDataSetSequenceList;

    static vtkCallbackCommand* TimerCallbackCommand;
    static vtkCallbackCommand* KeyboardCallbackCommand;
    
  };
  
}


#endif
