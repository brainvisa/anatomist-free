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
  Module:    $RCSfile: vtkQtRenderWindowInteractor.cpp,v $
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

#include <qapplication.h>

#include "anatomist/module/vtkQtRenderWindowInteractor.h"
//#include <ctype.h>

vtkQtRenderWindowInteractor::vtkQtRenderWindowInteractor() {
    qtRenWin = NULL;
    this->SetLightFollowCamera (0);
}

vtkQtRenderWindowInteractor::~vtkQtRenderWindowInteractor() {
    // to circumvent VTK warnings assuming our destructor is called correctly
    if (this->ReferenceCount > 0) {
        this->SetReferenceCount(0);
    }
}

void vtkQtRenderWindowInteractor::Start() {
    if (! this->qtRenWin) {
        vtkErrorMacro(<<"No vtkQtRenderWindow defined!");
        return;
    }
    if (! qApp) {
        vtkErrorMacro(<<"No QApplication defined!");
        return;
    }
    if (!qApp->mainWidget()) {
        qApp->setMainWidget(qtRenWin);
        qApp->exec();
    }
}

void vtkQtRenderWindowInteractor::SetRenderWindow(vtkQtRenderWindow* aren) {
    qtRenWin = aren;
    aren->SetInteractor(this);
    vtkRenderWindowInteractor::SetRenderWindow(aren);
    qtRenWin->GetSize(Size[0], Size[1]);
}

void vtkQtRenderWindowInteractor::PrintSelf(ostream&os, vtkIndent indent) {
    vtkRenderWindowInteractor::PrintSelf(os, indent);
}

void vtkQtRenderWindowInteractor::mousePressEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    qtRenWin->GetSize(Size[0], Size[1]);

    int ctrl = 0, shift = 0;
    if (me->state() & Qt::ControlButton)
        ctrl = 1;
    if (me->state() & Qt::ShiftButton)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->button()) {
    case QEvent::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
        break;
    case QEvent::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
        break;
    case QEvent::RightButton:
        InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
        break;
    default:
        return;
    }
}

void vtkQtRenderWindowInteractor::mouseReleaseEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    qtRenWin->GetSize(Size[0], Size[1]);

    int ctrl = 0, shift = 0;
    if (me->state() & Qt::ControlButton)
        ctrl = 1;
    if (me->state() & Qt::ShiftButton)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);

    switch (me->button()) {
    case QEvent::LeftButton:
        InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
        break;
    case QEvent::MidButton:
        InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
        break;
    case QEvent::RightButton:
        InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
        break;
    default:
        return;
    }

}

void vtkQtRenderWindowInteractor::mouseMoveEvent(QMouseEvent *me) {
    if (!Enabled)
        return;

    qtRenWin->GetSize(Size[0], Size[1]);

    int ctrl = 0, shift = 0;
    if (me->state() & Qt::ControlButton)
        ctrl = 1;
    if (me->state() & Qt::ShiftButton)
        shift = 1;
    int xp = me->x();
    int yp = me->y();
    SetEventInformationFlipY(xp, yp, ctrl, shift);
    InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
}

void vtkQtRenderWindowInteractor::timer() {
    if (!Enabled)
        return;
    InvokeEvent(vtkCommand::TimerEvent,NULL);
}

int vtkQtRenderWindowInteractor::CreateTimer(int timertype) {
    if (timertype == VTKI_TIMER_FIRST) {
        QObject::connect(&qTimer, SIGNAL(timeout()), SLOT(timer()));
        qTimer.start(10);
    }
    return 1;
}

int vtkQtRenderWindowInteractor::DestroyTimer() {
    qTimer.stop();
    QObject::disconnect(&qTimer, SIGNAL(timeout()), this, 0);
    return 1;
}

void vtkQtRenderWindowInteractor::keyPressEvent(QKeyEvent *ke)
{

  if (!Enabled)
    return;
  
  
  qtRenWin->GetSize(Size[0], Size[1]);
  int ctrl = 0, shift = 0;
  if (ke->state() & Qt::ControlButton)
    ctrl = 1;
  if (ke->state() & Qt::ShiftButton)
    shift = 1;
  QPoint cp = qtRenWin->mapFromGlobal(QCursor::pos());
  int xp = cp.x();
  int yp = cp.y();
  SetEventInformationFlipY(xp, yp, ctrl, shift, (char) tolower(ke->ascii()), 1, (const char *) ke->text());
  InvokeEvent(vtkCommand::KeyPressEvent, NULL);
  InvokeEvent(vtkCommand::CharEvent, NULL);
 
}
