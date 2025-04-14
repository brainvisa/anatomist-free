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


#include <anatomist/color/qwObjPalette.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/color/palette.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteselectwidget.h>
#include <anatomist/color/minipalette.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cSetObjectPalette.h>
#include <anatomist/dialogs/qScopeLineEdit.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/object/objectparamselect.h>
#include <anatomist/application/settings.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtablewidget.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qapplication.h>
#include <qaction.h>
#include <qtoolbar.h>
#include <QActionGroup>
#include <chrono>


// check / remove
// slrelmin, slrelmax ?
// updateObjPal, setValues()
// TODO: non-responsive mode


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{
  bool dummyfunc()
  {
    APaletteWinFactory::setCreator( QAPaletteWin::createPalWin );
    return( true );
  }

  bool dummy __attribute__((unused)) = dummyfunc();

}


struct QAPaletteWin::DimBox
{
  QGroupBox		*topBox;
  MiniPaletteWidgetEdit *paledit;
  QLabel		*palView;
  QScopeLineEdit	*minEd;
  QScopeLineEdit	*maxEd;
  QPushButton		*autoValBtn;
  QPushButton		*autoBoundsBtn;
  float			slrelmin;
  float			slrelmax;
  QCheckBox             *symbox;
  QPushButton           *cleatMin;
  QPushButton           *cleatMax;
  QPushButton           *palBtn;
};


struct QAPaletteWin::Private
{
  Private( const set<AObject *> & );

  DimBox			*dimBox1;
  DimBox			*dimBox2;
  QLabel			*view;
  bool				responsive;
  anatomist::AObjectPalette	*objpal;
  unsigned			dim;
  QCheckBox                     *usepal2 ;
  QButtonGroup	                *dimBgp ;
  QComboBox			*mixBox;
  QSlider	                *mixSlid ;
  QLabel			*mixValueLabel;
  QComboBox			*palette1dMappingBox ;
  QWidget			*pal2SetBox;
  float				objMin;
  float				objMax;
  float				objMin2;
  float				objMax2;
  bool				recursive;
  unsigned			sizes[2][2];
  bool                          updatingFlag;
  ObjectParamSelect		*objsel;
  set<AObject *>		initial;
  QWidget			*main;
  bool				modified;
  QToolBar                      *toolbar;
  QActionGroup                  *toolactions;
};


QAPaletteWin::Private::Private( const set<AObject *> & o )
  : responsive( true ), objpal( 0 ), dim( 1 ), objMin( 0 ), objMax( 1 ),
    objMin2( 0 ), objMax2( 1 ), recursive( false ), updatingFlag( false ),
    initial( o ), main( 0 ), modified( false ), toolbar( 0 ), toolactions( 0 )
{
}


APaletteWin* QAPaletteWin::createPalWin( const set<AObject* > & obj )
{
  return( new QAPaletteWin( obj ) );
}


namespace
{

  bool filterPalette( const AObject* o )
  {
    const GLComponent	*gl = o->glAPI();
    if( gl && gl->glNumTextures() > 0 )
      {
        const GLComponent::TexExtrema	& te = gl->glTexExtrema( 0 );
        if( te.minquant.size() > 0 && te.maxquant.size() > 0 )
          return true;
      }
    return false;
  }

}


QAPaletteWin::QAPaletteWin( const set<AObject *> & obj )
  : QWidget( theAnatomist->getQWidgetAncestor(), Qt::Window ),
    APaletteWin( obj ), d( new Private( obj ) )
{
  setWindowTitle( tr( "Object palette composition" ) );
  QPixmap	anaicon( Settings::findResourceFile(
    "icons/icon.xpm" ).c_str() );
  if( !anaicon.isNull() )
    setWindowIcon( anaicon );

  QVBoxLayout	*mainLay = new QVBoxLayout( this );
  mainLay->setObjectName( "mainLay" );
  mainLay->setContentsMargins( 0, 0, 0, 0 );
  mainLay->setSpacing( 0 );
  d->objsel = new ObjectParamSelect( obj, this );
  mainLay->addWidget( d->objsel );
  d->objsel->addFilter( filterPalette );
  d->objsel->layout()->setContentsMargins( 5, 5, 5, 5 );

  d->main = new QWidget( this );
  d->main->setObjectName( "mainLay" );
  QHBoxLayout *mainl = new QHBoxLayout( d->main );
  d->main->setLayout( mainl );
  mainLay->addWidget( d->main );
  mainl->setContentsMargins( 5, 5, 5, 5 );
  mainl->setSpacing( 5 );

  QWidget *rtPanel = new QWidget( d->main );
  rtPanel->setObjectName( "rtPanel" );
  mainl->addWidget( rtPanel );
  QVBoxLayout *rtPanell = new QVBoxLayout( rtPanel );
  rtPanel->setLayout( rtPanell );
  rtPanell->setContentsMargins( 0, 0, 0, 0 );
  rtPanell->setSpacing( 5 );

  d->toolbar = new QToolBar( rtPanel );
  d->toolbar->setObjectName( "palette toolbar" );
  rtPanell->addWidget( d->toolbar );
  d->toolactions = new QActionGroup( d->toolbar );
  fillToolBar();

  QGroupBox	*updateGrp = new QGroupBox( tr( "Update mode :" ), rtPanel );
  rtPanell->addWidget( updateGrp );
  QHBoxLayout *updateGrpl = new QHBoxLayout( updateGrp );
  updateGrp->setLayout( updateGrpl );
  updateGrpl->setContentsMargins( 5, 5, 5, 5 );
  updateGrpl->setSpacing( 5 );
  QCheckBox	*respBtn = new QCheckBox( tr( "Responsive" ), updateGrp );
  updateGrpl->addWidget( respBtn );
  respBtn->setChecked( d->responsive );
  QPushButton	*updateBtn = new QPushButton( tr( "Update" ), updateGrp );
  updateGrpl->addWidget( updateBtn );
  updateGrp->setFixedHeight( updateGrp->sizeHint().height() );
  updateGrp->hide(); // FIXME: make it work or remove it

  QGroupBox *dimBgpw = new QGroupBox( tr( "Dimension :" ), rtPanel );
  d->dimBgp = new QButtonGroup( dimBgpw );
  d->dimBgp->setExclusive( true );
  rtPanell->addWidget( dimBgpw );
  QHBoxLayout *dimBgpl = new QHBoxLayout( dimBgpw );
  dimBgpw->setLayout( dimBgpl );
  dimBgpl->setContentsMargins( 5, 5, 5, 5 );
  dimBgpl->setSpacing( 5 );
  QRadioButton *rb = new QRadioButton( tr( "1D" ), dimBgpw );
  rb->setObjectName( "btn1D" );
  dimBgpl->addWidget( rb );
  d->dimBgp->addButton( rb, 0 );
  rb = new QRadioButton( tr( "2D" ), dimBgpw );
  rb->setObjectName( "btn2D" );
  dimBgpl->addWidget( rb );
  d->dimBgp->addButton( rb, 1 );
  dimBgpw->setSizePolicy( 
    QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );

  d->dimBox1 = new DimBox;
  makeDimBox( tr( "1st dimension settings :" ), rtPanel, d->dimBox1, false );
  rtPanell->addWidget( d->dimBox1->topBox );

  QWidget *pal2Panel = new QWidget( d->main );
  mainl->addWidget( pal2Panel );
  pal2Panel->setObjectName( "pal2Panel" );
  QVBoxLayout *pal2Panell = new QVBoxLayout( pal2Panel );
  pal2Panel->setLayout( pal2Panell );
  pal2Panell->setContentsMargins( 0, 0, 0, 0 );
  pal2Panell->setSpacing( 5 );

  d->dimBox2 = new DimBox;
  makeDimBox( tr( "2nd dimension settings :" ), pal2Panel, d->dimBox2, true );
  pal2Panell->addWidget( d->dimBox2->topBox );

  d->usepal2 = new QCheckBox( tr( "Use a second palette for 2D" ),
                              d->dimBox2->topBox );
  d->dimBox2->topBox->layout()->addWidget( d->usepal2 );
  d->pal2SetBox = new QWidget( d->dimBox2->topBox );
  d->dimBox2->topBox->layout()->addWidget( d->pal2SetBox );
  QVBoxLayout *pal2SetBoxl = new QVBoxLayout( d->pal2SetBox );
  d->pal2SetBox->setLayout( pal2SetBoxl );
  pal2SetBoxl->setContentsMargins( 5, 5, 5, 5 );
  pal2SetBoxl->setSpacing( 5 );
  QWidget *pal2box = new QWidget( d->pal2SetBox );
  pal2SetBoxl->addWidget( pal2box );
  QGridLayout *pal2boxl = new QGridLayout( pal2box );
  pal2box->setLayout( pal2boxl );
  pal2boxl->setContentsMargins( 0, 0, 0, 0 );
  pal2boxl->setSpacing( 5 );
  pal2boxl->addWidget( new QLabel( tr( "Second palette :" ), pal2box ), 0, 0 );
  pal2boxl->addWidget( new QLabel( tr( "Palettes mixing method :" ), pal2box ),
                       1, 0 );
  d->mixBox = new QComboBox( pal2box );
  pal2boxl->addWidget( d->mixBox, 1, 1 );
  d->mixBox->setObjectName( "2d_mixing_method" );
  pal2boxl->addWidget( new QLabel( tr( "Palette 1D Mapping :" ), pal2box ),
                       2, 0 );
  d->palette1dMappingBox = new QComboBox( pal2box );
  d->palette1dMappingBox->setObjectName( "palette_1d_mapping_method" );
  pal2boxl->addWidget( d->palette1dMappingBox, 2, 1 );

  fillMixMethods();
  fillPalette1DMappingMethods() ;

  QWidget *mixFacBox = new QWidget( d->pal2SetBox );
  pal2SetBoxl->addWidget( mixFacBox );
  QHBoxLayout *mixFacBoxl = new QHBoxLayout( mixFacBox );
  mixFacBox->setLayout( mixFacBoxl );
  mixFacBoxl->setContentsMargins( 0, 0, 0, 0 );
  mixFacBoxl->setSpacing( 5 );
  mixFacBoxl->addWidget( new QLabel( tr( "Mixing factor :" ), mixFacBox ) );
  d->mixSlid = new QSlider( mixFacBox );
  d->mixSlid->setRange( 0, 100 );
  d->mixSlid->setValue( 0 );
  d->mixSlid->setOrientation( Qt::Horizontal );
  d->mixSlid->setObjectName( "mixSlid" );
  mixFacBoxl->addWidget( d->mixSlid );
  d->mixValueLabel = new QLabel( "100", mixFacBox );
  mixFacBoxl->addWidget( d->mixValueLabel );
  d->mixValueLabel->setMinimumWidth( 30 );
  pal2SetBoxl->addStretch( 1 );

  if( !obj.empty() )
  {
    d->dimBox1->paledit->setObject( *obj.begin(), 0 );
    d->dimBox2->paledit->setObject( *obj.begin(), 1 );
  }

  updateInterface();

  connect( d->dimBgp, SIGNAL( idClicked( int ) ), this,
           SLOT( dimChanged( int ) ) );
  connect( d->dimBox1->minEd, SIGNAL( returnPressed() ), this,
	   SLOT( min1EditChanged() ) );
  connect( d->dimBox1->minEd, SIGNAL( focusLost() ), this,
	   SLOT( min1EditChanged() ) );
  connect( d->dimBox1->maxEd, SIGNAL( returnPressed() ), this,
	   SLOT( max1EditChanged() ) );
  connect( d->dimBox1->maxEd, SIGNAL( focusLost() ), this,
	   SLOT( max1EditChanged() ) );
  connect( d->dimBox1->autoValBtn, SIGNAL( clicked() ), this,
	   SLOT( resetValues1() ) );
  connect( d->dimBox1->autoBoundsBtn, SIGNAL( clicked() ), this,
	   SLOT( resetBounds1() ) );
  connect( d->dimBox1->symbox, SIGNAL( stateChanged( int ) ),
           this, SLOT( zeroCentered1Changed( int ) ) );

  connect( d->dimBox2->minEd, SIGNAL( returnPressed() ), this,
	   SLOT( min2EditChanged() ) );
  connect( d->dimBox2->minEd, SIGNAL( focusLost() ), this,
	   SLOT( min2EditChanged() ) );
  connect( d->dimBox2->maxEd, SIGNAL( returnPressed() ), this,
	   SLOT( max2EditChanged() ) );
  connect( d->dimBox2->maxEd, SIGNAL( focusLost() ), this,
	   SLOT( max2EditChanged() ) );
  connect( d->dimBox2->autoValBtn, SIGNAL( clicked() ), this,
	   SLOT( resetValues2() ) );
  connect( d->dimBox2->autoBoundsBtn, SIGNAL( clicked() ), this,
	   SLOT( resetBounds2() ) );
  connect( d->dimBox2->symbox, SIGNAL( stateChanged( int ) ),
           this, SLOT( zeroCentered2Changed( int ) ) );

  connect( respBtn, SIGNAL( toggled( bool ) ),
	   this, SLOT( responsiveToggled( bool ) ) );
  connect( updateBtn, SIGNAL( clicked() ), this, SLOT( updateClicked() ) );

  connect( d->mixBox, SIGNAL( activated( int ) ),
	   this, SLOT( mixMethodChanged( int ) ) );
  connect( d->palette1dMappingBox, SIGNAL( activated( int ) ),
	   this, SLOT( palette1DMappingMethodChanged( int ) ) );
  connect( d->usepal2, SIGNAL( toggled( bool ) ),
	   this, SLOT( enablePalette2( bool ) ) );
  connect( d->mixSlid, SIGNAL( valueChanged( int ) ),
	   this, SLOT( mixFactorChanged( int ) ) );
  connect( d->objsel, SIGNAL( selectionStarts() ), this,
           SLOT( chooseObject() ) );
  connect( d->objsel,
           SIGNAL( objectsSelected( const std::set<anatomist::AObject *> & ) ),
           this,
           SLOT( objectsChosen( const std::set<anatomist::AObject *> & ) ) );
  connect( d->toolactions, SIGNAL( triggered( QAction * ) ), this, 
           SLOT( extensionActionTriggered( QAction * ) ) );
  connect( d->dimBox1->paledit->miniPaletteWidget(),
           SIGNAL( rangeChanged( float, float ) ),
           this, SLOT( palette1RangeChanged( float, float ) ) );
  connect( d->dimBox1->palBtn, SIGNAL( clicked() ),
           d->dimBox1->paledit, SLOT( selectPalette() ) );
  connect( d->dimBox2->paledit->miniPaletteWidget(),
           SIGNAL( rangeChanged( float, float ) ),
           this, SLOT( palette2RangeChanged( float, float ) ) );
  connect( d->dimBox2->palBtn, SIGNAL( clicked() ),
           d->dimBox2->paledit, SLOT( selectPalette() ) );
  connect( d->dimBox1->paledit,
           SIGNAL( paletteSelected( const std::string & ) ),
           this, SLOT( palette1Changed( const std::string & ) ) );
  connect( d->dimBox2->paledit,
           SIGNAL( paletteSelected( const std::string & ) ),
           this, SLOT( palette2Changed( const std::string & ) ) );
  connect( d->dimBox1->cleatMin, SIGNAL( clicked() ),
           this, SLOT( cleatMin1() ) );
  connect( d->dimBox1->cleatMax, SIGNAL( clicked() ),
           this, SLOT( cleatMax1() ) );
  connect( d->dimBox2->cleatMin, SIGNAL( clicked() ),
           this, SLOT( cleatMin2() ) );
  connect( d->dimBox2->cleatMax, SIGNAL( clicked() ),
           this, SLOT( cleatMax2() ) );
  connect( d->dimBox1->paledit->minSlider(),
           SIGNAL( absValueChanged( float ) ),
           this, SLOT( min1Changed( float ) ) );
  connect( d->dimBox1->paledit->maxSlider(),
           SIGNAL( absValueChanged( float ) ),
           this, SLOT( max1Changed( float ) ) );
  connect( d->dimBox2->paledit->minSlider(),
           SIGNAL( absValueChanged( float ) ),
           this, SLOT( min2Changed( float ) ) );
  connect( d->dimBox2->paledit->maxSlider(),
           SIGNAL( absValueChanged( float ) ),
           this, SLOT( max2Changed( float ) ) );

  if( objPalette()->palette1DMapping() == AObjectPalette::DIAGONAL
      || objPalette()->is2dMode() )
  {
    d->dimBgp->button(1)->setChecked( true );
    d->dimBox1->paledit->miniPaletteWidget()->setMinimumHeight( 250 );
    d->dimBox1->paledit->miniPaletteWidget()->setMaximumHeight(
      QWIDGETSIZE_MAX );
  }
  resize( minimumSize() );

  show();
  d->sizes[0][0] = 0;
  d->sizes[0][1] = 0;
  d->sizes[1][0] = 0;
  d->sizes[1][1] = 0;

  d->dimBox1->paledit->adjustRange();
  d->dimBox2->paledit->adjustRange();
}


QWidget* QAPaletteWin::makeDimBox( const QString & title, QWidget* parent,
				   struct DimBox* dbox, bool second )
{
  dbox->topBox = new QGroupBox( title, parent );
  QVBoxLayout *topBoxl = new QVBoxLayout( dbox->topBox );
  dbox->topBox->setLayout( topBoxl );
  topBoxl->setContentsMargins( 5, 5, 5, 5 );
  topBoxl->setSpacing( 5 );

  if( second )
    dbox->paledit = new MiniPaletteWidgetEdit( 0, 1, false, false, true );
  else
  {
    dbox->paledit = new MiniPaletteWidgetEdit;
    dbox->paledit->miniPaletteWidget()->setMinimumHeight( 80 );
    dbox->paledit->miniPaletteWidget()->setMaximumHeight( 80 );
  }
  topBoxl->addWidget( dbox->paledit );
  topBoxl->addStretch( 1 );

  QWidget *minmaxbox = new QWidget( dbox->topBox );
  topBoxl->addWidget( minmaxbox );

  QGridLayout *minmaxboxl = new QGridLayout( minmaxbox );
  minmaxbox->setLayout( minmaxboxl );
  minmaxboxl->setContentsMargins( 0, 0, 0, 0 );
  minmaxboxl->setSpacing( 5 );

  QWidget *boundsbox = new QWidget( dbox->topBox );
  topBoxl->addWidget( boundsbox );
  QHBoxLayout *boundsboxl = new QHBoxLayout( boundsbox );
  boundsbox->setLayout( boundsboxl );
  boundsboxl->setContentsMargins( 0, 0, 0, 0 );
  boundsboxl->setSpacing( 5 );
  boundsboxl->addWidget( new QLabel( tr( "Bounds:" ), boundsbox ) );
  dbox->cleatMin = new QPushButton( "⇤" );
  dbox->cleatMin->setFixedWidth( 32 );
  boundsboxl->addWidget( dbox->cleatMin );
  dbox->minEd = new QScopeLineEdit( "0", boundsbox, "minEd" );
  boundsboxl->addWidget( dbox->minEd );
  dbox->minEd->setMinimumWidth( 50 );
  dbox->maxEd = new QScopeLineEdit( "1", boundsbox, "maxEd" );
  boundsboxl->addWidget( dbox->maxEd );
  dbox->maxEd->setMinimumWidth( 50 );
  dbox->cleatMax = new QPushButton( "⇥" );
  dbox->cleatMax->setFixedWidth( 32 );
  boundsboxl->addWidget( dbox->cleatMax );

  dbox->symbox = new QCheckBox( tr( "Value 0 at center" ), dbox->topBox );
  topBoxl->addWidget( dbox->symbox );

  QWidget *autobox = new QWidget( dbox->topBox );
  topBoxl->addWidget( autobox );
  QHBoxLayout *autoboxl = new QHBoxLayout( autobox );
  autobox->setLayout( autoboxl );
  autoboxl->setContentsMargins( 0, 0, 0, 0 );
  autoboxl->setSpacing( 5 );
  dbox->autoValBtn = new QPushButton( tr( "Reset values" ), autobox );
  autoboxl->addWidget( dbox->autoValBtn );
  dbox->autoBoundsBtn = new QPushButton( tr( "Reset bounds" ), autobox );
  autoboxl->addWidget( dbox->autoBoundsBtn );
  QIcon icon = QIcon(
    Settings::findResourceFile( "icons/palette.jpg" ).c_str() );
  dbox->palBtn = new QPushButton( icon, "", autobox );
  dbox->palBtn->setFixedSize( dbox->palBtn->minimumSizeHint() );
  autoboxl->addWidget( dbox->palBtn );

  dbox->palView = new QLabel( dbox->topBox );
  topBoxl->addWidget( dbox->palView );
  dbox->palView->setFrameStyle( QFrame::Sunken | QFrame::Box );
  dbox->palView->setSizePolicy( QSizePolicy(
    QSizePolicy::Preferred, QSizePolicy::Fixed ) );
  dbox->palView->setScaledContents( true );

  dbox->slrelmin = 0;
  dbox->slrelmax = 1;

  return dbox->topBox;
}


QAPaletteWin::~QAPaletteWin()
{
  runCommand();

  cleanupObserver();

  delete d->objpal;
  delete d->dimBox1;
  delete d->dimBox2;
  delete d;
}


void QAPaletteWin::updateInterface()
{
  set<AObject *>::const_iterator	io, eo = _parents.end();
  AObject				*o = 0;
  GLComponent				*gl;

  delete d->objpal;
  d->objpal = 0;

  for( io=_parents.begin(); io!=eo; ++io )
  {
    gl = (*io)->glAPI();

    if( gl && gl->glNumTextures() > 0 )
    {
      const GLComponent::TexExtrema	& te = gl->glTexExtrema( 0 );
      if( te.minquant.size() > 0 && te.maxquant.size() > 0 )
      {
        o = *io;
        break;
      }
    }
  }
  if( o )
  {
    d->main->setEnabled( true );
    const GLComponent::TexExtrema	& te = gl->glTexExtrema( 0 );
    AObject	*refobj = *_parents.begin();
    refobj->getOrCreatePalette();
    d->objpal = refobj->palette();
    d->objMin = te.minquant[0];
    d->objMax = te.maxquant[0];
    d->dimBox1->slrelmin = 0;
    d->dimBox1->slrelmax = 1;
    d->dimBox2->slrelmin = 0;
    d->dimBox2->slrelmax = 1;
    if( d->objMin == d->objMax )
      d->objMax = d->objMin + 1;	// avoid pbs of division by 0
  }
  else
    d->main->setEnabled( false );

  if( !d->objpal )
  {
    // cout << "creating new object palette\n";
    const PaletteList	& pallist = theAnatomist->palettes();
    if( pallist.size() == 0 )
    {
      cerr << "Palettes list empty. Set your ANATOMIST_PATH variable\n";
      return;
    }
    d->objpal = new AObjectPalette( *pallist.palettes().begin() );
  }
  else
    d->objpal = d->objpal->clone();

  if( o != d->dimBox1->paledit->getObject() )
  {
    d->dimBox1->paledit->setObject( o, 0 );
    d->dimBox2->paledit->setObject( o, 1 );
  }

  if( !o )
    return;

  d->dimBox1->symbox->setChecked( d->objpal->zeroCenteredAxis1() );
  d->dimBox2->symbox->setChecked( d->objpal->zeroCenteredAxis2() );
  if( d->objpal->is2dMode() )
    d->dim = 2;
  //cout << "using palette " << d->objpal->refPalette()->name() << endl;

  d->mixSlid->blockSignals( true );
  d->dimBgp->blockSignals( true );
  d->mixBox->blockSignals( true );

  d->objsel->updateLabel( _parents );
  d->dimBgp->button( d->dim - 1 )->setChecked( true );

  fillPalette1();
  setValues1();
  fillPalette2();
  setValues2();
  if( d->dim ==1 )
    d->dimBox2->topBox->hide();
  else
    d->dimBox2->topBox->show();

  d->mixSlid->setValue( (int) ( 100 * objPalette()->linearMixFactor() ) );
  d->mixValueLabel->setText
    ( QString::number( 100 * objPalette()->linearMixFactor() ) );

  int i, n;

  if( objPalette()->refPalette2() )
  {
    if( d->objpal->refPalette2() )
      d->usepal2->setChecked( true );
    else
      d->pal2SetBox->hide();
    }

  n = d->mixBox->count();
  for( i=0; i<n; ++i )
    if( d->mixBox->itemText( i ) == d->objpal->mixMethodName().c_str() )
    {
      d->mixBox->setCurrentIndex( i );
      break;
    }

  d->mixSlid->blockSignals( false );
  d->dimBgp->blockSignals( false );
  d->mixBox->blockSignals( false );
}


void QAPaletteWin::update( const anatomist::Observable* obs, void* )
{
  if (d->updatingFlag)
    return ;
  d->updatingFlag = true ;

  // cout << "QAPaletteWin::update\n";

  const AObject * obj = dynamic_cast<const AObject*>( obs );
  if( !obj || !theAnatomist->hasObject( obj ) )
  {
    if( objects().size() == 1 )
      obj = *objects().begin();
    else
      return;  // no object to update
  }

  if( obj && theAnatomist->hasObject( obj ) && obj->hasChanged() )
  {
    const AObjectPalette	*pal = obj->getOrCreatePalette();
    if( pal )
    {
      rc_ptr<AObjectPalette> oldpal;
      if( d->objpal )
        oldpal.reset( new AObjectPalette( *d->objpal ) );

      if( d->objpal )
        *d->objpal = *pal;
      else
        d->objpal = new AObjectPalette( *pal ) ;

      if( d->objpal->palette1DMapping() == AObjectPalette::DIAGONAL )
      {
        d->dimBgp->button(1)->setChecked( true );
        d->palette1dMappingBox->setCurrentIndex( 1 ) ;
        dimChanged(1) ;
      }
      else
        d->palette1dMappingBox->setCurrentIndex( 0 ) ;

      if( !oldpal || d->objpal->refPalette() != oldpal->refPalette() )
        fillPalette1();
      if( !oldpal || d->objpal->refPalette2() != oldpal->refPalette2() )
        fillPalette2();

      d->dimBox1->symbox->setChecked( pal->zeroCenteredAxis1() );
      d->dimBox2->symbox->setChecked( pal->zeroCenteredAxis2() );

      const GLComponent	*gl = obj->glAPI();
      if( gl )
      {
        if( gl->glNumTextures() > 0 )
        {
          const GLComponent::TexExtrema	& te = gl->glTexExtrema( 0 );
          if( !te.minquant.empty() )
          {
            d->objMin = te.minquant[0];
          }
          if( !te.maxquant.empty() )
            d->objMax = te.maxquant[0];
          if( d->objMin == d->objMax )
            d->objMax = d->objMin + 1;	// avoid pbs of division by 0
          setValues( d->dimBox1, d->objpal->min1(), d->objpal->max1(),
                      d->objMin, d->objMax, pal->zeroCenteredAxis1() );
          setValues( d->dimBox2, d->objpal->min2(), d->objpal->max2(),
                      d->objMin2, d->objMax2, pal->zeroCenteredAxis2() );
        }
      }
    }

    int i, n=d->mixBox->count();
    for( i=0; i<n; ++i )
      if( d->mixBox->itemText( i )
          == d->objpal->mixMethodName().c_str() )
      {
        d->mixBox->setCurrentIndex( i );
        break;
      }
    d->mixSlid->setValue( (int) ( 100 * d->objpal->linearMixFactor() ) );
    d->mixValueLabel->setText( QString::number
                                ( 100 * d->objpal->linearMixFactor() ) );

  }

  updateObjPal();

  d->updatingFlag = false ;
}


void QAPaletteWin::unregisterObservable( anatomist::Observable* obs )
{
  Observer::unregisterObservable( obs );
  AObject	*o = dynamic_cast<AObject *>( obs );
  if( !o )
    return;
  _parents.erase( o );
  d->objsel->updateLabel( _parents );
  updateInterface();
}


AObjectPalette* QAPaletteWin::objPalette()
{
  return( d->objpal );
}


void QAPaletteWin::fillPalette1()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    {
      cout << "No object palette\n";
      return;
    }

  const rc_ptr<APalette> pal = objpal->refPalette();

  if( !pal )
    {
      cout << "no ref pal in object palette\n";
      return;
    }

  QPixmap	pm;

  PaletteSelectWidget::fillPalette( pal, pm );

  d->dimBox1->palView->setPixmap( pm );
}


void QAPaletteWin::fillPalette2()
{
  AObjectPalette	*objpal = objPalette();
  QPixmap	pm;

  if( objpal )
  {
    const rc_ptr<APalette> pal = objpal->refPalette2();
    if( pal )
    {
      PaletteSelectWidget::fillPalette( pal, pm );
      d->dimBox2->palView->setPixmap( pm );
    }
  }
  d->dimBox2->palView->setPixmap( pm );
}


void QAPaletteWin::palette1Changed( const string & )
{
  if( d->recursive )
    return;

  fillPalette1();
  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::palette2Changed( const string & )
{
  if( d->recursive )
    return;

  fillPalette2();
  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::setValues( DimBox* dimBox, float m, float M,
                              float objMin, float objMax, bool zeroCentered )
{
  bool rec = d->recursive;
  d->recursive = true;

  double	min, max;

  if( zeroCentered )
  {
    double omax = std::max( std::abs( objMin ), std::abs( objMax ) );
    float cmin = std::min( m, M );
    float cmax = std::max( m, M );
    dimBox->slrelmin = std::min( cmin / 2 + 0.5f, dimBox->slrelmin );
    dimBox->slrelmax = std::max( cmax / 2 + 0.5f, dimBox->slrelmax );
    min = -omax + omax * 2 * dimBox->slrelmin;
    max = -omax + omax * 2 * dimBox->slrelmax;
  }
  else
  {
    min = double(objMin) + ( double(objMax) - objMin ) * dimBox->slrelmin;
    max = double(objMin) + ( double(objMax) - objMin ) * dimBox->slrelmax;
  }
  dimBox->minEd->setText( QString::number( min ) );
  dimBox->maxEd->setText( QString::number( max ) );
  dimBox->paledit->setRange( min, max );
  dimBox->paledit->updateDisplay();

  d->recursive = rec;
}


void QAPaletteWin::setValues1()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  float	m = objpal->min1(), M = objpal->max1();

  setValues( d->dimBox1, m, M, d->objMin, d->objMax,
             objpal->zeroCenteredAxis1() );
}


void QAPaletteWin::setValues2()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  float	m = objpal->min2(), M = objpal->max2();

  setValues( d->dimBox2, m, M, d->objMin2, d->objMax2,
             objpal->zeroCenteredAxis2() );
}


void QAPaletteWin::updateObjects()
{
  bool upd = d->updatingFlag;
  d->updatingFlag = true;

  const set<AObject *>	& obj = objects();

  if( obj.empty() )
    {
      d->updatingFlag = upd;
      return;
    }

  set<AObject *>::const_iterator io, fo = obj.end();
  AObjectPalette *pal = objPalette();
  double mi, ma, omi, oma;
  AObject *o;

  // convert to absolute values
  if( pal->zeroCenteredAxis1() )
  {
    double omax = std::max( std::abs( d->objMax ), std::abs( d->objMin ) );
    omi = pal->min1() * omax;
    oma = pal->max1() * omax;
  }
  else
  {
    omi = pal->min1() * ( double(d->objMax) - d->objMin ) + double(d->objMin);
    oma = pal->max1() * ( double(d->objMax) - d->objMin ) + double(d->objMin);
  }

  for( io=obj.begin(); io!=fo; ++io )
    {
      o = *io;
      GLComponent  *gl = o->glAPI();
      if( gl && gl->glNumTextures() > 0 )
        {
          GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
          if( !te.minquant.empty() && !te.maxquant.empty() )
          {
            mi = te.minquant[0];
            ma = te.maxquant[0];
            if( pal->zeroCenteredAxis1() )
            {
              ma = std::max( std::abs( ma ), std::abs( mi ) );
              mi = 0.;
            }
            if( mi == ma )	// protect against division by 0
              ma = mi + 1;
            // convert to object scale
            AObjectPalette op = *pal;
            op.setMin1( float( ( omi - mi ) / ( ma - mi ) ) );
            op.setMax1( float( ( oma - mi ) / ( ma - mi ) ) );
            o->setPalette( op );
            gl->glSetTexImageChanged();
          }
        }
#ifdef ANA_DEBUG
      cout << "QAPaletteWin " << (Observer *) this
           << " ::updateObjects, obj: " << o << ": " << o->name() << endl;
#endif
      o->notifyObservers( o );
      // o->clearHasChangedFlags();
    }
  d->updatingFlag = upd;
}


void QAPaletteWin::updateClicked()
{
  runCommand();
  //updateObjects();
}


void QAPaletteWin::responsiveToggled( bool val )
{
  d->responsive = val;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::closeEvent( QCloseEvent* )
{
  delete this;
}


void QAPaletteWin::dimChanged( int num )
{
  unsigned	oldDim = d->dim;
  d->dim = num + 1;
  d->sizes[oldDim-1][0] = size().width();
  d->sizes[oldDim-1][1] = size().height();

  if( d->dim >= 2 && oldDim == 1 )
  {
    d->dimBox2->topBox->show();
    d->dimBox1->paledit->miniPaletteWidget()->setMinimumHeight( 250 );
    d->dimBox1->paledit->miniPaletteWidget()->setMaximumHeight(
      QWIDGETSIZE_MAX );
  }
  else if( d->dim == 1 )
  {
    d->dimBox2->topBox->hide();
    d->dimBox1->paledit->miniPaletteWidget()->setMinimumHeight( 80 );
    d->dimBox1->paledit->miniPaletteWidget()->setMaximumHeight( 80 );
  }

  objPalette()->set2dMode( num == 1 );
  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
  if( d->sizes[d->dim-1][0] != 0 && d->sizes[d->dim-1][1] != 0 )
    resize( d->sizes[d->dim-1][0], d->sizes[d->dim-1][1] );
}


void QAPaletteWin::updateObjPal()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;
  const rc_ptr<APalette> pal = objpal->refPalette();
  if( !pal )
    return;
  const rc_ptr<APalette> pal2 = objpal->refPalette2();
  unsigned		dimx = pal->getSizeX(), dimy = pal->getSizeY();
  unsigned		dimxmax = 256, dimymax = 256;

  if( d->dim <= 1 )
    dimy = 1;
  else if( pal2 )
    {
      dimy = pal2->getSizeX();
      if( dimy < 1 )
	dimy = 1;
      else if( dimy > dimymax )
	dimy = dimymax;
      if( dimx < dimxmax )
	dimx = dimxmax;
    }
  objpal->set2dMode( d->dim >= 2 );

  objpal->create( dimx, dimy );
  objpal->fill();
  /*cout << "objpal dims : " << objpal->colors()->dimX() << " x "
    << objpal->colors()->dimY() << endl;*/
}


void QAPaletteWin::fillMixMethods()
{
  map<string, AObjectPalette::MixMethod>::const_iterator	im,
    fm = AObjectPalette::mixMethods.end();

  for( im=AObjectPalette::mixMethods.begin(); im!=fm; ++im )
    d->mixBox->addItem( (*im).first.c_str() );
}


void QAPaletteWin::mixMethodChanged( int mid )
{
  if( d->recursive )
    return;

  QString methname = d->mixBox->itemText( mid );
  AObjectPalette	*objpal = objPalette();

  objpal->setMixMethod( string( methname.toStdString() ) );
  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}

void
QAPaletteWin::fillPalette1DMappingMethods()
{
  d->palette1dMappingBox->addItem("FirstLine") ;
  d->palette1dMappingBox->addItem("Diagonal") ;
}

void
QAPaletteWin::palette1DMappingMethodChanged( int mid )
{
  if( d->recursive )
    return;

  QString methname = d->palette1dMappingBox->itemText( mid );
  AObjectPalette	*objpal = objPalette();

  if( methname == "FirstLine" )
    objpal->setPalette1DMapping( AObjectPalette::FIRSTLINE ) ;
  else
    objpal->setPalette1DMapping( AObjectPalette::DIAGONAL ) ;

  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}

void QAPaletteWin::enablePalette2( bool f )
{
  if( !f )
    {
      d->pal2SetBox->hide();

      AObjectPalette	*objpal = objPalette();
      if( !objpal )
	return;

      objpal->setRefPalette2( rc_ptr<APalette>() );
      fillPalette2();
      updateObjPal();
      d->modified = true;
      if( d->responsive )
        updateObjects();
    }
  else
    {
      d->pal2SetBox->show();
    }
}


void QAPaletteWin::mixFactorChanged( int val )
{
  if( d->recursive )
    return;

  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;
  objpal->setLinearMixFactor( 0.01 * val );
  d->mixValueLabel->setText( QString::number( val ) );
  updateObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::min1EditChanged()
{
  if( d->recursive )
    return;

  double val = d->dimBox1->minEd->text().toDouble();
  if( !d->dimBox1->symbox->isChecked() )
  {
    d->dimBox1->slrelmin = (val - d->objMin )
      / ( double(d->objMax) - d->objMin );
  }
  else
  {
    double absmax = std::max( std::abs( d->objMin ),
                              std::abs( d->objMax ) );
    d->dimBox1->slrelmin = float( ( val + absmax ) / ( absmax * 2. ) );
  }

  if( d->dimBox1->paledit->range().first != val )
  {
    d->dimBox1->paledit->setRange( val, d->dimBox1->paledit->range().second );
    d->dimBox1->paledit->updateDisplay();
  }
}


void QAPaletteWin::max1EditChanged()
{
  if( d->recursive )
    return;

  double val = d->dimBox1->maxEd->text().toDouble();
  if( !d->dimBox1->symbox->isChecked() )
  {
    d->dimBox1->slrelmax = float( (val - d->objMin )
      / ( double(d->objMax) - d->objMin ) );
  }
  else
  {
    double absmax = std::max( std::abs( d->objMin ),
                              std::abs( d->objMax ) );
    d->dimBox1->slrelmax = float( ( val + absmax ) / ( absmax * 2. ) );
  }

  if( d->dimBox1->paledit->range().second != val )
  {
    d->dimBox1->paledit->setRange( d->dimBox1->paledit->range().first, val );
    d->dimBox1->paledit->updateDisplay();
  }
}


void QAPaletteWin::min2EditChanged()
{
  if( d->recursive )
    return;

  double val = d->dimBox2->minEd->text().toDouble();
  if( !d->dimBox2->symbox->isChecked() )
  {
    d->dimBox2->slrelmin = (val - d->objMin2 )
      / ( double(d->objMax2) - d->objMin2 );
  }
  else
  {
    double absmax = std::max( std::abs( d->objMin2 ),
                              std::abs( d->objMax2 ) );
    d->dimBox2->slrelmin = float( ( val + absmax ) / ( absmax * 2. ) );
  }

  if( d->dimBox2->paledit->range().first != val )
  {
    d->dimBox2->paledit->setRange( val, d->dimBox2->paledit->range().second );
    d->dimBox2->paledit->updateDisplay();
  }
}


void QAPaletteWin::max2EditChanged()
{
  if( d->recursive )
    return;

  double val = d->dimBox2->maxEd->text().toDouble();
  if( !d->dimBox2->symbox->isChecked() )
  {
    d->dimBox2->slrelmax = float( (val - d->objMin2 )
      / ( double(d->objMax2) - d->objMin2 ) );
  }
  else
  {
    double absmax = std::max( std::abs( d->objMin2 ),
                              std::abs( d->objMax2 ) );
    d->dimBox2->slrelmax = float( ( val + absmax ) / ( absmax * 2. ) );
  }

  if( d->dimBox2->paledit->range().second != val )
  {
    d->dimBox2->paledit->setRange( d->dimBox2->paledit->range().first, val );
    d->dimBox2->paledit->updateDisplay();
  }
}


void QAPaletteWin::resetValues1()
{
  AObject	*obj = *_parents.begin();
  obj->adjustPalette();
  AObjectPalette	*op = objPalette(),*rp = obj->palette();
  float m = rp->min1();
  float M = rp->max1();
  if( op->zeroCenteredAxis1() )
  {
    // fix min/max which have been calculated in non-centered mode
    float vmin = rp->min1() * ( d->objMax - d->objMin ) + d->objMin;
    float vmax = rp->max1() * ( d->objMax - d->objMin ) + d->objMin;
    float omax = std::max( std::abs( d->objMin ), std::abs( d->objMax ) );
    m = vmin / omax;
    M = vmax / omax;
    if( m < 0. )
      m = 0.;
  }
  op->setMin1( m );
  op->setMax1( M );
  setValues1();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::resetBounds1()
{
  d->dimBox1->slrelmin = 0;
  d->dimBox1->slrelmax = 1;
  setValues1();
}


void QAPaletteWin::resetValues2()
{
  AObjectPalette	*op = objPalette();
  op->setMin2( 0 );
  op->setMax2( 1 );
  setValues2();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::resetBounds2()
{
  d->dimBox2->slrelmin = 0;
  d->dimBox2->slrelmax = 1;
  setValues2();
}


void QAPaletteWin::chooseObject()
{
  // cout << "chooseObject\n";
  // filter out objects that don't exist anymore
  set<AObject *>::iterator	ir = d->initial.begin(),
    er = d->initial.end(), ir2;
  while( ir!=er )
    if( theAnatomist->hasObject( *ir ) )
      ++ir;
    else
      {
        ir2 = ir;
        ++ir;
        d->initial.erase( ir2 );
      }

  d->objsel->selectObjects( d->initial, _parents );
}


void QAPaletteWin::objectsChosen( const set<AObject *> & o )
{
  // cout << "objects chosen: " << o.size() << endl;
  runCommand();

  while( !_parents.empty() )
    (*_parents.begin())->deleteObserver( this );

  set<AObject *>::const_iterator	i, e = _parents.end();
  _parents = o;
  for( i=_parents.begin(); i!=e; ++i )
    (*i)->addObserver( this );

  updateInterface();
}


void QAPaletteWin::runCommand()
{
  if( d->objpal && d->modified && !_parents.empty() )
    {
      SetObjectPaletteCommand	*com;
      // convert to absolute values
      AObjectPalette *pal = objPalette();
      double omi, oma;
      if( d->objpal->zeroCenteredAxis1() )
      {
        double omax = std::max( std::abs( d->objMax ), std::abs( d->objMin ) );
        omi = pal->min1() * omax;
        oma = pal->max1() * omax;
      }
      else
      {
        omi = pal->min1() * ( double(d->objMax) - d->objMin ) + d->objMin;
        oma = pal->max1() * ( double(d->objMax) - d->objMin ) + d->objMin;
      }
      double omi2, oma2;
      if( d->objpal->zeroCenteredAxis2() )
      {
        double omax = std::max( std::abs( d->objMax2 ),
                                std::abs( d->objMin2 ) );
        omi2 = pal->min2() * omax;
        oma2 = pal->max2() * omax;
      }
      else
      {
        omi2 = pal->min2() * ( double(d->objMax2) - d->objMin2 ) + d->objMin2;
        oma2 = pal->max2() * ( double(d->objMax2) - d->objMin2 ) + d->objMin2;
      }

      if( d->objpal->refPalette2() )
      {
        com = new SetObjectPaletteCommand(
          _parents, d->objpal->refPalette()->name(),
          true, float(omi), true, float(oma),
          d->objpal->refPalette2()->name(),
          true, float(omi2), true, float(oma2),
          d->objpal->mixMethodName(), true,
          d->objpal->linearMixFactor(), "",
          true, -2, -2, true, d->objpal->zeroCenteredAxis1(), true,
          d->objpal->zeroCenteredAxis2() );
      }
      else
        com = new SetObjectPaletteCommand(
          _parents, d->objpal->refPalette()->name(),
          true, float(omi), true, float(oma),
          "", true, omi2,  true, oma2, "",
          false, 0.5, "", true, -2, -2, true, d->objpal->zeroCenteredAxis1(),
          true, d->objpal->zeroCenteredAxis2() );

      // pb: unnecessary command execution: should be only writen, not executed
      // because it has already been done before.
      theProcessor->execute( com );
    }
  d->modified = false;
}


void QAPaletteWin::fillToolBar()
{
  anatomist::internal::PaletteWinExtensionActions *ext 
    = anatomist::internal::PaletteWinExtensionActions::instance();
  QList<APaletteExtensionAction *> actions 
    = ext->findChildren<APaletteExtensionAction *>();
  if( actions.count() == 0 )
    d->toolbar->hide();
  else
  {
    QList<APaletteExtensionAction *>::iterator ia, ea = actions.end();
    for( ia=actions.begin(); ia!=ea; ++ia )
    {
      d->toolactions->addAction( *ia );
      d->toolbar->addAction( *ia );
    }
    d->toolbar->show();
  }
}


void QAPaletteWin::addExtensionAction( APaletteExtensionAction* action )
{
  anatomist::internal::PaletteWinExtensionActions *ext = anatomist::internal::PaletteWinExtensionActions::instance();
  action->setParent( ext );
}


void QAPaletteWin::extensionActionTriggered( QAction *action )
{
  APaletteExtensionAction* ac 
    = dynamic_cast<APaletteExtensionAction *>( action );
  if( ac )
    ac->extensionTriggered( d->objsel->selectedObjects() );
//     ac->emit extensionTriggered( d->objsel->selectedObjects() );
}


void QAPaletteWin::zeroCentered1Changed( int state )
{
  d->objpal->setZeroCenteredAxis1( bool( state ) );

  double absmax = std::max( std::abs( d->objMin ),
                            std::abs( d->objMax ) );
  float min, max;
  if( !state )
  {
    float maxval = d->objpal->max1() * absmax;
    float curmin = std::min( -maxval, d->objMin );
    float curmax = std::max( maxval, d->objMax );
    d->objpal->setMin1( (-maxval - d->objMin)
                        / ( double(d->objMax) - d->objMin ) );
    min = curmin;
    max = curmax;
    d->dimBox1->slrelmin = ( curmin - d->objMin )
      / ( double(d->objMax) - d->objMin );
    d->dimBox1->slrelmax = ( curmax - d->objMin )
      / ( double(d->objMax) - d->objMin );
  }
  else
  {
    float maxval = d->objMin
      + d->objpal->max1() * ( double(d->objMax) - d->objMin );
    d->objpal->setMin1( 0. );
    d->objpal->setMax1( maxval / absmax );
    d->dimBox1->slrelmin = 0.f;
    d->dimBox1->slrelmax = std::max( 1.f, std::abs( d->objpal->max1() ) );
    min = -absmax;
    max = absmax;
  }
  d->dimBox1->minEd->setText( QString::number( min ) );
  d->dimBox1->maxEd->setText( QString::number( max ) );
  d->dimBox1->paledit->setRange( min, max );
  d->dimBox1->paledit->updateDisplay();

  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::zeroCentered2Changed( int state )
{
  d->objpal->setZeroCenteredAxis2( bool( state ) );
  d->objpal->setMin2( 0. );

  if( !state )
  {
    d->dimBox2->slrelmin = float( -d->objMin2
      / ( double(d->objMax2) - d->objMin2 ) );
    d->dimBox2->minEd->setText( QString::number( d->objMin2 ) );
    d->dimBox2->slrelmin = 0.f;
  }
  else
  {
    double absmax = std::max( std::abs( d->objMin2 ),
                              std::abs( d->objMax2 ) );
    d->dimBox2->minEd->setText( QString::number( -absmax ) );
    d->dimBox2->slrelmin = 0.f;
  }

  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::palette1RangeChanged( float min, float max )
{
  bool changed = false;
  if( d->dimBox1->minEd->text() != QString::number( min ) )
  {
    d->dimBox1->minEd->setText( QString::number( min ) );
    min1EditChanged();
    changed = true;
  }
  if( d->dimBox1->maxEd->text() != QString::number( max ) )
  {
    d->dimBox1->maxEd->setText( QString::number( max ) );
    max1EditChanged();
    changed = true;
  }
  if( changed )
  {
    d->modified = true;
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::palette2RangeChanged( float min, float max )
{
  d->dimBox1->paledit->miniPaletteWidget()->miniPaletteGraphics()->setRange(
    min, max, 1 );
  d->dimBox1->paledit->miniPaletteWidget()->miniPaletteGraphics()
    ->updateDisplay();
  if( d->dimBox2->minEd->text() != QString::number( min ) )
  {
    d->dimBox2->minEd->setText( QString::number( min ) );
    min2EditChanged();
  }
  if( d->dimBox2->maxEd->text() != QString::number( min ) )
  {
    d->dimBox2->maxEd->setText( QString::number( max ) );
    max2EditChanged();
  }
}


void QAPaletteWin::cleatMin1()
{
  if( !_parents.empty() )
  {
    objPalette()->setAbsMin1( *_parents.begin(),
                              d->dimBox1->paledit->range().first );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::cleatMax1()
{
  if( !_parents.empty() )
  {
    objPalette()->setAbsMax1( *_parents.begin(),
                              d->dimBox1->paledit->range().second );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::cleatMin2()
{
  if( !_parents.empty() )
  {
    objPalette()->setAbsMin2( *_parents.begin(),
                              d->dimBox2->paledit->range().first );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::cleatMax2()
{
  if( !_parents.empty() )
  {
    objPalette()->setAbsMax2( *_parents.begin(),
                              d->dimBox2->paledit->range().second );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::min1Changed( float value )
{
  if( !_parents.empty()
      && objPalette()->absMin1( *_parents.begin() ) != value )
  {
    objPalette()->setAbsMin1( *_parents.begin(), value );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::max1Changed( float value )
{
  if( !_parents.empty()
      && objPalette()->absMax1( *_parents.begin() ) != value )
  {
    objPalette()->setAbsMax1( *_parents.begin(), value );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::min2Changed( float value )
{
  if( !_parents.empty()
      && objPalette()->absMin2( *_parents.begin() ) != value )
  {
    objPalette()->setAbsMin2( *_parents.begin(), value );
    if( d->responsive )
      updateObjects();
  }
}


void QAPaletteWin::max2Changed( float value )
{
  if( !_parents.empty()
      && objPalette()->absMax2( *_parents.begin() ) != value )
  {
    objPalette()->setAbsMax2( *_parents.begin(), value );
    if( d->responsive )
      updateObjects();
  }
}


// ----

APaletteExtensionAction::APaletteExtensionAction( QObject* parent )
  : QAction( parent )
{
}


APaletteExtensionAction::APaletteExtensionAction( const QString & text, 
                                                  QObject * parent )
  : QAction( text, parent )
{
}


APaletteExtensionAction::APaletteExtensionAction( const QIcon & icon, 
                                                  const QString & text, 
                                                  QObject * parent )
  : QAction( icon, text, parent )
{
}


APaletteExtensionAction::~APaletteExtensionAction()
{
}

// ----

namespace anatomist
{

  namespace internal
  {

    PaletteWinExtensionActions *& PaletteWinExtensionActions::_instance()
    {
      static PaletteWinExtensionActions *ins = 0;
      return ins;
    }


    PaletteWinExtensionActions * PaletteWinExtensionActions::instance()
    {
      PaletteWinExtensionActions *& ins = _instance();
      if( !ins )
        ins = new PaletteWinExtensionActions( qApp );
      return ins;
    }

  }

}

