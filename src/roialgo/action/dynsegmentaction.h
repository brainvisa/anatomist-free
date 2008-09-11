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
#ifndef ROI_DYNSEGMENT_ACTION_H
#define ROI_DYNSEGMENT_ACTION_H

#include <anatomist/controler/action.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/object/Object.h>

#include <aims/bucket/bucketMap.h>
#include <queue>

#include <aims/qtcompat/qvbox.h>

namespace anatomist
{
  
  class Bucket ;
  class AObject ;
  class AGraphObject ;
  class RoiDynSegmentActionView_Private ;

  class RoiDynSegmentAction : public Action, public Observer, public Observable {
  public:
    enum DimensionMode{
      TWOD,
      THREED
    } ;
    
    RoiDynSegmentAction() ;
    virtual ~RoiDynSegmentAction() ;
    virtual void update( const anatomist::Observable *, void * ) ;
    
    virtual std::string name() const ;

    virtual QWidget * actionView( QWidget * ) ;
    virtual bool viewableAction( ) const { return true ; }
    
    static Action* creator() ;

    int dimensionMode() const { return myDimensionMode ; }
    int order() const { return myOrder ; }
    int faithInterval() const { return myFaithInterval ; }
    int refineMode() const { return int( myRefineMode) ; }
    int findNearestMinimum() const { return int( myFindNearestMinimumMode ) ; }
    float meanValue() const { return myInsideMeanError ; }
    float sigmaValue() const { return myInsideSigmaError ; }
    bool displayResults() const { return myDisplayResults ; }

    void replaceRegion( int x, int y, int , int ) ;
    void setPointToSegmentByDiscriminatingAnalyse( int x, int y, int, int ) ;
    void changeOrder( int newOrder ) ;
    void increaseOrder( ) { changeOrder(myOrder+1) ; }
    void decreaseOrder( ) { changeOrder(myOrder-1) ; }
    void changeFaithInterval( int newFaithInterval ) ;
    void increaseFaithInterval( )
    { changeFaithInterval( myFaithInterval+1 ) ; }
    void decreaseFaithInterval( )
    { changeFaithInterval( myFaithInterval-1 ) ; }
    void refineModeOn() { myRefineMode = true ; }
    void refineModeOff()  { myRefineMode = false ; }
    void findNearestMinimumModeOn() { myFindNearestMinimumMode = true ; }
    void findNearestMinimumModeOff()  { myFindNearestMinimumMode = false ; }
    void dimensionModeTo2D() 
      { 
	myDimensionMode = TWOD ; 
	std::cout << "Dymension Mode set to 2D" << std::endl ;
      }
    void dimensionModeTo3D() 
      { 
	myDimensionMode = THREED ; 
	std::cout << "Dymension Mode set to 3D" << std::endl ;
      }

  protected:
    virtual void unregisterObservable( anatomist::Observable*) ;

  private:
    AObject * getCurrentImage() ;
    bool in( const Point3d& dims, Point3d p ) ;
    
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
    
    bool findLocalBestSeed( const Point3d& dims, const Point3d& halfSize ) ;
    bool evaluateError( const Point3d& p, const Point3d& halfSize, 
			const Point3d& dims,
			AimsData<float>&  errorMatrix, 
			std::vector<float>& meanSignal,
			float& mean, float& var, 
			bool forceComputing = false ) ;
    void pcaRegionGrowth( ) ;
    void growth( std::list< std::pair< Point3d, ChangesItem> >* changes ) ;
    void refinePCA(  std::list< std::pair< Point3d, ChangesItem> >* changes ) ;
    void computeErrorMatrix( AimsData<float>& matriceIndiv, 
			     AimsData<float>& errorMatrix,
			     std::vector<float>& meanSignal  ) ;
    float error( const anatomist::AObject* data, 
		 const Point3df& p, const std::vector<float>& meanSignal,
		 AimsData<float>& errorMatrix ) ;
    bool valid( const anatomist::AObject* data, const Point3df& p ) ;
    Point3d maskHalfSize( const anatomist::AObject * vol, int nbIndiv )  ;

    AObject * myCurrentImage ;
    Point3d mySeed ;
/*     Point3d myPreviousSeed ; */
    bool mySeedChanged ;
    bool myOrderChanged ;
    bool myRefineMode ;
    bool myFindNearestMinimumMode ;
    bool myDisplayResults ;
    DimensionMode myDimensionMode ;
    int myFaithInterval ;
    int myOrder ;
    AimsData<float> myErrorMatrix ;
    float myInsideMeanError ;
    float myInsideSigmaError ;
    std::vector<float> myMeanSignal ;
    std::map<Point3d, float, PointLess> myPreviousComputing ;
    
    Point3d myXAxis ;
    Point3d myYAxis ;
    Point3d myZAxis ;
  } ;
} ;

inline
bool 
anatomist::RoiDynSegmentAction::valid( const anatomist::AObject* data, const Point3df& p )
{
  for( int t = 0 ; t <= data->MaxT() ; ++t )
    if( data->mixedTexValue( p, t) <= 0. )
      return false ;
  return true ;
}

inline
float 
anatomist::RoiDynSegmentAction::error( const anatomist::AObject* data, 
				     const Point3df& p, 
				     const std::vector<float>& meanSignal,
				     AimsData<float>& errorMatrix )
{
  AimsData<float> indiv( 1, int(data->MaxT()+1.1) ), indivTr(int(data->MaxT()+1.1 ) ) ;
  float norm2 = 0. ;
  for ( int t = 0 ; t < data->MaxT()+1 ; ++t ){
    float dt = data->mixedTexValue( p, t ) ;
    indiv(0,t) = dt - meanSignal[t] ;
    indivTr(t,0) = dt - meanSignal[t] ;
    norm2 += dt*dt ;
  }
  if( norm2 == 0. ){
    return 100. ;
  }
  return indiv.cross( errorMatrix.cross(indivTr) )(0,0) / norm2 ;
}


class RoiDynSegmentActionView : public QVBox, public anatomist::Observer
{
  Q_OBJECT
  
public:
  RoiDynSegmentActionView( anatomist::RoiDynSegmentAction *  myAction,
			   QWidget * parent ) ;
  virtual ~RoiDynSegmentActionView() ;
  
  virtual void update( const anatomist::Observable *, void * ) ;

public slots:
  void faithIntervalChanged( int nSigma ) ;
  void orderChanged( int order ) ;
  void dimensionModeChanged( int newDimensionMode ) ;
  void computeModeChanged( int growthMode ) ;
  void findNearestMinimumModeChanged( int findLocalMinimum ) ;
  //void maskChanged( const QString& mask ) ;
  void refineModeChanged( int mode ) ;
private:
  float myChangingFlag ;
  float myUpdatingFlag ;
  anatomist::RoiDynSegmentActionView_Private * _private ;
};

#endif
