
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
#include <anatomist/vtkobject/vtkreader.h>
#include <anatomist/vtkobject/vtkaobject.h>
#include <anatomist/object/oReader.h>

#include <vtkDataSetReader.h>
//#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkStructuredPoints.h>
#include <vtkPointData.h>
#ifndef ANATOMIST_NO_VTKINRIA3D
#include <anatomist/vtkobject/vtkfiberaobject.h>
#include <anatomist/vtkobject/vtktensoraobject.h>
#include <anatomist/vtkobject/vtkvectoraobject.h>
#include <anatomist/vtkobject/vtkmetadatasetaobject.h>
#include <vtkMetaDataSetSequence.h>
#endif
//#include <vtkSmartPointer.h>

// FIXME: test
#include <vtkTextActor3D.h>
#include <vtkTextProperty.h>


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
  ObjectReader::registerLoader( "fib", readVTK );
  ObjectReader::registerLoader( "seq", readVTKSequence );
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


list<AObject *> vtkAReader::readVTK (const std::string& filename,
                              ObjectReader::PostRegisterList &,
                              Object /*options*/)
{
  // cout << "readVTK\n";

  vtkDataSetReader* reader = vtkDataSetReader::New();
  reader->SetFileName ( filename.c_str() );
  reader->Update();
  vtkDataSet* output = reader->GetOutput();

  // cout << "output: " << output << endl;

  vtkAObject* obj = 0;

  
  vtkPolyData* polydata = vtkPolyData::SafeDownCast (output);
  vtkStructuredPoints* spoints = vtkStructuredPoints::SafeDownCast (output);
  
  if( polydata )
  {
#ifndef ANATOMIST_NO_VTKINRIA3D
    if( polydata->GetNumberOfLines()>0 )
    {
      obj = vtkFiberAObject::New();
      obj->SetDataSet (output);
    }
    else
#endif
    {
      obj = vtkAObject::New();
      obj->SetDataSet (output);
    }
/*    {
      std::cerr << "Error: Only vtkPolyData with lines are supported for now, and you vtkPolyData has no line." << std::endl;
    }*/
  }
  else if (spoints)
  {
#ifndef ANATOMIST_NO_VTKINRIA3D
    if ( spoints->GetPointData()->GetTensors() )
    {
      obj = vtkTensorAObject::New();
      obj->SetDataSet (output);
    }
    else if ( spoints->GetPointData()->GetVectors() )
    {
      obj = vtkVectorAObject::New();
      obj->SetDataSet (output);
    }
#endif
    {
      obj = vtkAObject::New();
      obj->SetDataSet (output);
    }
/*    {
      std::cerr << "Error: Only vtkStructuredPoints with tensors are supported for now, and you vtkStructuredPoints has no tensor attribute." << std::endl;
    }*/
  }

  // FIXME: just trying things...
  /*
  if( !obj )
  {
    obj = vtkAObject::New();
    vtkTextActor3D *text = vtkTextActor3D::New();
    text->SetPosition( 100, 100, 80 );
    text->SetInput( "Rototor" );
    vtkTextProperty* textProperty = text->GetTextProperty();
    textProperty->SetFontSize( 18 );
    textProperty->SetFontFamilyToArial();
    textProperty->SetJustificationToCentered();
    textProperty->BoldOn();
    textProperty->ItalicOn();
    textProperty->ShadowOn();
    textProperty->SetColor( 0, 0, 1 );
    text->SetTextProperty( textProperty );
    obj->setVtkProp( text );
    cout << "text object created: " << text << ", font: " << textProperty->GetOpacity() << endl;
  }
  */

  list<AObject *> lobj;

  if( obj )
  {
    vtkAReader::ObjectList.push_back(obj);
    lobj.push_back( obj );
  }

  reader->Delete();

  return lobj;

}



list<AObject *> vtkAReader::readVTKSequence (
 #ifndef ANATOMIST_NO_VTKINRIA3D
                                     const std::string& filename,
#else
                                     const std::string& /*filename*/,
#endif
                                      ObjectReader::PostRegisterList &,
                                      Object /*options*/)
{
  list<AObject *> lobj;

#ifndef ANATOMIST_NO_VTKINRIA3D
  std::ifstream buffer (filename.c_str());
  if( buffer.fail() )
  {
    return lobj;
  }

  vtkMetaDataSetSequence* sequence = vtkMetaDataSetSequence::New();

  
  while ( !buffer.eof() && !buffer.fail() )
  {
    char file[256];
    buffer.getline( file, 256 );
    if( strcmp (file,"")!=0 )
    {
      sequence->ReadAndAddFile ( file );
    }
  }


  vtkMetaDataSetAObject* obj = vtkMetaDataSetAObject::New();
  obj->SetMetaDataSet (sequence);
  sequence->Delete();

  lobj.push_back( obj );
  return lobj;

#else
  return lobj;
#endif
}
