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

#include <anatomist/action/surfpaintaction.h>
#include <anatomist/control/surfpaintcontrol.h>
#include <anatomist/object/Object.h>
#include <cartobase/object/object.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/window/Window.h>
#include <anatomist/window3D/trackball.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/controler/view.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/control/graphParams.h>
#include <graph/tree/tree.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#include <anatomist/processor/Processor.h>

#include <cartobase/stream/fileutil.h>

using namespace std;
using namespace anatomist;
using namespace aims;

Action*
SurfpaintColorPickerAction::creator()
{
  return new SurfpaintColorPickerAction;
}

string SurfpaintColorPickerAction::name() const
{
  return QT_TRANSLATE_NOOP("ControlSwitch", "SurfpaintColorPickerAction");
}

SurfpaintColorPickerAction::SurfpaintColorPickerAction()
{
}

SurfpaintColorPickerAction::~SurfpaintColorPickerAction()
{
}

void SurfpaintColorPickerAction::colorpicker(int x, int y, int, int)
{
  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );

  Point3df  pos;
  int poly;
  string objtype;
  float texvalue;
  string textype;
  int indexVertex;
  Point3df positionNearestVertex;
  int indexNearestVertex;

  objselect = w3->objectAtCursorPosition( x, y );

  w3->getInfos3DFromClickPointNew( x, y, pos, &poly , objselect, objtype, &texvalue, textype, positionNearestVertex, &indexNearestVertex);
  w3->setTextureValue(texvalue);
  w3->setPolygon(poly);
  w3->setVertex(indexNearestVertex);
}

/////////////////

Action*
SurfpaintBrushAction::creator()
{
  return new SurfpaintBrushAction;
}

string SurfpaintBrushAction::name() const
{
  return QT_TRANSLATE_NOOP("ControlSwitch", "SurfpaintBrushAction");
}

/* Constr. */

SurfpaintBrushAction::SurfpaintBrushAction()
{
}

SurfpaintBrushAction::~SurfpaintBrushAction()
{
}

void
SurfpaintBrushAction::brushStart( int x, int y, int globalX, int globalY )
{
  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );
  objselect = w3->objectAtCursorPosition( x, y );

  //cout << "brushStart" << endl;
  brushMove(x,y,0,0);
  //hideCursor();
}

void
SurfpaintBrushAction::brushStop( int x, int y, int globalX, int globalY )
{
  //showCursor();
  //cout << "brushStop" << endl;
}

void SurfpaintBrushAction::brushMove(int x, int y, int, int)
{
  AWindow3D *w3 = dynamic_cast<AWindow3D *>( view()->window() );

  //cout << "brushMove" << endl;

  Point3df  pos;
  int poly;
  string objtype;
  float texvalue;
  string textype;
  int indexVertex;
  Point3df positionNearestVertex;
  int indexNearestVertex;

  w3->getInfos3DFromClickPointNew( x, y, pos, &poly , objselect, objtype, &texvalue, textype, positionNearestVertex, &indexNearestVertex);

  texvalue = w3->getTextureValue();
  w3->setPolygon(poly);

  w3->setVertex(indexNearestVertex);
  w3->updateTextureValue( objselect, textype, indexNearestVertex, texvalue);
}
/////////////////

Action*
SurfpaintEraseAction::creator()
{
  return new SurfpaintEraseAction;
}

string SurfpaintEraseAction::name() const
{
  return QT_TRANSLATE_NOOP("ControlSwitch", "SurfpaintEraseAction");
}

/* Constr. */

SurfpaintEraseAction::SurfpaintEraseAction()
{
}

SurfpaintEraseAction::~SurfpaintEraseAction()
{
}
/////////////////

Action*
SurfpaintShortestPathAction::creator()
{
  return new SurfpaintShortestPathAction;
}

string SurfpaintShortestPathAction::name() const
{
  return QT_TRANSLATE_NOOP("ControlSwitch", "SurfpaintShortestPathAction");
}

/* Constr. */

SurfpaintShortestPathAction::SurfpaintShortestPathAction()
{
}

SurfpaintShortestPathAction::~SurfpaintShortestPathAction()
{
}
