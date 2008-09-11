
#include "anatomist/vtkobject/vtkaobject.h"

#include <vtkObjectFactory.h>

using namespace anatomist;
using namespace carto;
using namespace std;

vtkCxxRevisionMacro(vtkAObject, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkAObject);


int vtkAObject::_classType = vtkAObject::registerClass();


vtkAObject::~vtkAObject()
{
  if (this->DataSet )
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
