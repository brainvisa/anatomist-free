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
#include "anatomist/module/vtkAnatomistCamera.h"

#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOutputWindow.h"
#include "vtkOpenGLRenderWindow.h"
// vtkgluPickMatrix.h is not provided with an install of VTK. Is it on purpose? Or a bug?
//#include "vtkgluPickMatrix.h"

#include "vtkOpenGL.h"

#include "vtkMath.h"
#include <math.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
#if VTK_MAJOR_VERSION < 6
vtkCxxRevisionMacro(vtkAnatomistCamera, "$Revision: 1.0 $");
#endif
vtkStandardNewMacro(vtkAnatomistCamera);
#endif


#if VTK_MAJOR_VERSION>5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >=4)
#define THE_MATRIX ProjectionTransform
#else
#define THE_MATRIX PerspectiveTransform
#endif


// This function was coped from vtkgluPickMatrix, itself
// copied from Mesa and sets up the pick matrix
inline void vtkgluAnatomistPickMatrix( GLdouble x, GLdouble y,
				       GLdouble width, GLdouble height,
				       int *origin, int *size )
{
  GLfloat m[16];
  GLfloat sx, sy;
  GLfloat tx, ty;
  
  sx = size[0] / width;
  sy = size[1] / height;
  tx = (size[0] + 2.0 * (origin[0] - x)) / width;
  ty = (size[1] + 2.0 * (origin[1] - y)) / height;

#define M(row,col)  m[col*4+row]
   M(0,0) = sx;   M(0,1) = 0.0;  M(0,2) = 0.0;  M(0,3) = tx;
   M(1,0) = 0.0;  M(1,1) = sy;   M(1,2) = 0.0;  M(1,3) = ty;
   M(2,0) = 0.0;  M(2,1) = 0.0;  M(2,2) = 1.0;  M(2,3) = 0.0;
   M(3,0) = 0.0;  M(3,1) = 0.0;  M(3,2) = 0.0;  M(3,3) = 1.0;
#undef M

   glMultMatrixf( m );
}




vtkAnatomistCamera::vtkAnatomistCamera()
{
  this->AnaPerspectiveTransform = vtkMatrix4x4::New();
  this->AnaViewTransform = vtkMatrix4x4::New();

  this->AnaPerspectiveTransform->Identity();
  this->AnaViewTransform->Identity();
  this->PerspectiveBounds[0]=0.0;
  this->PerspectiveBounds[1]=1.0;
  this->PerspectiveBounds[2]=0.0;
  this->PerspectiveBounds[3]=1.0;
  
}


vtkAnatomistCamera::~vtkAnatomistCamera()
{
  this->AnaPerspectiveTransform->Delete();
  this->AnaViewTransform->Delete();
}


void vtkAnatomistCamera::SetAnaPerspectiveTransform (const double data[16])
{
  this->AnaPerspectiveTransform->DeepCopy (data);
}


void vtkAnatomistCamera::SetAnaPerspectiveTransform (vtkMatrix4x4* mat)
{
  this->AnaPerspectiveTransform->DeepCopy (mat);
}


void vtkAnatomistCamera::SetAnaViewTransform (vtkMatrix4x4* mat)
{
  this->AnaViewTransform->DeepCopy (mat);
}



void vtkAnatomistCamera::Render(vtkRenderer *ren)
{
  //double aspect[2];
  int  lowerLeft[2];
  int usize, vsize;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

  //vtkOpenGLRenderWindow *win=vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
  
  // find out if we should stereo render
  this->Stereo = (ren->GetRenderWindow())->GetStereoRender();
  ren->GetTiledSizeAndOrigin(&usize,&vsize,lowerLeft,lowerLeft+1);

  /* WARNING:
     usize, vsize are always the same, fixed in my case to 482x512, whatever
     the window/screen size. Setting the viewport according to these values
     is wrong, so I commented out viewport commands below.
     But other commands may be affected to. I don't know what difference it
     makes, I don't see any visible change.
  */

  // Copied from VTK-CVS (5.3) -> not compatible with vtk-5-0-3 as shipped with Mandriva.
  // Maybe later?
  
  // if were on a stereo renderer draw to special parts of screen
  /*
  if (this->Stereo)
    {
    switch ((ren->GetRenderWindow())->GetStereoType())
      {
      case VTK_STEREO_CRYSTAL_EYES:
        if (this->LeftEye)
          {
          if(ren->GetRenderWindow()->GetDoubleBuffer())
            {
            glDrawBuffer(static_cast<GLenum>(win->GetBackLeftBuffer()));
            glReadBuffer(static_cast<GLenum>(win->GetBackLeftBuffer()));
            }
          else
            {
            glDrawBuffer(static_cast<GLenum>(win->GetFrontLeftBuffer()));
            glReadBuffer(static_cast<GLenum>(win->GetFrontLeftBuffer()));
            }
          }
        else
          {
           if(ren->GetRenderWindow()->GetDoubleBuffer())
            {
            glDrawBuffer(static_cast<GLenum>(win->GetBackRightBuffer()));
            glReadBuffer(static_cast<GLenum>(win->GetBackRightBuffer()));
            }
          else
            {
            glDrawBuffer(static_cast<GLenum>(win->GetFrontRightBuffer()));
            glReadBuffer(static_cast<GLenum>(win->GetFrontRightBuffer()));
            }
          }
        break;
      case VTK_STEREO_LEFT:
        this->LeftEye = 1;
        break;
      case VTK_STEREO_RIGHT:
        this->LeftEye = 0;
        break;
      default:
        break;
      }
    }
  else
    {
    if (ren->GetRenderWindow()->GetDoubleBuffer())
      {
      glDrawBuffer(static_cast<GLenum>(win->GetBackBuffer()));
      
      // Reading back buffer means back left. see OpenGL spec.
      // because one can write to two buffers at a time but can only read from
      // one buffer at a time.
      glReadBuffer(static_cast<GLenum>(win->GetBackBuffer()));
      }
    else
      {
      glDrawBuffer(static_cast<GLenum>(win->GetFrontBuffer()));
      
      // Reading front buffer means front left. see OpenGL spec.
      // because one can write to two buffers at a time but can only read from
      // one buffer at a time.
      glReadBuffer(static_cast<GLenum>(win->GetFrontBuffer()));
      }
    }
  */
  


  // Copied from VTK-5.0.3, does not seem to make much difference in our case with
  // the previous code.
  // if were on a stereo renderer draw to special parts of screen

  
  if (this->Stereo)
    {
    switch ((ren->GetRenderWindow())->GetStereoType())
      {
      case VTK_STEREO_CRYSTAL_EYES:
        if (this->LeftEye)
          {
          glDrawBuffer(GL_BACK_LEFT);
          }
        else
          {
          glDrawBuffer(GL_BACK_RIGHT);
          }
        break;
      case VTK_STEREO_LEFT:
        this->LeftEye = 1;
        break;
      case VTK_STEREO_RIGHT:
        this->LeftEye = 0;
        break;
      default:
        break;
      }
    }
  else
    {
    if (ren->GetRenderWindow()->GetDoubleBuffer())
      {
      glDrawBuffer(GL_BACK);
      }
    else
      {
      glDrawBuffer(GL_FRONT);
      }
    }
  
  
  
  // some renderer subclasses may have more complicated computations for the
  // aspect ratio. SO take that into account by computing the difference
  // between our simple aspect ratio and what the actual renderer is
  // reporting.


  /*
    ren->ComputeAspect();
    ren->GetAspect(aspect);
    double aspect2[2];
    ren->vtkViewport::ComputeAspect();
    ren->vtkViewport::GetAspect(aspect2);
    double aspectModification = aspect[0]*aspect2[1]/(aspect[1]*aspect2[0]);
  */

  glMatrixMode( GL_PROJECTION);
  glLoadIdentity();
  
  if(usize && vsize)
  {
#if VTK_MAJOR_VERSION>5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >=4)
    matrix->DeepCopy(this->GetProjectionTransformMatrix(
							/*aspectModification*usize/vsize*/0.0, -1,1));
#else
    matrix->DeepCopy(this->GetPerspectiveTransformMatrix(
							 /*aspectModification*usize/vsize*/0.0, -1,1));
#endif
    matrix->Transpose();
  }
  
  if(ren->GetIsPicking())
    {
      int size[2]; size[0] = usize; size[1] = vsize;
      glLoadIdentity();
      vtkgluAnatomistPickMatrix(ren->GetPickX(), ren->GetPickY(),
				1, 1, 
				//ren->GetPickWidth(), ren->GetPickHeight(),
				lowerLeft, size);
      glMultMatrixd(matrix->Element[0]);
    }
  
  else
    {
    // insert camera view transformation 
      glMultMatrixd(matrix->Element[0]);
    }


  // push the model view matrix onto the stack, make sure we 
  // adjust the mode first


  /* WARNING:
   * glViewport and glScissor disabled because the dimensions are wrong.
     See above.
  */
//   glViewport(lowerLeft[0],lowerLeft[1], usize, vsize);
  glEnable( GL_SCISSOR_TEST );
//   glScissor(lowerLeft[0],lowerLeft[1], usize, vsize);
  
  
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  
  matrix->DeepCopy(this->GetViewTransformMatrix());
  matrix->Transpose();
  

  // insert camera view transformation 
  glMultMatrixd(matrix->Element[0]);
  
  /*
  if ((ren->GetRenderWindow())->GetErase() && ren->GetErase() 
      && !ren->GetIsPicking())
  {
    ren->Clear();
  }
  */
  
  if (this->Stereo)
    {
    if (this->LeftEye)
      {
      this->LeftEye = 0;
      }
    else
      {
      this->LeftEye = 1;
      }
    }
  
  matrix->Delete();
}



vtkMatrix4x4 *vtkAnatomistCamera::GetProjectionTransformMatrix(double aspect,
								double nearz,
								double farz)
{

  this->THE_MATRIX->Identity();

  // apply user defined transform last if there is one
  if (this->UserTransform)
    {
      this->THE_MATRIX->Concatenate(this->UserTransform->GetMatrix());
    }

  // adjust Z-buffer range
  this->THE_MATRIX->AdjustZBuffer(-1, +1, nearz, farz);

  
  if (this->ParallelProjection)
    {
    // set up a rectangular parallelipiped
      /*
	double width = this->ParallelScale*aspect;
	double height = this->ParallelScale;
	
	double xmin = (this->WindowCenter[0]-1.0)*width;
	double xmax = (this->WindowCenter[0]+1.0)*width;
	double ymin = (this->WindowCenter[1]-1.0)*height;
	double ymax = (this->WindowCenter[1]+1.0)*height;
      */
    this->THE_MATRIX->Ortho(this->PerspectiveBounds[0], this->PerspectiveBounds[1],
				     this->PerspectiveBounds[2], this->PerspectiveBounds[3],
				     this->PerspectiveBounds[4], this->PerspectiveBounds[5]);
				     //this->ClippingRange[0],
				     //this->ClippingRange[1]);

    }
  
  else
    {
    // set up a perspective frustum
#if VTK_MAJOR_VERSION>5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >=4)
    double tmp = tan(this->ViewAngle*vtkMath::RadiansFromDegrees(1.f)/2);
#else
    double tmp = tan(this->ViewAngle*vtkMath::DoubleDegreesToRadians()/2);
#endif
    double width;
    double height;
    if (this->UseHorizontalViewAngle)
      {
      width = this->ClippingRange[0]*tmp;
      height = this->ClippingRange[0]*tmp/aspect;
      }
    else
      {
      width = this->ClippingRange[0]*tmp*aspect;
      height = this->ClippingRange[0]*tmp;
      }

    double xmin = (this->WindowCenter[0]-1.0)*width;
    double xmax = (this->WindowCenter[0]+1.0)*width;
    double ymin = (this->WindowCenter[1]-1.0)*height;
    double ymax = (this->WindowCenter[1]+1.0)*height;

    this->THE_MATRIX->Frustum(xmin, xmax, ymin, ymax,
                                       this->ClippingRange[0],
                                       this->ClippingRange[1]);
    }

  if (this->Stereo)
    {
    // set up a shear for stereo views
    if (this->LeftEye)
      {
      this->THE_MATRIX->Stereo(-this->EyeAngle/2,
                                         this->Distance);
      }
    else
      {
      this->THE_MATRIX->Stereo(+this->EyeAngle/2,
                                         this->Distance);
      }
    }

  if (this->ViewShear[0] != 0.0 || this->ViewShear[1] != 0.0)
    {
    this->THE_MATRIX->Shear(this->ViewShear[0],
                                      this->ViewShear[1],
                                      this->ViewShear[2]*this->Distance);
    }

  return this->THE_MATRIX->GetMatrix();
  
}



void vtkAnatomistCamera::SetPerspectiveBounds (const double& xmin, const double& xmax, const double& ymin, const double& ymax, const double& near, const double& far)
{
  this->PerspectiveBounds[0] = xmin;
  this->PerspectiveBounds[1] = xmax;
  this->PerspectiveBounds[2] = ymin;
  this->PerspectiveBounds[3] = ymax;
  this->PerspectiveBounds[4] = near;
  this->PerspectiveBounds[5] = far;
}
