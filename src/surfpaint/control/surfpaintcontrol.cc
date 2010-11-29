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
#include <anatomist/module/surfpainttools.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <anatomist/action/surfpaintaction.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/window3D/control3D.h>
#include <anatomist/controler/view.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/misc/error.h>
#include <qlabel.h>
#include <qobject.h>
#include <qcursor.h>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;

Control *
SurfpaintToolsControl::creator()
{
  SurfpaintToolsControl * ns = new SurfpaintToolsControl();
  return (ns);
}

SurfpaintToolsControl::SurfpaintToolsControl() :
  Control(518, QT_TRANSLATE_NOOP("ControlledWindow",
      "SurfpaintToolsControl"))
{
}

SurfpaintToolsControl::SurfpaintToolsControl(
    const SurfpaintToolsControl & c) :
  Control(c)
{
}

SurfpaintToolsControl::~SurfpaintToolsControl()
{
}

void SurfpaintToolsControl::eventAutoSubscription(ActionPool * actionPool)
{
//  mousePressButtonEventSubscribe(Qt::LeftButton, Qt::NoButton,
//        MouseActionLinkOf<SurfpaintToolsAction> (actionPool->action(
//            "SurfpaintToolsAction"),
//            &SurfpaintToolsAction::pressLeftButton));

  mousePressButtonEventSubscribe(Qt::RightButton, Qt::NoButton,
          MouseActionLinkOf<SurfpaintToolsAction> (actionPool->action(
              "SurfpaintToolsAction"),
              &SurfpaintToolsAction::pressRightButton));

  mouseLongEventSubscribe(Qt::LeftButton, Qt::NoButton, MouseActionLinkOf<
      SurfpaintToolsAction> (actionPool->action("SurfpaintToolsAction"),
      &SurfpaintToolsAction::longLeftButtonStart), MouseActionLinkOf<
      SurfpaintToolsAction> (actionPool->action("SurfpaintToolsAction"),
      &SurfpaintToolsAction::longLeftButtonMove),
      MouseActionLinkOf<SurfpaintToolsAction> (actionPool->action(
          "SurfpaintToolsAction"), &SurfpaintToolsAction::longLeftButtonStop), true);

  mouseLongEventSubscribe(Qt::MidButton, Qt::NoButton, MouseActionLinkOf<
      Trackball> (actionPool->action("Trackball"), &Trackball::beginTrackball),
      MouseActionLinkOf<Trackball> (actionPool->action("Trackball"),
          &Trackball::moveTrackball), MouseActionLinkOf<Trackball> (
          actionPool->action("Trackball"), &Trackball::endTrackball), true);
  //
  //  // zoom
  //
  mouseLongEventSubscribe(Qt::MidButton, Qt::ShiftButton, MouseActionLinkOf<
      Zoom3DAction> (actionPool->action("Zoom3DAction"),
      &Zoom3DAction::beginZoom), MouseActionLinkOf<Zoom3DAction> (
      actionPool->action("Zoom3DAction"), &Zoom3DAction::moveZoom),
      MouseActionLinkOf<Zoom3DAction> (actionPool->action("Zoom3DAction"),
          &Zoom3DAction::endZoom), true);

  wheelEventSubscribe(WheelActionLinkOf<Zoom3DAction> (actionPool->action(
      "Zoom3DAction"), &Zoom3DAction::zoomWheel));
  //  //  translation
  //
  mouseLongEventSubscribe(Qt::MidButton, Qt::ControlButton, MouseActionLinkOf<
      Translate3DAction> (actionPool->action("Translate3DAction"),
      &Translate3DAction::beginTranslate),
      MouseActionLinkOf<Translate3DAction> (actionPool->action(
          "Translate3DAction"), &Translate3DAction::moveTranslate),
      MouseActionLinkOf<Translate3DAction> (actionPool->action(
          "Translate3DAction"), &Translate3DAction::endTranslate), true);


    /*Creation of action*/
  myAction = dynamic_cast<SurfpaintToolsAction *> (actionPool->action(
      "SurfpaintToolsAction"));
}



void SurfpaintToolsControl::doAlsoOnSelect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());

    if (w3)
      {

      if (!w3->surfpaintIsVisible())
      {
        SurfpaintTools::instance()->addToolBarInfosTexture(w3);
        SurfpaintTools::instance()->addToolBarControls(w3);
        SurfpaintTools::instance()->initSurfPaintModule(w3);
        w3->setVisibleSurfpaint(true);
      }

//      GLWidgetManager* w = dynamic_cast<GLWidgetManager *> (w3->view());
//      if (w)
//        w->copyBackBuffer2Texture();
    }
  }
}

void SurfpaintToolsControl::doAlsoOnDeselect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
    if (w3)
      {
      SurfpaintTools::instance()->removeToolBarInfosTexture(w3);
      SurfpaintTools::instance()->removeToolBarControls(w3);

      if (w3->surfpaintIsVisible())
        w3->setVisibleSurfpaint(false);
      }
  }

}
