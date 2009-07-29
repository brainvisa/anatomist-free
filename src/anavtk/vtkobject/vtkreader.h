#ifndef _vtk_reader_h_
#define _vtk_reader_h_

#include <string>
#include <cartobase/object/object.h>

namespace anatomist
{

  class AObject;
  class vtkAObject;
  

  class vtkAReader
  {

  public:
    vtkAReader( const std::string& );
    ~vtkAReader();

    static AObject* readVTK (const std::string & filename, carto::Object options);
    static AObject* readVTKSequence (const std::string & filename, carto::Object options);
    
    static void deleteVTKAObjects();

  private:
    vtkAReader(){}

    static bool registerLoader();
    static bool initialized;

    std::string _filename;
	
    static std::vector<vtkAObject*> ObjectList;
    
  };
  
  
}


#endif
