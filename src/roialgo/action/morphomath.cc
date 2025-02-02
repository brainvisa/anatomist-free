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
#include <anatomist/misc/error.h>
#include <aims/morphology/morphology_g.h>

#include <QGroupBox>
#include <qslider.h>
#include <qlabel.h>
#include <QButtonGroup>
#include <qpushbutton.h>
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

    QSlider * myStructElRadiusSlider ;
    QLabel * myStructElRadiusLabel ;

    QButtonGroup * myDistanceMode ;
    QButtonGroup * myRegionMode ;

    QPushButton * myDefaultButton ;

    QPushButton * myErosionButton ;
    QPushButton * myDilationButton ;
    QPushButton * myOpeningButton ;
    QPushButton * myClosureButton ;
  } ;
}

RoiMorphoMathActionView::RoiMorphoMathActionView( anatomist::RoiMorphoMathAction * action,
						  QWidget * parent ) :
  QWidget(parent), Observer(), myChangingFlag(false), myUpdatingFlag(false)
{
  _private = new RoiMorphoMathActionView_Private ;
  _private->myRoiMorphoMathAction = action ;
  _private->myRoiMorphoMathAction->addObserver(this) ;

  QVBoxLayout *lay = new QVBoxLayout( this );
  QGroupBox *myStructElRadiusBox = new QGroupBox(
    tr("Structuring Element Radius"), this );
  lay->addWidget( myStructElRadiusBox );
  QHBoxLayout *serlay = new QHBoxLayout( myStructElRadiusBox );
  _private->myStructElRadiusSlider = new QSlider( Qt::Horizontal,
                                                  myStructElRadiusBox );
  serlay->addWidget( _private->myStructElRadiusSlider );
  _private->myStructElRadiusSlider->setRange( 1, 1000 );
  _private->myStructElRadiusSlider->setPageStep( 4 );
  _private->myStructElRadiusSlider->setValue( 20 );
  _private->myStructElRadiusLabel = new QLabel( "1 mm" , myStructElRadiusBox );
  serlay->addWidget( _private->myStructElRadiusLabel );
  _private->myStructElRadiusLabel->setFixedWidth( 50 ) ;
  
  QGroupBox *distmode = new QGroupBox( tr("Distance Mode"), this );
  lay->addWidget( distmode );
  QHBoxLayout *dmlay = new QHBoxLayout( distmode );
  _private->myDistanceMode = new QButtonGroup( distmode );

  QRadioButton *b = new QRadioButton( tr("mm"), distmode );
  dmlay->addWidget( b );
  _private->myDistanceMode->addButton( b, 0 );
  b = new QRadioButton( tr("voxel"), distmode );
  dmlay->addWidget( b );
  _private->myDistanceMode->addButton( b, 1 );
  _private->myDistanceMode->setExclusive( true );
  _private->myDistanceMode->button( 0 )->setChecked( true );
  // myDistanceMode is NEVER used !
  distmode->hide();

  QGroupBox *myMorphoButtons = new QGroupBox(
    tr("Mathematic Morphology Actions"), this );
  lay->addWidget( myMorphoButtons );
  QGridLayout *mblay = new QGridLayout( myMorphoButtons );
  _private->myErosionButton = new QPushButton(
    tr("Erosion"), myMorphoButtons );
  mblay->addWidget( _private->myErosionButton, 0, 0 );
  _private->myDilationButton = new QPushButton(
    tr("Dilation"), myMorphoButtons );
  mblay->addWidget( _private->myDilationButton, 0, 1 );
  _private->myOpeningButton = new QPushButton(
    tr("Opening"), myMorphoButtons );
  mblay->addWidget( _private->myOpeningButton, 1, 0 );
  _private->myClosureButton = new QPushButton(
    tr("Closure"), myMorphoButtons );
  mblay->addWidget( _private->myClosureButton, 1, 1 );

  QGroupBox *regionmode = new QGroupBox(
    tr("Apply on ROI region or session"), this );
  lay->addWidget( regionmode );
  QHBoxLayout *rmlay = new QHBoxLayout( regionmode );
  _private->myRegionMode = new QButtonGroup( regionmode );
  b = new QRadioButton( tr("Region"), regionmode );
  rmlay->addWidget( b );
  _private->myRegionMode->addButton( b, 0 );
  b = new QRadioButton( tr("Session"), regionmode );
  rmlay->addWidget( b );
  _private->myRegionMode->addButton( b, 1 );
  _private->myRegionMode->setExclusive(true) ;
  _private->myRegionMode->button(0)->setChecked( true );

  myStructElRadiusBox->setSizePolicy( QSizePolicy::Expanding,
                                      QSizePolicy::Fixed );
  distmode->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  myMorphoButtons->setSizePolicy( QSizePolicy::Expanding,
                                  QSizePolicy::Fixed );
  regionmode->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  lay->addStretch();

  connect( _private->myStructElRadiusSlider, SIGNAL( valueChanged(int) ),
          this, SLOT( structuringElementRadiusChanged(int) ) ) ;
  connect( _private->myDistanceMode, SIGNAL( idClicked(int) ),
           this, SLOT( distanceModeChanged(int) ) ) ;
  connect( _private->myErosionButton, SIGNAL( clicked() ),
           this, SLOT( erosion() ) ) ;
  connect( _private->myDilationButton, SIGNAL( clicked() ),
           this, SLOT( dilation() ) ) ;
  connect( _private->myOpeningButton, SIGNAL( clicked() ),
           this, SLOT( opening() ) ) ;
  connect( _private->myClosureButton, SIGNAL( clicked() ),
           this, SLOT( closure() ) ) ;
  connect( _private->myRegionMode, SIGNAL( idClicked(int) ),
	   this, SLOT( regionModeChanged(int) ) ) ;
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
    
  if( _private->myDistanceMode->checkedId() !=
      _private->myRoiMorphoMathAction->distanceMode() )
    _private->myDistanceMode->button(
      _private->myRoiMorphoMathAction->distanceMode() )->setChecked( true );
  
  if( _private->myRegionMode->checkedId() !=
      _private->myRoiMorphoMathAction->regionMode() )
    _private->myRegionMode->button(
      _private->myRoiMorphoMathAction->regionMode() )->setChecked( true );

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


VolumeRef<int16_t>
RoiMorphoMathAction::regionBinaryMask(AGraphObject * go) const
{
  AGraph * g = RoiChangeProcessor::instance()->getGraph( 0 ) ;
  if (!g) 
    return 0 ;
  
  VolumeRef<AObject*>& volOfLabels = g->volumeOfLabels( 0 ) ;
  vector<float> bmin, bmax, vs;
  g->boundingBox2D( bmin, bmax );
  vs = g->voxelSize();
  vector<int> dims( 3 );
  dims[0] = int( rint( ( bmax[0] - bmin[0] ) / vs[0] ) );
  dims[1] = int( rint( ( bmax[1] - bmin[1] ) / vs[1] ) );
  dims[2] = int( rint( ( bmax[2] - bmin[2] ) / vs[2] ) );
  if( volOfLabels.getSizeX() != dims[0]
      || volOfLabels.getSizeY() != dims[1]
      || volOfLabels.getSizeZ() != dims[2] )
  {
    g->clearLabelsVolume();
    g->setLabelsVolumeDimension( dims[0], dims[1], dims[2] );
  }
  
  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  
  VolumeRef<int16_t> binMask(
    labels.getSizeX(), labels.getSizeY(), labels.getSizeZ(), 1, 1 );
  binMask->setVoxelSize( vs[0], vs[1], vs[2], 1.0 );

  if(!go)
    return 0 ;
  
  AGraphObject::iterator	ic, ec =  go->end() ;
  Bucket * bk = 0 ;
  for( ic = go->begin() ; ic!=ec; ++ic )
    if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
      break;
  
  if ( bk != 0 ) { 
    BucketMap<Void>::Bucket::iterator 
      iter( bk->bucket()[0].begin() ), 
      last( bk->bucket()[0].end() ) ;
    
    while ( iter != last){
      binMask->at(iter->first) = 255 ;
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
  VolumeRef<int16_t> binMask = regionBinaryMask(go) ;
  if( !binMask.get() )
    return ;
  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  
  int maskZ = min(3, binMask->getSizeZ() ) ;
  int maskY = min(3, binMask->getSizeY() ) ;
  int maskX = min(3, binMask->getSizeX() ) ;
  VolumeRef<int16_t> dilMask
    = AimsMorphoChamferDilation<int16_t>( binMask, myStructuringElementRadius,
                                          maskX, maskY, maskZ, 50 );
  
  ChangesItem item ;
  item.before = 0 ;
  item.after = go ;
  vector<int> msize = dilMask.getSize();

  for(int z = 0 ; z < msize[2]; ++z )
    for(int y = 0 ; y < msize[1]; ++y )
      for(int x = 0 ; x < msize[0]; ++x )
        if( dilMask->at(x, y, z) && (!binMask->at(x, y, z) ) &&
            ( replaceMode || labels->at(x, y, z) == 0 ) )
        {
          changes->push_back( pair<Point3d, ChangesItem>( Point3d(x, y, z),
                                                          item ) );
          labels->at( x, y, z ) = go;
        }
}

void
RoiMorphoMathAction::erosion()
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
  
  VolumeRef<int16_t> binMask = regionBinaryMask(go);
  if( !binMask.get() )
    return ;
  
  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  maskZ = min(3, binMask->getSizeZ() ) ;
  maskY = min(3, binMask->getSizeX() ) ;
  maskX = min(3, binMask->getSizeX() ) ;
  VolumeRef<int16_t> erodeMask =
    AimsMorphoChamferErosion<int16_t>( binMask, myStructuringElementRadius,
                                       maskX, maskY, maskZ, 50 );
  
  ChangesItem item ;
  item.before = go ;
  item.after = 0 ;
  vector<int> msize = erodeMask.getSize();
  
  for(int z = 0 ; z < msize[2]; ++z )
    for(int y = 0 ; y < msize[1]; ++y )
      for(int x = 0 ; x < msize[0]; ++x )
        if( binMask->at(x, y, z) && (!erodeMask->at(x, y, z) ) )
        {
          changes->push_back(pair<Point3d, ChangesItem>( Point3d(x, y, z),
                                                         item ) );
          labels->at( x, y, z ) = go;
        }
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

