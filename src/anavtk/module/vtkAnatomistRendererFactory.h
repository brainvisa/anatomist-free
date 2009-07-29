#ifndef __vtkAnatomistRendererFactory_h
#define __vtkAnatomistRendererFactory_h

#include "vtkObjectFactory.h"

class vtkAnatomistRendererFactory : public vtkObjectFactory
{
public: 
// Methods from vtkObject
  vtkTypeRevisionMacro(vtkAnatomistRendererFactory,vtkObjectFactory);
  static vtkAnatomistRendererFactory *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual const char* GetVTKSourceVersion();
  virtual const char* GetDescription();
protected:
  vtkAnatomistRendererFactory();
  ~vtkAnatomistRendererFactory() { }
private:
  vtkAnatomistRendererFactory(const vtkAnatomistRendererFactory&);  // Not implemented.
  void operator=(const vtkAnatomistRendererFactory&);  // Not implemented.
};

extern "C" vtkObjectFactory* vtkLoad();
#endif
