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

//#include <anatomist/module/surfpainttools.h>
//#include "../../surfpaint/action/surfpaintaction.h"

#include <anatomist/surface/texsurface.h>
#include <anatomist/surface/texture.h>
#include <anatomist/surface/surface.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/glcomponent.h>

#include <anatomist/constrainteditor/wConstraintEditor.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/colorstyle.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <aims/qtcompat/qlistbox.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qvbox.h>
#include <qlineedit.h>

#include <anatomist/object/Object.h>
#include <qlabel.h>

#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/fusion/fusionChooser.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cCloseWindow.h>
#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/commands/cReloadObject.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cRemoveObject.h>
#include <anatomist/commands/cGroupObjects.h>
#include <anatomist/commands/cFusionObjects.h>
#include <anatomist/color/objectPalette.h>

#include <aims/io/finder.h>
#include <aims/io/reader.h>
#include <aims/data/header.h>
#include <aims/io/finderFormats.h>
#include <aims/data/pheader.h>

#include <anatomist/surface/texture.h>
#include <aims/mesh/texture.h>
#include <aims/utility/converter_texture.h>

using namespace anatomist;
using namespace aims;
using namespace std;
using namespace carto;

struct ConstraintEditorWindow::Private
{
  Private( const set<AObject *>& objects );
  set<AObject *> objects;
  QListBox  *listobjet;
  QComboBox *latlon;

  QLineEdit  *newTextureName;

  bool textureExist;
  bool meshExist;
  int nnodesMesh;
  int nnodesTexture;
  AObject *meshSelect;
  AObject *texSelect;
};

ConstraintEditorWindow::Private::Private( const set<AObject *> &obj )
  : listobjet( 0 )
{
  set<AObject *>::const_iterator i, e = obj.end();
  for( i=obj.begin(); i!=e; ++i )
    objects.insert( *i );

  textureExist = false;
  meshExist = false;
}

ConstraintEditorWindow::ConstraintEditorWindow( const set<AObject*> &objects,
						  const char *name, Qt::WFlags f ) : QDialog( 0, name, true, f ), d( new Private( objects ) )
{
  drawContents( name );
}

ConstraintEditorWindow::~ConstraintEditorWindow()
{
}

void ConstraintEditorWindow::drawContents( const char *name )
{
  setCaption( name );
  this->setFixedWidth(300);

  QVBoxLayout *mainlay = new QVBoxLayout( this, 5, 5 );

  //d->latlon->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

//  if( !d->objects.empty())
//  {
//    d->listobjet = new QListBox( this );
//    d->listobjet->setFixedWidth(250);
//  }

  set<AObject *>::const_iterator i, e = d->objects.end();

  for( i=d->objects.begin(); i!=e; ++i )
  {
    cout << (*i)->fileName() << "\n" << (*i)->type() << "\n";

    //on cherche une surface parmi les fichiers sélectionnés
    aims::Finder f;
    if( f.check(  (*i)->fileName() ))
        cout << "objtype = " << f.objectType() << "\ndatatype = " << f.dataType() << endl;
    else
      cout << "file not found\n";

    int nnodes;
    const PythonHeader  *hdr = dynamic_cast<const PythonHeader *>( f.header() );
    if( !hdr )
      exit( 1 );

    if( !hdr->getProperty( "vertex_number", nnodes ) )
      {
        cerr << "Couldn't determine mesh nodes number (strange !)\n";
        exit( 1 );
      }

    if (f.objectType() == "Mesh")
      {
      d->nnodesMesh = nnodes;
      d->meshSelect = (*i);
      d->meshExist = true;
      }

    if (f.objectType() == "Texture" && f.dataType() == "FLOAT")
    {
      d->nnodesTexture = nnodes;
      d->textureExist = true;
      d->texSelect = (*i);
    }

    cout << "vertex_number " << nnodes << endl;
  }

  QHBox *hbm = new QHBox();
  QLabel  *meshLabel = new QLabel( "Mesh : ",hbm);
  QLabel  *meshName;

  if (d->meshExist)
    meshName = new QLabel( d->meshSelect->name().c_str(),hbm);
  else
    meshName = new QLabel( "no mesh selected",hbm);
  meshName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QHBox *hbt = new QHBox();
  QLabel  *textureLabel = new QLabel( "Texture : ",hbt);

  if ((d->textureExist && (d->nnodesMesh==d->nnodesTexture)) )
    d->newTextureName = new QLineEdit(  d->texSelect->name().c_str(),hbt);
  else
    d->newTextureName = new QLineEdit( "TexConstraint",hbt);;

  d->newTextureName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  //->setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Maximum)

//  if( d->listobjet )
//  {
//    set<AObject *>::const_iterator i, e = d->objects.end();
//    int       x = 0;
//    for( i=d->objects.begin(); i!=e; ++i, ++x )
//      {
//      //cout << (*i)->name() << " ";
//      d->listobjet->insertItem(  (*i)->name().c_str());
//        //d->objectsmap[ d->order->qListBox()->item( x ) ] = *i;
//      }
//  }

  QHBox *hblatlon = new QHBox();
  new QLabel( "Type : ",hblatlon);
  d->latlon = new QComboBox( hblatlon );
//  d->latlon->setFixedHeight(30);
//  d->latlon->setFixedWidth(50);
  d->latlon->insertItem("lat");
  d->latlon->insertItem("lon");
  d->latlon->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QHBox   *butts = new QHBox( this );
  butts->setSpacing( 10 );
  QPushButton *ok = new QPushButton( tr( "OK" ), butts );
  QPushButton *cancel = new QPushButton( tr( "Cancel" ), butts );

  mainlay->addWidget(hbm);
  mainlay->addWidget(hbt);
  mainlay->addWidget(hblatlon);

//  if( d->listobjet )
//    mainlay->addWidget( d->listobjet );

  mainlay->addWidget( butts );
  ok->setDefault( true );

  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void ConstraintEditorWindow::accept()
{
  FusionFactory *ff = FusionFactory::factory();
  FusionMethod  *fm;

  vector<AObject *> vObjSelect;
  vObjSelect.reserve(2);

  if (d->meshExist)
    vObjSelect.push_back(d->meshSelect);

  //on crée un texture FLOAT (0 - 360 pour lon) ou (0- 180 pour lat)
  //si il y a un maillage
  //s'il n'y a pas de texture sélectionnée ou que les dimensions de la texture et du mesh ne correspondent pas

  if (d->meshExist && (!d->textureExist || (d->nnodesMesh!=d->nnodesTexture)) )
  {
    TimeTexture<float> texLatLon;
    texLatLon = TimeTexture<float>(1, d->nnodesMesh);

    ATexture  *aTex;
    aTex = new ATexture;
    rc_ptr<Texture1d> tex(new Texture1d);
    Converter<TimeTexture<float> , Texture1d> c;
    c.convert(texLatLon, *tex);
    aTex->setTexture(tex);

    // aTex->setTexExtrema(0,360);

    string  constraintLabel = "TexConstraint" + string(d->latlon->currentText());

    aTex->setName( theAnatomist->makeObjectName( string(d->newTextureName->text()) ) );
    aTex->attributed()->setProperty( "object_type", string( "Texture" )  );
    aTex->attributed()->setProperty( "data_type", DataTypeCode<float>::name() );
    aTex->attributed()->setProperty( "nb_t_pos",1);
    aTex->attributed()->setProperty( "ascii",  0);
    aTex->attributed()->setProperty( "byte_swapping",  0);
    aTex->attributed()->setProperty( "vertex_number",  d->nnodesMesh);
    theAnatomist->registerObject( aTex );

    aTex->getOrCreatePalette();
    AObjectPalette *pal = aTex->palette();
    pal->setMin1( 0 );
    pal->setMax1( 360. );
    aTex->setPalette( *pal );

    vObjSelect.push_back(aTex);
  }

  if ((d->textureExist && (d->nnodesMesh==d->nnodesTexture)) )
    vObjSelect.push_back(d->texSelect);

  if (d->meshExist)
  {
    fm = ff->method( "FusionTexSurfMethod" );
    AObject *tso = fm->fusion( vObjSelect );

    tso->setName( theAnatomist->makeObjectName( "FusionConstraint" ) );
    theAnatomist->registerObject( tso );

    AWindow *w;
    w = AWindowFactory::createWindow("3D") ;
    w->registerObject(tso);

    AWindow3D *w3 = dynamic_cast<AWindow3D *> (w);

    w3->setActiveConstraintEditor(true);


    ATexSurface *go = dynamic_cast<ATexSurface *> (tso);
    AObject *surf = go->surface();
    AObject *tex = go->texture();
    ATexture *at = dynamic_cast<ATexture *> (tex);
    ATriangulated *as = dynamic_cast<ATriangulated *> (surf);

    cout << surf << " " << tex << " " << at << " " << as << " " <<endl;
    cout << surf->name() << " " << tex->name() << " " << at->name() << " " << as->name() << " " <<endl;

    //w3->setVisibleSurfpaint(true);

//    w3->view()->controlSwitch()->notifyAvailableControlChange();
//    w3->view()->controlSwitch()->notifyActivableControlChange();
//
//    w3->view()->controlSwitch()->setActiveControl("SurfpaintToolsControl");
//    w3->view()->controlSwitch()->notifyActiveControlChange();
//    w3->view()->controlSwitch()->notifyActionChange();

  }

  QDialog::accept();
}

void ConstraintEditorWindow::update( const Observable*, void* arg )
{
  if (arg == 0)
    {
      cout << "called obsolete ConstraintEditorWindow::update( obs, NULL )\n";
      delete this;
    }
}


void ConstraintEditorWindow::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  delete this;
}

