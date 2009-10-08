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
 
#ifndef ROI_MORPHOMATH_ACTION_H
#define ROI_MORPHOMATH_ACTION_H


#include <anatomist/controler/action.h>
#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/object/Object.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <aims/bucket/bucketMap.h>

#include <aims/qtcompat/qvbox.h>


namespace anatomist
{
  class RoiMorphoMathActionView_Private ;

  class RoiMorphoMathAction : public Action, public Observable {
  public:
    enum DistanceMode{
      MM,
      VOXEL
    } ;
    enum RegionMode{
      REGION,
      SESSION
    } ;

    RoiMorphoMathAction() ;
    virtual ~RoiMorphoMathAction() ;
    
    virtual std::string name() const ;

    virtual QWidget * actionView( QWidget * ) ;
    virtual bool viewableAction( ) const { return true ; }
    
    static Action* creator() ;

    void dilation( bool partOfOpening = false ) ;
    void erosion() ;
    void opening() ;
    void closure() ;
    
    void structuringElementRadiusChange( float structuringElementRadius) ;
    void setDistanceToMm( ) ;
    void setDistanceToVoxel( ) ;
    void setRegionModeToRegion( ) ;
    void setRegionModeToSession( ) ;
    
    float structuringElementRadius() { return myStructuringElementRadius ; }
    int distanceMode() { return myDistanceMode ; }
    int regionMode() { return myRegionMode ; }
    
  private:
    AimsData<int16_t> * regionBinaryMask(  AGraphObject * go ) const ;
    
    DistanceMode myDistanceMode ;
    RegionMode myRegionMode ;
    float myStructuringElementRadius ;

    void doingDilation( anatomist::AGraphObject * graphObject,  
			std::list< std::pair< Point3d, ChangesItem> >* changes ) ;
    void doingErosion( anatomist::AGraphObject * graphObject,  
		       std::list< std::pair< Point3d, ChangesItem> >* changes ) ;
  } ;
}

class RoiMorphoMathActionView : public QVBox, public anatomist::Observer 
{
  Q_OBJECT
    
public:
  RoiMorphoMathActionView( anatomist::RoiMorphoMathAction * action,
			   QWidget * parent ) ;
  virtual ~RoiMorphoMathActionView() ;
  
  virtual void update( const anatomist::Observable *, void * ) ;

public slots:
  void dilation( ) ;
  void erosion() ;
  void opening() ;
  void closure() ;
/*   void default() ; */
  void structuringElementRadiusChanged( int newStructElRadius ) ;
  void distanceModeChanged( int newDistanceMode ) ;
  void regionModeChanged( int newRegionMode ) ;

 private:
  anatomist::RoiMorphoMathActionView_Private * _private ;
  bool myChangingFlag ;
  bool myUpdatingFlag ;
  bool myRegionMode ;
  
} ;

#endif 
