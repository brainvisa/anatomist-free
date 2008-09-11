
#include <anatomist/vtkobject/vtkreader.h>
#include <anatomist/vtkobject/vtkaobject.h>
#include <anatomist/object/oReader.h>

#include <vtkDataSetReader.h>
//#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
//#include <vtkSmartPointer.h>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;



bool vtkAReader::initialized = registerLoader();
std::vector<vtkAObject*> vtkAReader::ObjectList;

void vtkAReader::deleteVTKAObjects()
{
  for( unsigned int i=0; i<vtkAReader::ObjectList.size(); i++)
    {
      vtkAReader::ObjectList[i]->Delete();
    }
}


bool vtkAReader::registerLoader()
{
  ObjectReader::registerLoader( "vtk", readVTK );
  return true;
}


vtkAReader::vtkAReader (const std::string& filename) : _filename (filename)
{
}


vtkAReader::~vtkAReader()
{
  for(unsigned int i=0; i<this->ObjectList.size(); i++)
  {
    vtkAReader::ObjectList[i]->Delete();
  }
}


AObject* vtkAReader::readVTK (const std::string& filename, Object options)
{
  
  vtkDataSetReader* reader = vtkDataSetReader::New();
  reader->SetFileName (filename.c_str());
  reader->Update();
  
  vtkDataSet* output = reader->GetOutput();

  vtkAObject* obj = vtkAObject::New();
  obj->SetDataSet (output);

  vtkAReader::ObjectList.push_back(obj);

  reader->Delete();

  return obj;
  
}
