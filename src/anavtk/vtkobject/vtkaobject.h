#ifndef _vtk_aobject_h_
#define _vtk_aobject_h_

#include <qobject.h>

#include <anatomist/object/Object.h>
#include <cartobase/object/object.h>
#include "anatomist/window/vtkglwidget.h"

#include <vtkObject.h>
#include <vtkDataSet.h>

namespace anatomist
{
  
  class vtkAObject : public QObject, public AObject, public vtkObject
  {

    Q_OBJECT
      
  public :
    vtkTypeRevisionMacro(vtkAObject, vtkObject);
    
    vtkSetObjectMacro(DataSet, vtkDataSet);
    vtkGetObjectMacro(DataSet, vtkDataSet);

    virtual bool Is2DObject() { return( false ); }
    virtual bool Is3DObject() { return( true ); }

    virtual bool boundingBox( Point3df & bmin, Point3df & bmax ) const;

    virtual void registerWindow  (AWindow* window);    
    virtual void unregisterWindow(AWindow* window);

    virtual void setSlice (int);

    virtual void addActors (vtkQAGLWidget*) = 0;
    virtual void removeActors (vtkQAGLWidget*) = 0;
    
    
  public slots:
    void changeSlice (int);

    
  protected:
    vtkAObject();
    ~vtkAObject();
    
    vtkDataSet* DataSet;

  private:    
    
    static int registerClass();    
    static int	_classType;
  };
  
}

#endif
