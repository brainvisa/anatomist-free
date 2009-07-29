#include "qapplication.h"

#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include "vtkTestUtilities.h"

#include "anatomist/module/vtkQtRenderWindowInteractor2.h"


//#include <vtkViewImage3D.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>


int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  vtkQtRenderWindowInteractor2* widget = vtkQtRenderWindowInteractor2::New();
  widget->resize(256,256);
 
#if QT_VERSION < 0x040000
  app.setMainWidget(widget);
#endif
  
  vtkRenderer* ren = vtkRenderer::New();
  ren->SetBackground (1.0, 1.0, 1.0);
  widget->GetRenderWindow()->AddRenderer(ren);
  
  vtkConeSource* source = vtkConeSource::New();
  source->SetHeight (100.0);
  source->SetRadius (10.0);
  
  // Mapper
  vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
  mapper->SetInputConnection(source->GetOutputPort());
  
  // Actor in scene
  vtkActor* actor = vtkActor::New();
  actor->SetMapper(mapper);

  
  // Add Actor to renderer
  ren->AddActor(actor);
  actor->SetPosition (115.752, 175.976, 86.3164);
  actor->GetProperty()->SetColor (1.0,0.0,0.0);

  ren->ResetCamera();
  


  /*
  vtkViewImage3D* view = vtkViewImage3D::New();
  vtkRenderer* ren = vtkRenderer::New();
  widget->GetRenderWindow()->AddRenderer(ren);
  view->SetRenderWindow ( widget->GetRenderWindow() );
  view->SetRenderer (ren);

  vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
  reader->SetFileName("/volatile/pfillard/Work/data/vtkINRIA3DData/MRI.vtk");

  view->SetImage (reader->GetOutput());

  view->SyncResetCurrentPoint();
  view->SyncResetWindowLevel();

  view->SetCubeVisibilityOn();
  */
  
  widget->show();

  app.exec();
  
  /*
  vtkStructuredPointsReader* reader = vtkStructuredPointsReader::New();
  reader->SetFileName("/volatile/pfillard/Work/data/vtkINRIA3DData/MRI.vtk");
  
  vtkImageViewer2* image_view = vtkImageViewer2::New();
  image_view->SetInputConnection( reader->GetOutputPort());

  widget->SetRenderWindow( image_view->GetRenderWindow() );
  image_view->SetupInteractor( widget->GetRenderWindow()->GetInteractor() );

  image_view->SetColorLevel(138.5);
  image_view->SetColorWindow(233);

  widget->show();

  app.exec();
  

  image_view->Delete();
  reader->Delete();


  */
  return 0;
}


