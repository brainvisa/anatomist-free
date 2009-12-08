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

#include <anatomist/object/objectmenu.h>
#include <qapplication.h>
#include <anatomist/object/actions.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/graph/qgraphproperties.h>
#include <graph/tree/tree.h>

using namespace anatomist;
using namespace carto;
using namespace std;


static ObjectMenu* defaultOptionMenu()
{
  ObjectMenu	*om = NULL;

  om = new ObjectMenu();
  vector<string>  vs;
  vs.reserve( 1 );
  vs.push_back(QT_TRANSLATE_NOOP("QSelectMenu", "Default Menu"));
  om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu", "default"));
  vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "File");
  om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu", "Reload"),
                  &ObjectActions::fileReload );
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ),
                  &ObjectActions::saveStatic );
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                  "Rename object"), &ObjectActions::renameObject);
  return om;
}

static ObjectMenu* volumeScalarTraitsOptionMenu()
{
  ObjectMenu	*om = NULL;

  om = new ObjectMenu();
  vector<string>  vs;
  vs.reserve(1);
  vs.push_back(QT_TRANSLATE_NOOP( "QSelectMenu", "File"));
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Reload"),
                  &ObjectActions::fileReload);
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Save"),
                  &ObjectActions::saveStatic);
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                                          "Rename object"),
                  &ObjectActions::renameObject);
  vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "Color");
  om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu", "Palette"),
                  &ObjectActions::colorPalette);
  om->insertItem(vs, QT_TRANSLATE_NOOP("QSelectMenu", "Material"),
                  &ObjectActions::colorMaterial);
  om->insertItem(vs,QT_TRANSLATE_NOOP("QSelectMenu", "Texturing"),
                  &ObjectActions::textureControl);
  vs[0] = QT_TRANSLATE_NOOP("QSelectMenu", "Referential");
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Load"),
                  &ObjectActions::referentialLoad );
  om->insertItem(vs, QT_TRANSLATE_NOOP( "QSelectMenu",
                  "Apply builtin referential (SPM/NIFTI)"),
                  &ObjectActions::setAutomaticReferential);
  return om;
}

static ObjectMenu* volumeVectorTraitsOptionMenu()
{
  ObjectMenu	*om = NULL;

  om = new ObjectMenu();
  vector<string>  vs;
  vs.reserve( 1 );
  vs.push_back( QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Reload" ),
                  &ObjectActions::fileReload );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ),
                  &ObjectActions::saveStatic );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Rename object" ),
                  &ObjectActions::renameObject );
  vs[0] = QT_TRANSLATE_NOOP( "QSelectMenu", "Color" );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Palette" ),
                  &ObjectActions::colorPalette );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Material" ),
                  &ObjectActions::colorMaterial );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Texturing" ),
                  &ObjectActions::textureControl );
  vs[0] = QT_TRANSLATE_NOOP( "QSelectMenu", "Referential" );
  om->insertItem( vs, QT_TRANSLATE_NOOP( "QSelectMenu", "Load" ),
                  &ObjectActions::referentialLoad );
  return om;
}


static ObjectMenu* agraphOptionMenu()
{
  ObjectMenu	*om = NULL;

  Tree	*optionTree, *t, *t2;
  optionTree = new Tree(true, "option tree");

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Display"));
  optionTree->insert(t);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Display nodes"));
  t2->setProperty("callback", &ObjectActions::displayGraphChildren);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Display relations"));
  t2->setProperty("callback", &ObjectActions::displayGraphRelations);
  t->insert(t2);

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "File"));
  optionTree->insert(t);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                "Load all sub-objects"));
  t2->setProperty("callback", &ObjectActions::loadGraphSubObjects);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Save"));
  t2->setProperty("callback", &ObjectActions::saveStatic);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                          "Rename object"));
  t2->setProperty("callback", &ObjectActions::renameObject);
  t->insert(t2);

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Color"));
  optionTree->insert(t);
  t2 = new Tree(true,QT_TRANSLATE_NOOP("QSelectMenu", "Palette"));
  t2->setProperty("callback", &ObjectActions::colorPalette);
  t->insert(t2);
  t2 = new Tree(true, "Material");
  t2->setProperty("callback", &ObjectActions::colorMaterial);
  t->insert(t2);
  t2 = new Tree(true, "Graph display properties");
  t2->setProperty("callback", &QGraphProperties::openProperties);
  t->insert(t2);

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                                  "Referential"));
  optionTree->insert(t);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Load"));
  t2->setProperty("callback", &ObjectActions::referentialLoad);
  t->insert(t2);

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Labeling"));
  optionTree->insert(t);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                "Move automatic labeling ('label') to manual ('name')") );
  t2->setProperty("callback", &ObjectActions::graphLabelToName);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                "Use automatic labeling ('label')") );
  t2->setProperty("callback", &ObjectActions::graphUseLabel);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                "Use manual labeling ('name')") );
  t2->setProperty("callback", &ObjectActions::graphUseName);
  t->insert(t2);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
                "Use default labeling property") );
  t2->setProperty("callback", &ObjectActions::graphUseDefaultLabelProperty);
  t->insert(t2);

  om = new ObjectMenu(*optionTree);
  return om;
}



static ObjectMenu* agraphObjectOptionMenu()
{
  ObjectMenu	*om;

  Tree	*optionTree, *t, *t2;
  optionTree = new Tree( true, "option tree" );
  t = new Tree( true, "Color" );
  optionTree->insert( t );
  t2 = new Tree( true, "Palette" );
  t2->setProperty( "callback", &ObjectActions::colorPalette );
  t->insert( t2 );
  t2 = new Tree( true, "Material" );
  t2->setProperty( "callback", &ObjectActions::colorMaterial );
  t->insert( t2 );

  t = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu",
               "Referential"));
  optionTree->insert(t);
  t2 = new Tree(true, QT_TRANSLATE_NOOP("QSelectMenu", "Load"));
  t2->setProperty("callback", &ObjectActions::referentialLoad);
  t->insert(t2);

  om = new ObjectMenu(*optionTree);
  return om;
}




namespace anatomist
{
  void	initMenuObjects(void)
  {
    rc_ptr<ObjectMenu>	om;

    //Default menu
    om.reset( defaultOptionMenu() );
    AObject::setObjectMenu("__default__", om);

    //Volume
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<int8_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<uint8_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<int16_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<uint16_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<int32_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<uint32_t>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<float>", om);
    om.reset( volumeScalarTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<double>", om);

    om.reset( volumeVectorTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<AimsRGB>", om);
    om.reset( volumeVectorTraitsOptionMenu() );
    AObject::setObjectMenu("VOLUME<AimsRGBA>", om);

    //Graph
    om.reset( agraphOptionMenu() );
    AObject::setObjectMenu("GRAPH", om);
    om.reset( agraphObjectOptionMenu() );
    AObject::setObjectMenu("GRAPHOBJECT", om);
  }
}
