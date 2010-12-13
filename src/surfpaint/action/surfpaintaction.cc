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

//#include <cartobase/stream/fileutil.h>
//#include <aims/utility/converter_texture.h>
//#include <anatomist/color/colortraits.h>


using namespace std;
using namespace anatomist;
using namespace aims;

using namespace geodesic;

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
{
  //cout << "SurfpaintToolsAction\n";
}

SurfpaintToolsAction::~SurfpaintToolsAction()
{
}

void SurfpaintToolsAction::pressRightButton(int x, int y, int globalX, int globalY)
{
  //cout << "pressRightButton\n" ;

  activeControl = getTools()->getActiveControl();
  //cout << "active control = " << activeControl <<endl;

  getTools()->setClosePath(true);

  switch (activeControl)
  {
  case 3 :
    shortestpathClose(x,y,globalX,globalY);
    break;
  }
}

void SurfpaintToolsAction::longLeftButtonStart(int x, int y, int globalX, int globalY)
{
  win3D = dynamic_cast<AWindow3D *> (view()->window());

  objselect = win3D->objectAtCursorPosition(x, y);

  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);

  activeControl = getTools()->getActiveControl();
  //cout << "active control = " << activeControl <<endl;

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
    shortestpathStart(x,y,globalX,globalY);
    break;
  case 4 :
    brushStart(x,y,globalX,globalY);
    break;
  case 5 :
    eraseStart(x,y,globalX,globalY);
    break;
  }

}

void SurfpaintToolsAction::longLeftButtonMove(int x, int y, int globalX, int globalY)
{
  //cout << "longLeftButtonMove\n" ;

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
  }
}

void SurfpaintToolsAction::longLeftButtonStop(int x, int y, int globalX, int globalY)
{
  //cout << "longLeftButtonStop\n" ;

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
  }
}

void SurfpaintToolsAction::colorpicker(int x, int y, int globalX, int globalY)
{
  win3D = dynamic_cast<AWindow3D *> (view()->window());

  objselect = win3D->objectAtCursorPosition(x, y);

  //cout << "win = " << win3D << " obj = " << objselect << endl;

  if (objselect)
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);
  else
  {
    texvalue = 0;
    poly = -1;
    indexNearestVertex = -1;
  }

  getTools()->setTextureValueFloat(texvalue);
  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);
}

void SurfpaintToolsAction::magicselection(int x, int y, int globalX, int globalY)
{
  win3D = dynamic_cast<AWindow3D *> (view()->window());

  objselect = win3D->objectAtCursorPosition(x, y);

  if (objselect)
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);
  else
  {
    poly = -1;
    indexNearestVertex = -1;
  }

  getTools()->clearRegion();

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  if (indexNearestVertex >= 0)
  {
    getTools()->floodFillStart (indexNearestVertex);
    getTools()->floodFillStop ();
  }
}

void SurfpaintToolsAction::brushStart(int x, int y, int globalX, int globalY)
{
  //cout << "brushStart" << endl;
  brushMove(x, y, globalX, globalY);
}

void SurfpaintToolsAction::brushStop(int x, int y, int globalX, int globalY)
{
  cout << "brushStop" << endl;

  //getTools()->fillHolesOnPath();
}

void SurfpaintToolsAction::brushMove(int x, int y, int globalX, int globalY)
{
  //cout << "brushMove" << endl;
  win3D = dynamic_cast<AWindow3D *> (view()->window());

  objselect = win3D->objectAtCursorPosition(x, y);
  if (objselect)
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);
  else
  {
    poly = -1;
    indexNearestVertex = -1;
  }

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  texvalue = (float)(getTools()->getTextureValueFloat());

  if (objselect && indexNearestVertex>=0)
    getTools()->updateTextureValue(indexNearestVertex, texvalue);

  win3D->refreshNow();
}

void SurfpaintToolsAction::eraseStart(int x, int y, int globalX, int globalY)
{
  //cout << "eraseStart" << endl;
  eraseMove(x, y, 0, 0);
}

void SurfpaintToolsAction::eraseStop(int x, int y, int globalX, int globalY)
{
  //cout << "eraseStop" << endl;
}

void SurfpaintToolsAction::eraseMove(int x, int y, int, int)
{
  //cout << "eraseMove" << endl;

  win3D = dynamic_cast<AWindow3D *> (view()->window());

  objselect = win3D->objectAtCursorPosition(x, y);
  if (objselect)
  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);
  else
  {
    poly = -1;
    indexNearestVertex = -1;
  }

  getTools()->setPolygon(poly);
  getTools()->setVertex(indexNearestVertex);

  if (objselect && indexNearestVertex>=0 )
    getTools()->restoreTextureValue(indexNearestVertex);

  win3D->refreshNow();
}

void SurfpaintToolsAction::shortestpathClose(int x, int y, int globalX, int globalY)
{
  //cout << "shortestpathClose" << endl;

  shortestpathStart(x, y, globalX, globalY);
}

void SurfpaintToolsAction::shortestpathStart(int x, int y, int globalX, int globalY)
{
  //cout << "shortestpathStart" << endl;

  win3D->getInfos3DFromClickPoint(x, y, pos, &poly, objselect, objtype,
      &texvalue, textype, positionNearestVertex, &indexNearestVertex);

  //cout << "index " << indexNearestVertex << endl;

  texvalue = (float)(getTools()->getTextureValueFloat());

  if (!getTools()->pathIsClosed())
    {
    getTools()->setPolygon(poly);
    getTools()->setVertex(indexNearestVertex);
    }

  if (indexNearestVertex>= 0)
  {
    getTools()->addGeodesicPath (indexNearestVertex,positionNearestVertex);
  }

  win3D->refreshNow();
}

void SurfpaintToolsAction::shortestpathStop(int x, int y, int globalX,
    int globalY)
{
  //cout << "shortestpathStop" << endl;
}

//void SurfpaintToolsAction::shortestpathMove(int x, int y, int, int)
//{
//  cout << "shortestpathMove" << endl;
//}
