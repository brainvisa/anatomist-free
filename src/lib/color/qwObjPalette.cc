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
#include <aims/qtcompat/qgroupbox.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <aims/qtcompat/qhbuttongroup.h>
#include <aims/qtcompat/qhbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <aims/qtcompat/qlistbox.h>
#include <qlabel.h>
#include <aims/qtcompat/qvbox.h>
#include <aims/qtcompat/qgrid.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qcombobox.h>


using namespace anatomist;
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
  QVGroupBox		*topBox;
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

  QListBox			*palettes;
  DimBox			*dimBox1;
  DimBox			*dimBox2;
  QLabel			*view;
  bool				responsive;
  anatomist::AObjectPalette	*objpal;
  unsigned			dim;
  QCheckBox                     *usepal2 ;
  QHButtonGroup	                *dimBgp ;
  QComboBox			*palette2Box;
  QComboBox			*mixBox;
  QSlider	                *mixSlid ;
  QLabel			*mixValueLabel;
  QComboBox			*palette1dMappingBox ;
  QVBox				*pal2SetBox;
  float				objMin;
  float				objMax;
  float				objMin2;
  float				objMax2;
  bool				recursive;
  unsigned			sizes[2][2];
  bool                          updatingFlag;
  ObjectParamSelect		*objsel;
  set<AObject *>		initial;
  QHBox				*main;
  bool				modified;
};


QAPaletteWin::Private::Private( const set<AObject *> & o )
  : responsive( true ), objpal( 0 ), dim( 1 ), objMin( 0 ), objMax( 1 ),
    objMin2( 0 ), objMax2( 1 ), recursive( false ), updatingFlag( false ),
    initial( o ), main( 0 ), modified( false )
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
  : QWidget( 0 ), APaletteWin( obj ), d( new Private( obj ) )
{
  setCaption( tr( "Object palette composition" ) );
  QPixmap	anaicon( ( Settings::globalPath()
			   + "/icons/icon.xpm" ).c_str() );
  if( !anaicon.isNull() )
    setIcon( anaicon );

  QVBoxLayout	*mainLay = new QVBoxLayout( this, 10, 10, "mainLay" );
  d->objsel = new ObjectParamSelect( obj, this );
  mainLay->addWidget( d->objsel );
  d->objsel->addFilter( filterPalette );

  d->main = new QHBox( this, "mainLay" );
  mainLay->addWidget( d->main );
  d->main->setSpacing( 10 );

  QVBox	*ltPanel = new QVBox( d->main, "ltPanel" );
  ltPanel->setSpacing( 10 );

  new QLabel( tr( "Available palettes :" ), ltPanel );

  d->palettes = new QListBox( ltPanel, "palettes" );
  d->palettes->setMinimumSize( 200, 200 );
  fillPalettes();

  QFrame	*rtPanel = new QVBox( d->main, "rtPanel" );

  QHGroupBox	*updateGrp = new QHGroupBox( tr( "Update mode :" ), rtPanel );
  QCheckBox	*respBtn = new QCheckBox( tr( "Responsive" ), updateGrp );
  respBtn->setChecked( d->responsive );
  QPushButton	*updateBtn = new QPushButton( tr( "Update" ), updateGrp );
  updateGrp->setFixedHeight( updateGrp->sizeHint().height() );

  d->dimBgp = new QHButtonGroup( tr( "Dimension :" ), rtPanel );
  new QRadioButton( tr( "1D" ), d->dimBgp, "btn1D" );
  new QRadioButton( tr( "2D" ), d->dimBgp, "btn2D" );
  d->dimBgp->setFixedHeight( d->dimBgp->sizeHint().height() );

  d->dimBox1 = new DimBox;
  makeDimBox( tr( "1st dimension settings :" ), rtPanel, d->dimBox1 );

  QVBox	*pal2Panel = new QVBox( d->main, "pal2Panel" );

  d->dimBox2 = new DimBox;
  makeDimBox( tr( "2nd dimension settings :" ), pal2Panel, d->dimBox2 );

  d->usepal2 = new QCheckBox( tr( "Use a second palette for 2D" ),
					  d->dimBox2->topBox );
  d->pal2SetBox = new QVBox( d->dimBox2->topBox );
  d->pal2SetBox->setSpacing( 10 );
  QGrid	*pal2box = new QGrid( 2, d->pal2SetBox );
  pal2box->setSpacing( 5 );
  new QLabel( tr( "Second palette :" ), pal2box );
  d->palette2Box = new QComboBox( pal2box, "second_palette" );
  new QLabel( tr( "Palettes mixing method :" ), pal2box );
  d->mixBox = new QComboBox( pal2box, "2d_mixing_method" );
  new QLabel( tr( "Palette 1D Mapping :" ), pal2box );
  d->palette1dMappingBox = new QComboBox( pal2box,
                                          "palette_1d_mapping_method" );

  fillPalette2List();
  fillMixMethods();
  fillPalette1DMappingMethods() ;

  QHBox		*mixFacBox = new QHBox( d->pal2SetBox );
  mixFacBox->setSpacing( 10 );
  new QLabel( tr( "Mixing factor :" ), mixFacBox );
  d->mixSlid = new QSlider( 0, 100, 1, 0, Qt::Horizontal,
					mixFacBox, "mixSlid" );
  d->mixValueLabel = new QLabel( "100", mixFacBox );
  d->mixValueLabel->setMinimumWidth( 30 );

  QGroupBox	*dispGp = new QGroupBox( tr( "Palette view :" ), rtPanel );
  QVBoxLayout	*dispGpLay = new QVBoxLayout( dispGp, 10, 10 );
  dispGpLay->addSpacing( 10 );
  d->view = new QLabel( dispGp );
  dispGpLay->addWidget( d->view );
  d->view->setFrameStyle( QFrame::Sunken | QFrame::Box );

  updateInterface();

  connect( d->dimBgp, SIGNAL( clicked( int ) ), this,
           SLOT( dimChanged( int ) ) );
#if QT_VERSION >= 0x040000
  connect( d->palettes, SIGNAL( selectionChanged( Q3ListBoxItem* ) ),
           this, SLOT( palette1Changed( Q3ListBoxItem* ) ) );
#else
  connect( d->palettes, SIGNAL( selectionChanged( QListBoxItem* ) ),
	   this, SLOT( palette1Changed( QListBoxItem* ) ) );
#endif
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

  if( objPalette()->palette1DMapping() == AObjectPalette::DIAGONAL )
    d->dimBgp->setButton(1) ;

  show();
  d->sizes[0][0] = 0;
  d->sizes[0][1] = 0;
  d->sizes[1][0] = 0;
  d->sizes[1][1] = 0;
}


QWidget* QAPaletteWin::makeDimBox( const QString & title, QWidget* parent,
				   struct DimBox* dbox )
{
  dbox->topBox = new QVGroupBox( title, parent );

  QGrid		*minmaxbox = new QGrid( 3, dbox->topBox );
  minmaxbox->setSpacing( 10 );
  new QLabel( tr( "Min:" ), minmaxbox );
  dbox->minSlider = new QSlider( 0, 1000, 1, 0, Qt::Horizontal,
				 minmaxbox, "minSlider" );
  dbox->minSlider->setLineStep( 1 );
  dbox->minLabel = new QLabel( "0", minmaxbox, "minLabel" );
  dbox->minLabel->setMinimumWidth( 50 );

  new QLabel( tr( "Max:" ), minmaxbox );
  dbox->maxSlider = new QSlider( 0, 1000, 1, 1000, Qt::Horizontal,
				 minmaxbox, "maxSlider" );
  dbox->maxSlider->setLineStep( 1 );
  dbox->maxLabel = new QLabel( "1", minmaxbox, "max1dlabel" );

  QHBox	*boundsbox = new QHBox( dbox->topBox );
  boundsbox->setSpacing( 10 );
  new QLabel( tr( "Bounds:" ), boundsbox );
  dbox->minEd = new QScopeLineEdit( "0", boundsbox, "minEd" );
  dbox->minEd->setMinimumWidth( 50 );
  dbox->maxEd = new QScopeLineEdit( "1", boundsbox, "maxEd" );
  dbox->maxEd->setMinimumWidth( 50 );

  dbox->symbox = new QCheckBox( tr( "Value 0 at center" ), dbox->topBox );

  QHBox	*autobox = new QHBox( dbox->topBox );
  autobox->setSpacing( 10 );
  dbox->autoValBtn = new QPushButton( tr( "Reset values" ), autobox );
  dbox->autoBoundsBtn = new QPushButton( tr( "Reset bounds" ), autobox );

  dbox->palView = new QLabel( dbox->topBox );
  dbox->palView->setFrameStyle( QFrame::Sunken | QFrame::Box );

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

  for( i=0, ip=pal.begin(); ip!=fp; ++ip, ++i )
    {
      //cout << "insert palette " << (*ip)->name() << endl;
      d->palettes->insertItem( (*ip)->name().c_str() );
    }
}


void QAPaletteWin::fillPalette2List()
{
  const list<rc_ptr<APalette> >	& pal = theAnatomist->palettes().palettes();
  list<rc_ptr<APalette> >::const_iterator	ip, fp=pal.end();
  int				i = 0;

  d->palette2Box->insertItem( tr( "None" ) );

  d->palette2Box->setCurrentItem( 0 );

  for( i=0, ip=pal.begin(); ip!=fp; ++ip, ++i )
    d->palette2Box->insertItem( (*ip)->name().c_str() );
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
  d->dimBgp->setButton( d->dim - 1 );
  int i, n = d->palettes->count();
  QString	name = d->objpal->refPalette()->name().c_str();
  QListBoxItem *item = d->palettes->findItem( name, Qt::CaseSensitive );
  if( item )
    d->palettes->setCurrentItem( item );
  else
  {
    d->palettes->setCurrentItem( 0 );
    item = d->palettes->selectedItem();
    if( item )
      d->palettes->setSelected( item, false );
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
        if( d->palette2Box->text( i )
            == d->objpal->refPalette2()->name().c_str() )
          {
            d->palette2Box->setCurrentItem( i );
            break;
          }
    }
    else
      d->pal2SetBox->hide();
    }

  n = d->mixBox->count();
  for( i=0; i<n; ++i )
    if( d->mixBox->text( i ) == d->objpal->mixMethodName().c_str() )
    {
      d->mixBox->setCurrentItem( i );
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
              d->dimBgp->setButton(1) ;
              d->palette1dMappingBox->setCurrentItem( 1 ) ;
              dimChanged(1) ;
            }
          else
            d->palette1dMappingBox->setCurrentItem( 0 ) ;

          QListBoxItem *palitem = d->palettes->findItem
              ( d->objpal->refPalette()->name().c_str(), Qt::CaseSensitive );
          if( !palitem )
            fillPalette1();

          d->palettes->setCurrentItem( palitem );

          if ( d->objpal->refPalette2() )
            {
              d->usepal2->setChecked( true );
              int i, n=d->palette2Box->count();
              for( i=0; i<n; ++i )
                if( d->palette2Box->text( i )
                    == d->objpal->refPalette2()->name().c_str() )
                  {
                    d->palette2Box->setCurrentItem( i );
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
	if( d->mixBox->text( i )
	    == d->objpal->mixMethodName().c_str() )
	  {
	    d->mixBox->setCurrentItem( i );
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

  QImage	im( dimx, dimy, 32 );

  for( y=0; y<dimy; ++y )
    for( x=0; x<dimx; ++x )
      {
	rgb = (*pal)( (unsigned) ( facx * x), (unsigned) ( facy * y ) );
	im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
      }
  pm.convertFromImage( im );
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
  d->dimBox1->palView->setFixedSize( d->dimBox1->palView->sizeHint() );
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
	  d->dimBox2->palView->setFixedSize( d->dimBox2->palView->sizeHint() );
	}
    }
  d->dimBox2->palView->setPixmap( pm );
}


void QAPaletteWin::fillObjPal()
{
  AObjectPalette	*objpal = objPalette();
  if( !objpal )
    return;

  AimsData<AimsRGBA>	*col = objpal->colors();

  if( !col || col->dimX() == 0 || col->dimY() == 0 )
    {
      //cout << "no/empty colors in objpalette\n";
      return;
    }

  unsigned	dimpx = col->dimX(), dimpy = col->dimY();
  unsigned	dimx = 256, dimy = dimpy, x, y;
  int		xp, yp;
  float		m1 = objpal->min1(), M1 = objpal->max1();
  float		m2 = objpal->min2(), M2 = objpal->max2();

  if( dimy < 32 )
    dimy = 32;
  if( dimy > 256 )
    dimy = 256;
  if( dimx == 0 )
    dimx = 1;
  if( m1 == M1 )
    {
      m1 = 0;
      M1 = 1;
    }
  if( m2 == M2 )
    {
      m2 = 0;
      M2 = 1;
    }

  float		facx = ((float) dimpx) / ( (M1 - m1) * dimx );
  float		facy = ((float) dimpy) / ( (M2 - m2) * dimy );
  float		dx = m1 * dimx;
  float		dy = m2 * dimy;
  AimsRGBA	rgb;

  QImage	im( dimx, dimy, 32 );
  QPixmap	pm( dimx, dimy );

  for( y=0; y<dimy; ++y )
    {
      yp = (int) ( facy * ( ((float) y) - dy ) );
      if( yp < 0 )
	yp = 0;
      else if( yp >= (int) dimpy )
	yp = dimpy - 1;
      for( x=0; x<dimx; ++x )
	{
	  xp = (int) ( facx * ( ((float) x) - dx ) );
	  if( xp < 0 )
	    xp = 0;
	  else if( xp >= (int) dimpx )
	    xp = dimpx - 1;
	  rgb = (*col)( xp, yp );
	  im.setPixel( x, y, qRgb( rgb.red(), rgb.green(), rgb.blue() ) );
	}
    }
  pm.convertFromImage( im );
  d->view->setPixmap( pm );
  d->view->setFixedSize( d->view->sizeHint() );
}


void QAPaletteWin::palette1Changed( QListBoxItem* item )
{
  if( d->recursive )
    return;

  string		name = (const char *) item->text();
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
        pal = pallist.find( string( palname.utf8().data() ) );

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
    d->mixBox->insertItem( (*im).first.c_str() );
}


void QAPaletteWin::mixMethodChanged( const QString & methname )
{
  if( d->recursive )
    return;

  AObjectPalette	*objpal = objPalette();

  objpal->setMixMethod( string( methname.utf8().data() ) );
  updateObjPal();
  fillObjPal();
  d->modified = true;
  if( d->responsive )
    updateObjects();
}

void
QAPaletteWin::fillPalette1DMappingMethods()
{
  d->palette1dMappingBox->insertItem("FirstLine") ;
  d->palette1dMappingBox->insertItem("Diagonal") ;
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
      d->palette2Box->setCurrentItem( 0 );
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


