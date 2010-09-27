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

//#include <anatomist/application/hierarchyeditor.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <anatomist/action/surfpaintaction.h>
#include <anatomist/window/glwidgetmanager.h>
//#include <anatomist/graph/Graph.h>
//#include <anatomist/hierarchy/hierarchy.h>
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
SurfpaintColorPickerControl::creator()
{
  //cout<<"creator Control"<<endl;
  SurfpaintColorPickerControl * ns = new SurfpaintColorPickerControl();
  return (ns);
}

SurfpaintColorPickerControl::SurfpaintColorPickerControl() :
  Control(555, QT_TRANSLATE_NOOP("ControlledWindow",
      "SurfpaintColorPickerControl"))
{
  //cout<<"Constructeur SurfpaintColorPickerControl"<<endl;
}

SurfpaintColorPickerControl::SurfpaintColorPickerControl(
    const SurfpaintColorPickerControl & c) :
  Control(c)
{
  //cout<<"Constructeur COPIE SurfpaintColorPickerControl"<<endl;
}

SurfpaintColorPickerControl::~SurfpaintColorPickerControl()
{
  //cout<<"Destructor SurfpaintColorPickerControl"<<endl;
}

void SurfpaintColorPickerControl::eventAutoSubscription(ActionPool * actionPool)
{
  mousePressButtonEventSubscribe(Qt::LeftButton, Qt::NoButton,
      MouseActionLinkOf<SurfpaintColorPickerAction> (actionPool->action(
          "SurfpaintColorPickerAction"),
          &SurfpaintColorPickerAction::colorpicker));

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
  myAction = dynamic_cast<SurfpaintColorPickerAction *> (actionPool->action(
      "SurfpaintColorPickerAction"));
}

void SurfpaintColorPickerControl::doAlsoOnSelect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
    if (w3)
    {
      w3->showPaintingToolbox();
      GLWidgetManager* w = dynamic_cast<GLWidgetManager *> (w3->view());
      if (w)
        w->copyBackBuffer2Texture();
    }
  }
}

void SurfpaintColorPickerControl::doAlsoOnDeselect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
  }

}

Control *
SurfpaintBrushControl::creator()
{
  //cout<<"creator Control"<<endl;
  SurfpaintBrushControl * ns = new SurfpaintBrushControl();
  return (ns);
}

SurfpaintBrushControl::SurfpaintBrushControl() :
  Control(556, QT_TRANSLATE_NOOP("ControlledWindow", "SurfpaintBrushControl"))
{
  //cout<<"Constructeur SurfpaintBrushControl"<<endl;
}

SurfpaintBrushControl::SurfpaintBrushControl(const SurfpaintBrushControl & c) :
  Control(c)
{
  //cout<<"Constructeur COPIE SurfpaintBrushControl"<<endl;
}

SurfpaintBrushControl::~SurfpaintBrushControl()
{
  //cout<<"Destructor SurfpaintBrushControl"<<endl;
}

void SurfpaintBrushControl::eventAutoSubscription(ActionPool * actionPool)
{
  //  cout<<"EVENT SUSCRIPTION"<<endl;

  mousePressButtonEventSubscribe(Qt::RightButton, Qt::NoButton,
      MouseActionLinkOf<MenuAction> (actionPool->action("MenuAction"),
          &MenuAction::execMenu));

  mouseLongEventSubscribe(Qt::LeftButton, Qt::NoButton, MouseActionLinkOf<
      SurfpaintBrushAction> (actionPool->action("SurfpaintBrushAction"),
      &SurfpaintBrushAction::brushStart), MouseActionLinkOf<
      SurfpaintBrushAction> (actionPool->action("SurfpaintBrushAction"),
      &SurfpaintBrushAction::brushMove),
      MouseActionLinkOf<SurfpaintBrushAction> (actionPool->action(
          "SurfpaintBrushAction"), &SurfpaintBrushAction::brushStop), true);
  //  // rotation
  //
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
  myAction = dynamic_cast<SurfpaintBrushAction *> (actionPool->action(
      "SurfpaintBrushAction"));
}

void SurfpaintBrushControl::doAlsoOnSelect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
    if (w3)
    {
      w3->showPaintingToolbox();
      GLWidgetManager* w = dynamic_cast<GLWidgetManager *> (w3->view());
      if (w)
        w->copyBackBuffer2Texture();
    }
  }
}

void SurfpaintBrushControl::doAlsoOnDeselect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
  }
}

Control *
SurfpaintEraseControl::creator()
{
  //cout<<"creator Control"<<endl;
  SurfpaintEraseControl * ns = new SurfpaintEraseControl();
  return (ns);
}

SurfpaintEraseControl::SurfpaintEraseControl() :
  Control(557, QT_TRANSLATE_NOOP("ControlledWindow", "SurfpaintEraseControl"))
{
  //cout<<"Constructeur SurfpaintEraseControl"<<endl;
}

SurfpaintEraseControl::SurfpaintEraseControl(const SurfpaintEraseControl & c) :
  Control(c)
{
  //cout<<"Constructeur COPIE SurfpaintEraseControl"<<endl;
}

SurfpaintEraseControl::~SurfpaintEraseControl()
{
  //cout<<"Destructor SurfpaintEraseControl"<<endl;
}

void SurfpaintEraseControl::eventAutoSubscription(ActionPool * actionPool)
{
  //  cout<<"EVENT SUSCRIPTION"<<endl;

  mousePressButtonEventSubscribe(Qt::RightButton, Qt::NoButton,
      MouseActionLinkOf<MenuAction> (actionPool->action("MenuAction"),
          &MenuAction::execMenu));

  //  // rotation
  //
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
  myAction = dynamic_cast<SurfpaintEraseAction *> (actionPool->action(
      "SurfpaintEraseAction"));
}

void SurfpaintEraseControl::doAlsoOnSelect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
    w3->showPaintingToolbox();
  }
}

void SurfpaintEraseControl::doAlsoOnDeselect(ActionPool * /* pool */)
{
}

Control *
SurfpaintShortestPathControl::creator()
{
  //cout<<"creator Control"<<endl;
  SurfpaintShortestPathControl * ns = new SurfpaintShortestPathControl();
  return (ns);
}

SurfpaintShortestPathControl::SurfpaintShortestPathControl() :
  Control(558, QT_TRANSLATE_NOOP("ControlledWindow",
      "SurfpaintShortestPathControl"))
{
  //cout<<"Constructeur SurfpaintShortestPathControl"<<endl;
}

SurfpaintShortestPathControl::SurfpaintShortestPathControl(
    const SurfpaintShortestPathControl & c) :
  Control(c)
{
  //cout<<"Constructeur COPIE SurfpaintShortestPathControl"<<endl;
}

SurfpaintShortestPathControl::~SurfpaintShortestPathControl()
{
  //cout<<"Destructor SurfpaintShortestPathControl"<<endl;
}

void SurfpaintShortestPathControl::eventAutoSubscription(
    ActionPool * actionPool)
{
  //  cout<<"EVENT SUSCRIPTION"<<endl;

  mousePressButtonEventSubscribe(Qt::RightButton, Qt::NoButton,
      MouseActionLinkOf<MenuAction> (actionPool->action("MenuAction"),
          &MenuAction::execMenu));

  //  // rotation
  //
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
  //
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
  myAction = dynamic_cast<SurfpaintShortestPathAction *> (actionPool->action(
      "SurfpaintShortestPathAction"));
}

void SurfpaintShortestPathControl::doAlsoOnSelect(ActionPool * /* pool */)
{
  if (myAction)
  {
    AWindow3D *w3 = dynamic_cast<AWindow3D *> (myAction->view()->window());
    w3->showPaintingToolbox();
  }
}

void SurfpaintShortestPathControl::doAlsoOnDeselect(ActionPool * /* pool */)
{
}
