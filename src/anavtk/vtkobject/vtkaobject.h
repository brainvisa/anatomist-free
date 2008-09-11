#ifndef _vtk_aobject_h_
#define _vtk_aobject_h_

#include <anatomist/object/Object.h>
#include <cartobase/object/object.h>

#include <vtkObject.h>
#include <vtkDataSet.h>

namespace anatomist
{
  
  class vtkAObject : public AObject, public vtkObject
  {
    
  public :
    static vtkAObject *New();
    vtkTypeRevisionMacro(vtkAObject, vtkObject);
    
    vtkSetObjectMacro(DataSet, vtkDataSet);
    vtkGetObjectMacro(DataSet, vtkDataSet);
/*
    void SetDataSet (vtkDataSet* data)
    { _vtkDataSet = data; }

    vtkDataSet* GetDataSet (void) const
    { return _vtkDataSet; }
*/
    virtual bool Is2DObject() { return( false ); }
    virtual bool Is3DObject() { return( true ); }

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;
    
  protected:
    vtkAObject()
    {
      _type = _classType;
      this->DataSet = 0;
    }
    ~vtkAObject();
    
    
    vtkDataSet* DataSet;
    
  private:    
    
    static int registerClass();    
    static int	_classType;
  };
  
}

#endif
