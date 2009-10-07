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
#ifndef _vtk_AnatomistCamera_h_
#define _vtk_AnatomistCamera_h_

#include "vtkOpenGLCamera.h"
#include "vtkPerspectiveTransform.h"
#include "vtkTransform.h"

class AnaPerspectiveTransform;
class AnaViewTransform;


class vtkAnatomistCamera : public vtkOpenGLCamera
{
public:
  static vtkAnatomistCamera *New();
  vtkTypeRevisionMacro(vtkAnatomistCamera,vtkOpenGLCamera);

  // Description:
  // Implement base class method.
  void Render(vtkRenderer *ren);

  void SetAnaPerspectiveTransform (const double[16]);
  void SetAnaPerspectiveTransform (vtkMatrix4x4*);
  vtkGetObjectMacro (AnaPerspectiveTransform, vtkMatrix4x4);
  
  void SetAnaViewTransform (const double[16]);
  void SetAnaViewTransform (vtkMatrix4x4*);
  vtkGetObjectMacro (AnaViewTransform, vtkMatrix4x4);


#if VTK_MAJOR_VERSION>5 || (VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >=4)
  virtual vtkMatrix4x4 *GetProjectionTransformMatrix(double,
                                                      double,
                                                      double);
#else
  virtual vtkMatrix4x4 *GetPerspectiveTransformMatrix(double,
                                                      double,
                                                      double);
#endif
  
  void SetPerspectiveBounds (const double&, const double&, const double&, const double&, const double&, const double&);
  
protected:  
  vtkAnatomistCamera();
  ~vtkAnatomistCamera();

  vtkMatrix4x4* AnaPerspectiveTransform;
  vtkMatrix4x4* AnaViewTransform;
  double PerspectiveBounds[6];
   
  
private:
  vtkAnatomistCamera(const vtkAnatomistCamera&);  // Not implemented.
  void operator=(const vtkAnatomistCamera&);  // Not implemented.
};


#endif
