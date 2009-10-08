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

#include <anatomist/action/dynsegmentaction.h>
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
#include <qpushbutton.h>
#include <aims/qtcompat/qhbuttongroup.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcombobox.h>
#include <queue>
#include <aims/resampling/quaternion.h>
#include <aims/math/math_g.h>
#include <anatomist/misc/error.h>

using namespace anatomist ;
using namespace aims;
using namespace std;

namespace anatomist
{
  class RoiDynSegmentActionView_Private {
  public:
    RoiDynSegmentAction * myDynSegmentAction ;
    
    QHGroupBox * myFaithBox ;
    QSlider * myFaithSlider ;
    QLabel * myFaithLabel ;

    QHGroupBox * myOrderBox ;
    QSlider * myOrderSlider ;
    QLabel * myOrderLabel ;
    
    QHGroupBox * myResultsBox ;    
    QLabel * myMeanValueLabel ;
    QLabel * mySigmaValueLabel ;
    
    QHBox * myModes ;
    QVBox * myLeftMode ;

    QHButtonGroup * myDimensions ;
    QHButtonGroup * myFindNearestMinimum ;

    QVBox * myRightMode ;
    QHButtonGroup * myComputeMode ;
    QHButtonGroup * myRefineMode ;
    
    QHGroupBox * myMaskBox ;
    QComboBox * myMask ;
  } ;
};

RoiDynSegmentActionView::RoiDynSegmentActionView( 
          anatomist::RoiDynSegmentAction *  action,
	  QWidget * parent ) :
  QVBox(parent), Observer(), myChangingFlag(false),  myUpdatingFlag(false)
{
  _private = new RoiDynSegmentActionView_Private ;
  _private->myDynSegmentAction = action ;
  _private->myDynSegmentAction->addObserver(this) ;

  _private->myFaithBox = new QHGroupBox( tr("Faith Interval"), this ) ;
  _private->myFaithSlider = new QSlider( 2, 6, 1, 3, Qt::Horizontal, _private->myFaithBox ) ;
  _private->myFaithLabel = new QLabel( "3 sigmas" , _private->myFaithBox ) ;

  _private->myOrderBox = new QHGroupBox( tr("Order Interval"), this ) ;
  _private->myOrderSlider = new QSlider( 1, 6, 1, 1, Qt::Horizontal, _private->myOrderBox ) ;
  _private->myOrderLabel = new QLabel( "1" , _private->myOrderBox ) ;

  _private->myResultsBox = new QHGroupBox( tr("Confidence Measurements"), this ) ;
  new QLabel( tr("Mean Error : "), _private->myResultsBox ) ;
  _private->myMeanValueLabel = new QLabel( "", _private->myResultsBox ) ;
  new QLabel( tr("Error Deviation : "), _private->myResultsBox ) ;
  _private->mySigmaValueLabel = new QLabel( "", _private->myResultsBox ) ;

  _private->myModes = new QHBox( this ) ;
  _private->myLeftMode = new QVBox( _private->myModes ) ;
  _private->myDimensions = new QHButtonGroup( tr("Dimension"), _private->myLeftMode ) ;
  new QRadioButton(tr("2D"), _private->myDimensions) ;
  new QRadioButton(tr("3D"), _private->myDimensions) ;
  _private->myDimensions->setExclusive(true) ;
  _private->myDimensions->setButton(0) ;

  _private->myFindNearestMinimum = new QHButtonGroup( tr("Find Nearest Minimum"), _private->myLeftMode ) ;
  new QRadioButton(tr("Yes"), _private->myFindNearestMinimum) ;
  new QRadioButton(tr("No"), _private->myFindNearestMinimum) ;
  _private->myFindNearestMinimum->setExclusive(true) ;
  _private->myFindNearestMinimum->setButton(0) ;

  _private->myRightMode = new QVBox( _private->myModes ) ;
  _private->myComputeMode = new QHButtonGroup( tr("Compute Mode"), _private->myRightMode ) ;
  new QRadioButton(tr("Region"), _private->myComputeMode) ;
  new QRadioButton(tr("Whole Image"), _private->myComputeMode) ;
  _private->myComputeMode->setExclusive(true) ;
  _private->myComputeMode->setButton(0) ;
  _private->myComputeMode->setEnabled(false) ;
  

  _private->myRefineMode = new QHButtonGroup( tr("Refine Mode"), _private->myRightMode ) ;
  new QRadioButton(tr("Yes"), _private->myRefineMode) ;
  new QRadioButton(tr("No"), _private->myRefineMode) ;
  _private->myRefineMode->setExclusive(true) ;
  _private->myRefineMode->setButton(0) ;

//   _private->myMaskBox = new QHGroupBox( tr("Mask"), _private->myRightMode ) ;
//   _private->myMask = new QComboBox( _private->myMaskBox ) ;
  
  
  connect( _private->myFaithSlider, SIGNAL(valueChanged(int)), 
	   this, SLOT(faithIntervalChanged(int) ) ) ;
  connect( _private->myOrderSlider, SIGNAL(valueChanged(int)), 
	   this, SLOT(orderChanged(int) ) ) ;
  connect( _private->myDimensions, SIGNAL(clicked(int)), 
	   this, SLOT(dimensionModeChanged(int) ) ) ;
  connect( _private->myComputeMode, SIGNAL(clicked(int)), 
	   this, SLOT(computeModeChanged(int) ) ) ;
  connect( _private->myRefineMode, SIGNAL(clicked(int)), 
	   this, SLOT(refineModeChanged(int) ) ) ;
  connect( _private->myFindNearestMinimum, SIGNAL(clicked(int)), 
	   this, SLOT(findNearestMinimumModeChanged(int) ) ) ;
}

RoiDynSegmentActionView::~RoiDynSegmentActionView()
{
  _private->myDynSegmentAction->deleteObserver(this) ;
}

void 
RoiDynSegmentActionView::faithIntervalChanged( int newFaithInterval )
{
  myChangingFlag = true ;
  _private->myDynSegmentAction->changeFaithInterval( newFaithInterval ) ;
  _private->myFaithLabel->setText( QString::number( newFaithInterval ) + QString( " Sigmas") ) ;
  myChangingFlag = false ;
}

void 
RoiDynSegmentActionView::orderChanged( int newOrder )
{
  myChangingFlag = true ;
  _private->myDynSegmentAction->changeOrder( newOrder ) ;
  _private->myOrderLabel->setText( QString::number( newOrder ) ) ;
  myChangingFlag = false ;
}

void 
RoiDynSegmentActionView::dimensionModeChanged( int mode ) 
{
  myChangingFlag = true ;
  if( mode == 0 )
    _private->myDynSegmentAction->dimensionModeTo2D() ;
  else
    _private->myDynSegmentAction->dimensionModeTo3D() ;    
  myChangingFlag = false ;
}

void 
RoiDynSegmentActionView::findNearestMinimumModeChanged( int mode ) 
{
  myChangingFlag = true ;
  if( mode == 0 )
    _private->myDynSegmentAction->findNearestMinimumModeOn() ;
  else
    _private->myDynSegmentAction->findNearestMinimumModeOff() ;    
  myChangingFlag = false ;
}

void 
RoiDynSegmentActionView::computeModeChanged( int /*mode*/ ) 
{
//   if( mode == 0 )
//     _private->myDynSegmentAction->setComputeModeToRegion() ;
//   else
//     _private->myDynSegmentAction->setComputeModeToImage() ;    
}

void 
RoiDynSegmentActionView::refineModeChanged( int mode ) 
{
  myChangingFlag = true ;
  if( mode == 0 )
    _private->myDynSegmentAction->refineModeOn() ;
  else
    _private->myDynSegmentAction->refineModeOff() ;    
  myChangingFlag = false ;
}

void 
RoiDynSegmentActionView::update( const anatomist::Observable *, void * )
{
  //cout << "RoiDynSegmentActionView::update" << endl ;

  if( myChangingFlag || myUpdatingFlag )
    return ;
  myUpdatingFlag = true ;
  
  //cout << "Updating ... " << endl ;

  _private->myFaithSlider->setValue( _private->myDynSegmentAction->faithInterval() ) ;
  _private->myFaithLabel->setText( QString::number( _private->myDynSegmentAction->faithInterval() )
					   +QString(" Sigmas") ) ;
  
  _private->myOrderSlider->setValue( _private->myDynSegmentAction->order() ) ;
  _private->myOrderLabel->setText( QString::number( _private->myDynSegmentAction->order() ) ) ;
  
  if( _private->myDynSegmentAction->dimensionMode() != 
      _private->myDimensions->id(_private->myDimensions->selected() ) )
    _private->myDimensions->setButton(_private->myDynSegmentAction->dimensionMode() ) ;

//   if( _private->myDynSegmentAction->computeMode() != 
//       _private->myComputeMode->id(_private->myComputeMode->selected() ) )
//     _private->myComputeMode->setButton(_private->myDynSegmentAction->computeMode() ) ;

  if( _private->myDynSegmentAction->refineMode() != 
      _private->myRefineMode->id(_private->myRefineMode->selected() ) )
    _private->myRefineMode->setButton( 1 - _private->myDynSegmentAction->refineMode() ) ;

  if( _private->myDynSegmentAction->findNearestMinimum() != 
      _private->myFindNearestMinimum->id(_private->myFindNearestMinimum->selected() ) )
    _private->myFindNearestMinimum->setButton( 1 - _private->myDynSegmentAction->findNearestMinimum() ) ;
  
  if( _private->myDynSegmentAction->displayResults() ){
    //cout << "DisplayResults" << endl ;
    _private->myMeanValueLabel->setText( QString::number( _private->myDynSegmentAction->meanValue() ) ) ;
    _private->mySigmaValueLabel->setText( QString::number( _private->myDynSegmentAction->sigmaValue() ) ) ;    
  } else{
    _private->myMeanValueLabel->setText( "" ) ;
    _private->mySigmaValueLabel->setText( "" ) ;
  }

  myUpdatingFlag = false ;
}



RoiDynSegmentAction::RoiDynSegmentAction() : 
  Action(), Observer(), Observable(), myCurrentImage(0),
  mySeed(0,0,0), mySeedChanged(true), myOrderChanged(true), myRefineMode(false), myFindNearestMinimumMode(false),
  myDisplayResults(false), myDimensionMode(TWOD), myFaithInterval(4), myOrder(1)
{
}

Action*
RoiDynSegmentAction::creator()
{
 return new RoiDynSegmentAction( ) ;
}

RoiDynSegmentAction::~RoiDynSegmentAction()
{
  if ( myCurrentImage )
    myCurrentImage->deleteObserver(this) ;
}

void 
RoiDynSegmentAction::unregisterObservable( Observable* o )
{
  Observer::unregisterObservable( o );
  myCurrentImage = 0 ;
}


bool 
RoiDynSegmentAction::in( const Point3d& dims, Point3d p )
{
  if ( p[0] < 0 || p[0] > dims[0] - 1 ||  
       p[1] < 0 || p[1] > dims[1] - 1 ||
       p[2] < 0 || p[2] > dims[2] - 1 )
    return false ;
  
  return true ;
}

string RoiDynSegmentAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "DynSegmentAction" );
}

AObject* 
RoiDynSegmentAction::getCurrentImage()
{
  //   set<AObject*> objs = view()->window()->Objects() ;
  
  //cout << "RoiDynSegmentAction::getCurrentImage" << endl ;
  AObject * gotIt = RoiManagementActionSharedData::instance()->
    getObjectByName( AObject::VOLUME, 
		     RoiManagementActionSharedData::instance()->
		     currentImage() ) ;
  if( gotIt != 0 )
    {
      if ( gotIt != myCurrentImage )
	{
	  if(myCurrentImage)
	    {
	      myCurrentImage->deleteObserver(this) ;
	      myPreviousComputing.clear() ;
	      myDisplayResults = false ;
	      setChanged() ;
	    }
	  myCurrentImage = gotIt ;
	  myCurrentImage->addObserver( this ) ;
	}
      setChanged() ;
      notifyObservers() ;
    }
  return gotIt ;
}

void
RoiDynSegmentAction::replaceRegion( int x, int y, int, int )
{
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
  if(!g)
    return ;
  
  Bucket * currentModifiedRegion ; 
  if ( ! ( currentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->window()*/ ) ) ) {
    return ;
  }
  
  // Effacer la région courante
  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;
  
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
      changes->push_back(pair<Point3d, ChangesItem>( iter->first, item ) )  ;
//       if( (*_sharedData->myCurrentChanges)[0][iter->first].before == 0 ) 
//   	cout << "change.before == 0" << endl ;
      
      labels( iter->first ) = 0  ;
      ++iter ;
    }
    
    //cout << "Region erased size : " << (*changes)[0].size() << endl ;
    
    if ( ! (*changes).empty() )
      RoiChangeProcessor::instance()->applyChange( changes ) ;
    
    //cout << "Region Erased" << endl ;
  }  
  setPointToSegmentByDiscriminatingAnalyse(x, y, 0, 0) ;
}

void
RoiDynSegmentAction::setPointToSegmentByDiscriminatingAnalyse( int x, int y, int, int ) 
{
  //cout << "Entering RoiDynSegmentAction::setPointToSegmentByDiscriminatingAnalyse" << endl ;
  
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }
  
  //cout << "\tx = " << x << "\ty = " << y << endl ;
  
  Referential* winRef = win->getReferential() ;

  Bucket * region =  RoiChangeProcessor::instance()->getCurrentRegion( 0 ) ;
  if( !region ){
    cerr << "warning : no region selected\n" ;
    return ;
  }
  Referential* buckRef = region->getReferential() ;
  //bool		newbck = myDeltaModifications->empty();
  Point3df pos ;
  if( win->positionFromCursor( x, y, pos ) )
    {
//       cout << "Inside Window" << endl ;
//       cout << "Pos : " << pos << endl ;
      
//       cout << "Position from cursor : (" << x << " , "<< y << ") = " 
// 	    << pos << endl ;
      
      Point3df voxelSize = region->VoxelSize() ;

      Point3df normalVector( win->sliceQuaternion().
			     apply(Point3df(0., 0., 1.1) ) ) ;
      Point3df xAx( win->sliceQuaternion().
			     apply(Point3df(1.1, 0., 0.) ) ) ;
      Point3df yAx( win->sliceQuaternion().
			     apply(Point3df(0., 1.1, 0.) ) ) ;
      
      myXAxis = Point3d( (int)xAx[0], (int)xAx[1], (int)xAx[2] ) ;
      myYAxis = Point3d( (int)yAx[0], (int)yAx[1], (int)yAx[2] ) ;
      myZAxis = Point3d( (int)normalVector[0], (int)normalVector[1], (int)normalVector[2] ) ;
      
      //       cout << "Normal Vector : " << normalVector << endl ;
      normalVector *= normalVector.dot( pos - win->GetPosition() ) ;
      pos = pos - normalVector ;
      
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
      Point3d pToInt( static_cast<int> ( p[0] +.5 ), 
		      static_cast<int> ( p[1] +.5 ), 
		      static_cast<int> ( p[2] +.5 ) ) ;
      Point3d pVL( static_cast<int> ( p[0] - vlOffset[0] +.5 ), 
		   static_cast<int> ( p[1] - vlOffset[1] +.5 ), 
		   static_cast<int> ( p[2] - vlOffset[2] +.5 ) );
      
//       myPreviousSeed = mySeed ;
      mySeed = pVL ;
      mySeedChanged = true ;
      
      //cout << "Seed changed" << endl ;
      
      pcaRegionGrowth() ;
    }
  else{
    //cout << "En dehors de l'image : x = " << x << "   y = " << y << endl ;
  }
}

void 
RoiDynSegmentAction::changeOrder( int newOrder )
{
  //cout << "New Order : " << newOrder << endl ;

  myOrder = newOrder ;
  myOrderChanged = true ;

//   if( mySeed != Point3d(0, 0, 0) )
//     pcaRegionGrowth() ;
  setChanged() ;
  notifyObservers() ;
}

void 
RoiDynSegmentAction::changeFaithInterval( int newFaithInterval )
{
  //cout << "New Faith Interval : " << newFaithInterval << endl ;
  
  myFaithInterval = newFaithInterval ;
  
//   if( mySeed != Point3d(0, 0, 0) )
//     pcaRegionGrowth() ;
  setChanged() ;
  notifyObservers() ;
}

Point3d 
RoiDynSegmentAction::maskHalfSize( const AObject * vol, int nbIndiv )
{
  // cas 3D
  Point3df voxSize( vol->VoxelSize() ) ;
  int max, min1, min2 ;
  if( voxSize[0] > voxSize[1] && voxSize[0] > voxSize[2] ){
    max = 0 ; min1 = 1 ; min2 = 2 ;
  } else if( voxSize[1] > voxSize[2] && voxSize[1] > voxSize[0]){
    max = 1 ; min1 = 0 ; min2 = 2 ;    
  } else {
    max = 2 ; min1 = 0 ; min2 = 1 ;    
  }
  Point3d halfSize(0, 0, 0) ;
  int ind = 0 ;
  while( ind < nbIndiv ){
    ++halfSize[max] ;
    halfSize[min1] = halfSize[max] * int( voxSize[max] / voxSize[min1] + 0.5 ) ;
    halfSize[min2] = halfSize[max] * int( voxSize[max] / voxSize[min2] + 0.5 ) ;
    ind = (2*halfSize[0]+1) * (2*halfSize[1]+1) * (2*halfSize[2]+1) ;
  }
  cout << "Nb max Insiv = " << ind  << endl ;
  //cout << "Mask Half Size = " << halfSize <<endl ;
  return halfSize ;
}

void 
RoiDynSegmentAction::pcaRegionGrowth( )
{
  //cout << "Entering RoiDynSegmentAction::pcaRegionGrowth" << endl ;
  
  getCurrentImage() ;
  if( !myCurrentImage ){
    cerr << "warning : no current image\n" ; 
    return ;
  }
  if( myCurrentImage->type() != AObject::VOLUME ){
    AWarning("No volume selected") ;
    return ;
  }
  
  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;

  Point3df	bmin, bmax ;
  Point3d       dims ;
  myCurrentImage->boundingBox( bmin, bmax );
  
  // the .1 addition is due to some unfortunate rounding
  dims[0] = static_cast<int>( (bmax[0] - bmin[0]) / myCurrentImage->VoxelSize()[0] + .1 ) ;
  dims[1] = static_cast<int>( (bmax[1] - bmin[1]) / myCurrentImage->VoxelSize()[1] + .1 ) ;
  dims[2] = static_cast<int>( (bmax[2] - bmin[2]) / myCurrentImage->VoxelSize()[2] + .1 ) ;
  
  //cout << "Dims = " << dims << endl ;
 
  int nbFrame = int(myCurrentImage->MaxT() + 1.1) ;
  if ( nbFrame == 1 ){
    AWarning("Can only be used on dynamic data") ;
    return ;
  }
  
  Point3d halfSize = maskHalfSize( myCurrentImage, int( ( myCurrentImage->MaxT()+1 ) * 2.5 ) ) ;
  cout << "Mask half size " << halfSize << endl ;
  // s'il est necessaire de recalculer la matrice d'erreur et l'ereur de référence
  if( mySeedChanged || myOrderChanged ){
    // Preparation des individus our l'acp.
    myMeanSignal = vector<float>(nbFrame, 0.) ;
    
    if( !myFindNearestMinimumMode )
      if( !evaluateError( mySeed, halfSize, dims,
			myErrorMatrix, myMeanSignal,
			myInsideMeanError, myInsideSigmaError, true ) )
	return ;
    else
      if( !findLocalBestSeed( dims, halfSize ) )
	return ;
  }

  // Region growth from seed point with error < meanError + faithInterval * errorDeviation
  
  
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  //AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
  if(!g)
    return ;
  
  Bucket * currentModifiedRegion ; 
  if ( ! ( currentModifiedRegion = 
	   RoiChangeProcessor::instance()->getCurrentRegion( 0/*view()->window()*/ ) ) ) {
    return ;
  }
  
  growth( changes ) ;
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
   if( myRefineMode == true ){
     RoiChangeProcessor::instance()->undo() ;
     refinePCA( changes ) ;
   }
  
  mySeedChanged = false ;
  myOrderChanged = false ;

  myDisplayResults = true ;
  setChanged() ;
  notifyObservers() ;
}

bool
RoiDynSegmentAction::findLocalBestSeed( const Point3d& dims, const Point3d& halfSize )
{
//   if( search ){
//     if( myPreviousSearch->bucket()[0].find( mySeed ) != myPreviousSearch->bucket()[0].end() ){
//       mySeed = myPreviousSeed ;
//       return false ;
//     } else 
//       myPreviousSearch.clear() ;
//   }else
//     myPreviousSearch.clear() ;
  
  //cout << "RoiDynSegmentAction::findLocalBestSeed" << endl ;
  Point3d currentPoint(mySeed), minDirection(0,0,0) ;
  AimsData<float> errorMatrix ;
  vector<float> meanSignal ;
  float meanError = 0., varError = 0., minMeanError = 1. ;
  bool minFound = false, valid = false ;
  while( !minFound ){
    currentPoint += minDirection ;
    minDirection = -myXAxis ;
    
    if( in(dims, currentPoint - myXAxis) ){
      if( evaluateError( currentPoint - myXAxis, halfSize, dims, errorMatrix, meanSignal, 
			 minMeanError, varError ) )
	valid = true ;
    }
    if( in(dims, currentPoint + myXAxis) ){
      if( evaluateError( currentPoint + myXAxis, halfSize, dims, errorMatrix, meanSignal, 
		     meanError, varError ) )
	valid = true ;
      if( meanError < minMeanError ){
	minMeanError = meanError ;
	minDirection = myXAxis ;
      }
    }
    if( in(dims, currentPoint - myYAxis) ){
      if( evaluateError( currentPoint - myYAxis, halfSize, dims, errorMatrix, meanSignal, 
		     meanError, varError ) )
	valid = true ;
      if( meanError < minMeanError ){
	minMeanError = meanError ;
	minDirection = -myYAxis ;
      }
    }
    if( in(dims, currentPoint + myYAxis) ){
      if( evaluateError( currentPoint + myYAxis, halfSize, dims, errorMatrix, meanSignal, 
		     meanError, varError ) )
	valid = true ;
      if( meanError < minMeanError ){
	minMeanError = meanError ;
	minDirection = myYAxis ;
      }
    }
    if( myDimensionMode == THREED ){
      if( in(dims, currentPoint - myZAxis) ){
	if( evaluateError( currentPoint - myZAxis, halfSize, dims, errorMatrix, meanSignal, 
		       meanError, varError ) )
	  valid = true ;
	if( meanError < minMeanError ){
	  minMeanError = meanError ;
	  minDirection = -myZAxis ;
	}
      }
      if( in(dims, currentPoint + myZAxis) ){
	if( evaluateError( currentPoint + myZAxis, halfSize, dims, errorMatrix, meanSignal, 
		       meanError, varError ) )
	  valid = true ;
	if( meanError < minMeanError ){
	  minMeanError = meanError ;
	  minDirection = myZAxis ;
	}
      }
    }
    
    if( evaluateError( currentPoint, halfSize, dims, errorMatrix, meanSignal, 
		   meanError, varError ) )
      valid = true ;
    //cout << "Current mean error = " << meanError <<  endl ;
    if (!valid )
      return false ;
    if( meanError < minMeanError ){
      minMeanError = meanError ;
      minFound = true ;
    }
  }
  evaluateError( currentPoint, halfSize, dims, errorMatrix, meanSignal, 
		 meanError, varError, true ) ;
  
  mySeed = currentPoint ;
  myMeanSignal = meanSignal ;
  myInsideMeanError = minMeanError ;
  myInsideSigmaError = varError ;
  myErrorMatrix = errorMatrix.clone() ;
  
  return true ;
}

bool 
RoiDynSegmentAction::evaluateError( const Point3d& p, 
				    const Point3d& halfSize, 
				    const Point3d& dims,
				    AimsData<float>&  errorMatrix, 
				    vector<float>& meanSignal,
				    float& mean, float& var, 
				    bool forceComputing )
{
  //cout << "RoiDynSegmentAction::evaluateError" << endl ;
  
  if( !myCurrentImage )
    return false ;
  int nbFrame = int(myCurrentImage->MaxT() + 1.1) ;
  AimsData<float> matriceIndiv( (2*halfSize[0]+1)*(2*halfSize[1]+1)
				*(2*halfSize[2]+1), nbFrame );
  map<Point3d, float, PointLess>::iterator found ;
  if(!forceComputing )
    if( (found = myPreviousComputing.find(p) ) != myPreviousComputing.end() ){
      mean = found->second ;
      //cout << "Already Computed"  << endl ;
      return true ;
    }
  float val ;
  int nbIndiv = 0, nbInvalid = 0 ;
  meanSignal = vector<float>( nbFrame, 0. ) ;
  for( int i = -halfSize[0] ; i <= halfSize[0]  ; ++i )
    for( int j = -halfSize[1] ; j <= halfSize[1]  ; ++j )
      for( int k = -halfSize[2] ; k <= halfSize[2]  ; ++k ){
	bool valid = true ;
	for( int t = 0 ; t < nbFrame && valid == true ; ++t ){
	  if( in( dims, p + Point3d(i, j, k) ) ){
	    val = myCurrentImage->mixedTexValue( Point3df( p[0] + i, p[1] + j, p[2] + k ), t ) ;
	    if ( val <= 0. )
		valid = false ;
	    matriceIndiv( nbIndiv, t ) = val  ;
	    meanSignal[t] += val ;
	  }
	}
	if ( valid )
	  ++nbIndiv ;
	else 
	  ++nbInvalid ;
      }
  
  if ( nbIndiv < nbFrame ) {
    cout << "Non physiological data " << endl ;
    return false ;
  }

  computeErrorMatrix( matriceIndiv, errorMatrix, meanSignal ) ;
  
  int nbInd = 0 ;
  float sum = 0., sum2 = 0. ;
  float err ;
  
  // Calcul de la moyenne et l'écart type des erreurs
  for( int i = -halfSize[0] ; i <= halfSize[0]  ; ++i )
    for( int j = -halfSize[1] ; j <= halfSize[1]  ; ++j )
      for( int k = -halfSize[2] ; k <= halfSize[2]  ; ++k ){
	bool valid = true ;
	for( int t = 0 ; t < nbFrame ; ++t )
	  if( myCurrentImage->mixedTexValue( Point3df( p[0] + i, p[1] + j, p[2] + k ), t ) <= 0. )
	    valid = false ;
	if ( valid ) {
	  err = error( myCurrentImage, Point3df( p[0] + i, p[1] + j, p[2] + k ), meanSignal, errorMatrix ) ;
	  //cout << "\tError : " << err << endl ;
	  
	  sum += err  ;
	  sum2 += err * err ;
	  ++nbInd ;
	}
      }
  if( nbInd <= nbFrame )
    {
      cout << "No significant data around selected point" << endl ;
      return false ;
    }
  mean = sum / nbInd ;
  var = sqrt( sum2 / nbInd -  mean*mean ) ;
  
  
  // To avoid to compute twice
  myPreviousComputing[p] = mean ;
  return true ;
}


void 
RoiDynSegmentAction::growth( list< pair< Point3d, ChangesItem> >* changes )
{
  bool replaceMode = PaintActionSharedData::instance()->replaceMode() ;

  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
  if(!g)
    return ;

  Point3df	bmin, bmax ;
  Point3d       dims ;
  
  if( !myCurrentImage )
    return ;
  
  myCurrentImage->boundingBox( bmin, bmax );
  
  // the .1 addition is due to some unfortunate rounding
  dims[0] = static_cast<int>( (bmax[0] - bmin[0]) / 
			      myCurrentImage->VoxelSize()[0] + .1 ) ;
  dims[1] = static_cast<int>( (bmax[1] - bmin[1]) / 
			      myCurrentImage->VoxelSize()[1] + .1 ) ;
  dims[2] = static_cast<int>( (bmax[2] - bmin[2]) / 
			      myCurrentImage->VoxelSize()[2] + .1 ) ;
  
  AimsData<AObject*>& volumeOfLabels = g->volumeOfLabels( 0 ) ;
  Bucket * currentModifiedRegion = RoiChangeProcessor::instance()->getCurrentRegion( 0 ) ;
  ChangesItem item ;
  item.after = go ;
  item.before = 0 ;

  queue<Point3d> insideList ;
  item.before = volumeOfLabels( mySeed ) ;
  
  if( (!replaceMode) && (volumeOfLabels( mySeed ) != 0) )
    return ;

  volumeOfLabels( mySeed ) = currentModifiedRegion ;
  changes->push_back(pair<Point3d, ChangesItem>( mySeed, item ) )  ;
  insideList.push(mySeed) ;
    
  while( !insideList.empty() ){
    Point3d p = insideList.front() ;
    insideList.pop() ;
    
    Point3d pc = p - myXAxis ;
    if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ) )
      if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	    < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( pc ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
    pc = p + myXAxis ;
    if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ))
      if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	    < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( pc ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
    
    pc = p - myYAxis ;
    if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ))
      if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	    < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( pc ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
    
    pc = p + myYAxis ;
    if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ))
      if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	    < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( pc ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
    
    if( myDimensionMode == THREED ){
      pc = p - myZAxis ;
      if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ))
	if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	  if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	      < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( p ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
      
      pc = p + myZAxis ;
      if( in( dims, pc ) && volumeOfLabels( pc ) != currentModifiedRegion && 
	( replaceMode || volumeOfLabels( pc ) == 0 ) )
	if( valid( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ) ) )
	  if( error ( myCurrentImage, Point3df( pc[0], pc[1], pc[2] ), myMeanSignal, myErrorMatrix ) 
	      < myInsideMeanError + myFaithInterval * myInsideSigmaError ){
	  item.before = volumeOfLabels( pc ) ;
	  volumeOfLabels( pc ) = currentModifiedRegion ;
	  changes->push_back(pair<Point3d, ChangesItem>( pc, item ) )  ;
	  insideList.push( pc ) ;
	}
    }
  }
}

void 
RoiDynSegmentAction::refinePCA(  list< pair< Point3d, ChangesItem> >* changes )
{
  if( !myCurrentImage )
    return ;
  int nbFrame = int(myCurrentImage->MaxT() + 1.1) ;
  list< pair< Point3d, ChangesItem> >::const_iterator bckIter((*changes).begin()), 
    bckLast( (*changes).end() ) ;
  AimsData<float> matriceIndiv( (*changes).size(), nbFrame );
  
  myMeanSignal = vector<float>(nbFrame, 0.) ;
  
  int indiv = 0 ;
  while( bckIter != bckLast ){
    Point3df pIn( (bckIter->first)[0], (bckIter->first)[1], (bckIter->first)[2] ) ;
    for( int t = 0 ; t < nbFrame ; ++t ) {
      matriceIndiv( indiv, t ) = myCurrentImage->mixedTexValue( pIn, t ) ;
      myMeanSignal[t] += matriceIndiv( indiv, t ) ;
    }
    ++bckIter ; ++indiv ;
  }
  
  if( indiv <= 1 )
    return ;
  computeErrorMatrix( matriceIndiv, myErrorMatrix, myMeanSignal ) ;
  
  bckIter = (*changes).begin() ;
  
  float sum = 0., sum2 = 0. ;
  float err ;
  
  // Calcul de la moyenne et l'écart type des erreurs
  while( bckIter != bckLast )
    {
      err = error( myCurrentImage, Point3df((bckIter->first)[0], (bckIter->first)[1], (bckIter->first)[2] ),
		   myMeanSignal, myErrorMatrix ) ;
      //cout << "\tError : " << err << endl ;
      
      sum += err  ;
      sum2 += err * err ;
      ++bckIter ;
    }
  
  
  myInsideMeanError = sum / indiv ;
  myInsideSigmaError = sqrt( sum2 / indiv -  myInsideMeanError*myInsideMeanError ) ;

  //cout << "MeanError = " << myInsideMeanError << endl ;
  //cout << "DeviationError = " << myInsideSigmaError << endl ;
    
  list< pair< Point3d, ChangesItem> > * newChanges = new list< pair< Point3d, ChangesItem> > ;
  growth( newChanges ) ;

  if ( ! (*newChanges).empty() )
    RoiChangeProcessor::instance()->applyChange( newChanges ) ;
  myDisplayResults = true ;
  setChanged() ;
  notifyObservers() ;
}

void 
RoiDynSegmentAction::computeErrorMatrix( AimsData<float>& matriceIndiv, 
				       AimsData<float>& errorMatrix,
				       vector<float>& meanSignal )
{
  if( !myCurrentImage )
    return ;
  int nbFrame = int(myCurrentImage->MaxT() + 1.1) ;
  
  for( int t = 0 ; t < nbFrame ; ++t ){
    meanSignal[t] /= matriceIndiv.dimX() ;
    for( int ind = 0 ; ind < matriceIndiv.dimX() ; ++ind )
      matriceIndiv( ind, t ) -= meanSignal[t] ;
  }
  
  // Matrice des correlations
  AimsData< double >  matVarCov(nbFrame, nbFrame);
  
  int                 x1, y1;
  ForEach2d( matVarCov, x1, y1 )
    {
      for(int k=0;  k < matriceIndiv.dimX()  ;++k)
	matVarCov(x1, y1) += matriceIndiv(k, x1) * matriceIndiv(k, y1);	
      matVarCov(x1, y1) /= matriceIndiv.dimX() - 1 ;
    }
  
  // Decomposition SVD 
  AimsSVD< double >  svd;
  svd.setReturnType( AimsSVD< double >::VectorOfSingularValues );
  AimsData< double > eigenVal  = svd.doit( matVarCov );
  
  svd.sort(matVarCov, eigenVal);
  
  // Calcul de la matrice d'erreurs
  AimsData< float > errorMatrixk( nbFrame, nbFrame ) ;
  vector< AimsData<float>* > pkk(myOrder) ;
  for(int order = 0 ; order < myOrder ; ++order )
    pkk[order] = new AimsData<float>(nbFrame) ;
  
  for( int i = 0 ; i < myOrder ; ++i )
    for( int t = 0 ; t < nbFrame ; ++t )
      (*pkk[i])(t) = matVarCov( t, i ) ;
  
  errorMatrix = AimsData<float>(nbFrame, nbFrame) ;
  for( int t = 0 ; t < nbFrame ; ++t )
    errorMatrix(t, t) = 1. ;
  
  cerr << "Crossing " << endl ;
  for(int order = 0 ; order < myOrder ; ++order )
    errorMatrix = errorMatrix - pkk[order]->cross( pkk[order]->clone().transpose() ) ;
  cerr << "Crossed " << endl ;
}

void 
RoiDynSegmentAction::update( const Observable *, void * )
{
  //cout << "RoiDynSegmentAction::update" << endl ;

  if( !getCurrentImage() )
    return ;

  setChanged() ;
  notifyObservers() ;
}

QWidget * 
RoiDynSegmentAction::actionView( QWidget * parent )
{
  RoiDynSegmentActionView * obs = new RoiDynSegmentActionView( this, parent ) ;
  
  return obs ;
}
