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
