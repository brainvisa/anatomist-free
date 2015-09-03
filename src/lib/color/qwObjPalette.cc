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
#include <qheaderview.h>


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

  bool dummy = dummyfunc();

}


struct QAPaletteWin::DimBox
{
  QGroupBox		*topBox;
  QSlider		*minSlider;
  QSlider		*maxSlider;
  QLabel		*minLabel;
  QLabel		*maxLabel;
  QLabel		*palView;
  QScopeLineEdit	*minEd;
  QScopeLineEdit	*maxEd;
  QPushButton		*autoValBtn;
  QPushButton		*autoBoundsBtn;
  float			slrelmin;
  float			slrelmax;
  QCheckBox             *symbox;
};


struct QAPaletteWin::Private
{
  Private( const set<AObject *> & );

  QTableWidget			*palettes;
  DimBox			*dimBox1;
  DimBox			*dimBox2;
  QLabel			*view;
  bool				responsive;
  anatomist::AObjectPalette	*objpal;
  unsigned			dim;
  QCheckBox                     *usepal2 ;
  QButtonGroup	                *dimBgp ;
  QComboBox			*palette2Box;
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
  mainLay->setMargin( 0 );
  mainLay->setSpacing( 0 );
  d->objsel = new ObjectParamSelect( obj, this );
  mainLay->addWidget( d->objsel );
  d->objsel->addFilter( filterPalette );
  d->objsel->layout()->setMargin( 5 );

  d->main = new QWidget( this );
  d->main->setObjectName( "mainLay" );
  QHBoxLayout *mainl = new QHBoxLayout( d->main );
  d->main->setLayout( mainl );
  mainLay->addWidget( d->main );
  mainl->setMargin( 5 );
  mainl->setSpacing( 5 );

  QWidget *ltPanel = new QWidget( d->main );
  mainl->addWidget( ltPanel );
  ltPanel->setObjectName( "ltPanel" );
  QVBoxLayout *ltPanell = new QVBoxLayout( ltPanel );
  ltPanel->setLayout( ltPanell );
  ltPanell->setMargin( 0 );
  ltPanell->setSpacing( 5 );

  d->toolbar = new QToolBar( ltPanel );
  d->toolbar->setObjectName( "palette toolbar" );
  ltPanell->addWidget( d->toolbar );
  d->toolactions = new QActionGroup( d->toolbar );
  fillToolBar();

  ltPanell->addWidget( new QLabel( tr( "Available palettes :" ), ltPanel ) );

  d->palettes = new QTableWidget( ltPanel );
  ltPanell->addWidget( d->palettes );
  d->palettes->setObjectName( "palettes" );
  d->palettes->setMinimumSize( 200, 200 );
  d->palettes->setSortingEnabled( true );
  d->palettes->setCornerButtonEnabled( false );
  d->palettes->setAlternatingRowColors( true );
  d->palettes->setSelectionBehavior( QTableWidget::SelectRows );
  d->palettes->setSelectionMode( QTableWidget::SingleSelection );
  d->palettes->setEditTriggers( QTableWidget::NoEditTriggers );
  d->palettes->verticalHeader()->setVisible( false );
  d->palettes->setIconSize( QSize( 64, 20 ) );
  fillPalettes();

  QWidget *rtPanel = new QWidget( d->main );
  rtPanel->setObjectName( "rtPanel" );
  mainl->addWidget( rtPanel );
  QVBoxLayout *rtPanell = new QVBoxLayout( rtPanel );
  rtPanel->setLayout( rtPanell );
  rtPanell->setMargin( 0 );
  rtPanell->setSpacing( 5 );

  QGroupBox	*updateGrp = new QGroupBox( tr( "Update mode :" ), rtPanel );
  rtPanell->addWidget( updateGrp );
  QHBoxLayout *updateGrpl = new QHBoxLayout( updateGrp );
  updateGrp->setLayout( updateGrpl );
  updateGrpl->setMargin( 5 );
  updateGrpl->setSpacing( 5 );
  QCheckBox	*respBtn = new QCheckBox( tr( "Responsive" ), updateGrp );
  updateGrpl->addWidget( respBtn );
  respBtn->setChecked( d->responsive );
  QPushButton	*updateBtn = new QPushButton( tr( "Update" ), updateGrp );
  updateGrpl->addWidget( updateBtn );
  updateGrp->setFixedHeight( updateGrp->sizeHint().height() );

  QGroupBox *dimBgpw = new QGroupBox( tr( "Dimension :" ), rtPanel );
  d->dimBgp = new QButtonGroup( dimBgpw );
  d->dimBgp->setExclusive( true );
  rtPanell->addWidget( dimBgpw );
  QHBoxLayout *dimBgpl = new QHBoxLayout( dimBgpw );
  dimBgpw->setLayout( dimBgpl );
  dimBgpl->setMargin( 5 );
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
  makeDimBox( tr( "1st dimension settings :" ), rtPanel, d->dimBox1 );
  rtPanell->addWidget( d->dimBox1->topBox );

  QWidget *pal2Panel = new QWidget( d->main );
  mainl->addWidget( pal2Panel );
  pal2Panel->setObjectName( "pal2Panel" );
  QVBoxLayout *pal2Panell = new QVBoxLayout( pal2Panel );
  pal2Panel->setLayout( pal2Panell );
  pal2Panell->setMargin( 0 );
  pal2Panell->setSpacing( 5 );

  d->dimBox2 = new DimBox;
  makeDimBox( tr( "2nd dimension settings :" ), pal2Panel, d->dimBox2 );
  pal2Panell->addWidget( d->dimBox2->topBox );

  d->usepal2 = new QCheckBox( tr( "Use a second palette for 2D" ),
                              d->dimBox2->topBox );
  d->dimBox2->topBox->layout()->addWidget( d->usepal2 );
  d->pal2SetBox = new QWidget( d->dimBox2->topBox );
  d->dimBox2->topBox->layout()->addWidget( d->pal2SetBox );
  QVBoxLayout *pal2SetBoxl = new QVBoxLayout( d->pal2SetBox );
  d->pal2SetBox->setLayout( pal2SetBoxl );
  pal2SetBoxl->setMargin( 5 );
  pal2SetBoxl->setSpacing( 5 );
  QWidget *pal2box = new QWidget( d->pal2SetBox );
  pal2SetBoxl->addWidget( pal2box );
  QGridLayout *pal2boxl = new QGridLayout( pal2box );
  pal2box->setLayout( pal2boxl );
  pal2boxl->setMargin( 0 );
  pal2boxl->setSpacing( 5 );
  pal2boxl->addWidget( new QLabel( tr( "Second palette :" ), pal2box ), 0, 0 );
  d->palette2Box = new QComboBox( pal2box );
  pal2boxl->addWidget( d->palette2Box, 0, 1 );
  d->palette2Box->setObjectName( "second_palette" );
  d->palette2Box->setSizePolicy( 
    QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
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

  fillPalette2List();
  fillMixMethods();
  fillPalette1DMappingMethods() ;

  QWidget *mixFacBox = new QWidget( d->pal2SetBox );
  pal2SetBoxl->addWidget( mixFacBox );
  QHBoxLayout *mixFacBoxl = new QHBoxLayout( mixFacBox );
  mixFacBox->setLayout( mixFacBoxl );
  mixFacBoxl->setMargin( 0 );
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

  QGroupBox	*dispGp = new QGroupBox( tr( "Palette view :" ), rtPanel );
  rtPanell->addWidget( dispGp );
  QVBoxLayout	*dispGpLay = new QVBoxLayout( dispGp );
  dispGpLay->setMargin( 5 );
  dispGpLay->setSpacing( 5 );
  d->view = new QLabel( dispGp );
  dispGpLay->addWidget( d->view );
  d->view->setFrameStyle( QFrame::Sunken | QFrame::Box );

  updateInterface();

  connect( d->dimBgp, SIGNAL( buttonClicked( int ) ), this,
           SLOT( dimChanged( int ) ) );
  connect( d->palettes, SIGNAL( itemSelectionChanged() ),
           this, SLOT( palette1Changed() ) );
  connect( d->dimBox1->minSlider, SIGNAL( valueChanged( int ) ),
	   this, SLOT( min1Changed( int ) ) );
  connect( d->dimBox1->maxSlider, SIGNAL( valueChanged( int ) ),
	   this, SLOT( max1Changed( int ) ) );
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

  connect( d->dimBox2->minSlider, SIGNAL( valueChanged( int ) ),
	   this, SLOT( min2Changed( int ) ) );
  connect( d->dimBox2->maxSlider, SIGNAL( valueChanged( int ) ),
	   this, SLOT( max2Changed( int ) ) );
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

  connect( respBtn, SIGNAL( toggled( bool ) ),
	   this, SLOT( responsiveToggled( bool ) ) );
  connect( updateBtn, SIGNAL( clicked() ), this, SLOT( updateClicked() ) );

  connect( d->palette2Box, SIGNAL( activated( const QString & ) ),
	   this, SLOT( palette2Changed( const QString & ) ) );
  connect( d->mixBox, SIGNAL( activated( const QString & ) ),
	   this, SLOT( mixMethodChanged( const QString & ) ) );
  connect( d->palette1dMappingBox, SIGNAL( activated( const QString & ) ),
	   this, SLOT( palette1DMappingMethodChanged( const QString & ) ) );
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

  if( objPalette()->palette1DMapping() == AObjectPalette::DIAGONAL )
    d->dimBgp->button(1)->setChecked( true );

  show();
  d->sizes[0][0] = 0;
  d->sizes[0][1] = 0;
  d->sizes[1][0] = 0;
  d->sizes[1][1] = 0;
}


QWidget* QAPaletteWin::makeDimBox( const QString & title, QWidget* parent,
				   struct DimBox* dbox )
{
  dbox->topBox = new QGroupBox( title, parent );
  QVBoxLayout *topBoxl = new QVBoxLayout( dbox->topBox );
  dbox->topBox->setLayout( topBoxl );
  topBoxl->setMargin( 5 );
  topBoxl->setSpacing( 5 );

  QWidget *minmaxbox = new QWidget( dbox->topBox );
  topBoxl->addWidget( minmaxbox );
  QGridLayout *minmaxboxl = new QGridLayout( minmaxbox );
  minmaxbox->setLayout( minmaxboxl );
  minmaxboxl->setMargin( 0 );
  minmaxboxl->setSpacing( 5 );
  minmaxboxl->addWidget( new QLabel( tr( "Min:" ), minmaxbox ), 0, 0 );
  dbox->minSlider = new QSlider( minmaxbox );
  minmaxboxl->addWidget( dbox->minSlider, 0, 1 );
  dbox->minSlider->setRange( 0, 1000 );
  dbox->minSlider->setValue( 0 );
  dbox->minSlider->setOrientation( Qt::Horizontal );
  dbox->minSlider->setObjectName( "minSlider" );
  dbox->minSlider->setSingleStep( 1 );
  dbox->minLabel = new QLabel( "0", minmaxbox );
  minmaxboxl->addWidget( dbox->minLabel, 0, 2 );
  dbox->minLabel->setObjectName( "minLabel" );
  dbox->minLabel->setMinimumWidth( 50 );

  minmaxboxl->addWidget( new QLabel( tr( "Max:" ), minmaxbox ), 1, 0 );
  dbox->maxSlider = new QSlider( minmaxbox );
  dbox->maxSlider->setRange( 0, 1000 );
  dbox->maxSlider->setValue( 1000 );
  dbox->maxSlider->setObjectName( "maxSlider" );
  dbox->maxSlider->setOrientation( Qt::Horizontal );
  minmaxboxl->addWidget( dbox->maxSlider, 1, 1 );
  dbox->maxSlider->setSingleStep( 1 );
  dbox->maxLabel = new QLabel( "1", minmaxbox );
  dbox->maxLabel->setObjectName( "max1dlabel" );
  minmaxboxl->addWidget( dbox->maxLabel, 1, 2 );

  QWidget *boundsbox = new QWidget( dbox->topBox );
  topBoxl->addWidget( boundsbox );
  QHBoxLayout *boundsboxl = new QHBoxLayout( boundsbox );
  boundsbox->setLayout( boundsboxl );
  boundsboxl->setMargin( 0 );
  boundsboxl->setSpacing( 5 );
  boundsboxl->addWidget( new QLabel( tr( "Bounds:" ), boundsbox ) );
  dbox->minEd = new QScopeLineEdit( "0", boundsbox, "minEd" );
  boundsboxl->addWidget( dbox->minEd );
  dbox->minEd->setMinimumWidth( 50 );
  dbox->maxEd = new QScopeLineEdit( "1", boundsbox, "maxEd" );
  boundsboxl->addWidget( dbox->maxEd );
  dbox->maxEd->setMinimumWidth( 50 );

  dbox->symbox = new QCheckBox( tr( "Value 0 at center" ), dbox->topBox );
  topBoxl->addWidget( dbox->symbox );

  QWidget *autobox = new QWidget( dbox->topBox );
  topBoxl->addWidget( autobox );
  QHBoxLayout *autoboxl = new QHBoxLayout( autobox );
  autobox->setLayout( autoboxl );
  autoboxl->setMargin( 0 );
  autoboxl->setSpacing( 5 );
  dbox->autoValBtn = new QPushButton( tr( "Reset values" ), autobox );
  autoboxl->addWidget( dbox->autoValBtn );
  dbox->autoBoundsBtn = new QPushButton( tr( "Reset bounds" ), autobox );
  autoboxl->addWidget( dbox->autoBoundsBtn );

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


void QAPaletteWin::fillPalettes()
{
  const list<rc_ptr<APalette> >	& pal = theAnatomist->palettes().palettes();
  list<rc_ptr<APalette> >::const_iterator	ip, fp=pal.end();
  int				i = 0;

  d->palettes->setRowCount( pal.size() );
  d->palettes->setColumnCount( 2 );
  d->palettes->setHorizontalHeaderItem( 
    0, new QTableWidgetItem( tr( "name" ) ) );
  d->palettes->setHorizontalHeaderItem( 
    1, new QTableWidgetItem( tr( "look" ) ) );
  map<string, int> cols;
  for( i=0, ip=pal.begin(); ip!=fp; ++ip, ++i )
  {
    QTableWidgetItem *item = new QTableWidgetItem( (*ip)->name().c_str() );
    d->palettes->setItem( i, 0, item );
    // icon
    QPixmap pix;
    fillPalette( *ip, pix );
    QTableWidgetItem *iconitem = new QTableWidgetItem;
    iconitem->setIcon( QIcon( pix ) );
    d->palettes->setItem( i, 1, iconitem );
    const PythonHeader *ph 
      = dynamic_cast<const PythonHeader *>( (*ip)->header() );
    if( ph && ph->hasProperty( "display_properties" ) )
    {
      // fill columns
      Object props = ph->getProperty( "display_properties" );
      Object piter = props->objectIterator();
      while( piter->isValid() )
      {
        string pname = piter->key();
        string pval;
        try
        {
          pval = piter->currentValue()->getString();
        }
        catch( ... )
        {
          piter->next();
          continue;
        }
        map<string, int>::iterator ic = cols.find( pname );
        int col;
        if( ic == cols.end() )
        {
          col = d->palettes->columnCount();
          cols[ pname ] = col;
          d->palettes->setColumnCount( col + 1 );
          d->palettes->setHorizontalHeaderItem( 
            col, new QTableWidgetItem( tr( pname.c_str() ) ) );
        }
        else
          col = ic->second;
        d->palettes->setItem( i, col, new QTableWidgetItem( pval.c_str() ) );
        piter->next();
      }
    }
  }
  d->palettes->setColumnWidth( 0, 160 );
  d->palettes->resizeColumnToContents( 1 );
  int ncol = d->palettes->columnCount();
  for( i=2; i<ncol; ++i )
    d->palettes->setColumnWidth( i, 20 );
//   cout << "pal width: " << d->palettes->width() << endl;
//   cout << "sizehint: " << d->palettes->sizeHint().width() << endl;
//   d->palettes->resizeRowsToContents();
//   if( d->palettes->columnWidth( 0 )
//       > d->palettes->sizeHint().width() - d->palettes->columnWidth( 1 ) )
//     d->palettes->setColumnWidth(
//       0, d->palettes->sizeHint().width() - d->palettes->columnWidth( 1 ) );
}


void QAPaletteWin::fillPalette2List()
{
  const list<rc_ptr<APalette> >	& pal = theAnatomist->palettes().palettes();
  list<rc_ptr<APalette> >::const_iterator	ip, fp=pal.end();
  int				i = 0;

  int w = d->palette2Box->width();
  d->palette2Box->addItem( tr( "None" ) );

  d->palette2Box->setCurrentIndex( 0 );

  for( i=0, ip=pal.begin(); ip!=fp; ++ip, ++i )
    d->palette2Box->addItem( (*ip)->name().c_str() );
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
    if( !&pallist || pallist.size() == 0 )
    {
      cerr << "Palettes list empty. Set your ANATOMIST_PATH variable\n";
      return;
    }
    d->objpal = new AObjectPalette( *pallist.palettes().begin() );
  }
  else
    d->objpal = d->objpal->clone();

  if( !o )
    return;

  if( d->objpal->is2dMode() )
    d->dim = 2;
  //cout << "using palette " << d->objpal->refPalette()->name() << endl;

  d->mixSlid->blockSignals( true );
  d->dimBgp->blockSignals( true );
  d->palettes->blockSignals( true );
  d->palette2Box->blockSignals( true );
  d->mixBox->blockSignals( true );

  d->objsel->updateLabel( _parents );
  d->dimBgp->button( d->dim - 1 )->setChecked( true );
  int i, n = d->palettes->rowCount();
  QString	name = d->objpal->refPalette()->name().c_str();
  QList<QTableWidgetItem *> items = d->palettes->findItems(
    name, Qt::MatchCaseSensitive | Qt::MatchExactly );
  QTableWidgetItem *item = 0;
  if( items.count() != 0 )
    item = items.front();
  if( item )
    d->palettes->setCurrentItem( item );
  else
  {
    d->palettes->setCurrentItem( 0 );
    items = d->palettes->selectedItems();
    if( items.count() != 0 )
      item = items.front();
    if( item )
      item->setSelected( false );
  }

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

  if( objPalette()->refPalette2() )
  {
    if( d->objpal->refPalette2() )
    {
      d->usepal2->setChecked( true );
      n = d->palette2Box->count();
      for( i=0; i<n; ++i )
        if( d->palette2Box->itemText( i )
            == d->objpal->refPalette2()->name().c_str() )
          {
            d->palette2Box->setCurrentIndex( i );
            break;
          }
    }
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

  fillObjPal();

  d->mixSlid->blockSignals( false );
  d->dimBgp->blockSignals( false );
  d->palettes->blockSignals( false );
  d->palette2Box->blockSignals( false );
  d->mixBox->blockSignals( false );
}


void QAPaletteWin::update( const anatomist::Observable* obs, void* )
{
  if (d->updatingFlag)
    return ;
  d->updatingFlag = true ;

  //cout << "QAPaletteWin::update\n";

  const AObject * obj = dynamic_cast<const AObject*>( obs );
  if( ( !obj || !theAnatomist->hasObject( obj ) ) && objects().size() == 1 )
    obj = *objects().begin();

  if( obj && theAnatomist->hasObject( obj ) && obj->hasChanged() )
  {
    const AObjectPalette	*pal = obj->getOrCreatePalette();
    if( pal )
    {
      if( d->objpal )
        *d->objpal = *pal ;
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

      QList<QTableWidgetItem *> palitems = d->palettes->findItems
          ( d->objpal->refPalette()->name().c_str(),
            Qt::MatchCaseSensitive | Qt::MatchExactly );
      QTableWidgetItem *palitem = 0;
      if( palitems.count() != 0 )
        palitem = palitems.front();
      if( !palitem )
        fillPalette1();

      d->palettes->setCurrentItem( palitem );

      if ( d->objpal->refPalette2() )
      {
        d->usepal2->setChecked( true );
        int i, n=d->palette2Box->count();
        for( i=0; i<n; ++i )
          if( d->palette2Box->itemText( i )
              == d->objpal->refPalette2()->name().c_str() )
          {
            d->palette2Box->setCurrentIndex( i );
            break;
          }
      }
    }

    const GLComponent	*gl = obj->glAPI();
    if( gl )
    {
      if( gl->glNumTextures() > 0 )
      {
        const GLComponent::TexExtrema	& te = gl->glTexExtrema( 0 );
        if( !te.minquant.empty() )
          d->objMin = te.minquant[0];
        if( !te.maxquant.empty() )
          d->objMax = te.maxquant[0];
        if( d->objMin == d->objMax )
          d->objMax = d->objMin + 1;	// avoid pbs of division by 0
        setValues( d->dimBox1, d->objpal->min1(), d->objpal->max1(),
                    d->objMin, d->objMax );
        setValues( d->dimBox2, d->objpal->min2(), d->objpal->max2(),
                    d->objMin2, d->objMax2 );
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
  fillObjPal();

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


void QAPaletteWin::fillPalette( const rc_ptr<APalette> pal, QPixmap & pm )
{
  unsigned		dimpx = pal->dimX(), dimpy = pal->dimY();
  unsigned		dimx = dimpx, dimy = dimpy;
  unsigned		x, y;

  if( dimy < 32 )
    dimy = 32;
  if( dimx > 256 )
    dimx = 256;
  else if( dimx == 0 )
    dimx = 1;
  if( dimy > 256 )
    dimy = 256;

  float		facx = ((float) dimpx) / dimx;
  float		facy = ((float) dimpy) / dimy;
  AimsRGBA	rgb;

  QImage	im( dimx, dimy, QImage::Format_ARGB32 );

  for( y=0; y<dimy; ++y )
    for( x=0; x<dimx; ++x )
    {
      rgb = (*pal)( (unsigned) ( facx * x), (unsigned) ( facy * y ) );
      im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
    }
#if QT_VERSION < 0x040700
  // convertFromImage is in Qt3Support before officialized in Qt 4.7
  pm = QPixmap::fromImage( im, Qt::AutoColor );
#else
  pm.convertFromImage( im );
#endif
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

  fillPalette( pal, pm );

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
      fillPalette( pal, pm );
      d->dimBox2->palView->setPixmap( pm );
    }
  }
  d->dimBox2->palView->setPixmap( pm );
}


void QAPaletteWin::fillObjPal()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  QImage *im = objpal->toQImage();
  QPixmap pm;
#if QT_VERSION < 0x040700
  // convertFromImage is in Qt3Support before officialized in Qt 4.7
  pm = QPixmap::fromImage( *im, Qt::AutoColor );
#else
  pm.convertFromImage( *im );
#endif
  delete im;
  d->view->setPixmap( pm );
  d->view->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  d->view->setScaledContents( true );
}


void QAPaletteWin::palette1Changed()
{
  if( d->recursive )
    return;

  QTableWidgetItem* item = 0;
  QList<QTableWidgetItem *> selected = d->palettes->selectedItems();
  if( selected.count() == 0 )
    return;
  item = selected.front();

  string		name = item->text().toStdString();
  PaletteList		& pallist = theAnatomist->palettes();
  const rc_ptr<APalette> pal = pallist.find( name );

  if( !pal )
    return;

  AObjectPalette	*objpal = objPalette();

  if( !objpal )
    return;

  objpal->setRefPalette( pal );
  fillPalette1();
  updateObjPal();
  fillObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::palette2Changed( const QString & palname )
{
  if( d->recursive )
    return;

  AObjectPalette	*objpal = objPalette();

  if( palname == tr( "None" ) )
    {
      objpal->setRefPalette2( rc_ptr<APalette>() );
    }
  else
    {
      PaletteList	& pallist = theAnatomist->palettes();
      const rc_ptr<APalette>
        pal = pallist.find( string( palname.toStdString() ) );

      if( !pal )
	{
	  cerr << "Palette not found\n";
	  return;
	}

      if( !objpal )
	return;

      objpal->setRefPalette2( pal );
    }
  fillPalette2();
  updateObjPal();
  fillObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::setValues( DimBox* dimBox, float m, float M,
			      float objMin, float objMax )
{
  bool rec = d->recursive;
  d->recursive = true;

  dimBox->minSlider->blockSignals( true );
  dimBox->maxSlider->blockSignals( true );

  float	min, max;

  if( m < M )
    {
      min = m;
      max = M;
    }
  else
    {
      min = M;
      max = m;
    }
  dimBox->minSlider->setValue( (int) ( ( m - dimBox->slrelmin ) * 1000
                                       / ( dimBox->slrelmax
                                           - dimBox->slrelmin ) ) );
  dimBox->maxSlider->setValue( (int) ( ( M - dimBox->slrelmin ) * 1000
                                       / ( dimBox->slrelmax
                                           - dimBox->slrelmin ) ) );
  min = objMin + ( objMax - objMin ) * m;
  max = objMin + ( objMax - objMin ) * M;
  dimBox->minLabel->setText( QString::number( min ) );
  dimBox->maxLabel->setText( QString::number( max ) );
  min = objMin + ( objMax - objMin ) * dimBox->slrelmin;
  max = objMin + ( objMax - objMin ) * dimBox->slrelmax;
  dimBox->minEd->setText( QString::number( min ) );
  dimBox->maxEd->setText( QString::number( max ) );

  dimBox->minSlider->blockSignals( false );
  dimBox->maxSlider->blockSignals( false );

  d->recursive = rec;
}


void QAPaletteWin::setValues1()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  float	m = objpal->min1(), M = objpal->max1();

  setValues( d->dimBox1, m, M, d->objMin, d->objMax );
}


void QAPaletteWin::setValues2()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  float	m = objpal->min2(), M = objpal->max2();

  setValues( d->dimBox2, m, M, d->objMin2, d->objMax2 );
}


void QAPaletteWin::min1Changed( int value )
{
  if( d->recursive )
    return;

  // d->recursive = true;
  float	relval = d->dimBox1->slrelmin + 0.001 * value
    * ( d->dimBox1->slrelmax - d->dimBox1->slrelmin );

  float	min = d->objMin + ( d->objMax - d->objMin ) * relval;

  d->dimBox1->minLabel->setText( QString::number( min ) );

  bool sym = d->dimBox1->symbox->isChecked();
  float relval2 = 0;
  if( sym )
  {
    relval2 = ( -min - d->objMin ) / ( d->objMax - d->objMin );
    int val2 = (int) rint( ( relval2 - d->dimBox1->slrelmin ) * 1000
      / ( d->dimBox1->slrelmax - d->dimBox1->slrelmin ) );
    d->dimBox1->maxSlider->setValue( val2 );
    d->dimBox1->maxLabel->setText( QString::number( -min ) );
  }

  AObjectPalette	*objpal = objPalette();

  if( objpal )
    {
      objpal->setMin1( relval );
      if( sym )
        objpal->setMax1( relval2 );
      fillObjPal();
    }
  d->modified = true;
  if( d->responsive )
    updateObjects();
  // d->recursive = false;
}


void QAPaletteWin::max1Changed( int value )
{
  if( d->recursive )
    return;

  float	relval = d->dimBox1->slrelmin + 0.001 * value
    * ( d->dimBox1->slrelmax - d->dimBox1->slrelmin );

  float	max = d->objMin + ( d->objMax - d->objMin ) * relval;

  d->dimBox1->maxLabel->setText( QString::number( max ) );

  bool sym = d->dimBox1->symbox->isChecked();
  float relval2 = 0;
  if( sym )
  {
    relval2 = ( -max - d->objMin ) / ( d->objMax - d->objMin );
    int val2 = (int) rint( ( relval2 - d->dimBox1->slrelmin ) * 1000
      / ( d->dimBox1->slrelmax - d->dimBox1->slrelmin ) );
    d->dimBox1->minSlider->setValue( val2 );
    d->dimBox1->minLabel->setText( QString::number( -max ) );
  }

  AObjectPalette	*objpal = objPalette();

  if( objpal )
    {
      objpal->setMax1( relval );
      if( sym )
        objpal->setMin1( relval2 );
      fillObjPal();
    }
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::min2Changed( int value )
{
  if( d->recursive )
    return;

  float	relval = d->dimBox2->slrelmin + 0.001 * value
    * ( d->dimBox2->slrelmax - d->dimBox2->slrelmin );

  d->dimBox2->minLabel->setText( QString::number( relval ) );

  bool sym = d->dimBox2->symbox->isChecked();
  float relval2 = 0;
  if( sym )
  {
    relval2 = 1. - relval;
    int val2 = (int) rint( ( relval2 - d->dimBox2->slrelmin ) * 1000
      / ( d->dimBox2->slrelmax - d->dimBox2->slrelmin ) );
    d->dimBox2->maxSlider->setValue( val2 );
    d->dimBox2->maxLabel->setText( QString::number( relval2 ) );
  }

  AObjectPalette	*objpal = objPalette();

  if( objpal )
    {
      objpal->setMin2( relval );
      if( sym )
        objpal->setMax2( relval2 );
      fillObjPal();
    }
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::max2Changed( int value )
{
  if( d->recursive )
    return;

  float	relval = d->dimBox2->slrelmin + 0.001 * value
    * ( d->dimBox2->slrelmax - d->dimBox2->slrelmin );

  d->dimBox2->maxLabel->setText( QString::number( relval ) );

  bool sym = d->dimBox2->symbox->isChecked();
  float relval2 = 0;
  if( sym )
  {
    relval2 = 1. - relval;
    int val2 = (int) rint( ( relval2 - d->dimBox2->slrelmin ) * 1000
      / ( d->dimBox2->slrelmax - d->dimBox2->slrelmin ) );
    d->dimBox2->minSlider->setValue( val2 );
    d->dimBox2->minLabel->setText( QString::number( relval2 ) );
  }

  AObjectPalette	*objpal = objPalette();

  if( objpal )
    {
      objpal->setMax2( relval );
      if( sym )
        objpal->setMin2( relval2 );
      fillObjPal();
    }
  d->modified = true;
  if( d->responsive )
    updateObjects();
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

  set<AObject *>::const_iterator	io, fo = obj.end();
  AObjectPalette			*pal = objPalette();
  float					mi, ma, omi, oma;
  AObject				*o;

  // convert to absolute values
  omi = pal->min1() * ( d->objMax - d->objMin ) + d->objMin;
  oma = pal->max1() * ( d->objMax - d->objMin ) + d->objMin;

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
            if( mi == ma )	// protect against division by 0
              ma = mi + 1;
            // convert to object scale
            AObjectPalette op = *pal;
            op.setMin1( ( omi - mi ) / ( ma - mi ) );
            op.setMax1( ( oma - mi ) / ( ma - mi ) );
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
    d->dimBox2->topBox->show();
  else if( d->dim == 1 )
    d->dimBox2->topBox->hide();

  updateObjPal();
  fillObjPal();
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
  unsigned		dimx = pal->dimX(), dimy = pal->dimY();
  unsigned		dimxmax = 256, dimymax = 256;

  if( d->dim <= 1 )
    dimy = 1;
  else if( pal2 )
    {
      dimy = pal2->dimX();
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


void QAPaletteWin::mixMethodChanged( const QString & methname )
{
  if( d->recursive )
    return;

  AObjectPalette	*objpal = objPalette();

  objpal->setMixMethod( string( methname.toStdString() ) );
  updateObjPal();
  fillObjPal();
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
QAPaletteWin::palette1DMappingMethodChanged( const QString & methname )
{
  if( d->recursive )
    return;

  AObjectPalette	*objpal = objPalette();

  if( methname == "FirstLine" )
    objpal->setPalette1DMapping( AObjectPalette::FIRSTLINE ) ;
  else
    objpal->setPalette1DMapping( AObjectPalette::DIAGONAL ) ;

  updateObjPal();
  fillObjPal();
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
      d->palette2Box->setCurrentIndex( 0 );
      fillPalette2();
      updateObjPal();
      fillObjPal();
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
  fillObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}


void QAPaletteWin::min1EditChanged()
{
  if( d->recursive )
    return;

  float	val = d->dimBox1->minEd->text().toFloat();
  float curval = float( d->dimBox1->minSlider->value() ) * 0.001
    * ( d->dimBox1->slrelmax - d->dimBox1->slrelmin )
    + d->dimBox1->slrelmin;
  d->dimBox1->slrelmin = (val - d->objMin ) / ( d->objMax - d->objMin );
  int	ival = (int) rint( ( curval - d->dimBox1->slrelmin ) * 1000
                           / ( d->dimBox1->slrelmax - d->dimBox1->slrelmin ) );
  d->dimBox1->minSlider->setValue( ival );
}


void QAPaletteWin::max1EditChanged()
{
  if( d->recursive )
    return;

  float	val = d->dimBox1->maxEd->text().toFloat();
  float curval = float( d->dimBox1->maxSlider->value() ) * 0.001
    * ( d->dimBox1->slrelmax - d->dimBox1->slrelmin )
    + d->dimBox1->slrelmin;
  d->dimBox1->slrelmax = (val - d->objMin ) / ( d->objMax - d->objMin );
  int	ival = (int) rint( ( curval - d->dimBox1->slrelmin ) * 1000
                           / ( d->dimBox1->slrelmax - d->dimBox1->slrelmin ) );
  d->dimBox1->maxSlider->setValue( ival );
}


void QAPaletteWin::min2EditChanged()
{
  if( d->recursive )
    return;

  float	val = d->dimBox2->minEd->text().toFloat();
  float curval = float( d->dimBox2->minSlider->value() ) * 0.001
    * ( d->dimBox2->slrelmax - d->dimBox2->slrelmin )
    + d->dimBox2->slrelmin;
  d->dimBox2->slrelmin = val;
  int	ival = (int) rint( ( curval - d->dimBox2->slrelmin ) * 1000
                           / ( d->dimBox2->slrelmax - d->dimBox2->slrelmin ) );
  d->dimBox2->minSlider->setValue( ival );
}


void QAPaletteWin::max2EditChanged()
{
  if( d->recursive )
    return;

  float	val = d->dimBox2->maxEd->text().toFloat();
  float curval = float( d->dimBox2->maxSlider->value() ) * 0.001
    * ( d->dimBox2->slrelmax - d->dimBox2->slrelmin )
    + d->dimBox2->slrelmin;
  d->dimBox2->slrelmax = val;
  int	ival = (int) rint( ( curval - d->dimBox2->slrelmin ) * 1000
                           / ( d->dimBox2->slrelmax - d->dimBox2->slrelmin ) );
  d->dimBox2->maxSlider->setValue( ival );
}


void QAPaletteWin::resetValues1()
{
  AObject	*obj = *_parents.begin();
  obj->adjustPalette();
  AObjectPalette	*op = objPalette(),*rp = obj->palette();
  op->setMin1( rp->min1() );
  op->setMax1( rp->max1() );
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
      float omi = pal->min1() * ( d->objMax - d->objMin ) + d->objMin;
      float oma = pal->max1() * ( d->objMax - d->objMin ) + d->objMin;
      float omi2 = pal->min2() * ( d->objMax2 - d->objMin2 ) + d->objMin2;
      float oma2 = pal->max2() * ( d->objMax2 - d->objMin2 ) + d->objMin2;

      if( d->objpal->refPalette2() )
      {
        com = new SetObjectPaletteCommand( _parents,
					   d->objpal->refPalette()->name(),
					   true, omi, true, oma,
					   d->objpal->refPalette2()->name(),
					   true, omi2, true, oma2,
					   d->objpal->mixMethodName(), true,
                                           d->objpal->linearMixFactor(), "",
                                           true );
      }
      else
	com = new SetObjectPaletteCommand( _parents,
					   d->objpal->refPalette()->name(),
					   true, omi, true, oma,
					   "", true, omi2,  true, oma2, "",
                                           false, 0.5, "", true );

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

