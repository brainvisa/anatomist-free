/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */
#include <cartobase/stream/fileutil.h>
#include <anatomist/action/blobsegmentation.h>
#include <anatomist/action/levelsetaction.h>
#include <anatomist/action/roimanagementaction.h>
#include <anatomist/action/paintaction.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/commands/cAddNode.h>
#include <anatomist/commands/cCreateGraph.h>
#include <anatomist/commands/cSaveObject.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/hierarchy/hierarchy.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/control/wControl.h>
#include <anatomist/control/graphParams.h>
#include <anatomist/commands/cSelect.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/commands/cLoadObject.h>
#include <anatomist/commands/cDeleteObject.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/commands/cSetControl.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/misc/error.h>
#include <anatomist/object/actions.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/controler/view.h>
#include <anatomist/controler/icondictionary.h>
#include <cartobase/object/attributed.h>
#include <cartobase/type/string_conversion.h>
#include <graph/tree/tree.h>
#include <aims/resampling/quaternion.h>
#include <aims/utility/converter_bucket.h>
#include <aims/connectivity/connectivity.h>
#include <aims/def/path.h>
#include <cartobase/object/object.h>
#include <anatomist/application/settings.h>
#include <graph/tree/tfactory.h>

using namespace std ;
using namespace carto ;
using namespace aims ;
using namespace anatomist ;

struct More : public std::binary_function< float, float , bool>
{
  bool operator () ( float f1, float f2 ) const
  {
    return( f1 > f2 ) ;
  }
};

struct PointLess : public std::binary_function< Point3d, Point3d , bool>
{
  bool operator () ( const Point3d & p1, const Point3d & p2 ) const
  {
    return( p1[2] < p2[2] 
	    || ( (p1[2] == p2[2]) && (p1[1] < p2[1])  )
	    || ( (p1[2] == p2[2]) 
		 && (p1[1] == p2[1]) && (p1[0] < p2[0]) ) ) ;
  }
};

template <class T>
bool 
in( const AimsData<T>& o, Point3df p, 
    const Point3df & offset )
{
  p -= offset;
  if ( p[0] < 0 || p[0] > o.dimX() - 1 ||  
       p[1] < 0 || p[1] > o.dimY() - 1 ||
	 p[2] < 0 || p[2] > o.dimZ() - 1 )
    return false ;
  
  return true ;
}


RoiBlobSegmentationAction::RoiBlobSegmentationAction() :
  _blobType(BLOB), _blobVolume(-1.) {}

RoiBlobSegmentationAction::~RoiBlobSegmentationAction() {}

void 
RoiBlobSegmentationAction::blobDetection() 
{ 
  cout << "Blob Segmentation : Mode Bump On" << endl ; 
  _blobType = BLOB ; 
}

void 
RoiBlobSegmentationAction::holeDetection() 
{ 
  cout << "Blob Segmentation : Mode Hole On" << endl ; 
  _blobType = HOLE ; 
}

void
RoiBlobSegmentationAction::segmentBlob(int x, int y, int , int ) 
{
  Bucket * currentRegion = RoiChangeProcessor::instance()->getCurrentRegion(view()->window() ) ;
  if( !currentRegion )
    return ;

  list< pair< Point3d, ChangesItem> > * currentChanges = new list< pair< Point3d, ChangesItem> > ;
  RoiChangeProcessor::instance()->setRedoable( false ) ;
  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->window() ) ;
  if (!g) return ;

  AimsData<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  if( labels.dimX() != ( g->MaxX2D() - g->MinX2D() + 1 ) || 
      labels.dimY() != ( g->MaxY2D() - g->MinY2D() + 1 ) ||
      labels.dimZ() != ( g->MaxZ2D() - g->MinZ2D() + 1 ) )
    {
      g->clearLabelsVolume() ;
      g->setLabelsVolumeDimension( static_cast<int>( g->MaxX2D() - g->MinX2D() ) + 1, 
				   static_cast<int>( g->MaxY2D() - g->MinY2D() ) + 1,
				   static_cast<int>( g->MaxZ2D() - g->MinZ2D() ) + 1 ) ;
    }
  
  AGraphObject * grao = RoiChangeProcessor::instance()->getGraphObject( view()->window() ) ;
  grao->attributed()->setProperty("modified", true) ;

  AimsData<AObject*> volOfLabels( g->volumeOfLabels( 0 ) ) ;
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->window() ) ;
  if( !win )
    {
      cerr << "warning: RoiBlobSegmentationAction operating on wrong view type\n" ;
      return;
    }
  
  Referential* winRef = win->getReferential() ;
  Referential* buckRef = currentRegion->getReferential() ;
  Point3df pos ;
  int time = int( rint( win->GetTime() ) ) ;
  //cout << "Time = " << time << endl ;
  if( win->positionFromCursor( x, y, pos ) )
    {
      Point3df normalVector( win->sliceQuaternion().apply(Point3df(0., 0., 1.) ) ) ;
    
      //     cout << "Normal Vector : " << normalVector << endl ;
      normalVector *= normalVector.dot( pos - win->GetPosition() ) ;
      pos = pos - normalVector ;

      AObject * image = RoiManagementActionSharedData::instance()->
	getObjectByName( AObject::VOLUME, 
			 RoiManagementActionSharedData::instance()->
			 currentImage() ) ;
      
      Point3df voxSize = image->VoxelSize() ;
      float volOfElt = voxSize[0] * voxSize[1] * voxSize[2] ;
      _blobVolume = RoiLevelSetActionSharedData::instance()->maxSize()  ;
      if( _blobVolume <= 0 )
        _blobVolume = labels.dimX() * labels.dimY() * labels.dimZ() * labels.sizeX() * labels.sizeY() * labels.sizeZ() ;
      
      float percentageOfMax = RoiLevelSetActionSharedData::instance()->percentageOfMaximum() / 100. ;
      float minimum = 10000000. ;
      float maximum = -10000000. ;
      float val ;
      
      if( image != 0 )
	{
	  Point3df voxelSize = currentRegion->VoxelSize() ;
	  Transformation * transf = theAnatomist->getTransformation( winRef, buckRef ) ;
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
	  Point3df vlOffset( g->MinX2D(), g->MinY2D(), g->MinZ2D() ) ;
	  Point3df pVl (static_cast<int> ( p[0] - vlOffset[0] +.5 ), 
			static_cast<int> ( p[1] - vlOffset[1] +.5 ), 
			static_cast<int> ( p[2] - vlOffset[2] +.5 ) ) ;
	  
	  Point3d posInt( static_cast<int>(p[0] + 0.5), static_cast<int>(p[1] + 0.5), static_cast<int>(p[2] + 0.5) ) ;
	  set<Point3d, PointLess> added ;
	  added.insert(posInt) ;
	  Connectivity connec(0, 0, Connectivity::CONNECTIVITY_6_XYZ) ;
	  multimap<float, Point3d> front ;
	  multimap<float, Point3d, More> rfront ;
	  val = image->mixedTexValue( Point3df(posInt[0], posInt[1], posInt[2]), time ) ;
	  if( _blobType == BLOB ){
	    rfront.insert( pair<float, Point3d>( val, posInt ) ) ;
	  }else{
	    front.insert( pair<float, Point3d>( val, posInt ) );
	  }
	  Point3d pCurr, neigh ;
	  float threshold = image->mixedTexValue( Point3df(posInt[0], posInt[1], posInt[2]), time ) ;
	  
	  AimsData<short> finalMap( volOfLabels.dimX(), volOfLabels.dimY(), volOfLabels.dimZ() ) ;
	  finalMap.setSizeXYZT( volOfLabels.sizeX(), volOfLabels.sizeY(), volOfLabels.sizeZ(), 1.0 ) ;
	  finalMap(posInt) = 1 ;
	  
	  Point3d pMax, pMin ;

	  while( ( ( _blobType == BLOB && !rfront.empty() /*&& minimum > percentageOfMax * maximum*/ ) || 
		   ( _blobType != BLOB && !front.empty() /*&& maximum < percentageOfMax * minimum*/ ) ) 
		 && (added.size() * volOfElt < _blobVolume || _blobVolume < 0 ) ){
	    if( _blobType == BLOB ){
	      if( rfront.empty() ) 
		cerr << "map should not be empty !" << endl ;
	      pCurr = rfront.begin()->second ;
	      //cout << "RFront size : " << rfront.size() << endl ;
	      
	      rfront.erase( rfront.begin() ) ;
	    } else {
	      if( front.empty() ) 
		cerr << "map should not be empty !" << endl ;
	      pCurr = front.begin()->second ;
     
	      //cout << "Front size : " << front.size() << endl ;
	      front.erase( front.begin() ) ;
	    }
	    
	    val = image->mixedTexValue( Point3df(pCurr[0], pCurr[1], pCurr[2]), time ) ;

	    if( val > maximum ){
	      maximum = val ;
	      pMax = pCurr ;
	    }
	    if( val < minimum ){
	      minimum = val ;
	      pMin = pCurr ;
	    }

	    //cout << "maximum = " << maximum << "\tminimum = " << minimum << endl ;
	    AObject * vOLVal ;
	    for( int n = 0 ; n < connec.nbNeighbors() ; ++n ){
	      neigh = pCurr + connec.xyzOffset(n) ;
	      if( in(volOfLabels, Point3df(neigh[0], neigh[1], neigh[2]), vlOffset) )
              {
		if( _blobType == BLOB ){
		  // float imVal = image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ) ;
		  // bool replMode = PaintActionSharedData::instance()->replaceMode() ;
		  vOLVal = volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ), 
					(unsigned) rint( neigh[1] - vlOffset[1] ), 
					(unsigned) rint( neigh[2] - vlOffset[2] ) ) ;
		  if( image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ) > threshold 
		      && !finalMap(neigh) ){
		    finalMap(neigh) = 1 ;
		    if( PaintActionSharedData::instance()->replaceMode() ||  
			volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ), 
				     (unsigned) rint( neigh[1] - vlOffset[1] ),  
				     (unsigned) rint( neigh[2] - vlOffset[2] ) ) == 0 )
		      added.insert(neigh) ;
		    
		    rfront.insert( pair<float, Point3d>( image->mixedTexValue( Point3df(neigh[0], 
											neigh[1], 
											neigh[2] ), 
									       time ), 
							 neigh ) ) ;
		  } 
		} else
		  if( image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ) < threshold 
		      && !finalMap(neigh) ){
		    finalMap(neigh) = 1 ;
		    if( PaintActionSharedData::instance()->replaceMode() ||  
			volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ), 
				     (unsigned) rint( neigh[1] - vlOffset[1] ),  
				     (unsigned) rint( neigh[2] - vlOffset[2] ) ) == 0 )
		      added.insert(neigh) ;
		    
		    front.insert( pair<float, Point3d>( image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ), 
							neigh ) ) ;
		  }
              }
	    }
	  }

	  if( ((!front.empty() || !rfront.empty()) && _blobVolume > 0. )|| 
	      ( _blobType == BLOB && (minimum <= percentageOfMax * maximum || 
				      volOfLabels( (unsigned) rint( pMax[0] - vlOffset[0] ), 
						   (unsigned) rint( pMax[1] - vlOffset[1] ), 
						   (unsigned) rint( pMax[2] - vlOffset[2] ) ) != 0 ) ) || 
	      ( _blobType != BLOB && (maximum >= percentageOfMax * minimum || 
				      volOfLabels( (unsigned) rint( pMin[0] - vlOffset[0] ), 
						   (unsigned) rint( pMin[1] - vlOffset[1] ), 
						   (unsigned) rint( pMin[2] - vlOffset[2] ) ) != 0 ) ) )
	    {
	      cerr << "Sorry, you should click nearer from the chosen blob or increase blob size" << endl ;
	      return ;
	    }
	  
	  if( _blobVolume > 0. ){	  
	    set<Point3d, PointLess> frontSet ;
	    for( set<Point3d, PointLess>::iterator itA = added.begin() ; itA != added.end() ; ++itA )
	      {
		for( int n = 0 ; n < connec.nbNeighbors() ; ++n ){
		  Point3d neigh = *itA + connec.xyzOffset(n) ;
		  if( in(finalMap, Point3df(neigh[0], neigh[1], neigh[2]), vlOffset) )
		    if( !finalMap( neigh ) )
		      if( frontSet.find(neigh) == frontSet.end() ){
			val = image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ) ;
			if( _blobType == BLOB && val > maximum * percentageOfMax )
			  rfront.insert( pair<float, Point3d>( val, neigh ) ) ;
			else if( val < minimum * percentageOfMax )
			  front.insert( pair<float, Point3d>( val, neigh ) ) ;
		  
			
			frontSet.insert(neigh) ;
		      }
		}
	      }
	    
	    //cout << "volume = " << added.size() * volOfElt << endl ;
	    while( added.size() * volOfElt < _blobVolume  && 
		   ( ( _blobType == BLOB && !rfront.empty() )|| ( _blobType != BLOB && !front.empty() ) ) ){	      
	      if( _blobType == BLOB ){
		if( rfront.empty() ) 
		  cerr << "map should not be empty !" << endl ;
		pCurr = rfront.begin()->second ;
		//cout << "RFront size : " << rfront.size() << endl ;
		rfront.erase( rfront.begin() ) ;
	      } else {
		if( front.empty() ) 
		  cerr << "map should not be empty !" << endl ;
		pCurr = front.begin()->second ;
		//cout << "Front size : " << front.size() << endl ;
		front.erase( front.begin() ) ;	      
	      }
	      
	      val = image->mixedTexValue( Point3df(pCurr[0], pCurr[1], pCurr[2]), time ) ;
	      if( val > maximum )
		maximum = val ;
	      if( val < minimum )
		minimum = val ;

	      //cout << "maximum = " << maximum << "\tminimum = " << minimum << endl ;
	      
	      
	      finalMap( pCurr ) = 1 ;
	      added.insert( pCurr ) ;
	      
	      for( int n = 0 ; n < connec.nbNeighbors() ; ++n ){
		neigh = pCurr + connec.xyzOffset(n) ;
		
		if( in(volOfLabels, Point3df(neigh[0], neigh[1], neigh[2]), vlOffset) )
		  if( (PaintActionSharedData::instance()->replaceMode() || 
		       volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ), 
				    (unsigned) rint( neigh[1] - vlOffset[1] ), 
				    (unsigned) rint( neigh[2] - vlOffset[2] ) ) == 0) 
		      && !finalMap(neigh) ){
		    val = image->mixedTexValue( Point3df(neigh[0], neigh[1], neigh[2] ), time ) ;
		    if( _blobType == BLOB && val > maximum * percentageOfMax )
		      rfront.insert( pair<float, Point3d>( val, neigh ) ) ;
		    else if( val < minimum * percentageOfMax )
		      front.insert( pair<float, Point3d>( val, neigh ) ) ;
		  }
	      }
	    }
	  }
	  for( set<Point3d>::iterator it = added.begin() ; it != added.end() ; ++it ){
	    ChangesItem item ;
	    item.before = volOfLabels( (unsigned) rint( (*it)[0] - vlOffset[0] ), 
					  (unsigned) rint( (*it)[1] - vlOffset[1] ), 
					  (unsigned) rint( (*it)[2] - vlOffset[2] ) ) ;
	    item.after = grao ;
	    
	    currentChanges->push_back(pair<Point3d, ChangesItem>( *it, item ) ) ;
	  }
	  
	  RoiChangeProcessor::instance()->applyChange( currentChanges ) ;
	  
	  cout << "Segmented blob volume : " << added.size() * volOfElt ;
	  if( _blobType == BLOB )
	    cout << "\tMinimum value segmented (in percentage of maximum value) : " 
		 << minimum / maximum * 100 << " %" << endl ;
	  else
	    cout << "\tMaximum value segmented (in percentage of minimum value) : " 
		 << maximum / minimum * 100 << " %"<< endl ;

	}
      
    }
}

Action*
RoiBlobSegmentationAction::creator()
{
  return  new RoiBlobSegmentationAction( ) ;
}
