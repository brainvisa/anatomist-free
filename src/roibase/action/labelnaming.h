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
#ifndef ROI_LABELNAMING_ACTION_H
#define ROI_LABELNAMING_ACTION_H

#include <anatomist/controler/action.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/volume/Volume.h>

#include <aims/bucket/bucketMap.h>

#include <queue>


namespace anatomist
{
  class Bucket ;
  class AObject ;
  class AGraphObject ;
  class RoiLabelNamingActionView_Private ;

  class RoiLabelNamingAction : public Action
  {
  public:    
    RoiLabelNamingAction() ;
    virtual ~RoiLabelNamingAction() ;
    
    virtual std::string name() const;

    void addWholeLabelToRegion( int x, int y, int globalX, int globalY ) ;
    void addConnecCompToRegion( int x, int y, int globalX, int globalY ) ;
    void removeConnecCompFromRegion( int x, int y, int globalX, int globalY ) ;
    void removeWholeLabelFromRegion( int x, int y, int globalX, int globalY ) ;

    static Action* creator() ;
    
    void setModeTo2D() { 
      myTwoDMode = 1 ; 
      std::cout << "Setting label naming mode to 2D" << std::endl ;
    }
    void setModeTo3D() { 
      myTwoDMode = 0 ; 
      std::cout << "Setting label naming mode to 3D" << std::endl ;
    }

    
  private:
    AObject * getCurrentImage() ;
    void fillRegion( int x, int y, anatomist::AGraphObject * region, 
		     std::list< std::pair< Point3d, ChangesItem> >& changes, 
		     bool add, bool wholeImage ) ;
    bool fillPoint( const Point3d& pc, int t,
		    AimsData<anatomist::AObject*>& volumeOfLabels, 
		    anatomist::AGraphObject * region, short label,
		    anatomist::AObject** toChange,
		    std::queue<Point3d>& trialPoints, bool replace = false, bool add = true )  ;
    bool in( const Point3d&, const Point3d& p ) ;
    template <class T> void computeImageValueMap(const anatomist::AVolume<T>& avol, int timePos ) ;
    static anatomist::AObject * myCurrentImage ;
    
    static std::vector< std::map< int16_t, int32_t> > myCurrentImageValues ;
    static std::vector<bool> myComputeCurrentImageValueMap ;
    static int32_t myNbOfPointsToSegmentLimit ;
    bool myTwoDMode ;
    
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
  };
}


inline bool 
anatomist::RoiLabelNamingAction::fillPoint( const Point3d& pc, int t,
					    AimsData<anatomist::AObject*>& volumeOfLabels, 
					    anatomist::AGraphObject * region, short label,
					    anatomist::AObject** toChange,
					    std::queue<Point3d>& trialPoints, bool replace, bool add )
{
  Point3d dims( volumeOfLabels.dimX(), volumeOfLabels.dimY(), volumeOfLabels.dimZ()) ;
  if( in( dims, pc ) ){
    float val = myCurrentImage->mixedTexValue( Point3df( pc[0], pc[1], pc[2] ), t ) ;
    if (add){
      if( (volumeOfLabels( pc ) != region) &&  (short(rint(val) == label) &&
						( replace || ( (!replace) && volumeOfLabels( pc ) == 0 )) ) ) {
	/*     if( (volumeOfLabels( pc ) != region) &&  (val >= realLowLevel) && (val <= realHighLevel) ){ */
	trialPoints.push(pc) ;
	*toChange = volumeOfLabels( pc ) ;
	
	volumeOfLabels( pc ) = region ;
	return true ;
      } 
    } else {
      if(  (volumeOfLabels( pc ) != 0 ) && (short(rint(val) == label) &&
	    ( replace || ( (!replace) && volumeOfLabels( pc ) == region ) ) ) ) {
	/*     if( (volumeOfLabels( pc ) != region) &&  (val >= realLowLevel) && (val <= realHighLevel) ){ */
	trialPoints.push(pc) ;
	*toChange = volumeOfLabels( pc ) ;
	
	volumeOfLabels( pc ) = 0 ;
	return true ;
      }
    }
  }
  return false ;
}


inline bool
anatomist::RoiLabelNamingAction::in( const Point3d& dims, const Point3d& p )
{
  if ( p[0] < 0 || p[0] > dims[0] - 1 ||  
       p[1] < 0 || p[1] > dims[1] - 1 ||
       p[2] < 0 || p[2] > dims[2] - 1 )
    return false ;
  
  return true ;
}



#endif
