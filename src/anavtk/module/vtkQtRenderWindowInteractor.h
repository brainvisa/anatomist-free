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
/*=========================================================================
  This module is an extension of the "Visualization Toolkit 
  ( copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen )".
  and combines it with "Qt (copyright (C) 1992-2000 Troll Tech AS)".
  =========================================================================*/
/*=========================================================================

  Author:    Matthias Koenig, last changed by $Author: koenig $
  Module:    $RCSfile: vtkQtRenderWindowInteractor.h,v $
  Date:      $Date: 2005/07/20 15:01:02 $
  Version:   $Revision: 1.2 $

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
  * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
   
  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
   
  * Modified source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  =========================================================================*/

#ifndef _vtkQtRenderWindowInteractor_h
#define _vtkQtRenderWindowInteractor_h

#include <cstdlib>
#include <qobject.h>
#include <qtimer.h>
#include <qcursor.h>


#include "vtkCommand.h"
#include "vtkInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "anatomist/module/vtkQtRenderWindow.h"


class VTK_QT_EXPORT vtkQtRenderWindowInteractor : public QObject, public vtkRenderWindowInteractor {
  Q_OBJECT
    friend class vtkQtRenderWindow;
 public:
  vtkTypeMacro(vtkQtRenderWindowInteractor, vtkRenderWindowInteractor);
  vtkQtRenderWindowInteractor();
  virtual ~vtkQtRenderWindowInteractor();
  static vtkQtRenderWindowInteractor *New(); // inline
  void PrintSelf(ostream& os, vtkIndent indent);
    
  virtual void Start();
    
  void SetRenderWindow(vtkQtRenderWindow*);
    
  // timer methods
  virtual int CreateTimer(int timertype);
  virtual int DestroyTimer(void);
    
  // own exit method 
  virtual void TerminateApp(); // inline 
    
 protected slots:
    void timer();
 protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void keyPressEvent(QKeyEvent*);
 private:
  vtkQtRenderWindow *qtRenWin;
  QTimer qTimer;
};

inline vtkQtRenderWindowInteractor *vtkQtRenderWindowInteractor::New() { return new vtkQtRenderWindowInteractor; }
// comment 'qApp->exit()' out to disable quitting of the application with the 'q' key
inline void vtkQtRenderWindowInteractor::TerminateApp()                { /*qApp->exit();*/ }

#endif


