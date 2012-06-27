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

#include <anatomist/action/morphomath.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/action/roimanagementaction.h>
#include <aims/morphology/morphology_g.h>
#include <anatomist/misc/error.h>

#include <aims/qtcompat/qhgroupbox.h>
#include <qslider.h>
#include <qlabel.h>
#include <aims/qtcompat/qhbuttongroup.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qpushbutton.h>
#include <aims/qtcompat/qgrid.h>
#include <qradiobutton.h>



using namespace anatomist ;
using namespace aims ;
using namespace carto ;
using namespace std ;

namespace anatomist
{
  class RoiMorphoMathActionView_Private{
  public:
    RoiMorphoMathAction * myRoiMorphoMathAction ;

    QHGroupBox * myStructElRadiusBox ;
    QSlider * myStructElRadiusSlider ;
    QLabel * myStructElRadiusLabel ;
    
    QHButtonGroup * myDistanceMode ;
    QHButtonGroup * myRegionMode ;
    
    QVGroupBox * myMorphoButtons ;
    QPushButton * myDefaultButton ;

    QGrid * myMorphoGrid ;
    
    QPushButton * myErosionButton ;
    QPushButton * myDilationButton ;
    QPushButton * myOpeningButton ;
    QPushButton * myClosureButton ;
  } ;
}

RoiMorphoMathActionView::RoiMorphoMathActionView( anatomist::RoiMorphoMathAction * action,
						  QWidget * parent ) :
  QVBox(parent), Observer(), myChangingFlag(false), myUpdatingFlag(false)
{
  _private = new RoiMorphoMathActionView_Private ;
  _private->myRoiMorphoMathAction = action ;
  _private->myRoiMorphoMathAction->addObserver(this) ;

  _private->myStructElRadiusBox = new QHGroupBox( tr("Structuring Element Radius"), this ) ;
  _private->myStructElRadiusSlider = new QSlider( 1, 1000, 4, 20, Qt::Horizontal, 
						_private->myStructElRadiusBox ) ;
  _private->myStructElRadiusLabel = new QLabel( "1 mm" , _private->myStructElRadiusBox ) ;
  _private->myStructElRadiusLabel->setFixedWidth( 50 ) ;
  
  _private->myDistanceMode = new QHButtonGroup( tr("Distance Mode"), this ) ;
  new QRadioButton(tr("mm"), _private->myDistanceMode) ;
  new QRadioButton(tr("voxel"), _private->myDistanceMode) ;
  _private->myDistanceMode->setExclusive(true) ;
  _private->myDistanceMode->setButton(0) ;
  // myDistanceMode is NEVER used !
  _private->myDistanceMode->hide();
  
  _private->myMorphoButtons =  new QVGroupBox( tr("Mathematic Morphology Actions"), this ) ;
  _private->myMorphoGrid = new QGrid( 2, _private->myMorphoButtons ) ;
  _private->myErosionButton = new QPushButton( tr("Erosion"), _private->myMorphoGrid ) ;
  _private->myDilationButton = new QPushButton( tr("Dilation"), _private->myMorphoGrid ) ;
  _private->myOpeningButton = new QPushButton( tr("Opening"), _private->myMorphoGrid ) ;
  _private->myClosureButton = new QPushButton( tr("Closure"), _private->myMorphoGrid ) ;

  _private->myRegionMode = new QHButtonGroup( tr("Apply on ROI region or session"), this ) ;
  new QRadioButton(tr("Region"), _private->myRegionMode) ;
  new QRadioButton(tr("Session"), _private->myRegionMode) ;
  _private->myRegionMode->setExclusive(true) ;
  _private->myRegionMode->setButton(0) ;

  _private->myStructElRadiusBox->setSizePolicy( QSizePolicy::Expanding,
                                                QSizePolicy::Fixed );
  _private->myDistanceMode->setSizePolicy( QSizePolicy::Expanding,
                                           QSizePolicy::Fixed );
  _private->myMorphoButtons->setSizePolicy( QSizePolicy::Expanding,
                                            QSizePolicy::Fixed );
  _private->myRegionMode->setSizePolicy( QSizePolicy::Expanding,
                                         QSizePolicy::Fixed );
  QWidget *w = new QWidget( this ); // act as stretch
  w->setMinimumHeight( 0 );

  connect( _private->myStructElRadiusSlider, SIGNAL(valueChanged(int)), 
	   this, SLOT(structuringElementRadiusChanged(int) ) ) ;
  connect( _private->myDistanceMode, SIGNAL(clicked(int)), 
	   this, SLOT(distanceModeChanged(int) ) ) ;
  connect( _private->myErosionButton, SIGNAL(clicked()), 
	   this, SLOT(erosion() ) ) ;
  connect( _private->myDilationButton, SIGNAL(clicked()), 
	   this, SLOT(dilation() ) ) ;
  connect( _private->myOpeningButton, SIGNAL(clicked()), 
	   this, SLOT(opening() ) ) ;
  connect( _private->myClosureButton, SIGNAL(clicked()), 
	   this, SLOT(closure() ) ) ;
  connect( _private->myRegionMode, SIGNAL(clicked(int)), 
	   this, SLOT(regionModeChanged(int) ) ) ;
}



RoiMorphoMathActionView::~RoiMorphoMathActionView()
{
  _private->myRoiMorphoMathAction->deleteObserver(this) ;
}



void RoiMorphoMathActionView::dilation()
{
  _private->myRoiMorphoMathAction->dilation() ;
}


void RoiMorphoMathActionView::erosion()
{
  _private->myRoiMorphoMathAction->erosion() ;
}


void RoiMorphoMathActionView::opening()
{
  _private->myRoiMorphoMathAction->opening() ;
}


void RoiMorphoMathActionView::closure()
{
  _private->myRoiMorphoMathAction->closure() ;
}


// void RoiMorphoMathActionView::default()
// {
  
// }


void RoiMorphoMathActionView::structuringElementRadiusChanged( int newStructElRadius )
{
  myChangingFlag = true ;
  _private->myRoiMorphoMathAction->structuringElementRadiusChange( newStructElRadius / 20.) ;
  _private->myStructElRadiusLabel->setText( QString::number( newStructElRadius/ 20. ) + 
	 ( _private->myRoiMorphoMathAction->distanceMode() == RoiMorphoMathAction::MM ?
	   QString(" mm") : QString(" voxels") ) ) ;
  myChangingFlag = false ;
}


void RoiMorphoMathActionView::distanceModeChanged( int newDistanceMode )
{
  myChangingFlag = true ;
  if( newDistanceMode == 0 )
    _private->myRoiMorphoMathAction->setDistanceToMm() ;
  else
    _private->myRoiMorphoMathAction->setDistanceToVoxel() ;
    
  _private->myStructElRadiusLabel->setText( 
         QString::number( _private->myRoiMorphoMathAction->structuringElementRadius() ) + 
	 ( _private->myRoiMorphoMathAction->distanceMode() == RoiMorphoMathAction::MM ?
	   QString(" mm") : QString(" voxels") ) ) ;
  myChangingFlag = false ;
}

void RoiMorphoMathActionView::regionModeChanged( int newRegionMode )
{
  myChangingFlag = true ;
  if( newRegionMode == 0 )
    _private->myRoiMorphoMathAction->setRegionModeToRegion() ;
  else
    _private->myRoiMorphoMathAction->setRegionModeToSession() ;
  
  myChangingFlag = false ;
}


void RoiMorphoMathActionView::update( const anatomist::Observable *, void * )
{
  //cout << "RoiMorphoMathActionView::update" << endl ;
  if( myChangingFlag || myUpdatingFlag )
    return ;
  
  myUpdatingFlag = true ;
  
  if( _private->myStructElRadiusSlider->value() != 
      _private->myRoiMorphoMathAction->structuringElementRadius() * 20. ){
    _private->myStructElRadiusSlider->setValue( int(_private->myRoiMorphoMathAction->
						    structuringElementRadius() * 20. ) ) ;
//     _private->myStructElRadiusLabel->setText( 
//          QString::number( _private->myRoiMorphoMathAction->structuringElementRadius() ) + 
// 	 ( _private->myRoiMorphoMathAction->distanceMode() == RoiMorphoMathAction::MM ?
// 	 QString(" mm") : QString(" voxels") ) ) ;
  } 
    
  if( _private->myDistanceMode->id( _private->myDistanceMode->selected() ) !=
      _private->myRoiMorphoMathAction->distanceMode() )
    _private->myDistanceMode->setButton( _private->myRoiMorphoMathAction->distanceMode() ) ;
  
  if( _private->myRegionMode->id( _private->myRegionMode->selected() ) !=
      _private->myRoiMorphoMathAction->regionMode() )
    _private->myRegionMode->setButton( _private->myRoiMorphoMathAction->regionMode() ) ;

  myUpdatingFlag = false ;
}

RoiMorphoMathAction::RoiMorphoMathAction() :
  Observable(), myDistanceMode(MM), myRegionMode(REGION), myStructuringElementRadius(1.0)
{
}

Action*
RoiMorphoMathAction::creator()
{
 return new RoiMorphoMathAction( ) ;
}

RoiMorphoMathAction::~RoiMorphoMathAction()
{}

string 
RoiMorphoMathAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "MorphoMathAction" );
}

void 
RoiMorphoMathAction::structuringElementRadiusChange( float structuringElementRadius)
{
  myStructuringElementRadius = structuringElementRadius ;

  setChanged() ;
  notifyObservers() ;
}

void 
RoiMorphoMathAction::setDistanceToMm( )
{
  myDistanceMode = MM ;
  cout << "Distance for morphological operations set to Mm" << endl ;

  setChanged() ;
  notifyObservers() ;
}

void 
RoiMorphoMathAction::setDistanceToVoxel( )
{
  myDistanceMode = VOXEL ;
  cout << "Distance for morphological operations set to Voxel" << endl ;

  setChanged() ;
  notifyObservers() ;
}

void 
RoiMorphoMathAction::setRegionModeToRegion( )
{
  myRegionMode = REGION ;

  setChanged() ;
  notifyObservers() ;
}

void 
RoiMorphoMathAction::setRegionModeToSession( )
{
  myRegionMode = SESSION ;

  setChanged() ;
  notifyObservers() ;
}

QWidget * 
RoiMorphoMathAction::actionView( QWidget * parent )
{
  RoiMorphoMathActionView * obs = new RoiMorphoMathActionView( this, parent ) ;
  
  return obs ;
}

AimsData<int16_t> *
RoiMorphoMathAction::regionBinaryMask(AGraphObject * go) const
{
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  if (!g) 
    return 0 ;
  
  AimsData<AObject*>& volOfLabels = g->volumeOfLabels( 0 ) ;
  if( volOfLabels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      volOfLabels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      volOfLabels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) ){
    g->clearLabelsVolume() ;
    g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() - g->MinX2D() ) + 1, 
				 static_cast<int>( g->MaxY2D() - g->MinY2D() ) + 1,
				 static_cast<int>( g->MaxZ2D() - g->MinZ2D() ) + 1 ) ;
  }
  
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  
  AimsData<int16_t> * binMask = new AimsData<int16_t>( labels.dimX(), labels.dimY(), labels.dimZ(), 1, 1 );
  binMask->setSizeXYZT( g->VoxelSize()[0], g->VoxelSize()[1], g->VoxelSize()[2], 1.0 ) ;

  if(!go)
    return 0 ;
  
  AGraphObject::iterator	ic, ec =  go->end() ;
  Bucket * bk ;
  for( ic = go->begin() ; ic!=ec; ++ic )
    if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
      break;
  
  if ( bk != 0 ) { 
    BucketMap<Void>::Bucket::iterator 
      iter( bk->bucket()[0].begin() ), 
      last( bk->bucket()[0].end() ) ;
    
    while ( iter != last){
      (*binMask)(iter->first) = 255 ;
      ++iter ;
    }
  }
  return binMask ;
}

void
RoiMorphoMathAction::dilation( bool partOfOpening ) 
{
  if( myRegionMode == SESSION && !partOfOpening ){
    AWarning("Not implemented yet !") ;
    return ;
  }
  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;
  if( myRegionMode == REGION ){
    AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
    if(!go)
      return ;
    doingDilation( go, changes ) ;
  } else {
    AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
    if( !g)
      return ;
    for( AGraph::iterator it = g->begin() ; it != g->end() ; ++it ){
      AGraphObject * go = dynamic_cast<AGraphObject *>(*it) ;
      if( !go )
	return ;
      doingDilation( go, changes ) ;
    }
  }
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
}

void 
RoiMorphoMathAction::doingDilation( AGraphObject * go,  list< pair< Point3d, ChangesItem> >* changes )
{
  if(!go)
    return ;

  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  if (!g) return ;

  bool replaceMode = PaintActionSharedData::instance()->replaceMode() ;
  AimsData<int16_t> * binMask = regionBinaryMask(go) ;
  if( !binMask )
    return ;
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  
  int maskZ = min(3, binMask->dimZ() ) ;
  int maskY = min(3, binMask->dimY() ) ;
  int maskX = min(3, binMask->dimX() ) ;
  AimsData<int16_t> dilMask
    = AimsMorphoChamferDilation( *binMask, myStructuringElementRadius,
                                 maskX, maskY, maskZ, 50 );
  
  ChangesItem item ;
  item.before = 0 ;
  item.after = go ;

  for(int z = 0 ; z < dilMask.dimZ() ; ++z )
    for(int y = 0 ; y < dilMask.dimY() ; ++y )
      for(int x = 0 ; x < dilMask.dimX() ; ++x )
        if( dilMask(x, y, z) && (!(*binMask)(x, y, z) ) &&
            ( replaceMode || labels(x, y, z) == 0 ) )
        {
          changes->push_back(pair<Point3d, ChangesItem>( Point3d(x, y, z), item ) )  ;
          labels( x, y, z ) = go  ;
        }
  delete binMask ;
}

void
RoiMorphoMathAction::erosion(  ) 
{
  list< pair< Point3d, ChangesItem> >* changes = new list< pair< Point3d, ChangesItem> > ;
  if( myRegionMode == REGION ){
    AGraphObject * go = RoiChangeProcessor::instance()->getGraphObject( 0 ) ;
    if(!go)
      return ;
    doingErosion( go, changes ) ;
  } else {
    AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
    if (!g) return ;
    for( AGraph::iterator it = g->begin() ; it != g->end() ; ++it ){
      AGraphObject * go = dynamic_cast<AGraphObject *>(*it) ;
      cout << "Aobject name : " <<  go->name() << endl ;
      if( !go )
	return ;
      doingErosion( go, changes ) ;
    }
  }
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
}

void 
RoiMorphoMathAction::doingErosion( AGraphObject * go, list< pair< Point3d, ChangesItem> >* changes )
{
  if(!go)
    return ;
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  if (!g) return ;
  
  int maskZ, maskY, maskX ;
  
  AimsData<int16_t> * binMask = regionBinaryMask(go) ;
  if( !binMask )
    return ;
  
  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  maskZ = min(3, binMask->dimZ() ) ;
  maskY = min(3, binMask->dimY() ) ;
  maskX = min(3, binMask->dimX() ) ;
  AimsData<int16_t> erodeMask = 
    AimsMorphoChamferErosion( *binMask, myStructuringElementRadius, 
			      maskX, maskY, maskZ, 50 ) ;
  
  ChangesItem item ;
  item.before = go ;
  item.after = 0 ;
  
  for(int z = 0 ; z < erodeMask.dimZ() ; ++z )
    for(int y = 0 ; y < erodeMask.dimY() ; ++y )
      for(int x = 0 ; x < erodeMask.dimX() ; ++x )
	if( (*binMask)(x, y, z) && (!erodeMask(x, y, z) ) ) {
	  changes->push_back(pair<Point3d, ChangesItem>( Point3d(x, y, z), item ) )  ;
	  labels( x, y, z ) = go  ;
	}
  
  delete binMask ;
}

void
RoiMorphoMathAction::closure() 
{
  if( myRegionMode == SESSION ){
    AWarning("Not implemented yet !") ;
    return ;
  }
  
  dilation() ;
  erosion() ;
}

void
RoiMorphoMathAction::opening() 
{
  erosion() ;
  dilation() ;
}

