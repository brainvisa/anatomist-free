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
#include <anatomist/action/labelnaming.h>
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
#include <qlabel.h>
#include <aims/qtcompat/qhgroupbox.h>
#include <aims/qtcompat/qvgroupbox.h>
#include <qcombobox.h>
#include <queue>
#include <qmessagebox.h>

using namespace anatomist ;
using namespace aims;
using namespace carto;
using namespace std;


RoiLabelNamingAction::RoiLabelNamingAction() : 
  Action(), myTwoDMode(false)
{
}

anatomist::AObject * RoiLabelNamingAction::myCurrentImage = 0 ;
int32_t RoiLabelNamingAction::myNbOfPointsToSegmentLimit = 300000 ;
std::vector< std::map< int16_t, int32_t> > RoiLabelNamingAction::myCurrentImageValues 
  = std::vector< std::map< int16_t, int32_t> >(0) ;
std::vector<bool> RoiLabelNamingAction::myComputeCurrentImageValueMap 
  = std::vector<bool>(0) ;
    
RoiLabelNamingAction::~RoiLabelNamingAction()
{
}


string RoiLabelNamingAction::name() const
{
  return QT_TRANSLATE_NOOP( "ControlSwitch", "LabelNamingAction" );
}


AObject* 
RoiLabelNamingAction::getCurrentImage()
{
  //   set<AObject*> objs = view()->aWindow()->Objects() ;
  AObject * gotIt = RoiManagementActionSharedData::instance()->
    getObjectByName( AObject::VOLUME, 
		     RoiManagementActionSharedData::instance()->
		     currentImage() ) ;
  return gotIt ;
}

Action*
RoiLabelNamingAction::creator()
{
  return new RoiLabelNamingAction( ) ;
}


void 
RoiLabelNamingAction::addConnecCompToRegion( int x, int y, int, int )
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

  fillRegion( x, y, go, *changes, true, false ) ;
  
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
  currentModifiedRegion->setBucketChanged() ;
}


void 
RoiLabelNamingAction::removeConnecCompFromRegion( int x, int y, int, int )
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
  
  fillRegion( x, y, go, *changes, false, false ) ;
  
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
  currentModifiedRegion->setBucketChanged() ;

  std::list<AWindow3D*> linkedWindows ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if (win3d)
      win3d->Refresh() ;
  }
}


void 
RoiLabelNamingAction::addWholeLabelToRegion( int x, int y, int, int )
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
  
  fillRegion( x, y, go, *changes, true, true ) ;
  
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
  currentModifiedRegion->setBucketChanged() ;
}


void 
RoiLabelNamingAction::removeWholeLabelFromRegion( int x, int y, int, int )
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
  fillRegion( x, y, go, *changes, false, true ) ;
  
  if ( ! (*changes).empty() )
    RoiChangeProcessor::instance()->applyChange( changes ) ;
  
  currentModifiedRegion->setBucketChanged() ;

  std::list<AWindow3D*> linkedWindows ;
  set<AWindow*> group = g->WinList() ;
  set<AWindow*>::iterator gbegin = group.begin();
  set<AWindow*>::iterator gend = group.end();
  for( set<AWindow*>::iterator i = gbegin; i != gend; ++i ){
    AWindow3D * win3d = dynamic_cast<AWindow3D*>(*i) ;
    if (win3d)
      win3d->Refresh() ;
  }
}

void 
RoiLabelNamingAction::fillRegion( int x, int y, AGraphObject * region, 
				  list< pair< Point3d, ChangesItem> >& changes,
				  bool add, bool wholeImage )
{
  anatomist::AObject * newSelectedImage = getCurrentImage() ;
  if( newSelectedImage != myCurrentImage ){
    myCurrentImage = getCurrentImage() ;
    if( myCurrentImage )
      myComputeCurrentImageValueMap.clear() ;
  } else
    myComputeCurrentImageValueMap = vector<bool>(1, false) ;

  if( !myCurrentImage )
    return ;
  
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->aWindow() ) ;
  if( !win )
    {
      cerr << "warning: PaintAction operating on wrong view type\n";
      return;
    }
  
  //cout << "\tx = " << x << "\ty = " << y << endl ;
  
  Referential * winRef = win->getReferential() ;
  
  Referential * buckRef = region->getReferential() ;
  //bool		newbck = myDeltaModifications->empty();
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
      AimsData<AObject*>& volumeOfLabels = g->volumeOfLabels( 0 ) ;
      Point3d pToInt( static_cast<int> ( p[0] +.5 ), 
		      static_cast<int> ( p[1] +.5 ), 
		      static_cast<int> ( p[2] +.5 ) ) ;
      Point3d pVL( static_cast<int> ( p[0] - vlOffset[0] +.5 ), 
		   static_cast<int> ( p[1] - vlOffset[1] +.5 ), 
		   static_cast<int> ( p[2] - vlOffset[2] +.5 ) );
      
	
      
      //cout << "\tpVL = " << pVL << endl ;
      
      bool replace = PaintActionSharedData::instance()->replaceMode() ;
      ChangesItem change ;
      AObject** toChange ;
      if( add ){
	change.after = region ;
	toChange = &change.before ;
      }else{
	//replace = true ;
	change.after = 0 ;
	toChange = &change.before ;
      }
      
      std::queue<Point3d> trialPoints ;
      Point3d dims( volumeOfLabels.dimX(), volumeOfLabels.dimY(), volumeOfLabels.dimZ() ) ;
      if( in( dims, pVL ) ){
	short currentLabel = short ( rint ( myCurrentImage->mixedTexValue( Point3df( pVL[0], 
									   pVL[1], 
									   pVL[2] ), timePos) ) ) ;
	AVolume<int16_t> * vol16bits = dynamic_cast<AVolume<int16_t> *>(myCurrentImage) ;
	if( vol16bits )
	  computeImageValueMap( *vol16bits, timePos ) ;
	else{
	  AVolume<int8_t> * vol8bits = dynamic_cast<AVolume<int8_t> *>(myCurrentImage) ;
	  if( vol8bits )
	    computeImageValueMap( *vol8bits, timePos ) ;
	  else{
	    AVolume<int32_t> * vol32bits = dynamic_cast<AVolume<int32_t> *>(myCurrentImage) ;
	    if( vol32bits )
	      computeImageValueMap( *vol32bits, timePos ) ;
	    else {
	      AVolume<uint16_t> * volu16bits = dynamic_cast<AVolume<uint16_t> *>(myCurrentImage) ;
	      if( volu16bits )
	        computeImageValueMap( *volu16bits, timePos ) ;
	      else{
	        AVolume<uint8_t> * volu8bits = dynamic_cast<AVolume<uint8_t> *>(myCurrentImage) ;
	        if( volu8bits )
	          computeImageValueMap( *volu8bits, timePos ) ;
	        else{
	          AVolume<uint32_t> * volu32bits = dynamic_cast<AVolume<uint32_t> *>(myCurrentImage) ;
	          if( volu32bits )
	            computeImageValueMap( *volu32bits, timePos ) ;
	        }
	      }
	    }
	  }
	}
	
	std::map< int16_t, int32_t>::iterator found
	  = myCurrentImageValues[timePos].find( currentLabel ) ;
	int32_t nbOfPointsInImageToSegment = 0 ;
	if( found == myCurrentImageValues[timePos].end() ){
// 	  cout << "myCurrentImageValues.size() = " << myCurrentImageValues[timePos].size() << endl ;
// 	  cout << "unfound label = " << currentLabel << endl ;
	} else
	  nbOfPointsInImageToSegment = found->second  ;
	
	bool proceed = true ;
	if( nbOfPointsInImageToSegment > myNbOfPointsToSegmentLimit ){
	  int res = 
	    QMessageBox::warning( 0,
				  RoiManagementActionView::tr("Huge amount of data !"),
				  RoiManagementActionView::tr("You can procced, but it will \n"
							      "take few minutes\n"
							      "Do you still want to proceed ?"),
				  RoiManagementActionView::tr("&Yes"), 
				  RoiManagementActionView::tr("&No"),
				  QString::null, 0, 1 ) ;
	  //cout << "Mess box res = " << res << endl ;
	  if( res == 1 )
	    proceed = false ;
	}
	if( !proceed )
	  return ;
	
	if( wholeImage ){
	  int x, y, z ;
	  ForEach3d(volumeOfLabels, x, y, z){
	    Point3d pc(x, y, z) ;
	    if( fillPoint( pc, timePos, volumeOfLabels, region, currentLabel, 
			   toChange, trialPoints, replace, add ) )
	      changes.push_back(pair<Point3d, ChangesItem>( pc, change ) )  ;
	    
	    // 	    short label = short ( rint ( myCurrentImage->mixedTexValue( Point3df( x, y, z ), timePos) ) ) ;
	    // 	    if( label == currentLabel )
	  } 
	}else{ 
	  trialPoints.push(pVL) ;
	  int regionSize = 0 ;
	  Point3d pc ;
	  Connectivity  * connec = 0 ; 
	  //cout << "_sharedData->myDimensionMode == " << (TWOD ? "TWOD" : "ThreeD") << endl ;
	  if( myTwoDMode == 1 ){
	    //cout << "inside " << "TWOD" << "Normal Vect = " << normalVector << endl ;
	    
	    if( normalVector[2] > 0.9 ){
	      //cout << "CONNECTIVITY_4_XY" << endl ;
	      connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_XY ) ;
	    }else if( normalVector[1] > 0.9 ){
	      //cout << "CONNECTIVITY_4_XZ" << endl ;	    
	      connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_XZ ) ;
	    }else if( normalVector[0] > 0.9 ){
	      connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_4_YZ ) ;
	      //cout << "CONNECTIVITY_4_YZ" << endl ;
	    }else{
	      AWarning( "Level set should not be used in 2d mode on oblique views" ) ;
	      return ;
	    }
	  } else
	    connec = new Connectivity(0, 0, Connectivity::CONNECTIVITY_6_XYZ ) ;
	  
	  while( !trialPoints.empty() ){
	    //cout << "Queue size : " <<  trialPoints.size() << "\tCurrent Point = " << pc << endl ;
	    
	    pc = trialPoints.front() ;
	    trialPoints.pop() ;
	    for( int n = 0 ; n < connec->nbNeighbors() ; ++n )
	      if( fillPoint( pc + connec->xyzOffset(n), timePos, volumeOfLabels, region, currentLabel, 
			     toChange, trialPoints, replace, add ) ){
		changes.push_back(pair<Point3d, ChangesItem>( pc + connec->xyzOffset(n), change ) )  ;
		++regionSize ;
	      }
	    }
	}
      }
    }
}
  
template <class T> 
void
RoiLabelNamingAction::computeImageValueMap( const anatomist::AVolume<T>& avol, int timePos )
{
  Volume<T>	& vol = *avol.volume();
  if( myComputeCurrentImageValueMap.empty() )
    myComputeCurrentImageValueMap = vector<bool>(vol.getSizeT(), true) ;
  
  
  if( myComputeCurrentImageValueMap[timePos] )
  {
    int x, y, z ;
    myCurrentImageValues = std::vector< std::map< int16_t, int32_t> >(
      vol.getSizeT() ) ;

    for( long z=0, nz=vol.getSizeZ(); z!=nz; ++z )
      for( long y=0, ny=vol.getSizeY(); y!=ny; ++y )
        for( long x=0, nx=vol.getSizeX(); x!=nx; ++x )
          ++myCurrentImageValues[timePos][vol( x, y, z, timePos )] ;
  }
}

