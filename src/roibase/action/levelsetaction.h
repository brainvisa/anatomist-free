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
#ifndef ROI_LEVELSET_ACTION_H
#define ROI_LEVELSET_ACTION_H

#include <anatomist/controler/action.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/object/Object.h>

#include <aims/bucket/bucketMap.h>
#include <QWidget>
#include <queue>


namespace anatomist
{
  
  class Bucket ;
  class AObject ;
  class AGraphObject ;
  class RoiLevelSetActionView_Private ;

  class RoiLevelSetActionSharedData : public Observer, public Observable {
  public:
    enum DimensionMode{
      TWOD,
      THREED
    } ;
    virtual ~RoiLevelSetActionSharedData() ;
    static RoiLevelSetActionSharedData* instance() ;
    virtual void update (const Observable *observable, void *arg) ;
    
    int maxSize() { return (int) myMaxSize ; }
    int percentageOfMaximum() { return (int) myPercentageOfMaximum ; }
    
  private:
    friend class RoiLevelSetAction ;

    RoiLevelSetActionSharedData() ;
    static RoiLevelSetActionSharedData * _instance ;
    
    bool myLevelSetActivation ;
    bool myLevelSetDeactivation ;
    anatomist::AObject * myCurrentImage ;
    DimensionMode myDimensionMode ;
    float myLowLevel ;
    float myHighLevel ;
    float myImageMax ;
    float myImageMin ;
    float myMaxSize ;
    float myPercentageOfMaximum ;
    std::string myMixMethod ;
    float myMixFactor ; 
    bool myGettingCurrentImage ;
    bool myActivatingLevelSet ;
    bool myUpdating ;
         
  } ;

  class RoiLevelSetAction : public Action//, public Observer, public Observable
  {
  public:
    enum DimensionMode{
      TWOD,
      THREED
    } ;
    
    RoiLevelSetAction() ;
    virtual ~RoiLevelSetAction() ;
    virtual void update( const anatomist::Observable *, void * ) ;
    
    virtual std::string name() const;

    void activateLevelSet() ;
    void deactivateLevelSet() ;
    void lowLevelChanged( float newLowLevel ) ;
    void highLevelChanged( float newHighLevel ) ;

    int dimensionMode() { return _sharedData->myDimensionMode ; }
    float lowLevel() { return _sharedData->myLowLevel ; }
    float highLevel() { return _sharedData->myHighLevel ; }
    float imageMax() { return _sharedData->myImageMax ; }
    float imageMin() { return _sharedData->myImageMin ; }
    bool levelSetActivation() { return _sharedData->myLevelSetActivation ; }
    float mixFactor() { return _sharedData->myMixFactor ; }
    float maxSize() { return _sharedData->myMaxSize ; }
    float percentageOfMaximum() { return _sharedData->myPercentageOfMaximum ; }
    std::string mixMethod() { return _sharedData->myMixMethod ; }
    
    void setDimensionModeTo2D() ;
    void setDimensionModeTo3D() ;
    void setMixMethod( const std::string& ) ;
    void setMixFactor( float ) ;
    void setMaxSize( float maxSize ) ;
    void setPercentageOfMaximum( float percentageOfMaximum ) ;
    
    void replaceRegion( int x, int y, int globalX, int globalY ) ;
    void addToRegion( int x, int y, int globalX, int globalY ) ;
    void removeFromRegion( int x, int y, int globalX, int globalY ) ;

    float realMin( ) const ;
    float realMax( ) const ;
    
    virtual QWidget * actionView( QWidget * ) ;
    virtual bool viewableAction( ) const { return true ; }
    
    static Action* creator() ;


  protected:    
    virtual void unregisterObservable( anatomist::Observable*) ;


  private:
    RoiLevelSetActionSharedData * _sharedData ;
    void updateObjPal() ;
    AObject * getCurrentImage() ;
    void fillRegion( int x, int y, anatomist::AGraphObject * region, 
		     std::list< std::pair< Point3d, ChangesItem> >& changes, bool add ) ;
    bool fillPoint( const Point3d& pc, int t,
		    AimsData<anatomist::AObject*>& volumeOfLabels, 
		    anatomist::AGraphObject * region, float realLowLevel, 
		    float realHighLevel,
		    anatomist::AObject** toChange,
		    std::queue<Point3d>& trialPoints, bool replace = false )  ;
    bool in( const Point3d&, Point3d p ) ;
    
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
};



class RoiLevelSetActionView : public QWidget, public anatomist::Observer
{
  Q_OBJECT
  
public:
  RoiLevelSetActionView( anatomist::RoiLevelSetAction *  myAction,
			 QWidget * parent ) ;
  virtual ~RoiLevelSetActionView() ;
  
  virtual void update( const anatomist::Observable *, void * ) ;

public slots:
  void levelSetActivationChanged( int button ) ;
  void lowLevelChanged( int newLowLevel ) ;
  void highLevelChanged( int newHighLevel ) ;
  void dimensionModeChanged( int newDimensionMode ) ;
  void maxSizeChanged( const QString& newMaxSize ) ;
  void percentageOfMaxChanged(const QString&) ;
  void mixFactorChanged( int newMixFactor ) ;
  void mixMethodChanged( const QString& newMixMethod ) ;
  
private:
  float myChangingFlag ;
  float myUpdatingFlag ;
  anatomist::RoiLevelSetActionView_Private * _private ;
};



#endif
