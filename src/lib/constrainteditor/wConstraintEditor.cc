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

#include <anatomist/surface/texture.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/constrainteditor/wConstraintEditor.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/fusion/fusionFactory.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cCloseWindow.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/mobject/MObject.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/wControl.h>

#include <cartobase/stream/fileutil.h>
#include <cartobase/config/paths.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <QToolButton>
#include <Qt>

using namespace anatomist;
using namespace aims;
using namespace std;
using namespace carto;

struct ConstraintEditorWindow::Private
{
  Private();
  QComboBox *latlon;
  QLineEdit  *newTextureName;
  QToolButton *constraintTextureButton;
  QLabel  *constraintTextureLabel;
  QToolButton *constraintListButton;
  QComboBox  *constraintListValues;

  AObject *meshSelect;
  AObject *texSelect;
  AObject *texConstraint;

  vector<string> constraintList;
  int constraintValuesType; //min_max or lat_lon

  size_t nnodes;
};


ConstraintEditorWindow::Private::Private()
: latlon(0), newTextureName(0),
  constraintTextureButton(0), constraintTextureLabel(0),
  constraintListButton(0), constraintListValues(0),
  meshSelect(0), texSelect(0), texConstraint(0),
  constraintList(0), constraintValuesType(0),
  nnodes(0)
{
}

void ConstraintEditorWindow::constraintTexOpen()
{
  QString filt = ControlWindow::tr( "Textures" ) + " ("
    + ObjectReader::supportedFileExtensions( "Texture" ).c_str() + ");;";
  QString capt = "Open Texture" ;

  QString filename;

  filename = QFileDialog::getOpenFileName( 0, capt, QString::null, filt );

  if( d->meshSelect )
  {
    if( !filename.isNull() )
    {
      LoadObjectCommand *cmd = 0 ;
      cmd = new LoadObjectCommand( filename.toStdString() ) ;
      theProcessor->execute( cmd ) ;

      ATexture *atex = 0;
      list<AObject *> objs = cmd->loadedObjects();
      if( !objs.empty() )
        atex = dynamic_cast<ATexture *>( *objs.begin() );

      cout << atex << " " << atex->dimTexture() << "\n";
      if( atex && atex->dimTexture() == 1 )
      {
        ViewState vs;
        size_t nnodest = atex->glTexCoordSize( vs, 0 );

        if (nnodest == d->nnodes)
        {
          list<AObject *> objs = cmd->loadedObjects();
          if( !objs.empty() )
          {
            d->texConstraint = *objs.begin();
            cout << d->texConstraint->name() << endl;
            d->constraintTextureLabel->setText(
              d->texConstraint->name().c_str());
          }
          else
            d->texConstraint = NULL;
        }
        else
        {
          d->texConstraint = NULL;
        }
      }
    }
  }
}

void ConstraintEditorWindow::constraintListOpen()
{
  QString filt = "Text files (*.txt)" ;
  QString capt = "Open constraints file" ;

  QString filename;

  filename = QFileDialog::getOpenFileName( 0, capt, QString::null, filt );

  d->constraintListValues->clear();
  d->constraintList.clear();

  string line;
  ifstream myfile( filename.toStdString().c_str() );

  if (myfile.is_open())
  {
    while (myfile.good())
    {
      getline(myfile, line);
      if (line.length() != 0)
        {
        d->constraintListValues->addItem(line.c_str()) ;
        d->constraintListValues->setEditable( false );
        d->constraintList.push_back(line.c_str());
        }

    }
    myfile.close();
  }

}

void ConstraintEditorWindow::constraintListInit()
{
  char sep = carto::FileUtil::separator();

 string consfile = Paths::findResourceFile( string( "nomenclature" ) + sep
   + "surfaceanalysis" + sep + "constraint_correspondance_2012.txt" );

  cout << "Loading constraints file : " << consfile << endl;

  d->constraintList.clear();

  string line;
  ifstream myfile(consfile.c_str());
  if (myfile.is_open())
  {
    while (myfile.good())
    {
      getline(myfile, line);
      if (line.length() != 0)
        {
        d->constraintListValues->addItem(line.c_str()) ;
        d->constraintListValues->setEditable( false );
        d->constraintList.push_back(line.c_str());
        }

    }
    myfile.close();
  }

  else
    cout << "Unable to open file " << consfile << endl;
}

ConstraintEditorWindow::ConstraintEditorWindow( const set<AObject*> &objects,
                                                const char *name, Qt::WindowFlags f )
  : QDialog( 0, f ), d( new Private )
{
  setAttribute( Qt::WA_DeleteOnClose );
  // using a modal dialog seems to cause the created AWindow (in accept())
  // never get events afterwards.
//   setWindowModality( Qt::ApplicationModal );
  drawContents( name, objects );
}


ConstraintEditorWindow::~ConstraintEditorWindow()
{
}


void ConstraintEditorWindow::drawContents( const char *name,
                                           const set<AObject *> & obj )
{
  setWindowTitle( name );
  this->setFixedWidth(350);

  QVBoxLayout *mainlay = new QVBoxLayout( this );
  mainlay->setMargin( 5 );
  mainlay->setSpacing( 5 );

  /* convert objects list to a real list because we may append sub-objects to
     the list to check
  */
  list<AObject *> objects( obj.begin(), obj.end() );
  list<AObject *>::const_iterator i, e = objects.end();

  d->nnodes = 0;

  for( i=objects.begin(); i!=e; ++i )
  {
    cout << (*i)->fileName() << "\n";

    // mesh+texture type
    if( (*i)->type()== AObject::TEXSURFACE )
    {
      MObject *mo = dynamic_cast<MObject *>( *i );

      if( mo )
      {
        MObject::iterator im, em = mo->end();
        for( im=mo->begin(); im!=em; ++im )
          objects.push_back( *im ); // check sub-objects
      }
    }

    // mesh type
    if( (*i)->type()== AObject::TRIANG )
    {
      ATriangulated *surf = dynamic_cast<ATriangulated *>( *i );

      if( surf && !d->meshSelect )
      {
        rc_ptr<AimsSurfaceTriangle> aimss = surf->surface();
        if( aimss->empty() )
          continue;

        size_t nnodesm = aimss->vertex().size();

        if( d->texSelect && d->nnodes != nnodesm )
        {
          cout << "texture is incompatible\n";
          d->texSelect = 0; // texture is incompatible: don't keep it'
        }

        d->nnodes = nnodesm;
        d->meshSelect = surf;
        continue;
      }
    }

    // texture type
    if( (*i)->type()== AObject::TEXTURE )
    {
      ATexture *atex = dynamic_cast<ATexture *>( *i );

      cout << atex << " " << atex->dimTexture() << "\n";
      if( atex && atex->dimTexture() == 1 )
      {
        ViewState vs;
        size_t nnodest = atex->glTexCoordSize( vs, 0 );
        if( !d->meshSelect || nnodest == d->nnodes )
          {
          d->nnodes = nnodest;
          d->texSelect = (*i);
          }
        continue;
      }
    }

  }

  QWidget *hbm = new QWidget;
  QHBoxLayout *hlay = new QHBoxLayout( hbm );
  hlay->setMargin( 0 );
  QLabel  *meshLabel = new QLabel( tr( "Mesh : " ),hbm);
  hlay->addWidget( meshLabel );
  QLabel  *meshName;

  if (d->meshSelect)
  {
    meshName = new QLabel( d->meshSelect->name().c_str(),hbm);
    hlay->addWidget( meshName );
  }
  else
  {
    meshName = new QLabel( tr( "no mesh selected" ),hbm);
    hlay->addWidget( meshName );
    QPushButton *cancel = new QPushButton( tr( "Cancel" ), this );
    mainlay->addWidget( meshName );
    mainlay->addWidget( cancel );
    cancel->setDefault( true );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    return;
  }
  meshName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                        QSizePolicy::Fixed ) );

  QWidget *hbt = new QWidget;
  hlay = new QHBoxLayout( hbt );
  hlay->setMargin( 0 );
  QLabel  *textureLabel = new QLabel( tr( "Texture : " ),hbt);
  hlay->addWidget( textureLabel );

  if ( d->texSelect )
  {
    d->newTextureName = new QLineEdit(  d->texSelect->name().c_str(),hbt);
    hlay->addWidget( d->newTextureName );
    d->newTextureName->setReadOnly (true);

  }
  else
  {
    d->newTextureName = new QLineEdit( tr( "TexConstraint" ),hbt);
    hlay->addWidget( d->newTextureName );
  }

  d->newTextureName->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                                 QSizePolicy::Fixed ) );

  QWidget *hblatlon = new QWidget;
  hlay = new QHBoxLayout( hblatlon );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "Type : " ),hblatlon) );
  d->latlon = new QComboBox( hblatlon );
  hlay->addWidget( d->latlon );
  //d->latlon->insertItem("lat");
  //d->latlon->insertItem("lon");
  d->latlon->addItem( tr( "predefined constraints" ) );
  d->latlon->addItem( tr( "user defined" ));
  d->latlon->setCurrentIndex( 1 );

  d->latlon->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                                         QSizePolicy::Fixed ) );

  QWidget *butts = new QWidget( this );
  hlay = new QHBoxLayout( butts );
  hlay->setMargin( 0 );
  hlay->setSpacing( 10 );
  QPushButton *ok = new QPushButton( tr( "OK" ), butts );
  hlay->addWidget( ok );
  QPushButton *cancel = new QPushButton( tr( "Cancel" ), butts );
  hlay->addWidget( cancel );

  string iconname;

  QWidget *hbtc = new QWidget;
  hlay = new QHBoxLayout( hbtc );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "weight map : " ),hbtc) );

  iconname = Settings::findResourceFile( "icons/meshPaint/constraint_map.png" );
  d->constraintTextureButton = new QToolButton(hbtc);
  hlay->addWidget( d->constraintTextureButton );
  d->constraintTextureButton->setIcon(QIcon(iconname.c_str()));
  d->constraintTextureButton->setToolTip( tr("Open map of constrained path"));
  d->constraintTextureButton->setIconSize(QSize(20, 20));
  connect(d->constraintTextureButton, SIGNAL(clicked()),
          this, SLOT(constraintTexOpen()));

  d->constraintTextureLabel = new QLabel( tr( "curvature (default)" ),hbtc);
  hlay->addWidget( d->constraintTextureLabel );
  d->constraintTextureLabel->setFixedWidth(180);

  QWidget *hbcv = new QWidget;
  hlay = new QHBoxLayout( hbcv );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "constraint list : " ),hbcv) );

  iconname = Settings::findResourceFile( "icons/meshPaint/list_constraint.png" );
  d->constraintListButton = new QToolButton(hbcv);
  hlay->addWidget( d->constraintListButton );
  d->constraintListButton->setIcon(QIcon(iconname.c_str()));
  d->constraintListButton->setToolTip( tr("Open list of constrained value") );
  d->constraintListButton->setIconSize(QSize(20, 20));

  d->constraintListValues = new QComboBox(hbcv );
  hlay->addWidget( d->constraintListValues );
  d->constraintListValues->setFixedWidth(180);
  constraintListInit();


  connect(d->constraintListButton, SIGNAL(clicked()),
          this, SLOT(constraintListOpen()));

  mainlay->addWidget(hbm);
  mainlay->addWidget(hbt);
  mainlay->addWidget(hblatlon);
  mainlay->addWidget(hbcv);
  mainlay->addWidget(hbtc);
  mainlay->addWidget( butts );


  ok->setDefault( true );

  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

void ConstraintEditorWindow::accept()
{
  float min,max;

  d->constraintValuesType = d->latlon->currentIndex();

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

    string  texName = d->newTextureName->text().toStdString();

    if ( d->constraintValuesType == 0) {min = 0.0; max = 0.0;}
    if ( d->constraintValuesType == 1) {min = 0.0; max = 1.0;}

    aTex->setName( theAnatomist->makeObjectName(texName));

    aTex->attributed()->setProperty( "object_type", string( "Texture" )  );
    aTex->attributed()->setProperty( "data_type", DataTypeCode<float>::name() );
    aTex->attributed()->setProperty( "nb_t_pos",1);
    aTex->attributed()->setProperty( "ascii",  0);
    aTex->attributed()->setProperty( "byte_swapping",  0);
    aTex->attributed()->setProperty( "vertex_number",  nnodes );
    theAnatomist->registerObject( aTex );

    aTex->getOrCreatePalette();
    AObjectPalette *pal = aTex->palette();
    pal->setMin1( min );
    pal->setMax1( max );
    aTex->setPalette( *pal );

    d->texSelect = aTex;
  }

  if ( d->texSelect )
  {
    if( d->latlon->currentIndex() == 1 )
      d->texSelect->createDefaultPalette( "Blue-Red-fusion" );
    else
      d->texSelect->createDefaultPalette( "Graph-Label" );

    GLComponent *glc = d->texSelect->glAPI();

    int tn = 0; // 1st texture
    GLComponent::TexExtrema & te = glc->glTexExtrema(tn);
    int tx = 0; // 1st tex coord
    float scl = (te.maxquant[tx] - te.minquant[tx]);

    AObjectPalette *pal = d->texSelect->palette();

    if ( d->constraintValuesType == 0) {min = 0.0; max = 0.0;}
    if ( d->constraintValuesType == 1)
    {
      min = te.minquant[tx];
      max = te.maxquant[tx];
    }

//    pal->setMin1( 0 );
//    if (scl != 0)
//      pal->setMax1( (float)(360./scl) );
//    else
//      pal->setMax1( 360 );

    cout << "min/max " << min << " " << max << endl;

    pal->setMin1( min );
    pal->setMax1( max );


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
//     w3->setWindowModality( Qt::NonModal );
    w3->setActiveConstraintEditor(true);
    w3->loadConstraintData( d->constraintList, d->constraintValuesType, d->texConstraint );

    map< unsigned, set< AObject *> > sel = SelectFactory::factory()->selected ();
    map< unsigned, set< AObject *> >::iterator iter( sel.begin( ) ),
      last( sel.end( ) ) ;

    while( iter != last ){
      SelectFactory::factory()->unselectAll( iter->first ) ;
      ++iter ;
    }
    /* notifyObservers + processEvents (maybe not both?) seem needed to update
       the window state otherwise nothing can be drawn. */
    w3->notifyObservers( this );
    qApp->processEvents();
    set<AWindow *> sw;
    sw.insert( w );
    SetControlCommand *c = new SetControlCommand( sw, "SurfpaintToolsControl" );
    theProcessor->execute( c );

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

