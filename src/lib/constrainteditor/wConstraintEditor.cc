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

#include <anatomist/selection/selectFactory.h>
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
  Private();
  QListBox  *listobjet;
  QComboBox *latlon;

  QLineEdit  *newTextureName;

  AObject *meshSelect;
  AObject *texSelect;
};


ConstraintEditorWindow::Private::Private()
  : listobjet( 0 ), meshSelect( 0 ), texSelect( 0 )
{
}


ConstraintEditorWindow::ConstraintEditorWindow( const set<AObject*> &objects,
                                                const char *name, Qt::WFlags f ) : QDialog( 0, name, true, f ), d( new Private )
{
  drawContents( name, objects );
}


ConstraintEditorWindow::~ConstraintEditorWindow()
{
}


void ConstraintEditorWindow::drawContents( const char *name,
                                           const set<AObject *> & obj )
{
  setCaption( name );
  this->setFixedWidth(300);

  QVBoxLayout *mainlay = new QVBoxLayout( this, 5, 5 );

  /* convert objects list to a real list because we may append sub-objects to
     the list to check
  */
  list<AObject *> objects( obj.begin(), obj.end() );
  list<AObject *>::const_iterator i, e = objects.end();
  size_t nnodes = 0;

  for( i=objects.begin(); i!=e; ++i )
  {
    cout << (*i)->fileName() << "\n" << (*i)->type() << "\n";

    ATriangulated *surf = dynamic_cast<ATriangulated *>( *i );
    if( surf && !d->meshSelect )
    {
      rc_ptr<AimsSurfaceTriangle> aimss = surf->surface();
      if( aimss->empty() )
        continue;

      size_t nnodesm = aimss->vertex().size();

      if( d->texSelect && nnodes != nnodesm )
        {
        cout << "texture is incompatible\n";
        d->texSelect = 0; // texture is incompatible: don't keep it'
        }

      nnodes = nnodesm;
      d->meshSelect = surf;
      continue;
    }

    ATexture *atex = dynamic_cast<ATexture *>( *i );

    cout << atex << " " << atex->dimTexture() << "\n";
    if( atex && atex->dimTexture() == 1 )
    {
      ViewState vs;
      size_t nnodest = atex->glTexCoordSize( vs, 0 );
      if( !d->meshSelect || nnodest == nnodes )
        {
        nnodes = nnodest;
        d->texSelect = (*i);
        }
      continue;
    }

    MObject *mo = dynamic_cast<MObject *>( *i );
    if( mo )
    {
      MObject::iterator im, em = mo->end();
      for( im=mo->begin(); im!=em; ++im )
        objects.push_back( *im ); // check sub-objects
    }

  }

  QHBox *hbm = new QHBox();
  QLabel  *meshLabel = new QLabel( "Mesh : ",hbm);
  QLabel  *meshName;

  if (d->meshSelect)
    meshName = new QLabel( d->meshSelect->name().c_str(),hbm);
  else
  {
    meshName = new QLabel( "no mesh selected",hbm);
    QPushButton *cancel = new QPushButton( tr( "Cancel" ), this );
    mainlay->addWidget( meshName );
    mainlay->addWidget( cancel );
    cancel->setDefault( true );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    return;
  }
  meshName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QHBox *hbt = new QHBox();
  QLabel  *textureLabel = new QLabel( "Texture : ",hbt);

  if ( d->texSelect )
    d->newTextureName = new QLineEdit(  d->texSelect->name().c_str(),hbt);
  else
    d->newTextureName = new QLineEdit( "TexConstraint",hbt);;

  d->newTextureName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

  QHBox *hblatlon = new QHBox();
  new QLabel( "Type : ",hblatlon);
  d->latlon = new QComboBox( hblatlon );
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

  if (d->meshSelect)
    vObjSelect.push_back(d->meshSelect);

  //on crée un texture FLOAT (0 - 360 pour lon) ou (0- 180 pour lat)
  //si il y a un maillage
  //s'il n'y a pas de texture sélectionnée ou que les dimensions de la texture et du mesh ne correspondent pas

  if (d->meshSelect && !d->texSelect )
  {
    ATexture  *aTex;
    aTex = new ATexture;
    ViewState vs;
    size_t nnodes = d->meshSelect->glAPI()->glNumVertex( vs );
    rc_ptr<Texture1d> tex( new Texture1d(1, nnodes) );
    aTex->setTexture(tex);

    string  constraintLabel = "TexConstraint" + string(d->latlon->currentText());

    aTex->setName( theAnatomist->makeObjectName( string(d->newTextureName->text()) ) );
    aTex->attributed()->setProperty( "object_type", string( "Texture" )  );
    aTex->attributed()->setProperty( "data_type", DataTypeCode<float>::name() );
    aTex->attributed()->setProperty( "nb_t_pos",1);
    aTex->attributed()->setProperty( "ascii",  0);
    aTex->attributed()->setProperty( "byte_swapping",  0);
    aTex->attributed()->setProperty( "vertex_number",  nnodes );
    theAnatomist->registerObject( aTex );

    //aTex->createDefaultPalette( "Blue-Red-fusion" );

    aTex->getOrCreatePalette();
    AObjectPalette *pal = aTex->palette();
    pal->setMin1( 0 );
    pal->setMax1( 360. );
    aTex->setPalette( *pal );

    d->texSelect = aTex;
  }

  if ( d->texSelect )
    {
    d->texSelect->createDefaultPalette( "Blue-Red-fusion" );
    //d->texSelect->getOrCreatePalette();

    GLComponent *glc = d->texSelect->glAPI();

    int tn = 0; // 1st texture
    GLComponent::TexExtrema & te = glc->glTexExtrema(tn);
    int tx = 0; // 1st tex coord
    float scl = (te.maxquant[tx] - te.minquant[tx]);

    AObjectPalette *pal = d->texSelect->palette();
    pal->setMin1( 0 );
    if (scl != 0)
      pal->setMax1( (float)(360./scl) );
    else
      pal->setMax1( 360 );

    d->texSelect->setPalette( *pal );

    vObjSelect.push_back(d->texSelect);
    }


  if (d->meshSelect)
  {
    fm = ff->method( "FusionTexSurfMethod" );
    AObject *tso = fm->fusion( vObjSelect );

    tso->setName( theAnatomist->makeObjectName( "FusionConstraint" ) );
    theAnatomist->registerObject( tso );

    AWindow *w;
    w = AWindowFactory::createWindow("3D") ;
    w->registerObject(tso);

    AWindow3D *w3 = static_cast<AWindow3D *> (w);
    w3->setActiveConstraintEditor(true);

    map< unsigned, set< AObject *> > sel = SelectFactory::factory()->selected ();
    map< unsigned, set< AObject *> >::iterator iter( sel.begin( ) ),
      last( sel.end( ) ) ;

    while( iter != last ){
      SelectFactory::factory()->unselectAll( iter->first ) ;
      ++iter ;
    }

//    unsigned id_insert = 0;
//    set<AObject *> vObjSelected;
//    vObjSelected.insert(tso);
//    SelectFactory::factory()->select( id_insert, vObjSelected ) ;
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

