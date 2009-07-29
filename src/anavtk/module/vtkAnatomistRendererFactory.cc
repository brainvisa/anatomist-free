#include <anatomist/window/vtkglwidget.h>

#include "vtkAnatomistRendererFactory.h"
#include "vtkAnatomistRenderer.h"
#include "vtkVersion.h"

vtkCxxRevisionMacro(vtkAnatomistRendererFactory, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkAnatomistRendererFactory);

void vtkAnatomistRendererFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "VTK AnatomistRenderer object factory" << endl;
}


VTK_CREATE_CREATE_FUNCTION(vtkAnatomistRenderer);

vtkAnatomistRendererFactory::vtkAnatomistRendererFactory()
{
  this->RegisterOverride("vtkRenderer",
                         "vtkAnatomistRenderer",
                         "AnatomistRenderer",
                         1,
                         vtkObjectFactoryCreatevtkAnatomistRenderer);
}

const char* vtkAnatomistRendererFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

const char* vtkAnatomistRendererFactory::GetDescription()
{
  return "VTK AnatomistRenderer Support Factory";
}


extern "C" vtkObjectFactory* vtkLoad()
{
  return vtkAnatomistRendererFactory::New();
}
