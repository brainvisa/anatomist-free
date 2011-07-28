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
#ifndef _vtk_MetaDataSetAObject_h_
#define _vtk_MetaDataSetAObject_h_

#ifndef ANATOMIST_NO_VTKINRIA3D

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

#endif // #ifndef ANATOMIST_NO_VTKINRIA3D

#endif
