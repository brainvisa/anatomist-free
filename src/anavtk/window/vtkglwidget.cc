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
// #include "anatomist/module/vtkAnatomistRenderer.h"
#include "vtkTransform.h"
#include "vtkPerspectiveTransform.h"
#include "vtkCommand.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#ifndef ANATOMIST_NO_VTKINRIA3D
#include <vtkFibersManager.h>
#endif
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>
#include <vtkIdentityTransform.h>
#include <QGraphicsView>

using namespace anatomist;
using namespace std;


vtkQAGLWidget::vtkQAGLWidget( AWindow* win, QWidget* parent, const char* name,
                              const QGLWidget * shareWidget,
                              Qt::WindowFlags f )
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


/*  vtkCornerAnnotation* corner = vtkCornerAnnotation::New();
  corner->SetNonlinearFontScaleFactor (0.3);
  vtkTextProperty* textProperty = vtkTextProperty::New();
  textProperty->SetColor (0.0,0.0,0.0);
  corner->SetTextProperty ( textProperty );
  corner->SetText (0, "Powered by VTK - Anatomist" );

  this->AddActor (corner);

  corner->Delete();
  textProperty->Delete();*/
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
  GLWidgetManager::resizeGL( w, h );
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

  vtkRotate();
}


void vtkQAGLWidget::paintGL()
{
  //carto::vtkGLWidget::paintGL();
  anatomist::GLWidgetManager::paintGL();
}

void vtkQAGLWidget::paintGL( DrawMode m )
{
  // cout << "vtkQAGLWidget::paintGL " << m << endl;
  qglWidget()->makeCurrent();
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();

  glPushAttrib( GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT );
  glEnable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glFrontFace( GL_CW );
  glCullFace( GL_BACK );
  glEnable( GL_CULL_FACE );
  glEnable( GL_NORMALIZE );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
  glClearColor( 1, 1, 1, 1 );

  project();


  if( m != ObjectSelect && m != ObjectsSelect && m != PolygonSelect )
  {
    // Lighting is described in the viewport coordinate

    if( glIsList( lightGLList() ) )
      glCallList( lightGLList() );
  }

  rotate();
  vtkUpdateCamera();

  // Clear the viewport // -> do it here otherwise flickering may occur
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if( hasTransparentObjects() && depthPeelingEnabled() )
    depthPeelingRender( m );
  else
    drawObjects( m );

  //this->SetDrawMode (m);

  glPopAttrib();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}


bool vtkQAGLWidget::positionFromCursor( int x, int y, Point3df & position )
{
  if( qglWidget()->parentWidget()
    && dynamic_cast<QGraphicsView *>( qglWidget()->parentWidget() ) )
    /* FIXME: temporary fix, I don't know wky the Z buffer sometimes changes
       when the GL widget is in a graphics view, and just redrawing it is
       not enough. */
  {
    paintGL( ZSelect );
    paintGL( ZSelect );
    setZBufferUpdated( true );
    setRGBBufferUpdated( false );
  }
  return GLWidgetManager::positionFromCursor( x, y, position );
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
  bool done = false;
  if( dynamic_cast<QGraphicsView *>( parent() ) )
  {
    // cout << "updateGL in a QGraphicsView\n";
    QGraphicsView *gv
      = dynamic_cast<QGraphicsView *>( parent() );
    if( gv->scene() )
      gv->scene()->update();
    done = true;
  }

  if( !done )
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

  double near =  -bmax[2];
  double far  =  -bmin[2];

  if( perspectiveEnabled() )
    {
      if( near <= 0 )
      {
        if( far <= 0 )
          near = 1;
        else
          near = far * 0.05;
      }
      if( far < near )
        far = near * 10;
      gluPerspective( 45, ratio, near, far );
    }
  // else
    {

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
	    double cnear = near<cam->GetClippingRange()[0]?near:cam->GetClippingRange()[0];
	    double cfar  = far>cam->GetClippingRange()[1]?far:cam->GetClippingRange()[1];
	    
	    cam->SetPerspectiveBounds ( (double)-sizex, (double)sizex, (double)-sizey, (double)sizey, cnear, cfar);

            //cam->SetParallelProjection( !perspectiveEnabled() );
            if( perspectiveEnabled() )
            {
              // cout << "cam distance: " << cam->GetDistance() << endl;
              // cout << "dolly: " << cam->GetDolly() << ", roll: " << cam->GetRoll() << endl;
              cam->SetViewAngle( 45. );
              cam->SetDistance( 50. );
              // taken from gluPerspective doc
              double	mat[16];
              double fovy = 0.25 * M_PI; // 45 degrees
              double f = 1./tan(fovy/2.);
              cout << "near: " << near << ", far: " << far << endl;
              mat[0] = f / ratio;
              mat[4] = 0.;
              mat[8] = 0.;
              mat[12] = 0.;
              mat[1] = 0.;
              mat[5] = f;
              mat[9] = 0.;
              mat[13] = 0.;
              mat[2] = 0.;
              mat[6] = 0.;
              mat[10] = ( far + near ) / ( near - far );
              mat[14] = -1.;
              mat[3] = 0.;
              mat[7] = 0.;
              mat[11] = far * near * 2. / ( near - far );
              mat[15] = 0.;
              vtkTransform *vtkt = vtkTransform::New();
              vtkt->SetMatrix( mat );
              if( invertedZ() )
              {
                vtkTransform *vtkt2 = vtkTransform::New();
                double	mat2[16] = { 1.,0,0,0, 0,1.,0,0, 0,0,-1.,0, 0,0,0,1. };
                vtkt2->SetMatrix( mat2 );
                vtkt2->Concatenate( vtkt );
                vtkt = vtkt2;
              }
              cam->SetUserTransform( vtkt );
            }
            else
            {
              cam->SetUserTransform( vtkIdentityTransform::New() );
            }

	    /*
	      Every time the camera is accessed, it recomputes its ViewTransform internally.
	      But as we have to -1 the first row, we have to do it again.
	    */
	    /*
	    vtkMatrix4x4* matrix = cam->GetViewTransformMatrix();
	    for( int i=0; i<4; i++)
	      matrix->SetElement (0, i, -1.0 * matrix->GetElement (0, i) );
	    cam->GetViewTransformObject()->Modified(); //Update();
	    */
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

	 cam->SetFocalPoint (center[0], center[1], center[2]);
	 cam->SetPosition (center[0]+cam->GetDistance()*mat->GetElement(0,2),
			   center[1]+cam->GetDistance()*mat->GetElement(1,2),
			   center[2]+cam->GetDistance()*mat->GetElement(2,2));
	 cam->SetViewUp (mat->GetElement(0,1), mat->GetElement(1,1), mat->GetElement(2,1));

	 vtkMatrix4x4* matrix = cam->GetViewTransformMatrix();
	 for( int i=0; i<4; i++)
	   for( int j=0; j<4; j++)
	     matrix->SetElement (i, j, mat->GetElement (j, i) );
	 cam->GetViewTransformObject()->Modified();
	 cam->GetViewTransformObject()->Update();
	 
	 //ren->ResetCameraClippingRange(); // new clipping range for vtk
       }
     }
   }

   mat->Delete();
   
 }
 

void vtkQAGLWidget::setupView()
{
  //rotate();
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


#if QT_VERSION >= 0x040600
bool vtkQAGLWidget::event( QEvent * event )
{
  if( event->type() == QEvent::Gesture )
  {
    gestureEvent( static_cast<QGestureEvent*>( event ) );
    return true;
  }
  return QWidget::event(event);
}
#endif


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


void vtkQAGLWidget::mouseDoubleClickEvent( QMouseEvent* ev )
{
  /*cout << "vtkQAGLWidget::mouseDoubleClickEvent\n";
  cout << "button : " << (int) ev->button() << endl;
  cout << "state  : " << (int) ev->state() << endl;*/

  GLWidgetManager::mouseDoubleClickEvent( ev );

  vtkGLWidget::mouseDoubleClickEvent( ev );

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
