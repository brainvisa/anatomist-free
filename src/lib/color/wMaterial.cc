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


#include <anatomist/color/wMaterial.h>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabbar.h>
#include <qradiobutton.h>
#include <qpixmap.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cSetMaterial.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>

using namespace anatomist;
using namespace std;


struct MaterialWindow::Private
{
  Private( const set<AObject *> & o )
    : tab( 0 ), shinlabel( 0 ), responsive( true ), operating( false ), 
      initial( o ), modified( false ) 
  {}
  unsigned		tab;
  vector<QWidget *>	tabs;
  map<int, unsigned>	tabnum;
  QLabel		*shinlabel;
  QSlider		*shinsl;
  bool			responsive;
  QLabel		*labels[5][4];
  QANumSlider		*sliders[5][4];
  bool			operating;
  set<AObject *>	initial;
  ObjectParamSelect	*objsel;
  bool			modified;
};


QANumSlider::QANumSlider( int num1, int num2, int minValue, int maxValue, 
                          int pageStep, int value, Qt::Orientation ori, 
                          QWidget * parent, const char * name )
  : QSlider( parent ), 
  _num1( num1 ), _num2( num2 )
{
  setObjectName( name );
  setOrientation( ori );
  setRange( minValue, maxValue );
  setPageStep( pageStep );
  setValue( value );
  connect( this, SIGNAL( valueChanged( int ) ), this, 
           SLOT( transformChange( int ) ) );
}


void QANumSlider::transformChange( int value )
{
  emit valueChanged( _num1, _num2, value );
}


//	Real MaterialWindow class


MaterialWindow::MaterialWindow( const set<AObject *> &objL, QWidget* parent, 
				const char *name, Qt::WindowFlags f ) 
  : QWidget( parent, f ), _parents( objL ), 
    _privdata( new Private( objL ) )
{
  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);
  if( _parents.size() > 0 )
    _material = (*_parents.begin())->GetMaterial();

  for (set<AObject *>::iterator it=_parents.begin();
       it!=_parents.end();++it)
    (*it)->addObserver( (Observer*)this );

  setWindowTitle( name );
  if( windowFlags() & Qt::Window )
  {
    QPixmap anaicon( Settings::findResourceFile( "icons/icon.xpm"
      ).c_str() );
    if( !anaicon.isNull() )
      setWindowIcon( anaicon );
  }

  drawContents();
}


MaterialWindow::~MaterialWindow()
{
  /* sending SetMaterial command only when window is closed, to avoid sending 
     thousands of commands while tuning the colors */
  runCommand();

  cleanupObserver();

  delete _privdata;
}


void MaterialWindow::update( const Observable*, void* arg )
{
  if( arg != this && !_privdata->operating )
    updateInterface();
}


void MaterialWindow::unregisterObservable( Observable* obs )
{
  // cout << "MaterialWindow::unregisterObservable\n";
  runCommand();
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  _parents.erase( o );
  _privdata->objsel->updateLabel( _parents );
  updateInterface();
}


void MaterialWindow::updateObjects()
{
  set<AObject *>::const_iterator io, fo=objects().end();

  for( io=objects().begin(); io!=fo; ++io )
    {
      (*io)->SetMaterial( getMaterial() );
      (*io)->notifyObservers( this );
      (*io)->clearHasChangedFlags();
    }
}


void MaterialWindow::drawContents()
{
  QVBoxLayout	*mainlay = new QVBoxLayout( this );
  mainlay->setMargin( 10 );
  mainlay->setSpacing( 10 );

  ObjectParamSelect	*sel = new ObjectParamSelect( _parents, this );
  _privdata->objsel = sel;
  // filter ?

  QTabBar	*tbar = new QTabBar( this );

  _privdata->tabnum[ tbar->addTab( tr( "Fast color selection" ) ) ] = 0;
  _privdata->tabnum[ tbar->addTab( tr( "Advanced settings" ) )    ] = 1;

  // -- fast selection

  QWidget *bpan = new QWidget( this );
  QVBoxLayout *vlay = new QVBoxLayout( bpan );
  _privdata->tabs.push_back( bpan );
  vlay->addWidget( buildRgbaBox( bpan, 4 ) ); 	// group 4: diffuse
  vlay->addStretch( 1 );

  // -- advanced

  QWidget *epan = new QWidget( this );
  QGridLayout *glay = new QGridLayout( epan );
  epan->hide();
  glay->setSpacing( 5 );
  glay->setMargin( 0 );
  _privdata->tabs.push_back( epan );

  QGroupBox	*ebox = new QGroupBox( tr( "Ambient :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 0, 0 );
  vlay->addWidget( buildRgbaBox( ebox, 0 ) );	// group 0: ambient
  vlay->addStretch( 1 );

  ebox = new QGroupBox( tr( "Diffuse :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 0, 1 );
  vlay->addWidget( buildRgbaBox( ebox, 1 ) );	// group 1: diffuse
  vlay->addStretch( 1 );

  ebox = new QGroupBox( tr( "Emission :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 1, 0 );
  vlay->addWidget( buildRgbaBox( ebox, 2 ) );	// group 2: emission
  vlay->addStretch( 1 );

  ebox = new QGroupBox( tr( "Specular :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 1, 1 );
  vlay->addWidget( buildRgbaBox( ebox, 3 ) );	// group 3: specular
  vlay->addStretch( 1 );

  ebox = new QGroupBox( tr( "Shininess :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 2, 0 );
  QWidget *grd = new QWidget( ebox );
  vlay->addWidget( grd );
  vlay->addStretch( 1 );
  QHBoxLayout *hlay = new QHBoxLayout( grd );
  hlay->setSpacing( 10 );
  hlay->setMargin( 0 );
  hlay->addWidget( new QLabel( tr( "Value:" ), grd ) );
  QSlider	*sl = _privdata->shinsl
    = new QSlider( grd );
  sl->setOrientation( Qt::Horizontal );
  sl->setRange( 0, 1280 );
  sl->setValue( 0 );
  hlay->addWidget( sl );
  sl->setMinimumSize( 50, sl->sizeHint().height() );
  QLabel	*lab = _privdata->shinlabel = new QLabel( "0", grd );
  hlay->addWidget( lab );
  lab->setMinimumSize( 30, lab->sizeHint().height() );
  connect( _privdata->shinsl, SIGNAL( valueChanged( int ) ), this, 
           SLOT( shininessChanged( int ) ) );

  ebox = new QGroupBox( tr( "Update mode :" ), epan );
  vlay = new QVBoxLayout( ebox );
  glay->addWidget( ebox, 2, 1 );
  QCheckBox	*respbtn = new QCheckBox( tr( "Responsive" ), ebox );
  vlay->addWidget( respbtn );
  vlay->addStretch( 1 );
  respbtn->setChecked( _privdata->responsive );
  connect( respbtn, SIGNAL( toggled( bool ) ), this, 
           SLOT( enableAutoUpdate( bool ) ) );

  // --

  mainlay->addWidget( sel );
  mainlay->addWidget( tbar );
  mainlay->addWidget( bpan );
  mainlay->addWidget( epan );

  connect( tbar, SIGNAL( currentChanged( int ) ),
           this, SLOT( enableTab( int ) ) );
  connect( sel, SIGNAL( selectionStarts() ), this, SLOT( chooseObject() ) );
  connect( sel, 
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this, 
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );

  enableAutoUpdate( _privdata->responsive );
  updateInterface();
}


QWidget* MaterialWindow::buildRgbaBox( QWidget* parent, int num )
{
  QLabel	*lab;
  QANumSlider	*sl;

  QWidget *grd = new QWidget( parent );
  QGridLayout *glay = new QGridLayout( grd );
  glay->setSpacing( 10 );
  glay->setMargin( 0 );
  glay->addWidget( new QLabel( tr( "Red:" ), grd ), 0, 0 );
  sl = _privdata->sliders[num][0]
    = new QANumSlider( num, 0, 0, 100, 1, 0, Qt::Horizontal, grd );
  glay->addWidget( sl, 0, 1 );
  sl->setMinimumSize( 100, sl->sizeHint().height() );
  lab = _privdata->labels[num][0] = new QLabel( "0", grd );
  glay->addWidget( lab, 0, 2 );
  lab->setMinimumSize( 25, lab->sizeHint().height() );
  connect( sl, SIGNAL( valueChanged( int, int, int ) ), 
           this, SLOT( valueChanged( int, int, int ) ) );

  glay->addWidget( new QLabel( tr( "Green:" ), grd ), 1, 0 );
  sl = _privdata->sliders[num][1]
    = new QANumSlider( num, 1, 0, 100, 1, 0, Qt::Horizontal, grd );
  glay->addWidget( sl, 1, 1 );
  sl->setMinimumSize( 50, sl->sizeHint().height() );
  lab = _privdata->labels[num][1] = new QLabel( "0", grd );
  glay->addWidget( lab, 1, 2 );
  lab->setMinimumSize( 25, lab->sizeHint().height() );
  connect( sl, SIGNAL( valueChanged( int, int, int ) ), 
           this, SLOT( valueChanged( int, int, int ) ) );

  glay->addWidget( new QLabel( tr( "Blue:" ), grd ), 2, 0 );
  sl = _privdata->sliders[num][2]
    = new QANumSlider( num, 2, 0, 100, 1, 0, Qt::Horizontal, grd );
  glay->addWidget( sl, 2, 1 );
  sl->setMinimumSize( 50, sl->sizeHint().height() );
  lab = _privdata->labels[num][2] = new QLabel( "0", grd );
  glay->addWidget( lab, 2, 2 );
  lab->setMinimumSize( 25, lab->sizeHint().height() );
  connect( sl, SIGNAL( valueChanged( int, int, int ) ), 
           this, SLOT( valueChanged( int, int, int ) ) );

  glay->addWidget( new QLabel( tr( "Opacity:" ), grd ), 3, 0 );
  sl = _privdata->sliders[num][3]
    = new QANumSlider( num, 3, 0, 100, 1, 0, Qt::Horizontal, grd );
  glay->addWidget( sl, 3, 1 );
  sl->setMinimumSize( 50, sl->sizeHint().height() );
  lab = _privdata->labels[num][3] = new QLabel( "0", grd );
  glay->addWidget( lab, 3, 2 );
  lab->setMinimumSize( 25, lab->sizeHint().height() );
  connect( sl, SIGNAL( valueChanged( int, int, int ) ), 
           this, SLOT( valueChanged( int, int, int ) ) );

  return grd;
}


void MaterialWindow::enableTab( int tabid )
{
  unsigned tab = _privdata->tabnum[ tabid ];
  if( tab != _privdata->tab )
    {
      _privdata->tabs[ _privdata->tab ]->hide();
      _privdata->tab = tab;
      _privdata->tabs[ tab ]->show();
    }
}


void MaterialWindow::valueChanged( int panel, int color, int value )
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  //cout << "MaterialWindow::valueChanged\n";
  unsigned	sl = 4;
  float		*col;

  if( panel == 4 )
    {
      sl = 1;
      panel = 1;	// groups 1 & 4: both diffuse
    }

  _privdata->labels[panel][color]->setText( QString::number( value ) );
  _privdata->modified = true;

  switch( panel )
    {
    case 0:
      col = _material.Ambient();
      col[color] = 0.01 * value;
      _material.SetAmbient( col[0], col[1], col[2], col[3] );
      break;
    case 1:
      _privdata->labels[4][color]->setText( QString::number( value ) );
      _privdata->sliders[sl][color]->setValue( value );
      col = _material.Diffuse();
      col[color] = 0.01 * value;
      _material.SetDiffuse( col[0], col[1], col[2], col[3] );
      break;
    case 2:
      col = _material.Emission();
      col[color] = 0.01 * value;
      _material.SetEmission( col[0], col[1], col[2], col[3] );
      break;
    default:
      col = _material.Specular();
      col[color] = 0.01 * value;
      _material.SetSpecular( col[0], col[1], col[2], col[3] );
      break;
    }
  updateObjects();

  _privdata->operating = false;
}


void MaterialWindow::shininessChanged( int value )
{
  if( _privdata->operating )
    return;
  _privdata->operating = true;

  _privdata->shinlabel->setText( QString::number( 0.1 * value ) );
  _material.SetShininess( 0.1 * value );
  updateObjects();

  _privdata->operating = false;
}


void MaterialWindow::enableAutoUpdate( bool state )
{
  //cout << "responsive : " << state << endl;
  unsigned panel, color;

  for( panel=0; panel<5; ++panel )
    for( color=0; color<4; ++color )
      _privdata->sliders[panel][color]->setTracking( state );
  _privdata->shinsl->setTracking( state );
}


namespace
{

  void toto()
  {
  }

  void setButtonState( QAbstractButton* b, int x )
  {
    QCheckBox	*cb = dynamic_cast<QCheckBox *>( b );
    if( !cb )
      return;
    switch( x )
    {
    case 0:
      cb->setCheckState( Qt::Unchecked );
      break;
    case 1:
      cb->setCheckState( Qt::Checked );
      break;
    default:
      cb->setCheckState( Qt::PartiallyChecked );
    }
  }

}


void MaterialWindow::updateInterface()
{
  if( _privdata->operating )
    return;

  _privdata->operating = true;

  blockSignals( true );

  unsigned	panel, i, n = _privdata->tabs.size();

  if( _parents.empty() )
    for( i=0; i<n; ++i )
      _privdata->tabs[i]->setEnabled( false );
  else
    {
      for( i=0; i<n; ++i )
        _privdata->tabs[i]->setEnabled( true );

      _material = (*_parents.begin())->GetMaterial();
      for( panel=0; panel<4; ++panel )
        updatePanel( panel );

      _privdata->shinsl->setValue( (int) ( _material.Shininess() * 10 ) );
      _privdata->shinlabel->setText
        ( QString::number( _material.Shininess() ) );

    }

  blockSignals( false );
  _privdata->operating = false;
}


void MaterialWindow::updatePanel( unsigned panel )
{
  float	*cols;

  switch( panel )
    {
    case 0:
      cols = _material.Ambient();
      break;
    case 1:
      cols = _material.Diffuse();
      break;
    case 2:
      cols = _material.Emission();
      break;
    default:
      cols = _material.Specular();
      break;
    }
  unsigned	c;
  for( c=0; c<4; ++c )
    {
      _privdata->sliders[panel][c]->setValue( (int) ( cols[c] * 100 ) );
      _privdata->labels[panel][c]->
	setText( QString::number( cols[c] * 100 ) );
      if( panel == 1 )
	{
	  _privdata->sliders[4][c]->setValue( (int) ( cols[c] * 100 ) );
	  _privdata->labels[4][c]->
	    setText( QString::number( cols[c] * 100 ) );
	}
    }
}


void MaterialWindow::chooseObject()
{
  // cout << "chooseObject\n";
  // filter out objects that don't exist anymore
  set<AObject *>::iterator	ir = _privdata->initial.begin(), 
    er = _privdata->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        _privdata->initial.erase( ir2 );
      }

  _privdata->objsel->selectObjects( _privdata->initial, _parents );
}


void MaterialWindow::objectsChosen( const set<AObject *> & o )
{
  _privdata->operating = true;
  runCommand();

  set<AObject *>::const_iterator	i, e = o.end();
  while( !_parents.empty() )
    (*_parents.begin())->deleteObserver( this );
  _parents = o;
  for( i=o.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  _privdata->objsel->updateLabel( o );
  _privdata->operating = false;
  updateInterface();
}


void MaterialWindow::runCommand()
{
  if( _privdata->modified && !_parents.empty() )
    {
      string	rmode;
      switch( _material.renderProperty( Material::RenderMode ) )
        {
        case Material::Normal:
          rmode = "normal";
          break;
        case Material::Wireframe:
          rmode = "wireframe";
          break;
        case Material::Outlined:
          rmode = "outlined";
          break;
        case Material::HiddenWireframe:
          rmode = "hiddenface_wireframe";
          break;
        default:
          rmode = "default";
        }

      SetMaterialCommand	*com 
        = new SetMaterialCommand
        ( _parents, _material.Ambient(), _material.Diffuse(), 
          _material.Emission(), _material.Specular(), _material.Shininess(), 
          true, _material.renderProperty( Material::RenderLighting ), 
          _material.renderProperty( Material::RenderSmoothShading ), 
          _material.renderProperty( Material::RenderFiltering ), 
          _material.renderProperty( Material::RenderZBuffer ), 
          _material.renderProperty( Material::RenderFaceCulling ), 
          rmode );
      theProcessor->execute( com );
      _privdata->modified = false;
    }
}
