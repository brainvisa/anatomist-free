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
#include <anatomist/action/surfpaintaction.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <anatomist/object/Object.h>
#include <cartobase/object/object.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/Window.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/controler/view.h>
#include <anatomist/application/Anatomist.h>


using namespace std;
using namespace anatomist;
using namespace aims;


Action*
SurfpaintToolsAction::creator()
{
  return new SurfpaintToolsAction;
}

string SurfpaintToolsAction::name() const
{
  return QT_TRANSLATE_NOOP("ControlSwitch", "SurfpaintToolsAction");
}

SurfpaintToolsAction::SurfpaintToolsAction()
  : texvalue( 100 ), myTools( new SurfpaintTools )
{
  //cout << "SurfpaintToolsAction\n";
}

SurfpaintToolsAction::~SurfpaintToolsAction()
{
  delete myTools;
}

void SurfpaintToolsAction::pressRightButton(int x, int y, int globalX, int globalY)
{
  //cout << "pressRightButton\n" ;

  int activeControl = getTools()->getActiveControl();
  //cout << "active control = " << activeControl <<endl;

  switch (activeControl)
  {
  case 3 :
    shortestpathClose(x,y,globalX,globalY);
    break;
  }
}


void SurfpaintToolsAction::setupTools()
{
  QWidget* pw = dynamic_cast<QWidget *>( view() );
  if( pw )
    myTools->setParent( pw );
}


void SurfpaintToolsAction::longLeftButtonStart(int x, int y, int globalX, int globalY)
{
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  int activeControl = getTools()->getActiveControl();

  getTools()->setClosePath(false);

  switch (activeControl)
  {
  case 1 :
    colorpicker(x,y,globalX,globalY);
    break;
  case 2 :
    magicselection(x,y,globalX,globalY);
    break;
  case 3 :
    shortestpathStart( x, y );
    break;
  case 4 :
    brushStart(x,y,globalX,globalY);
    break;
  case 5 :
    eraseStart(x,y,globalX,globalY);
    break;
  case 6 :
    magicbrushStart(x,y,globalX,globalY);
    break;
  case 7 :
    distanceStart(x,y,globalX,globalY);
    break;
  }

}

void SurfpaintToolsAction::longLeftButtonMove(int x, int y, int globalX, int globalY)
{
  //cout << "longLeftButtonMove\n" ;

  int activeControl = getTools()->getActiveControl();

  switch (activeControl)
  {
  case 1 :
    colorpicker(x,y,globalX,globalY);
    break;
  case 4 :
    brushMove(x,y,globalX,globalY);
    break;
  case 5 :
    eraseMove(x,y,globalX,globalY);
    break;
  case 6 :
    magicbrushMove(x,y,globalX,globalY);
    break;
  case 7 :
    distanceMove(x,y,globalX,globalY);
    break;
  }
}

void SurfpaintToolsAction::longLeftButtonStop(int x, int y, int globalX, int globalY)
{
  //cout << "longLeftButtonStop\n" ;

  int activeControl = getTools()->getActiveControl();
  texvalue = getTools()->getTextureValueFloat();

  switch (activeControl)
  {
  case 3 :
    shortestpathStop(x,y,globalX,globalY);
    break;
  case 4 :
    brushStop(x,y,globalX,globalY);
    break;
  case 5 :
    eraseStop(x,y,globalX,globalY);
    break;
  case 6 :
    magicbrushStop(x,y,globalX,globalY);
    break;
  case 7 :
    distanceStop(x,y,globalX,globalY);
    break;
  }
}

void SurfpaintToolsAction::colorpicker(int x, int y, int globalX, int globalY)
{
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);
  if( tval.size() >= 1 )
    texvalue = tval[0];
  else
    return;

  getTools()->setTextureValueFloat(texvalue);
  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);
}

void SurfpaintToolsAction::magicselection(int x, int y,
                                          int globalX, int globalY)
{
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  getTools()->clearRegion();

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  if (indexNearestVertex >= 0)
  {
    getTools()->floodFillStart (indexNearestVertex);
    getTools()->floodFillStop ();
  }
}

void SurfpaintToolsAction::distanceStart(int x, int y, int globalX, int globalY)
{
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  distanceMove(x, y, globalX, globalY);
}

void SurfpaintToolsAction::distanceStop(int x, int y, int globalX, int globalY)
{
  cout << "distanceStop" << endl;
}

void SurfpaintToolsAction::distanceMove(int x, int y, int globalX, int globalY)
{
  //cout << "brushMove" << endl;
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  if( indexNearestVertex >= 0 )
    getTools()->computeDistanceMap(indexNearestVertex);

  win3D->refreshNow();
}

void SurfpaintToolsAction::brushStart(int x, int y, int globalX, int globalY)
{
  //cout << "brushStart" << endl;
  getTools()->newEditOperation();
  brushMove(x, y, globalX, globalY);
}

void SurfpaintToolsAction::brushStop(int x, int y, int globalX, int globalY)
{
  // cout << "brushStop" << endl;
}

void SurfpaintToolsAction::brushMove(int x, int y, int globalX, int globalY)
{
  //cout << "brushMove" << endl;
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  float texvalue = (float)(getTools()->getTextureValueFloat());

  if( indexNearestVertex>=0 )
    getTools()->updateTextureValue( indexNearestVertex, texvalue );

  win3D->refreshNow();
}


void SurfpaintToolsAction::magicbrushStart(int x, int y,
                                           int globalX, int globalY)
{
  //cout << "brushStart" << endl;

  if( !getTools()->magicBrushStarted() )
    getTools()->newEditOperation();
  magicbrushMove(x, y, globalX, globalY);
}

void SurfpaintToolsAction::magicbrushStop(int x, int y,
                                          int globalX, int globalY)
{
  // cout << "magicbrushStop" << endl;

  getTools()->fillHolesOnPath();
}

void SurfpaintToolsAction::magicbrushMove(int x, int y,
                                          int globalX, int globalY)
{
  //cout << "brushMove" << endl;
  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
    tval, textype, positionNearestVertex, &indexNearestVertex);

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  float texvalue = (float)(getTools()->getTextureValueFloat());

  if( indexNearestVertex >= 0 )
    getTools()->updateTextureValue(indexNearestVertex, texvalue);

  win3D->refreshNow();
}

void SurfpaintToolsAction::eraseStart(int x, int y, int globalX, int globalY)
{
  //cout << "eraseStart" << endl;
  getTools()->newEditOperation();
  eraseMove(x, y, 0, 0);
}

void SurfpaintToolsAction::eraseStop(int x, int y, int globalX, int globalY)
{
  //cout << "eraseStop" << endl;
}

void SurfpaintToolsAction::eraseMove(int x, int y, int, int)
{
  //cout << "eraseMove" << endl;

  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  if ( indexNearestVertex >= 0 )
    getTools()->restoreTextureValue(indexNearestVertex);

  win3D->refreshNow();
}

void SurfpaintToolsAction::shortestpathClose(int x, int y,
                                             int globalX, int globalY)
{
  //cout << "shortestpathClose" << endl;

  getTools()->setClosePath(true);
  shortestpathStart( x, y );
}

void SurfpaintToolsAction::shortestpathStart(int x, int y )
{
  //cout << "shortestpathStart" << endl;

  AWindow3D *win3D = dynamic_cast<AWindow3D *> (view()->aWindow());

  AObject *objselect = win3D->objectAtCursorPosition(x, y);
  if( objselect != getTools()->workingObject() )
    return;

  int indexNearestVertex = -1;
  Point3df positionNearestVertex, pos;
  int poly;
  string objtype, textype;
  vector<float> tval;
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      tval, textype, positionNearestVertex, &indexNearestVertex);

  float texvalue = (float)(getTools()->getTextureValueFloat());

  if (!getTools()->pathIsClosed())
  {
    getTools()->setPolygon(poly);
    getTools()->setVertex(indexNearestVertex);
  }

  if (indexNearestVertex>= 0 )
  {
    getTools()->addGeodesicPath( indexNearestVertex, positionNearestVertex );
  }

  win3D->refreshNow();
}


void SurfpaintToolsAction::shortestpathStop(int x, int y, int globalX,
    int globalY)
{
  //cout << "shortestpathStop" << endl;
}


void SurfpaintToolsAction::editValidate()
{
  getTools()->validateEdit();
}


void SurfpaintToolsAction::editCancel()
{
  getTools()->clearAll();
}


void SurfpaintToolsAction::undo()
{
  getTools()->undo();
}


void SurfpaintToolsAction::redo()
{
  getTools()->redo();
}




