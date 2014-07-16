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
#include <aims/qtcompat/qhbuttongroup.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcombobox.h>
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
    QHButtonGroup * myActivateButtonGroup ;
    QPushButton * myActivateButton ;
    QPushButton * myDeactivateButton ;
    
    QHGroupBox * myLowLevelGroupBox ;
    QSlider * myLowLevelSlider ;
    QLabel * myLowLevelValueLabel ;
    
    QHGroupBox * myHighLevelGroupBox ;
    QSlider * myHighLevelSlider ;
    QLabel * myHighLevelValueLabel ;

    QHBox * myModes ;
    QVBox * myDimensionMode ;
    QVGroupBox * myDimensionBox ;
    QVButtonGroup * myDimensions ;
    QHGroupBox * myMaxSizeBox ;
    QSlider * myMaxSizeSlider ;
    QLabel * myMaxSizeLabel ;
    
    QVGroupBox * myBlobBox ;
    QLineEdit * myMaxSizeLineEdit ;
    QHGroupBox * myPercentMaxBox ;
    QLineEdit * myPercentMaxLineEdit ;
    
    QVBox * myMixBox ;
    QVGroupBox * myMixMethodBox ;
    QComboBox * myMixMethods ;
    QHGroupBox * myMixFactorBox ;
    QSlider * myMixFactor ;
    QLabel * myMixFactorLabel ;
  } ;
};

RoiLevelSetActionView::RoiLevelSetActionView( anatomist::RoiLevelSetAction *  action,
					      QWidget * parent ) :
  QVBox(parent), Observer(), myChangingFlag(false),  myUpdatingFlag(false)
{
  //cout << "RoiLevelSetActionView::RoiLevelSetActionView" << endl ;
  _private = new RoiLevelSetActionView_Private ;
  _private->myLevelSetAction = action ;
  RoiLevelSetActionSharedData::instance()->addObserver(this) ;

  _private->myHistoPlot = new RoiHistoPlot(this, 100) ;
  _private->myActivateButtonGroup = new QHButtonGroup(tr("Threshold Preview Activation"), this ) ;
  _private->myActivateButton = new QPushButton( tr("Activate Threshold Preview"), 
						_private->myActivateButtonGroup ) ;
  _private->myActivateButton->setToggleButton(true) ;

  _private->myDeactivateButton = new QPushButton( tr("Deactivate Threshold Preview"),
						  _private->myActivateButtonGroup ) ;
  _private->myDeactivateButton->setToggleButton(true) ;
  
  _private->myActivateButtonGroup->insert( _private->myActivateButton, 0 ) ;
  _private->myActivateButtonGroup->insert( _private->myDeactivateButton, 1 ) ;
  _private->myActivateButtonGroup->setExclusive(true) ;
  _private->myActivateButtonGroup->setButton( 1 ) ;

  _private->myLowLevelGroupBox =  new QHGroupBox( tr("Low Level"), this ) ;
  _private->myLowLevelSlider = 
    new QSlider( -10, 1010, 1, int(_private->myLevelSetAction->lowLevel() * 1000. ),
		 Qt::Horizontal, _private->myLowLevelGroupBox ) ;
  _private->myLowLevelValueLabel =  new QLabel( QString::number( _private->myLevelSetAction->realMin() ), 
						_private->myLowLevelGroupBox ) ;
  _private->myLowLevelValueLabel->setFixedWidth(80) ;
  _private->myLowLevelSlider->setEnabled(false) ;
  _private->myHistoPlot->lowChanged( _private->myLevelSetAction->realMin() ) ;
  _private->myHighLevelGroupBox =  new QHGroupBox( tr("High Level"), this ) ;
  _private->myHighLevelSlider = new QSlider( -10, 1010, 1, int(_private->myLevelSetAction->highLevel()*1000.),
					     Qt::Horizontal, 
					     _private->myHighLevelGroupBox );
  _private->myHighLevelValueLabel =  new QLabel( QString::number( _private->myLevelSetAction->realMax() ), 
						 _private->myHighLevelGroupBox ) ;
  _private->myHighLevelValueLabel->setFixedWidth(80) ;
  _private->myHighLevelSlider->setEnabled(false) ;
  _private->myHistoPlot->highChanged( _private->myLevelSetAction->realMax() ) ;

  _private->myModes = new QHBox( this ) ;
  _private->myDimensionMode = new QVBox( _private->myModes ) ;
  //_private->myDimensionBox = new QVGroupBox(tr("Dimension"), _private->myModes )  ;
  _private->myDimensions = new QVButtonGroup( tr("Dimension"), _private->myDimensionMode ) ;
  new QRadioButton(tr("2D"), _private->myDimensions) ;
  new QRadioButton(tr("3D"), _private->myDimensions) ;
  _private->myDimensions->setExclusive(true) ;
  _private->myDimensions->setButton(0) ;
  
//   _private->myMaxSizeSlider = new QSlider( 0, 100, 10, 30, Qt::Horizontal, _private->myMaxSizeBox ) ;
//   _private->myMaxSizeSlider->setEnabled(true) ;

//   _private->myMaxSizeLabel = new QLabel( "30 %", _private->myMaxSizeBox ) ;
//   _private->myMaxSizeLabel->setFixedWidth(80) ;

  _private->myBlobBox = new QVGroupBox(tr("Blob segmentation"), _private->myModes ) ;
  _private->myMaxSizeBox = new QHGroupBox(tr("Region max size"), _private->myDimensionMode ) ;
  _private->myMaxSizeLineEdit = new QLineEdit( QString::number( _private->myLevelSetAction->maxSize() ), 
					       _private->myMaxSizeBox ) ;
  _private->myMaxSizeLineEdit->setFixedWidth(60) ;
  new QLabel( "mm3", _private->myMaxSizeBox ) ;

  _private->myPercentMaxBox = new QHGroupBox(tr("Percentage of extremum"), _private->myDimensionMode ) ;
  _private->myPercentMaxLineEdit = new QLineEdit( 
			  QString::number( _private->myLevelSetAction->percentageOfMaximum() ), 
			  _private->myPercentMaxBox ) ;
  _private->myPercentMaxLineEdit->setFixedWidth(60) ;
  new QLabel( " %", _private->myPercentMaxBox ) ;

  _private->myMixBox = new QVBox( _private->myModes ) ;
  
  _private->myMixMethodBox = new QVGroupBox( tr("MixMethod"), _private->myMixBox ) ;
  _private->myMixMethods = new QComboBox( _private->myMixMethodBox ) ;
  _private->myMixMethods->insertItem( "GEOMETRIC" ) ;
  _private->myMixMethods->insertItem( "LINEAR" ) ;
  _private->myMixMethods->setCurrentItem( 0 ) ;
  
  _private->myMixFactorBox = new QHGroupBox(tr("Mixing Factor"), _private->myMixBox ) ;
  _private->myMixFactor = new QSlider( 0, 100, 10, 50, Qt::Horizontal, _private->myMixFactorBox ) ;
  _private->myMixFactor->setEnabled(false) ;

  _private->myMixFactorLabel = new QLabel( "10 %", _private->myMixFactorBox ) ;
  _private->myMixFactorLabel->setFixedWidth(80) ;
  
  connect( _private->myActivateButtonGroup, SIGNAL(clicked(int)),
	   this, SLOT(levelSetActivationChanged(int) ) ) ;
  connect( _private->myLowLevelSlider, SIGNAL(valueChanged(int)), 
	   this, SLOT(lowLevelChanged(int) ) ) ;
  connect( _private->myHighLevelSlider, SIGNAL(valueChanged(int)), 
	   this, SLOT(highLevelChanged(int) ) ) ;
  connect( _private->myDimensions, SIGNAL(clicked(int)), 
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
RoiLevelSetActionView::levelSetActivationChanged( int button )
{
  //cout << "RoiLevelSetActionView::levelSetActivationChanged" << endl ;
  myChangingFlag = true ;
  if( button == 0 )
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
  else if( button == 1 )
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
  _private->myLevelSetAction->setMixMethod( (const char *) method ) ;
  
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
      _private->myActivateButtonGroup->setButton(0) ;
      _private->myHistoPlot->activate() ;
      _private->myLowLevelSlider->setEnabled(true) ;
      _private->myHighLevelSlider->setEnabled(true) ;
    }
  else
    {
      _private->myActivateButtonGroup->setButton(1) ;
      _private->myHistoPlot->deactivate() ;
      _private->myLowLevelSlider->setEnabled(false) ;
      _private->myHighLevelSlider->setEnabled(false) ;
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
      _private->myDimensions->id(_private->myDimensions->selected() ) )
    _private->myDimensions->
      setButton(_private->myLevelSetAction->dimensionMode() ) ;

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
	  _private->myMixMethods->setCurrentItem( 0 ) ;
	  _private->myMixFactor->setEnabled(false) ;
	}
      else
	{
	  _private->myMixMethods->setCurrentItem( 1 ) ;
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
    / _sharedData->myCurrentImage->getOrCreatePalette()->colors()->dimX() ;
  
  
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
    / _sharedData->myCurrentImage->getOrCreatePalette()->colors()->dimX() ;
  
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
  unsigned		dimx = pal->dimX(), dimy = pal->dimY();
  unsigned		dimxmax = 256, dimymax = 256;

  if( objpal->palette1DMapping() == AObjectPalette::FIRSTLINE )
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
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) ){
    g->clearLabelsVolume() ;
    g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() - g->MinX2D() ) + 1, 
				 static_cast<int>( g->MaxY2D() - g->MinY2D() ) + 1,
				 static_cast<int>( g->MaxZ2D() - g->MinZ2D() ) + 1 ) ;
  } else {
    
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
      
      labels( iter->first ) = 0  ;
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

  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) )
    {
      g->clearLabelsVolume() ;
      g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() 
                                                     - g->MinX2D() ) + 1,
                                   static_cast<int>( g->MaxY2D()
                                                     - g->MinY2D() ) + 1,
                                   static_cast<int>( g->MaxZ2D()
                                                     - g->MinZ2D() ) + 1 ) ;
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
    int timePos = win->getTimeSliderPosition() ;

    //cout << "Pos : " << pos << endl ;

    // cout << "Position from cursor : (" << x << " , "<< y << ") = "
    //   << pos << endl ;

    Point3df voxelSize = region->VoxelSize() ;

    Point3df normalVector( win->sliceQuaternion().
                            apply(Point3df(0., 0., 1.) ) ) ;
    Point3df xAx( win->sliceQuaternion().
                            apply(Point3df(1., 0., 0.) ) ) ;
    Point3df yAx( win->sliceQuaternion().
                            apply(Point3df(0., 1., 0.) ) ) ;

    Point3d xAxis( (int)rint(xAx[0]), (int)rint(xAx[1]), (int)rint(xAx[2]) ) ;
    Point3d yAxis( (int)rint(yAx[0]), (int)rint(yAx[1]), (int)rint(yAx[2]) ) ;
    Point3d zAxis( (int)rint(normalVector[0]), (int)rint(normalVector[1]), (int)rint(normalVector[2]) ) ;

    //cout << "Normal Vector before 1 : " << normalVector << endl ;
    Point3df nVec = normalVector * normalVector.dot( pos - win->getPosition() ) ;
    pos = pos - nVec ;
    //cout << "Normal Vector before 2 : " << nVec << endl ;

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

    //cout << "P : " << p << endl ;


    Point3df vlOffset( g->MinX2D(), g->MinY2D(), g->MinZ2D() ) ;
    AimsData<AObject*>& volumeOfLabels = g->volumeOfLabels( 0 ) ;
    int maxNbOfPoints ;
    if( _sharedData->myMaxSize > 0 )
      maxNbOfPoints
        = int( _sharedData->myMaxSize /
          (volumeOfLabels.sizeX()*volumeOfLabels.sizeY()
            *volumeOfLabels.sizeZ() ) ) ;
    else
      maxNbOfPoints
        = volumeOfLabels.dimX()*volumeOfLabels.dimY()*volumeOfLabels.dimZ();
    Point3d pToInt( static_cast<int> ( p[0] +.5 ),
                    static_cast<int> ( p[1] +.5 ),
                    static_cast<int> ( p[2] +.5 ) ) ;
    Point3d pVL( static_cast<int> ( p[0] - vlOffset[0] +.5 ),
                  static_cast<int> ( p[1] - vlOffset[1] +.5 ),
                  static_cast<int> ( p[2] - vlOffset[2] +.5 ) );

    float realLowLevel = realMin() ;
    float realHighLevel = realMax() ;

    //cout << "\tpVL = " << pVL << endl ;

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
    bool replace = PaintActionSharedData::instance()->replaceMode() ;
    Point3d dims( volumeOfLabels.dimX(), volumeOfLabels.dimY(), volumeOfLabels.dimZ() ) ;
    if( in( dims, pVL ) )
    {
      float val
        = _sharedData->myCurrentImage->mixedTexValue(
          Point3df( pVL[0], pVL[1], pVL[2] ), timePos);
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
            if( fillPoint( neighbor, timePos, volumeOfLabels,
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
anatomist::RoiLevelSetAction::fillPoint( const Point3d& pc, int t,
					 AimsData<anatomist::AObject*>& volumeOfLabels, 
					 anatomist::AGraphObject * region, float realLowLevel, 
					 float realHighLevel,
					 anatomist::AObject** toChange,
					 std::queue<Point3d>& trialPoints, bool replace )
{
  Point3d dims( volumeOfLabels.dimX(), volumeOfLabels.dimY(), volumeOfLabels.dimZ()) ;
  if( in( dims, pc ) ){
    float val = _sharedData->myCurrentImage->mixedTexValue( Point3df( pc[0], pc[1], pc[2] ), t ) ;
    if( (volumeOfLabels( pc ) != region) &&  
	(val >= realLowLevel) && (val <= realHighLevel)  &&
	( replace || ( (!replace) && volumeOfLabels( pc ) == 0 )) ) {
/*     if( (volumeOfLabels( pc ) != region) &&  (val >= realLowLevel) && (val <= realHighLevel) ){ */
      trialPoints.push(pc) ;
      *toChange = volumeOfLabels( pc ) ;
      
      volumeOfLabels( pc ) = region ;
      return true ;
    }
  }
  return false ;
}

