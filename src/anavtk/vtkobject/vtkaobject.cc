#include <qslider.h>

#include "anatomist/window3D/window3D.h"

#include "anatomist/vtkobject/vtkaobject.h"
#include <vtkObjectFactory.h>


using namespace anatomist;
using namespace carto;
using namespace std;

vtkCxxRevisionMacro(vtkAObject, "$Revision: 1.0 $");


int vtkAObject::_classType = vtkAObject::registerClass();


vtkAObject::vtkAObject()
{
  _type = _classType;
  this->DataSet = 0;
  this->setReferential( theAnatomist->centralReferential() );
}

vtkAObject::~vtkAObject()
{
  if ( this->DataSet )
  {
    this->DataSet->Delete();
  }
}


int vtkAObject::registerClass()
{
  int type = registerObjectType( "VTK" );
  return type;
}



bool vtkAObject::boundingBox ( Point3df & bmin, Point3df & bmax) const
{

  if( !this->DataSet )
  {
    return false;
  }


  double* bounds = this->DataSet->GetBounds();

  bmin[0] = bounds[0];
  bmin[1] = bounds[2];
  bmin[2] = bounds[4];

  bmax[0] = bounds[1];
  bmax[1] = bounds[3];
  bmax[2] = bounds[5];

  return true;
  
}



void vtkAObject::registerWindow(AWindow* window)
{
  AObject::registerWindow ( window );

  AWindow3D*    win3D = dynamic_cast<AWindow3D*>( window );

  if( !win3D )
  {
    return;
  }

  connect( win3D->getSliceSlider(), SIGNAL( valueChanged( int ) ), this, 
	   SLOT( changeSlice( int ) ) );
  
  vtkQAGLWidget* vtkw = dynamic_cast<vtkQAGLWidget*>( win3D->view() );
  if( !vtkw )
  {
    return;
  }

  vtkw->registerVtkAObject ( this );
  this->addActors( vtkw );
}



void vtkAObject::unregisterWindow(AWindow* window)
{
  AObject::unregisterWindow ( window );

  AWindow3D*    win3D = dynamic_cast<AWindow3D*>( window );

  if( !win3D )
  {
    return;
  }
  
  vtkQAGLWidget* vtkw = dynamic_cast<vtkQAGLWidget*>( win3D->view() );

  if( !vtkw )
  {
    return;
  }

  vtkw->unregisterVtkAObject ( this );
  this->removeActors (vtkw);
}


void vtkAObject::changeSlice (int slice)
{
  this->setSlice (slice);
}


void vtkAObject::setSlice (int)
{}
