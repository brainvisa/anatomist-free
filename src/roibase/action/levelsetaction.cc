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

#include <aims/connectivity/connectivity.h>
#include <anatomist/action/levelsetaction.h>
#include <anatomist/action/histoplot.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/controler/view.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/color/palette.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/object/Object.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/misc/error.h>
#include <qpushbutton.h>
#include <aims/resampling/quaternion.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <QCheckBox>
#include <QButtonGroup>
#include <QGroupBox>
#include <queue>

using namespace anatomist ;
using namespace aims;
using namespace carto;
using namespace std;


namespace anatomist
{
  class RoiLevelSetActionView_Private {
  public:
    RoiLevelSetAction * myLevelSetAction ;

    RoiHistoPlot * myHistoPlot ;
    QCheckBox * myActivateButton ;

    QSlider * myLowLevelSlider ;
    QLabel * myLowLevelValueLabel ;

    QSlider * myHighLevelSlider ;
    QLabel * myHighLevelValueLabel ;

    QButtonGroup * myDimensions ;
    QSlider * myMaxSizeSlider ;
    QLabel * myMaxSizeLabel ;

    QLineEdit * myMaxSizeLineEdit ;
    QLineEdit * myPercentMaxLineEdit ;

    QComboBox * myMixMethods ;
    QSlider * myMixFactor ;
    QLabel * myMixFactorLabel ;
  } ;
};

RoiLevelSetActionView::RoiLevelSetActionView(
  anatomist::RoiLevelSetAction *  action,
  QWidget * parent ) :
  QWidget(parent), Observer(), myChangingFlag(false),  myUpdatingFlag(false)
{
  //cout << "RoiLevelSetActionView::RoiLevelSetActionView" << endl ;
  _private = new RoiLevelSetActionView_Private ;
  _private->myLevelSetAction = action ;
  RoiLevelSetActionSharedData::instance()->addObserver(this) ;

  QVBoxLayout *lay = new QVBoxLayout( this );

  _private->myHistoPlot = new RoiHistoPlot(this, 100) ;
  lay->addWidget( _private->myHistoPlot );
  QGroupBox *myActivateButtonGroup = new QGroupBox(
      tr("Threshold Preview Activation"), this );
  lay->addWidget( myActivateButtonGroup );
  QHBoxLayout *abglay = new QHBoxLayout( myActivateButtonGroup );
  _private->myActivateButton = new QCheckBox(
    tr("Activate Threshold Preview"), myActivateButtonGroup );
  abglay->addWidget( _private->myActivateButton );

  QWidget *myLevels = new QWidget( this );
  lay->addWidget( myLevels );
  QHBoxLayout *levellay = new QHBoxLayout( myLevels );
  levellay->setMargin( 0 );
  levellay->setSpacing( 5 );

  QGroupBox *myLowLevelGroupBox = new QGroupBox( tr("Low Level"), myLevels );
  levellay->addWidget( myLowLevelGroupBox );
  QHBoxLayout *llglay = new QHBoxLayout( myLowLevelGroupBox );
  llglay->setMargin( 5 );
  llglay->setSpacing( 5 );
  _private->myLowLevelSlider = new QSlider( Qt::Horizontal,
                                            myLowLevelGroupBox );
  _private->myLowLevelSlider->setRange( -10, 1010 );
  _private->myLowLevelSlider->setPageStep( 1 );
  _private->myLowLevelSlider->setValue(
    int( _private->myLevelSetAction->lowLevel() * 1000. ) );
  llglay->addWidget( _private->myLowLevelSlider );
  _private->myLowLevelValueLabel =  new QLabel(
    QString::number( _private->myLevelSetAction->realMin() ),
    myLowLevelGroupBox ) ;
  llglay->addWidget( _private->myLowLevelValueLabel );
  _private->myLowLevelValueLabel->setFixedWidth(80) ;
  _private->myLowLevelSlider->setEnabled(false) ;
  _private->myHistoPlot->lowChanged( _private->myLevelSetAction->realMin() ) ;
  QGroupBox *myHighLevelGroupBox = new QGroupBox( tr("High Level"), myLevels );
  levellay->addWidget( myHighLevelGroupBox );
  QHBoxLayout *hlglay = new QHBoxLayout( myHighLevelGroupBox );
  _private->myHighLevelSlider = new QSlider( Qt::Horizontal,
                                             myHighLevelGroupBox );
  _private->myHighLevelSlider->setRange( -10, 1010 );
  _private->myHighLevelSlider->setPageStep( 1 );
  _private->myHighLevelSlider->setValue(
    int( _private->myLevelSetAction->highLevel() * 1000. ) );
  hlglay->addWidget( _private->myHighLevelSlider );
  _private->myHighLevelValueLabel =  new QLabel(
    QString::number( _private->myLevelSetAction->realMax() ),
    myHighLevelGroupBox );
  hlglay->addWidget( _private->myHighLevelValueLabel );
  _private->myHighLevelValueLabel->setFixedWidth(80) ;
  _private->myHighLevelSlider->setEnabled(false) ;
  _private->myHistoPlot->highChanged( _private->myLevelSetAction->realMax() ) ;

  QWidget *myModes = new QWidget( this );
  lay->addWidget( myModes );
  QHBoxLayout *modelay = new QHBoxLayout( myModes );
  modelay->setMargin( 0 );
  modelay->setSpacing( 5 );

  QWidget *myDimensionMode = new QWidget( myModes );
  modelay->addWidget( myDimensionMode );
  QVBoxLayout *dimmlay = new QVBoxLayout( myDimensionMode );
  dimmlay->setMargin( 0 );
  dimmlay->setSpacing( 5 );
  QGroupBox *dimb = new QGroupBox( tr("Dimension"), myDimensionMode );
  dimmlay->addWidget( dimb );
  QHBoxLayout *dimblay = new QHBoxLayout( dimb );
  _private->myDimensions = new QButtonGroup( dimb );
  QRadioButton *r = new QRadioButton(tr("2D") );
  dimblay->addWidget( r );
  _private->myDimensions->addButton( r, 0 );
  r =  new QRadioButton(tr("3D") );
  dimblay->addWidget( r );
  _private->myDimensions->addButton( r, 1 );
  _private->myDimensions->setExclusive(true) ;
  _private->myDimensions->button(0)->setChecked( true );

  QGroupBox *myMaxSizeBox = new QGroupBox(
    tr("Region max size"), myDimensionMode );
  dimmlay->addWidget( myMaxSizeBox );
  QHBoxLayout *msblay = new QHBoxLayout( myMaxSizeBox );
  _private->myMaxSizeLineEdit = new QLineEdit(
    QString::number( _private->myLevelSetAction->maxSize() ),
    myMaxSizeBox );
  msblay->addWidget( _private->myMaxSizeLineEdit );
  _private->myMaxSizeLineEdit->setFixedWidth(60) ;
  msblay->addWidget( new QLabel( "mm3", myMaxSizeBox ) );

  QGroupBox *myPercentMaxBox = new QGroupBox( tr("Percentage of extremum"),
                                              myDimensionMode );
  dimmlay->addWidget( myPercentMaxBox );
  QHBoxLayout *pmblay = new QHBoxLayout( myPercentMaxBox );
  _private->myPercentMaxLineEdit = new QLineEdit(
    QString::number( _private->myLevelSetAction->percentageOfMaximum() ),
    myPercentMaxBox );
  pmblay->addWidget( _private->myPercentMaxLineEdit );
  _private->myPercentMaxLineEdit->setFixedWidth(60);
  pmblay->addWidget( new QLabel( " %", myPercentMaxBox ) );

  QWidget *myMixBox = new QWidget( myModes );
  modelay->addWidget( myMixBox );
  QVBoxLayout *minblay = new QVBoxLayout( myMixBox );
  minblay->setMargin( 0 );
  minblay->setSpacing( 5 );

  QGroupBox *myMixMethodBox = new QGroupBox( tr("MixMethod"), myMixBox );
  minblay->addWidget( myMixMethodBox );
  QVBoxLayout *mmblay = new QVBoxLayout( myMixMethodBox );
  _private->myMixMethods = new QComboBox( myMixMethodBox );
  mmblay->addWidget( _private->myMixMethods );
  _private->myMixMethods->addItem( "GEOMETRIC" ) ;
  _private->myMixMethods->addItem( "LINEAR" ) ;
  _private->myMixMethods->setCurrentIndex( 0 ) ;

  QGroupBox *myMixFactorBox = new QGroupBox(tr("Mixing Factor"), myMixBox );
  minblay->addWidget( myMixFactorBox );
  QHBoxLayout *mfblay = new QHBoxLayout( myMixFactorBox );
  _private->myMixFactor = new QSlider( Qt::Horizontal, myMixFactorBox );
  _private->myMixFactor->setRange( 0, 100 );
  _private->myMixFactor->setPageStep( 10 );
  _private->myMixFactor->setValue( 50 );
  mfblay->addWidget( _private->myMixFactor );
  _private->myMixFactor->setEnabled(false) ;

  _private->myMixFactorLabel = new QLabel( "10 %", myMixFactorBox );
  mfblay->addWidget( _private->myMixFactorLabel );
  _private->myMixFactorLabel->setFixedWidth(80) ;

  connect( _private->myActivateButton, SIGNAL(stateChanged(int)),
            this, SLOT(levelSetActivationChanged(int) ) ) ;
  connect( _private->myLowLevelSlider, SIGNAL(valueChanged(int)),
            this, SLOT(lowLevelChanged(int) ) ) ;
  connect( _private->myHighLevelSlider, SIGNAL(valueChanged(int)),
            this, SLOT(highLevelChanged(int) ) ) ;
  connect( _private->myDimensions, SIGNAL(idClicked(int)),
            this, SLOT(dimensionModeChanged(int) ) ) ;

  connect( _private->myMaxSizeLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(maxSizeChanged(const QString&) ) ) ;
  connect( _private->myPercentMaxLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(percentageOfMaxChanged(const QString&) ) ) ;

  connect( _private->myMixMethods, SIGNAL(activated(const QString&)),
            this, SLOT(mixMethodChanged(const QString&) ) ) ;
  connect( _private->myMixFactor, SIGNAL(valueChanged(int)),
            this, SLOT(mixFactorChanged(int) ) ) ;
}

RoiLevelSetActionView::~RoiLevelSetActionView()
{
  //cout << "RoiLevelSetActionView::~RoiLevelSetActionView" << endl ;
  //_private->myLevelSetAction->deactivateLevelSet() ;
  RoiLevelSetActionSharedData::instance()->deleteObserver(this) ;
  delete _private;
}

void
RoiLevelSetActionView::levelSetActivationChanged( int state )
{
  myChangingFlag = true ;
  if( state == Qt::Checked )
  {
    _private->myLevelSetAction->activateLevelSet() ;
    _private->myHistoPlot->activate() ;
    _private->myLowLevelSlider->setEnabled(true) ;
    _private->myHighLevelSlider->setEnabled(true) ;

    _private->myLowLevelSlider->setValue( int( _private->myLevelSetAction->
                                                lowLevel() * 1000. ) ) ;
    _private->myLowLevelValueLabel->
      setText( QString::number( _private->myLevelSetAction->realMin() ) ) ;

    _private->myHighLevelSlider->setValue( int( _private->myLevelSetAction->
                                                highLevel() * 1000. ) ) ;
    _private->myHighLevelValueLabel->
      setText( QString::number( _private->myLevelSetAction->realMax() ) ) ;
  }
  else
  {
    _private->myLevelSetAction->deactivateLevelSet() ;
    _private->myHistoPlot->deactivate() ;
    _private->myLowLevelSlider->setEnabled(false) ;
    _private->myHighLevelSlider->setEnabled(false) ;
  }
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::lowLevelChanged( int newLowLevel )
{
  //cout << "RoiLevelSetActionView::lowLevelChanged" << endl ;
  myChangingFlag = true ;
  _private->myLevelSetAction->lowLevelChanged( newLowLevel/1000. ) ;
  _private->myLowLevelValueLabel->setText( QString::number( _private->myLevelSetAction->realMin( ) ) ) ;
  _private->myHistoPlot->lowChanged( _private->myLevelSetAction->realMin() ) ;
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::highLevelChanged( int newHighLevel )
{
  //cout << "RoiLevelSetActionView::highLevelChanged" << endl ;
  myChangingFlag = true ;

  _private->myLevelSetAction->highLevelChanged( newHighLevel/1000. ) ;
  _private->myHighLevelValueLabel->setText( QString::number( _private->myLevelSetAction->realMax( ) ) ) ;
  _private->myHistoPlot->highChanged( _private->myLevelSetAction->realMax() ) ;
  
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::dimensionModeChanged(int mode) 
{
    //cout << "RoiLevelSetActionView::dimensionModeChanged" << endl ;

  myChangingFlag = true ;
  if( mode == 0 )
    _private->myLevelSetAction->setDimensionModeTo2D() ;
  else
    _private->myLevelSetAction->setDimensionModeTo3D() ;    
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::maxSizeChanged( const QString& maxSize ) 
{
  //cout << "RoiLevelSetActionView::maxSizeChanged" << endl ;
  myChangingFlag = true ;
  bool ok ;
  float fl = maxSize.toFloat( &ok ) ;
  if( ok )
    _private->myLevelSetAction->setMaxSize( fl ) ;
  else
    _private->myLevelSetAction->setMaxSize( -1 ) ;
    
  //_private->myMaxSizeLabel->setText( QString::number( maxSize ) + " %" ) ;
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::percentageOfMaxChanged( const QString& percentageOfMax )
{
  myChangingFlag = true ;
  
  bool ok ;
  float fl = percentageOfMax.toFloat( &ok ) ;
  if( ok )
    _private->myLevelSetAction->setPercentageOfMaximum( fl ) ;
  
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::mixFactorChanged( int fact ) 
{
  //cout << "RoiLevelSetActionView::mixFactorChanged" << endl ;
  myChangingFlag = true ;
  _private->myLevelSetAction->setMixFactor( fact/100. ) ;
  _private->myMixFactorLabel->setText( QString::number( fact ) + " %" ) ;
  myChangingFlag = false ;
}

void 
RoiLevelSetActionView::mixMethodChanged(const QString& method ) 
{
  myChangingFlag = true ;
  _private->myLevelSetAction->setMixMethod( method.toStdString() );
  
  if( method == "GEOMETRIC")
    _private->myMixFactor->setEnabled(false) ;
  else
    _private->myMixFactor->setEnabled(true) ;

  myChangingFlag = false ;  
}

void 
RoiLevelSetActionView::update( const anatomist::Observable *, void * )
{
  //cout << "RoiLevelSetActionView::update" << endl ;
  ////cout << "RoiLevelSetActionView::update" << endl ;
  if( myChangingFlag || myUpdatingFlag )
    return ;
  return;
  myUpdatingFlag = true ;
  
  if( _private->myLevelSetAction->levelSetActivation() )
    {
      _private->myActivateButton->setChecked( false );
      _private->myHistoPlot->activate();
      _private->myLowLevelSlider->setEnabled( true );
      _private->myHighLevelSlider->setEnabled( true );
    }
  else
    {
      _private->myActivateButton->setChecked( true );
      _private->myHistoPlot->deactivate();
      _private->myLowLevelSlider->setEnabled( false );
      _private->myHighLevelSlider->setEnabled( false );
      myUpdatingFlag = false ;
      return ;
    }

  if( _private->myLowLevelSlider->value() != _private->myLevelSetAction->lowLevel()*1000. ){
    _private->myLowLevelSlider->setValue( int (_private->myLevelSetAction->
					       lowLevel()*1000. ) ) ;
    _private->myLowLevelValueLabel->
      setText( QString::number( _private->myLevelSetAction->realMin() ) ) ;
  }
  
  if( _private->myHighLevelSlider->value() != _private->myLevelSetAction->highLevel()*1000. ){
    _private->myHighLevelSlider->setValue( int( _private->myLevelSetAction->
						highLevel() *1000.) ) ;
    _private->myHighLevelValueLabel->
      setText( QString::number(  _private->myLevelSetAction->realMax() ) ) ;
  }
  
  if( _private->myLevelSetAction->dimensionMode() != 
      _private->myDimensions->id(_private->myDimensions->checkedButton() ) )
    _private->myDimensions->button(
      _private->myLevelSetAction->dimensionMode() )->setChecked( true );

  if( _private->myLevelSetAction->maxSize() != _private->myMaxSizeLineEdit->
      text().toFloat() )
    {
      _private->myMaxSizeLineEdit->setText( QString::number(_private->myLevelSetAction->maxSize() ) ) ;
    }
  if( _private->myLevelSetAction->percentageOfMaximum() != _private->myPercentMaxLineEdit->
      text().toFloat() )
    {
      _private->myPercentMaxLineEdit->setText( QString::number(_private->myLevelSetAction->percentageOfMaximum() ) ) ;
    }

  if( _private->myLevelSetAction->mixMethod().c_str() 
      != _private->myMixMethods->currentText() )
    {
      if( _private->myLevelSetAction->mixMethod() == "GEOMETRIC" )
	{
	  _private->myMixMethods->setCurrentIndex( 0 ) ;
	  _private->myMixFactor->setEnabled(false) ;
	}
      else
	{
	  _private->myMixMethods->setCurrentIndex( 1 ) ;
	  _private->myMixFactor->setEnabled(true) ;
	}
    }

  if( _private->myLevelSetAction->mixFactor() 
      != _private->myMixFactor->value() )
    {
      _private->myMixFactor->setValue( int(_private->myLevelSetAction->
					   mixFactor()*100 ) ) ;
      _private->myMixFactorLabel->setText
	( QString::number( int(_private->myLevelSetAction->mixFactor()
			       *100 ) ) );
    }
  
  myUpdatingFlag = false ;
}

RoiLevelSetActionSharedData* RoiLevelSetActionSharedData::_instance = 0 ;

RoiLevelSetActionSharedData* RoiLevelSetActionSharedData::instance(){
  if( _instance == 0 )
    _instance = new RoiLevelSetActionSharedData ;
  return _instance ;
}

void 
RoiLevelSetActionSharedData::update (const Observable *, void *)
{
  //cout << "PaintActionSharedData::update" << endl ;
  setChanged() ;
  notifyObservers(this) ;
}

// Paint Action class

RoiLevelSetActionSharedData::RoiLevelSetActionSharedData() : 
  Observer(), Observable(), myLevelSetActivation(false), myLevelSetDeactivation(false), 
  myCurrentImage(0), myDimensionMode(TWOD),
  myLowLevel(0.), myHighLevel(0.), myImageMax(0.), myImageMin(0.),
  myMaxSize( 0. ), myPercentageOfMaximum( 1. ),
  myMixMethod("GEOMETRIC"), myMixFactor(0.5),
  myGettingCurrentImage(false), myActivatingLevelSet(false), myUpdating(false)
{
  //cout << "Constructor : myLevelSetActivation is " << (myLevelSetActivation ? "True" : "False") << endl ;
}

RoiLevelSetActionSharedData::~RoiLevelSetActionSharedData()
{
  if ( myCurrentImage )
    myCurrentImage->deleteObserver(this) ;
}

RoiLevelSetAction::RoiLevelSetAction() : Action()
{
  _sharedData = RoiLevelSetActionSharedData::instance() ;
} 

RoiLevelSetAction::~RoiLevelSetAction(){ } 

string RoiLevelSetAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "ConnectivityThresholdAction" );
}


AObject* 
RoiLevelSetAction::getCurrentImage()
{
  if ( _sharedData->myGettingCurrentImage )
    return _sharedData->myCurrentImage ;
  _sharedData->myGettingCurrentImage = true ;
  //cout << "RoiLevelSetAction::getCurrentImage" << endl ;
  //   set<AObject*> objs = view()->aWindow()->Objects() ;
  AObject * gotIt = RoiManagementActionSharedData::instance()->
    getObjectByName( AObject::VOLUME, 
		     RoiManagementActionSharedData::instance()->
		     currentImage() ) ;
  if( gotIt != 0 )
    {
      if ( gotIt != _sharedData->myCurrentImage )
	{
          GLComponent  *gl = gotIt->glAPI();
	  if(_sharedData->myCurrentImage)
	    {
              _sharedData->myCurrentImage->getOrCreatePalette();
	      _sharedData->myCurrentImage->palette()->
		setPalette1DMapping( AObjectPalette::FIRSTLINE ) ;
	      if( !_sharedData->myLevelSetDeactivation )
		deactivateLevelSet() ;
	      _sharedData->myCurrentImage->deleteObserver(_sharedData) ;
	      _sharedData->setChanged() ;
	    }
	  _sharedData->myCurrentImage = gotIt ;
          _sharedData->myImageMin =  0;
          _sharedData->myImageMin =  1;
          if( gl && gl->glNumTextures() > 0 )
          {
            GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
            if( !te.minquant.empty() )
	      _sharedData->myImageMin = te.minquant[0] ;
            if( !te.maxquant.empty() )
	      _sharedData->myImageMax = te.maxquant[0] ;
          }
	  _sharedData->myCurrentImage->addObserver( _sharedData ) ;
	  if( _sharedData->myCurrentImage->getOrCreatePalette()->palette1DMapping() 
	      == AObjectPalette::DIAGONAL ){
	    //if( _sharedData->myLevelSetActivation )
	      activateLevelSet() ;
	  }
	  else
	    //if( _sharedData->myLevelSetActivation )
	      deactivateLevelSet() ;
	  
	  _sharedData->notifyObservers() ;
	}
    }
  _sharedData->myGettingCurrentImage = false ;
  return _sharedData->myCurrentImage ;
}

Action*
RoiLevelSetAction::creator()
{
  return new RoiLevelSetAction( ) ;
}

void
RoiLevelSetAction::activateLevelSet() 
{
  if( _sharedData->myActivatingLevelSet )
    return ; 
  _sharedData->myActivatingLevelSet = true ;
  //cout << "RoiLevelSetAction::activateLevelSet" << endl ;
  if( !getCurrentImage() )
    {
      //cout << "No current image !!!!" << endl ;
      _sharedData->myLevelSetActivation = false ;
      _sharedData->setChanged() ;
      _sharedData->notifyObservers() ;
      return ;
    }
  _sharedData->myLevelSetActivation = true ;

  //cout << "Current Image = " << _sharedData->myCurrentImage << endl ;
  //cout << "Object Palette " << _sharedData->myCurrentImage->palette() << endl ;

  GLComponent  *gl = _sharedData->myCurrentImage->glAPI();
  _sharedData->myImageMin =  0;
  _sharedData->myImageMin =  1;
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      _sharedData->myImageMin = te.minquant[0] ;
    if( !te.maxquant.empty() )
      _sharedData->myImageMax = te.maxquant[0] ;
  }
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->set2dMode( true ) ;
  _sharedData->myCurrentImage->palette()->setPalette1DMapping( AObjectPalette::DIAGONAL ) ;
  
  if ( ! _sharedData->myCurrentImage->palette()->refPalette2() )
    _sharedData->myCurrentImage->palette()->setRefPalette2( 
	    theAnatomist->palettes().find("DoubleThreshold") ) ;
  _sharedData->myCurrentImage->palette()->setMixMethod( _sharedData->myMixMethod ) ;
  _sharedData->myCurrentImage->palette()->setLinearMixFactor( _sharedData->myMixFactor ) ;

  updateObjPal() ;

  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );

  view()->controlSwitch()->setActiveControl("ConnectivityThresholdControl") ;

  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
  _sharedData->myActivatingLevelSet = false ;
}

void
RoiLevelSetAction::deactivateLevelSet() 
{
    //cout << "RoiLevelSetAction::deactivateLevelSet" << endl ;
  if( _sharedData->myLevelSetDeactivation )
    return ;
  _sharedData->myLevelSetDeactivation = true ;
  _sharedData->myLevelSetActivation = false ;
  if( !getCurrentImage() ) {
    _sharedData->setChanged() ;
    _sharedData->notifyObservers() ;
    _sharedData->myLevelSetDeactivation = false ;
    return ;
  }
  //cout << "Current Image : " << _sharedData->myCurrentImage << endl ;
  
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->setPalette1DMapping( AObjectPalette::FIRSTLINE ) ;
  _sharedData->myCurrentImage->palette()->set2dMode( false ) ;
  _sharedData->myCurrentImage->palette()->setMixMethod( "GEOMETRIC" ) ;
  

  updateObjPal() ;
  
  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );

  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
  _sharedData->myLevelSetDeactivation = false ;
}

QWidget * 
RoiLevelSetAction::actionView( QWidget * parent )
{
  RoiLevelSetActionView * obs = new RoiLevelSetActionView( this, parent ) ;
  
  return obs ;
}

void 
RoiLevelSetAction::lowLevelChanged( float newLowLevel )
{
  //cout << "RoiLevelSetAction::lowLevelChanged" << endl ;
  //cout << "Low Level Changed = " << newLowLevel << endl ;
  
  _sharedData->myLowLevel = newLowLevel ;
  if(!_sharedData->myCurrentImage)
    return ;
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->setMin2(_sharedData->myLowLevel) ;
  
  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}



void 
RoiLevelSetAction::setDimensionModeTo2D()
{
  cout << "Threshold region growth Dimension Mode set to 2D " << endl ;
  _sharedData->myDimensionMode = RoiLevelSetActionSharedData::TWOD ;
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}

void 
RoiLevelSetAction::setDimensionModeTo3D()
{
  cout << "Threshold region growth Dimension Mode set to 3D " << endl ;
  _sharedData->myDimensionMode = RoiLevelSetActionSharedData::THREED ;
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}

void 
RoiLevelSetAction::setMaxSize( float maxSize )
{
  _sharedData->myMaxSize = maxSize ;
}

void 
RoiLevelSetAction::setPercentageOfMaximum( float percentageOfMaximum )
{
  _sharedData->myPercentageOfMaximum = percentageOfMaximum ;
}

void 
RoiLevelSetAction::setMixFactor( float mixFactor )
{
  _sharedData->myMixFactor = mixFactor ;
  if(!_sharedData->myCurrentImage)
    return ;
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->setLinearMixFactor(mixFactor) ;
  
  updateObjPal() ;

  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}

void 
RoiLevelSetAction::setMixMethod( const string& method )
{
  //cout << "RoiLevelSetAction::setMixMethod" << endl<< method  << endl ;

  _sharedData->myMixMethod = method ;
  if(!_sharedData->myCurrentImage)
    return ;
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->setMixMethod(method) ;
  cout << "Mix Method Changed" << endl ;
  
  updateObjPal() ;

  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}

void 
RoiLevelSetAction::highLevelChanged( float newHighLevel )
{
  //cout << "High Level Changed = " << newHighLevel << endl ;
  _sharedData->myHighLevel = newHighLevel ;

  if(!_sharedData->myCurrentImage)
    return ;
  _sharedData->myCurrentImage->getOrCreatePalette();
  _sharedData->myCurrentImage->palette()->setMax2(_sharedData->myHighLevel) ;
  
  _sharedData->myCurrentImage->glAPI()->glSetTexImageChanged();
  _sharedData->myCurrentImage->notifyObservers( _sharedData->myCurrentImage );
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
}

float 
RoiLevelSetAction::realMin( ) const
{
  if (!_sharedData->myCurrentImage )
    return 0. ;
  if ( _sharedData->myLowLevel <= 0. )
    return _sharedData->myImageMin ;
  if ( _sharedData->myLowLevel > 1. )
    return _sharedData->myImageMax ;
  float factor = _sharedData->myLowLevel + ( _sharedData->myHighLevel - _sharedData->myLowLevel ) 
    / _sharedData->myCurrentImage->getOrCreatePalette()->colors()->getSizeX();
  
  
  return _sharedData->myImageMin + factor * ( _sharedData->myImageMax - _sharedData->myImageMin ) ;
}


float 
RoiLevelSetAction::realMax( ) const
{
  if (!_sharedData->myCurrentImage )
    return 0. ;
  if ( _sharedData->myHighLevel <= 0. )
    return _sharedData->myImageMin ;
  if ( _sharedData->myHighLevel > 1. )
    return _sharedData->myImageMax ;
  float factor = _sharedData->myHighLevel - ( _sharedData->myHighLevel - _sharedData->myLowLevel ) 
    / _sharedData->myCurrentImage->getOrCreatePalette()->colors()->getSizeX();
  
  return _sharedData->myImageMin + factor * ( _sharedData->myImageMax - _sharedData->myImageMin ) ;
}

void 
RoiLevelSetAction::update( const Observable *, void * )
{
  //cout << "RoiLevelSetAction::update" << endl ;
  ////cout << "RoiLevelSetAction::update" << endl ;
  if ( _sharedData->myUpdating )
    return ;
  _sharedData->myUpdating = true ;
  if( !getCurrentImage() ){
    _sharedData->myUpdating = false ;
    return ;
  }
  GLComponent  *gl = _sharedData->myCurrentImage->glAPI();
  _sharedData->myImageMin =  0;
  _sharedData->myImageMin =  1;
  if( gl && gl->glNumTextures() > 0 )
  {
    GLComponent::TexExtrema  & te = gl->glTexExtrema( 0 );
    if( !te.minquant.empty() )
      _sharedData->myImageMin = te.minquant[0] ;
    if( !te.maxquant.empty() )
      _sharedData->myImageMax = te.maxquant[0] ;
  }
  _sharedData->myLowLevel = _sharedData->myCurrentImage->getOrCreatePalette()->min2() ;

  _sharedData->myHighLevel = _sharedData->myCurrentImage->palette()->max2() ;
  if( _sharedData->myCurrentImage->palette()->palette1DMapping() == AObjectPalette::DIAGONAL )
    _sharedData->myLevelSetActivation = true ;
  _sharedData->myMixMethod = _sharedData->myCurrentImage->palette()->mixMethodName() ;
  _sharedData->myMixFactor = _sharedData->myCurrentImage->palette()->linearMixFactor() ;
  
  _sharedData->setChanged() ;
  _sharedData->notifyObservers() ;
  _sharedData->myUpdating = false ;
}

void 
RoiLevelSetAction::unregisterObservable( Observable* o )
{
  _sharedData->Observer::unregisterObservable( o );
  _sharedData->myCurrentImage = 0 ;
}

void 
RoiLevelSetAction::updateObjPal()
{
  //cout << "RoiLevelSetAction::updateObjPal" << endl ;
  if( _sharedData->myCurrentImage == 0)
    return ;
  
  _sharedData->myCurrentImage->getOrCreatePalette();
  AObjectPalette	*objpal = _sharedData->myCurrentImage->palette();
  if( !objpal )
    return;
  rc_ptr<APalette>	pal = objpal->refPalette();
  if( !pal )
    return;
  rc_ptr<APalette>	pal2 = objpal->refPalette2();
  unsigned		dimx = pal->getSizeX(), dimy = pal->getSizeY();
  unsigned		dimxmax = 256, dimymax = 256;

  if( objpal->palette1DMapping() == AObjectPalette::FIRSTLINE )
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

  objpal->create( dimx, dimy );
  objpal->fill();
}

void 
RoiLevelSetAction::replaceRegion( int x, int y, int, int )
{
  Bucket * currentModifiedRegion ; 
  if ( ! ( currentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->aWindow()*/ ) ) ) {
    return ;
  }
  
  if (! _sharedData->myLevelSetActivation )
    return ;

  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
  
  list< pair< Point3d, ChangesItem> > * changes = new list< pair< Point3d, ChangesItem> > ;
  
  if (!g) return ;
  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  vector<float> bmin, bmax, vs;
  g->boundingBox2D( bmin, bmax );
  vs = g->voxelSize();
  vector<int> dims( 3 );
  dims[0] = int( rint( ( bmax[0] - bmin[0] ) / vs[0] ) );
  dims[1] = int( rint( ( bmax[1] - bmin[1] ) / vs[1] ) );
  dims[2] = int( rint( ( bmax[2] - bmin[2] ) / vs[2] ) );
  if( labels.getSizeX() != dims[0]
      || labels.getSizeY() != dims[1]
      || labels.getSizeZ() != dims[2] )
  {
    g->clearLabelsVolume();
    g->setLabelsVolumeDimension( dims[0], dims[1], dims[2] );
  }
  else
  {
    //cout << "item : " << endl ;
    //cout << "\tbefore " << item.before << endl
    //	 << "\tafter " << item.after << endl ;

    BucketMap<Void>::Bucket::iterator 
      iter( currentModifiedRegion->bucket()[0].begin() ), 
      last( currentModifiedRegion->bucket()[0].end() ) ;
    
    while ( iter != last){
      ChangesItem item ;
      item.after = 0 ;
      item.before = go ;
      changes->push_back( pair<Point3d, ChangesItem>( iter->first, item ) ) ;
      
      labels->at( iter->first ) = 0  ;
      ++iter ;
    }
    
    //cout << "Region erased size : " << (*changes)[0].size() << endl ;
    
    fillRegion( x, y, go, *changes, true ) ;

    //cout << "Once filled : " << (*changes)[0].size() << endl ;
    
    if ( ! (*changes).empty() )
      RoiChangeProcessor::instance()->applyChange( changes ) ;
    
  }
  
  RoiChangeProcessor::instance()->getGraphObject( view()->aWindow() )
    ->attributed()->setProperty("modified", true) ;

  currentModifiedRegion->setBucketChanged() ;
    
}

void 
RoiLevelSetAction::addToRegion( int x, int y, int, int )
{
  Bucket * currentModifiedRegion ;
  if ( ! ( currentModifiedRegion =
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->aWindow()*/ ) ) ) {
    return ;
  }


  if (!_sharedData->myLevelSetActivation)
    return ;


  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;

  list< pair< Point3d, ChangesItem> >* changes
    = new list< pair< Point3d, ChangesItem> > ;

  if (!g) return ;

  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  vector<float> bmin, bmax, vs;
  g->boundingBox2D( bmin, bmax );
  vs = g->voxelSize();
  vector<int> dims( 3 );
  dims[0] = int( rint( ( bmax[0] - bmin[0] ) / vs[0] ) );
  dims[1] = int( rint( ( bmax[1] - bmin[1] ) / vs[1] ) );
  dims[2] = int( rint( ( bmax[2] - bmin[2] ) / vs[2] ) );
  if( labels.getSizeX() != dims[0]
      || labels.getSizeY() != dims[1]
      || labels.getSizeZ() != dims[2] )
  {
    g->clearLabelsVolume();
    g->setLabelsVolumeDimension( dims[0], dims[1], dims[2] );
  }

  fillRegion( x, y, go, *changes, true ) ;

  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;

  currentModifiedRegion->setBucketChanged() ;

  RoiChangeProcessor::instance()->getGraphObject( view()->aWindow() )
    ->attributed()->setProperty("modified", true) ;
}

void 
RoiLevelSetAction::removeFromRegion( int x, int y, int, int )
{
  Bucket * currentModifiedRegion ; 
  if ( ! ( currentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->aWindow()*/ ) ) ) {
    return ;
  }
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
  
  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;
  
  if (!g) return ;
  
  fillRegion( x, y, go, *changes, false) ;
  
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
  
  currentModifiedRegion->setBucketChanged() ;
  RoiChangeProcessor::instance()->getGraphObject( view()->aWindow() )
    ->attributed()->setProperty("modified", true) ;
}

void 
RoiLevelSetAction::fillRegion( int x, int y, AGraphObject * region,
                               list< pair< Point3d, ChangesItem> >& changes,
                               bool add )
{
  if( !_sharedData->myCurrentImage )
    return ;
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->aWindow() ) ;
  if( !win )
  {
    cerr << "warning: PaintAction operating on wrong view type\n";
    return;
  }

  //cout << "\tx = " << x << "\ty = " << y << endl ;

  Referential* winRef = win->getReferential() ;

  Referential* buckRef = region->getReferential() ;
  //bool		newbck = _sharedData->myDeltaModifications->empty();
  Point3df pos ;
  if( win->positionFromCursor( x, y, pos ) )
  {
    // cout << "Pos : " << pos << endl ;

    // cout << "Position from cursor : (" << x << " , "<< y << ") = "
    //   << pos << endl ;

    Point3df voxelSize = Point3df( region->voxelSize() );

    Point3df normalVector( win->sliceQuaternion().
                           transformInverse(Point3df(0., 0., 1.) ) );

    //cout << "Normal Vector before 1 : " << normalVector << endl ;
    // snap to slice position
    Point3df nVec = normalVector * normalVector.dot( pos - win->getPosition() ) ;
    pos = pos - nVec ;
    //cout << "Normal Vector before 2 : " << nVec << endl ;
    // cout << "snapped pos: " << pos << endl;

    Transformation* transf = theAnatomist->getTransformation(winRef, buckRef) ;
    AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;

    Point3df p ;
    if ( transf )
      p = Transformation::transform( pos, transf, voxelSize ) ;
    else
    {
      p = pos ;
      p[0] /= voxelSize[0] ;
      p[1] /= voxelSize[1] ;
      p[2] /= voxelSize[2] ;
    }

    // cout << "P : " << p << endl ;


    vector<float> bmin, bmax;
    g->boundingBox2D( bmin, bmax );

    Point3df vlOffset( bmin[0] / voxelSize[0] + 0.5,
                       bmin[1] / voxelSize[1] + 0.5,
                       bmin[2] / voxelSize[2] + 0.5) ;
    VolumeRef<AObject*>& volumeOfLabels = g->volumeOfLabels( 0 ) ;
    Point3d pVL( static_cast<int>( rint( p[0] - vlOffset[0] ) ),
                 static_cast<int>( rint( p[1] - vlOffset[1] ) ),
                 static_cast<int>( rint( p[2] - vlOffset[2] ) ) );
    int maxNbOfPoints ;
    if( _sharedData->myMaxSize > 0 )
    {
      vector<float> vs = volumeOfLabels.getVoxelSize();
      maxNbOfPoints
        = int( _sharedData->myMaxSize / ( vs[0] * vs[1] * vs[2] ) );
    }
    else
      maxNbOfPoints
        = volumeOfLabels.getSizeX() * volumeOfLabels.getSizeY()
          * volumeOfLabels.getSizeZ();
    vector<float> vpos = win->getFullPosition();

    float realLowLevel = realMin() ;
    float realHighLevel = realMax() ;

    // cout << "\tpVL = " << pVL << endl ;

    std::queue<Point3d> trialPoints ;
    ChangesItem change ;
    AObject** toChange ;
    if( add )
    {
      change.after = region ;
      toChange = &change.before ;
    }
    else
    {
      change.before = region ;
      toChange = &change.after ;
    }
    Point3d neighbor ;
    bool replace = PaintActionSharedData::instance()->replaceMode();
    Point3d dims( volumeOfLabels.getSizeX(), volumeOfLabels.getSizeY(),
                  volumeOfLabels.getSizeZ() );
    if( in( dims, pVL ) )
    {
      vector<float> vs = _sharedData->myCurrentImage->voxelSize();
      // mixedTexValue is in mm, not in voxels.
      vpos[0] = pVL[0] * vs[0];
      vpos[1] = pVL[1] * vs[1];
      vpos[2] = pVL[2] * vs[2];
      float val
        = _sharedData->myCurrentImage->mixedTexValue( vpos );
      if ( val < realLowLevel || val > realHighLevel )
        return ;
      else
        trialPoints.push(pVL) ;
      int regionSize = 0 ;
      Point3d pc ;
      Connectivity  * connec = 0 ;
      //cout << "_sharedData->myDimensionMode == " << (TWOD ? "TWOD" : "ThreeD") << endl ;
      if( _sharedData->myDimensionMode == RoiLevelSetActionSharedData::TWOD )
      {
        //cout << "inside " << "TWOD" << "Normal Vect = " << normalVector << endl ;

        if( normalVector[2] > 0.9 )
        {
          //cout << "CONNECTIVITY_4_XY" << endl ;
          connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_XY );
        }
        else if( normalVector[1] > 0.9 )
        {
          //cout << "CONNECTIVITY_4_XZ" << endl ;
          connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_XZ );
        }
        else if( normalVector[0] > 0.9 )
        {
          connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_YZ );
          //cout << "CONNECTIVITY_4_YZ" << endl ;
        }
        else
        {
          AWarning(
            "Level set should not be used in 2d mode on oblique views" );
          return;
        }
      }
      else
        connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_6_XYZ );

      while( !trialPoints.empty() && regionSize < maxNbOfPoints )
      {
        //cout << "Queue size : " <<  trialPoints.size() << "\tCurrent Point = " << pc << endl ;

        pc = trialPoints.front() ;
        trialPoints.pop() ;

        for( int n = 0 ; n < connec->nbNeighbors() ; ++n )
        {
          neighbor = pc + connec->xyzOffset(n) ;
          if( in(dims, neighbor) )
            if( fillPoint( neighbor, vpos[3], volumeOfLabels,
                           region, realLowLevel, realHighLevel,
                           toChange, trialPoints, replace ) )
            {
              changes.push_back(pair<Point3d, ChangesItem>(
                neighbor, change ) );
              ++regionSize ;
            }
        }
      }
      if( regionSize >= maxNbOfPoints && !trialPoints.empty() )
        cout << "Warning: region has been truncated due to size limitation "
          "- it could grow bigger if limit is increaded or removed.\n";
    }
    else cout << "outside dims.\n";
  }
}


bool 
RoiLevelSetAction::in( const Point3d& dims, Point3d p )
{
  if ( p[0] < 0 || p[0] > dims[0] - 1 ||
       p[1] < 0 || p[1] > dims[1] - 1 ||
       p[2] < 0 || p[2] > dims[2] - 1 )
    return false ;

  return true ;
}



bool 
anatomist::RoiLevelSetAction::fillPoint(
  const Point3d& pc, int t, VolumeRef<anatomist::AObject*>& volumeOfLabels,
  anatomist::AGraphObject * region, float realLowLevel, float realHighLevel,
  anatomist::AObject** toChange, std::queue<Point3d>& trialPoints,
  bool replace )
{
  Point3d dims( volumeOfLabels.getSizeX(), volumeOfLabels.getSizeY(),
                volumeOfLabels.getSizeZ() );
  if( in( dims, pc ) )
  {
    vector<float> vs = _sharedData->myCurrentImage->voxelSize();
    vector<float> vpos( 4 );
    vpos[0] = pc[0] * vs[0];
    vpos[1] = pc[1] * vs[1];
    vpos[2] = pc[2] * vs[2];
    vpos[3] = t;

    float val = _sharedData->myCurrentImage->mixedTexValue( vpos );
    if( (volumeOfLabels->at( pc ) != region) &&
        (val >= realLowLevel) && (val <= realHighLevel)  &&
        ( replace || ( (!replace) && volumeOfLabels->at( pc ) == 0 )) )
    {
/*     if( (volumeOfLabels->at( pc ) != region) &&  (val >= realLowLevel) && (val <= realHighLevel) ){ */
      trialPoints.push(pc) ;
      *toChange = volumeOfLabels->at( pc ) ;

      volumeOfLabels->at( pc ) = region ;
      return true ;
    }
  }
  return false ;
}

