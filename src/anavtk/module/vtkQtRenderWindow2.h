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
#ifndef _vtkQtRenderWindow2_h_
#define _vtkQtRenderWindow2_h_

#include <QObject>

#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
#include <vtkXOpenGLRenderWindow.h>
#endif

#if defined( Q_WS_WIN32 ) || defined( Q_OS_WIN32 )
#include <vtkWin32OpenGLRenderWindow.h>
#endif

#if defined( Q_WS_MAC ) || defined( Q_OS_MAC )
#include <vtkCarbonRenderWindow.h>
#endif

class vtkQtRenderWindow2 : 
#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
  public vtkXOpenGLRenderWindow
#endif
#if defined( Q_WS_WIN32 ) || defined( Q_OS_WIN32 )
  public vtkWin32OpenGLRenderWindow
#endif
#if defined( Q_WS_MAC ) || defined( Q_OS_MAC )
  public vtkCarbonRenderWindow
#endif
{

 public:
  static vtkQtRenderWindow2 *New();
#if defined( Q_WS_X11 ) || defined( Q_OS_LINUX )
#if VTK_MAJOR_VERSION >= 6
  vtkTypeMacro (vtkQtRenderWindow2, vtkXOpenGLRenderWindow);
#else
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkXOpenGLRenderWindow);
#endif
#endif
#if defined( Q_WS_WIN32 ) || defined( Q_OS_WIN32 )
#if VTK_MAJOR_VERSION >= 6
  vtkTypeMacro (vtkQtRenderWindow2, vtkWin32OpenGLRenderWindow);
#else
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkWin32OpenGLRenderWindow);
#endif
#endif
#if defined( Q_WS_MAC ) || defined( Q_OS_MAC )
#if VTK_MAJOR_VERSION >= 6
  vtkTypeMacro (vtkQtRenderWindow2, vtkCarbonRenderWindow);
#else
  vtkTypeRevisionMacro (vtkQtRenderWindow2, vtkCarbonRenderWindow);
#endif
#endif

  void MakeCurrent (void);
  
 protected:
  vtkQtRenderWindow2(){};
  ~vtkQtRenderWindow2(){};

 private:
  vtkQtRenderWindow2 (const vtkQtRenderWindow2&);
  void operator=(const vtkQtRenderWindow2&);
  
};



#endif
