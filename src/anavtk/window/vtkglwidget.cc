/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#include <anatomist/window3D/window3D.h>
#include <anatomist/color/Light.h>


#include <anatomist/window/Window.h>
#include <anatomist/window/vtkglwidget.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/object/Object.h>
#include <anatomist/vtkobject/vtkaobject.h>
#include <anatomist/vtkobject/vtkreader.h>


#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkPropCollection.h"
#include "vtkMapper.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "anatomist/module/vtkAnatomistCamera.h"
#include "anatomist/module/vtkAnatomistRenderer.h"
#include "vtkTransform.h"
#include "vtkPerspectiveTransform.h"
#include "vtkCommand.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include <vtkFibersManager.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>

using namespace anatomist;
using namespace std;


vtkQAGLWidget::vtkQAGLWidget( AWindow* win, QWidget* parent, const char* name,
                              const QGLWidget * shareWidget, Qt::WFlags f )
  : vtkGLWidget( parent, name, shareWidget, f ), GLWidgetManager( win, this )
{

  vtkObject::SetGlobalWarningDisplay (0); // due to legacy warnings, standard output is polluted.
  
  AWindow3D* papa = dynamic_cast<AWindow3D*>(win);
  if( papa )
  {
    _parent = papa;    
    // change the light direction
    /*
      anatomist::Light* light = papa->light();
      light->SetPosition(0.0, 0.0, 10.0, 0.0);
      light->SetSpotDirection (0.0, 0.0, 1.0);
      light->SetModelTwoSide( 1.0 );
    */
  }
  else
  {
    std::cerr << "Cannot cast to AWindow3D!" << std::endl;
  }

  
  vtkRenderer* ren = vtkRenderer::New();
  ren->SetBackground (1.0, 1.0, 1.0);    
  this->RenderWindow->AddRenderer(ren);
  

  vtkCornerAnnotation* corner = vtkCornerAnnotation::New();
  corner->SetNonlinearFontScaleFactor (0.3);
  vtkTextProperty* textProperty = vtkTextProperty::New();
  textProperty->SetColor (0.0,0.0,0.0);
  corner->SetTextProperty ( textProperty );
  corner->SetText (0, "Powered by VTK - Anatomist" );
  
  this->AddActor (corner);

  corner->Delete();
  textProperty->Delete();
  ren->Delete();
  
  //this->RenderWindow->SetSwapBuffers (false);
}


vtkQAGLWidget::~vtkQAGLWidget()
{
  /* Remove any actors of remaining vtkAObjects, in case it was not done already. */
  for( unsigned int i=0; i<_vtkAObjects.size(); i++)
  {
    _vtkAObjects[i]->removeActors( this );
  }
  _vtkAObjects.clear();
  
  //vtkAReader::deleteVTKAObjects();
}


void vtkQAGLWidget::initializeGL()
{

  //glClearColor( 1, 1, 1, 1 );
  //glEnable(GL_DEPTH_TEST);

  
  //vtkGLWidget::initializeGL();
  GLWidgetManager::initializeGL();

  // copy-paster from GLWidgetManager::initializeGL()
  // only difference, culling is set to front and not back.

  /*
  // depth peeling checks and initialization
  checkDepthPeeling( _pd );
  */
  // "regular" stuff
  glEnable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHT0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  //glFrontFace( GL_CCW ); // might be due to SetZDirection(false)
  glFrontFace( GL_CW );
  
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
  glEnable( GL_NORMALIZE );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  glClearColor( 1, 1, 1, 1 );
  
}


void vtkQAGLWidget::resizeGL( int w, int h )
{
  vtkGLWidget::resizeGL (w, h);
}


void vtkQAGLWidget::vtkUpdateCamera()
{
  //vtkRendererCollection* collec = this->GetRenderers();
  vtkRendererCollection* collec = this->RenderWindow->GetRenderers();
  if( !collec )
  {
    return;
  }
  
  vtkCollectionSimpleIterator rsit;
  collec->InitTraversal(rsit);
  vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
  if (firstRen == NULL)
  {
    // We cannot determine the number of layers because there are no
    // renderers.  No problem, just return.
    return;
  }
  vtkRenderWindow* renWin = firstRen->GetRenderWindow();
  int numLayers = renWin->GetNumberOfLayers();

  qglWidget()->makeCurrent();
  
  vtkRenderer* ren;
  for (int i = 0; i < numLayers; i++)
  {
    for (collec->InitTraversal(rsit); (ren = collec->GetNextRenderer(rsit)); )
    {
      if (ren->GetLayer() == i)
      {
	if( ren->GetActiveCamera() )
	{
	  /*
	    Render() sets the openGL projection and model view matrices to values
	    computed by VTK. Using a vtkAnatomistCamera,those are ensured to
	    render Anatomist objects and VTK actors within the same context.
	   */
	  ren->GetActiveCamera()->Render (ren);
	}
      }
    }
  }

}


/*
  The following method replaces call to RenderWindow->Render().
 */
void vtkQAGLWidget::vtkRender()
{
  this->InvokeEvent(vtkCommand::StartEvent,NULL);

  if ( ! this->GetInitialized() )
    {
    this->Initialize();
    }
  
  //vtkRendererCollection* collec = this->GetRenderers();
  vtkRendererCollection* collec = this->RenderWindow->GetRenderers();
  if( !collec )
  {
    return;
  }


  //collec->Render();
  
  //return;

  
    
  vtkCollectionSimpleIterator rsit;
  collec->InitTraversal(rsit);
  vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
  if (firstRen == NULL)
  {
    // We cannot determine the number of layers because there are no
    // renderers.  No problem, just return.
    return;
  }

  //this->RenderWindow->Start();
  //this->RenderWindow->StereoUpdate();
  
  vtkRenderWindow* renWin = firstRen->GetRenderWindow();
  int numLayers = renWin->GetNumberOfLayers();

  qglWidget()->makeCurrent();
  
  vtkRenderer* ren;
  for (int i = 0; i < numLayers; i++)
  {
    for (collec->InitTraversal(rsit); (ren = collec->GetNextRenderer(rsit)); )
    {
      if (ren->GetLayer() == i)
      {
	ren->Render ();
      }
    }
  }
  
  //this->InvokeEvent(vtkCommand::EndEvent,NULL);
  
  
}


void vtkQAGLWidget::rotate()
{
  // Modelview matrix: we only apply rotation for now!
  //glTranslatef( d->campos[0], d->campos[1], d->campos[2] );

  
  /*  if (_compassOn)
    {
      // Projection matrix: save before redefining locally for compass
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();

      // Viewport to draw compass into
      compassWinDim = (_dimx/4 < 70) ? _dimx/4 : 70;
      glViewport(0, 0, compassWinDim, compassWinDim);
      
      // Projection matrix: compass needs this orthographic projection
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      orthoMinX = - 1.0;
      orthoMinY = - 1.0;
      orthoMinZ = - 1.0;
      orthoMaxX =   1.0;
      orthoMaxY =   1.0;
      orthoMaxZ =   1.0;
      glOrtho(orthoMinX, orthoMaxX, 
	      orthoMinY, orthoMaxY, 
	      orthoMinZ, orthoMaxZ);

      // Draw compass
      glClear(GL_DEPTH_BUFFER_BIT);
      glCallList(_3DGuide->GetCompassGLList());

      // Projection matrix: restore
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      }*/


  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glMultMatrixf( &rotation()[0] );

  
  // Viewport to draw objects into
  glViewport( 0, 0, width(), height() );
  glEnable( GL_SCISSOR_TEST );
  glScissor( 0, 0, width(), height() );
  
  
  // Modelview matrix: we can now apply translation and left-right mirroring
  glMatrixMode( GL_MODELVIEW );
  glScalef( invertedX() ? -1 : 1, invertedY() ? -1 : 1, invertedZ() ? -1 : 1 );
  Point3df center = rotationCenter();
  glTranslatef( -center[0], -center[1], -center[2] );
  
  
  // Draw frame
  /*if (_frameOn)
    glCallList(_3DGuide->GetFrameGLList());*/

  vtkRotate();
  
}


void vtkQAGLWidget::paintGL()
{
  //carto::vtkGLWidget::paintGL();
  anatomist::GLWidgetManager::paintGL();
}

void vtkQAGLWidget::paintGL( DrawMode m )
{
  qglWidget()->makeCurrent();

  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  project();


    // Lighting is described in the viewport coordinate
  
  if( glIsList( lightGLList() ) )
    glCallList( lightGLList() );
  
  
  rotate();
  

  vtkUpdateCamera();


  // Clear the viewport // -> do it here otherwise flickering may occur
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  
  
  if( hasTransparentObjects() && depthPeelingEnabled() )
    depthPeelingRender( m );
  else
    drawObjects( m );
  

  //this->SetDrawMode (m);
}


void vtkQAGLWidget::drawObjects( DrawMode m )
{
  GLWidgetManager::drawObjects( m );
  vtkRender();
}


QSize vtkQAGLWidget::sizeHint() const
{
  return GLWidgetManager::sizeHint();
}


void vtkQAGLWidget::updateGL()
{
  vtkGLWidget::updateGL();
  
  if( recording() )
    record();
  if( rightEye() )
    rightEye()->updateGL();
}


void vtkQAGLWidget::project()
{
  // Make our OpenGL context current
  qglWidget()->makeCurrent();

  // Projection matrix: should be defined only at init time and when resizing
  float	w = width(), h = height();
  float ratio = w / h;

  float	sizex = ( windowBoundingMax()[0] - windowBoundingMin()[0] ) / 2;
  float	sizey = ( windowBoundingMax()[1] - windowBoundingMin()[1] ) / 2;
  float oratio = ratio / sizex * sizey;
  if( oratio <= 1.0 )
    sizey /= oratio;
  else
    sizex *= oratio;
  sizex /= zoom();
  sizey /= zoom();

  Point3df		bmin, bmax;
  Transformation	t( 0, 0 );

  t.setQuaternion( quaternion() );
  t.invert();

  Point3df	bmino = boundingMin() - rotationCenter();
  Point3df	bmaxo = boundingMax() - rotationCenter();
  bmino[2] *= -1;
  bmaxo[2] *= -1;
  t.transformBoundingBox( bmino, bmaxo, bmin, bmax );

  
  Point3df bbmin = boundingMin();
  Point3df bbmax = boundingMax();

  
  //	viewport setup

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  if( perspectiveEnabled() )
    {
      float pnear = -bmax[2], pfar = -bmin[2];
      if( pnear <= 0 )
      {
        if( pfar <= 0 )
          pnear = 1;
        else
          pnear = pfar * 0.05;
      }
      if( pfar < pnear )
        pfar = pnear * 10;
      gluPerspective( 45, ratio, pnear, pfar );
    }
  else
    {

      //cout << "clip [ " << bmin[2] << ", " << bmax[2] << "]\n";
      //cout << "glWidget ortho : " << sizex*2 << " x " << sizey*2 << endl;
      //glOrtho( -sizex, sizex, -sizey, sizey, -bmax[2], -bmin[2] );

      double near =  -bmax[2];
      double far  =  -bmin[2];

      //vtkRendererCollection* collec = this->GetRenderers();
      vtkRendererCollection* collec = this->RenderWindow->GetRenderers();
      vtkCollectionSimpleIterator rsit;
      collec->InitTraversal(rsit);
      vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
      if (firstRen != NULL)
      {
	vtkRenderer* ren;
	for (collec->InitTraversal(rsit); (ren = collec->GetNextRenderer(rsit)); )
	{
	  vtkAnatomistCamera* cam = vtkAnatomistCamera::SafeDownCast (ren->GetActiveCamera());
	  if( cam)
	  {

	    /*
	      Compute the global bounding box of VTK and anatomist objects.
	     */
	    double bounds[6];
	    ren->ComputeVisiblePropBounds (bounds);

	    bounds[0] = bounds[0]<(double)bbmin[0]?bounds[0]:(double)bbmin[0];
	    bounds[1] = bounds[1]>(double)bbmax[0]?bounds[1]:(double)bbmax[0];
	    bounds[2] = bounds[2]<(double)bbmin[1]?bounds[2]:(double)bbmin[1];
	    bounds[3] = bounds[3]>(double)bbmax[1]?bounds[3]:(double)bbmax[1];
	    bounds[4] = bounds[4]<(double)bbmin[2]?bounds[4]:(double)bbmin[2];
	    bounds[5] = bounds[5]>(double)bbmax[2]?bounds[5]:(double)bbmax[2];
	    
	    ren->ResetCameraClippingRange(bounds);

	    /*
	      Find out the best clipping range.
	    */
	    near = near<cam->GetClippingRange()[0]?near:cam->GetClippingRange()[0];
	    far  = far>cam->GetClippingRange()[1]?far:cam->GetClippingRange()[1];
	    
	    cam->SetPerspectiveBounds ( (double)-sizex, (double)sizex, (double)-sizey, (double)sizey, near, far);

	    /*
	      Every time the camera is accessed, it recomputes its ViewTransform internally.
	      But as we have to -1 the first row, we have to do it again.
	    */	    
	    vtkMatrix4x4* matrix = cam->GetViewTransformMatrix();
	    for( int i=0; i<4; i++)
	      matrix->SetElement (0, i, -1.0 * matrix->GetElement (0, i) );
	    cam->GetViewTransformObject()->Update();
	    
	  }
	}
      }

      /*
	The next lines are unecessary, they are done by the call to vtkUpdateCamera().
       */
      //glDepthRange (-1, 1);
      //glOrtho( -sizex, sizex, -sizey, sizey, near, far );
      
    }
  
  // Modelview matrix: we now use the viewport coordinate system
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
}
 




 void vtkQAGLWidget::vtkRotate()
 {

   double buffer[16];
   glGetDoublev (GL_MODELVIEW_MATRIX, buffer);
   vtkMatrix4x4* mat = vtkMatrix4x4::New();
   mat->DeepCopy (buffer);
   
   
   //vtkRendererCollection* collec = this->GetRenderers();
   vtkRendererCollection* collec = this->RenderWindow->GetRenderers();

   // loop over all renderes
   vtkCollectionSimpleIterator rsit;
   collec->InitTraversal(rsit);
   vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
   if (firstRen != NULL)
   {
     //vtkRenderWindow* renWin = firstRen->GetRenderWindow();
     //int numLayers = renWin->GetNumberOfLayers();
     
     vtkRenderer* ren;
     for (collec->InitTraversal(rsit); (ren = collec->GetNextRenderer(rsit)); )
     {
       vtkAnatomistCamera* cam = vtkAnatomistCamera::SafeDownCast (ren->GetActiveCamera());
       if( cam)
       {
	 Point3df center = rotationCenter();
	 /*
	 double focal[3]={center[0], center[1], center[2]};
	 double position[3]={center[0]+cam->GetDistance()*mat->GetElement(0,2),
			     center[1]+cam->GetDistance()*mat->GetElement(1,2),
			     center[2]+cam->GetDistance()*mat->GetElement(2,2)};
	 double viewup[3]={mat->GetElement(0,1), mat->GetElement(1,1), mat->GetElement(2,1)};

	 cam->SetFocalPoint (0, 0, -1);
	 cam->SetPosition (0,0,0);
	 cam->SetViewUp (0,1,0);

	 vtkPerspectiveTransform* pTransform = vtkPerspectiveTransform::New();
	 pTransform->SetupCamera (position, focal, viewup );
	 vtkMatrix4x4* matrix = pTransform->GetMatrix();
	 for( int i=0; i<4; i++)
	   matrix->SetElement (0, i, -1.0 * matrix->GetElement (0, i) );

	 vtkTransform* transform = vtkTransform::New();
	 transform->SetMatrix ( matrix );
	 
	 cam->GetViewTransformObject()->SetInput ( transform );

	 pTransform->Delete();
	 transform->Delete();
	 
	 */
	 	
	 cam->SetFocalPoint (center[0], center[1], center[2]);
	 cam->SetPosition (center[0]+cam->GetDistance()*mat->GetElement(0,2),
			   center[1]+cam->GetDistance()*mat->GetElement(1,2),
			   center[2]+cam->GetDistance()*mat->GetElement(2,2));
	 cam->SetViewUp (mat->GetElement(0,1), mat->GetElement(1,1), mat->GetElement(2,1));
	 
	 vtkMatrix4x4* matrix = cam->GetViewTransformMatrix();
	 //std::cout << *matrix << std::endl;
	 for( int i=0; i<4; i++)
	   matrix->SetElement (0, i, -1.0 * matrix->GetElement (0, i) );
	 cam->GetViewTransformObject()->Update();

	 /*
	 vtkTransform* transform = vtkTransform::New();
	 transform->SetMatrix ( mat );
	 cam->GetViewTransformObject()->SetInput ( transform );
	 transform->Delete();
	 */
	 
	 //ren->ResetCameraClippingRange(); // new clipping range for vtk
       }
     }
   }

   mat->Delete();
   
 }
 

void vtkQAGLWidget::setupView()
{
  project();

  // Modelview matrix: we only apply rotation for now!
  glLoadMatrixf( &rotation()[0] );

  // Viewport to draw objects into
  glViewport( 0, 0, width(), height() );
  glEnable( GL_SCISSOR_TEST );
  glScissor( 0, 0, width(), height() );
  
  // Modelview matrix: we can now apply translation and left-right mirroring
  glMatrixMode( GL_MODELVIEW );
  glScalef( invertedX() ? -1 : 1, invertedY() ? -1 : 1, invertedZ() ? -1 : 1 );
  Point3df center = rotationCenter();
  glTranslatef( -center[0], -center[1], -center[2] );

  vtkRotate();

  vtkUpdateCamera();
}


void vtkQAGLWidget::mousePressEvent( QMouseEvent* ev )
{

  //cout << "vtkQAGLWidget::mousePressEvent\n";

  vtkGLWidget::mousePressEvent( ev );

  GLWidgetManager::mousePressEvent( ev );


}


void vtkQAGLWidget::mouseReleaseEvent( QMouseEvent* ev )
{
  /*cout << "vtkQAGLWidget::mouseReleaseEvent\n";
  cout << "button : " << (int) ev->button() << endl;
  cout << "state  : " << (int) ev->state() << endl;*/
  
  GLWidgetManager::mouseReleaseEvent( ev );

  vtkGLWidget::mouseReleaseEvent( ev );

}


void vtkQAGLWidget::mouseMoveEvent( QMouseEvent* ev )
{
  /*cout << "vtkQAGLWidget::mouseMoveEvent\n";
  cout << "button : " << (int) ev->button() << endl;
  cout << "state  : " << (int) ev->state() << endl;*/
  
  GLWidgetManager::mouseMoveEvent( ev );

  vtkGLWidget::mouseMoveEvent( ev );

}


void vtkQAGLWidget::keyPressEvent( QKeyEvent* ev )
{
  /* cout << "vtkQAGLWidget::keyPressEvent\n";
  cout << "key   : " << ev->key() << endl;
  cout << "state : " << (int) ev->state() << endl; */

  vtkGLWidget::keyPressEvent( ev );
  
  GLWidgetManager::keyPressEvent( ev );

  
}


void vtkQAGLWidget::keyReleaseEvent( QKeyEvent* ev )
{
  
  /* cout << "vtkQAGLWidget::keyReleaseEvent\n";
  cout << "key   : " << ev->key() << endl;
  cout << "state : " << (int) ev->state() << endl; */

  vtkGLWidget::keyReleaseEvent( ev );

  GLWidgetManager::keyReleaseEvent( ev );
  

}


void vtkQAGLWidget::focusInEvent( QFocusEvent * ev  )
{
  GLWidgetManager::focusInEvent( ev );
}


void vtkQAGLWidget::focusOutEvent( QFocusEvent * ev )
{
  GLWidgetManager::focusOutEvent( ev );
}


void vtkQAGLWidget::wheelEvent( QWheelEvent* ev )
{
  GLWidgetManager::wheelEvent( ev );

  vtkGLWidget::wheelEvent( ev );
}


string vtkQAGLWidget::name() const
{
  return( "QAGLWidget" );
}


QSize vtkQAGLWidget::minimumSizeHint() const
{
  return GLWidgetManager::minimumSizeHint();
}



void vtkQAGLWidget::AddActor(vtkProp* actor)
{
  vtkRendererCollection* collec = this->RenderWindow->GetRenderers();
  if( !collec )
  {
    return;
  }
  
  vtkCollectionSimpleIterator rsit;
  collec->InitTraversal(rsit);
  vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
  if (firstRen == NULL)
  {
    return;
  }
  
  firstRen->AddActor( actor );

}


void vtkQAGLWidget::RemoveActor(vtkProp* actor)
{
  vtkRendererCollection* collec = this->RenderWindow->GetRenderers();
  if( !collec )
  {
    return;
  }
  
  vtkCollectionSimpleIterator rsit;
  collec->InitTraversal(rsit);
  vtkRenderer* firstRen = collec->GetNextRenderer(rsit);
  if (firstRen == NULL)
  {
    return;
  }
  
  firstRen->RemoveActor( actor );
}


void vtkQAGLWidget::registerVtkAObject (anatomist::vtkAObject* object)
{
  if( object )
    _vtkAObjects.push_back (object);
}



void vtkQAGLWidget::unregisterVtkAObject (anatomist::vtkAObject* object)
{
  std::vector <anatomist::vtkAObject*>::iterator it = _vtkAObjects.begin();
  while( it != _vtkAObjects.end() )
  {
    if( (*it) == object )
    {
      _vtkAObjects.erase ( it );
      break;
    }
    ++it;
  }
  
}
