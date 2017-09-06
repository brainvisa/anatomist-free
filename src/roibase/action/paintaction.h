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


#ifndef PAINTACTION_H
#define PAINTACTION_H

#include "anatomist/controler/action.h"
#include <anatomist/bucket/Bucket.h>
#include <anatomist/object/Object.h>
#include <aims/data/data.h>
#include <aims/bucket/bucket.h>
#include <anatomist/observer/Observer.h>
#include <anatomist/observer/Observable.h>
#include <anatomist/action/roichangeprocessor.h>
#include <qobject.h>
#include <QWidget>
#include <string>
#include <vector>
#include <stack>

class AWindow3D ;
class QANumSlider ;

namespace anatomist
{
  class AObject ;
  class Transformation ;
  class AGraph ;
  class AGraphObject ;

  class PaintStrategy ;
  class PaintAction ;
  class PaintActionSharedData ;
  struct PaintActionView_Private ;
}


class PaintActionView : public QWidget, public anatomist::Observer
{

  Q_OBJECT 

public:
  PaintActionView( anatomist::PaintAction *, QWidget * ) ;
  ~PaintActionView() ;

  virtual void update( const anatomist::Observable * observable, void * arg ) ;

private slots :
  void brushSelection( int id ) ;
  void brushSizeChange( int size ) ;
  void regionTransparencyChange( int alpha ) ;
  //void transparencyChanged( int alpha ) ;
  void lineModeOn() ;
  void lineModeOff() ;
  void mmModeOn() ;
  void voxelModeOn() ;
  void replaceModeOn() ;
  void replaceModeOff() ;
  void linkedCursorModeOn() ;
  void linkedCursorModeOff() ;
  void undoAction() ;
  void redoAction() ;
  void fillAction() ;
  void clearRegionAction() ;
  
private:
  bool myUpdatingFlag ;
  anatomist::PaintActionView_Private * _private ;
} ;

namespace anatomist {

  class PaintStrategy
  {
  public:
    enum PaintType
    {
      POINT =0,
      SQUARE,
      //      CUBE,
      DISK,
      BALL
    } ;
    
    PaintStrategy( ) ;
    virtual ~PaintStrategy() ;
    virtual PaintType paintType() = 0 ;
    /** Draw
	\param vlOffset offset of the volume of labels (if VL coords don't 
	start at (0,0,0)
     */
    virtual void paint( AWindow3D * win,
			Transformation * transf, const Point3df& point,
			const AObject * originalLabel, AObject * finalLabel,
			float brushSize, bool lineMode,
			AimsData<AObject*> *volumeOfLabels,
			const Point3df & vlOffset, 
			aims::BucketMap<Void>::Bucket & deltaModifications,
			std::list< std::pair< Point3d, ChangesItem> > & changes,
			const Point3df& voxelSize,
			bool line,
			bool replace,
			bool mm ) = 0 ;
    
    static std::list< Point3df > drawLine( const Point3df& from, 
                                           const Point3df& dep ) ;
    static std::list< Point3d > drawFastLine( const Point3d& from, 
                                              const Point3d& dep ) ;
    void reset() ;
    bool in( const AimsData<AObject*> *o, Point3df p,
             const Point3df & offset ) ;
    
  private:
    
  protected:
    Point3df myPreviousPoint ;
    bool myPreviousPointExists ;
    
    static void brushPainter( ) ;
  } ;

  inline bool 
  PaintStrategy::in( const AimsData<AObject*> *o, Point3df p,
                     const Point3df & offset )
  {
    if( !o )
      return true;
    p -= offset;
    if ( p[0] < 0 || p[0] > o->dimX() - 1 ||
         p[1] < 0 || p[1] > o->dimY() - 1 ||
         p[2] < 0 || p[2] > o->dimZ() - 1 )
      return false ;

    return true ;
  }


  class PointPaintStrategy : public PaintStrategy
  {
  public:
    PointPaintStrategy() ;
    virtual ~PointPaintStrategy() ;
    virtual PaintType paintType() ;
    
    virtual void paint( AWindow3D * win,
			Transformation * transf, const Point3df& point,
			const AObject * originalLabel, AObject * finalLabel,
			float brushSize, bool lineMode,
			AimsData<AObject*> *volumeOfLabels,
			const Point3df & vlOffset, 
			aims::BucketMap<Void>::Bucket & deltaModifications,
			std::list< std::pair< Point3d, ChangesItem> > & changes,
			const Point3df& voxelSize,
			bool line,
			bool replace,
			bool mm ) ;
    
  private:
    
  } ;

  class SquarePaintStrategy : public PaintStrategy {
  public:
    SquarePaintStrategy() ;
    virtual ~SquarePaintStrategy() ;
    virtual PaintType paintType() ;
    
    virtual void paint( AWindow3D * win,
			Transformation * transf, const Point3df& point,
			const AObject * originalLabel, AObject * finalLabel,
			float brushSize, bool lineMode,
			AimsData<AObject*> *volumeOfLabels,
			const Point3df & vlOffset, 
			aims::BucketMap<Void>::Bucket & deltaModifications,
			std::list< std::pair< Point3d, ChangesItem> > & changes,
			const Point3df& voxelSize,
			bool line,
			bool replace,
			bool mm  ) ;
    

  } ;
  
  //   class CubePaintStrategy : public PaintStrategy {
  //  public:
  //     CubePaintStrategy() ;
  //     virtual ~CubePaintStrategy() ;
  //     virtual PaintType paintType() ;
    
  //     virtual void paint( AWindow3D * win,
// 			   Transformation * transf, const Point3df& point, 
  // 			const AObject * originalLabel, AObject * finalLabel, 
  //                      float brushSize, bool lineMode,
  // 			AimsData<AObject*>& volumeOfLabels,
  // 			Bucket& deltaModifications,
  // 			const Point3df& voxelSize,
  //                      bool line ) ;
    

  //   } ;

  class DiskPaintStrategy : public PaintStrategy
  {
  public:
    DiskPaintStrategy() ;
    virtual ~DiskPaintStrategy() ;
    virtual PaintType paintType() ;
    
    virtual void paint( AWindow3D * win,
			Transformation * transf, const Point3df& point,
			const AObject * originalLabel, AObject * finalLabel,
			float brushSize, bool lineMode,
			AimsData<AObject*> *volumeOfLabels,
			const Point3df & vlOffset, 
			aims::BucketMap<Void>::Bucket & deltaModifications,
			std::list< std::pair< Point3d, ChangesItem> > & changes,
			const Point3df& voxelSize,
			bool line,
			bool replace,
			bool mm ) ;
    
    virtual void brushPainter( const Point3df& diskCenter, 
			       const Point3df& n,
			       const AObject * originalLabel, 
			       AObject * finalLabel,
			       float brushSize, 
			       AimsData<AObject*> *volumeOfLabels,
			       const Point3df & voxelSize, 
			       const Point3df & vlOffset, 
			       aims::BucketMap<Void>::Bucket & deltaModifications,
			       std::list< std::pair< Point3d, ChangesItem> > & changes,
			       bool replace,
			       bool mm ) ;
  private :
  } ;

  class BallPaintStrategy : public PaintStrategy
  {
  public:
    BallPaintStrategy() ;
    virtual ~BallPaintStrategy() ;
    virtual PaintType paintType() ;
    
    virtual void paint( AWindow3D * win,
			Transformation * transf, const Point3df& point,
			const AObject * originalLabel, AObject * finalLabel,
			float brushSize, bool lineMode,
			AimsData<AObject*> *volumeOfLabels,
			const Point3df & vlOffset, 
			aims::BucketMap<Void>::Bucket & deltaModifications,
			std::list< std::pair< Point3d, ChangesItem> > & changes,
			const Point3df& voxelSize,
			bool line,
			bool replace,
			bool mm ) ;
    
    virtual void brushPainter( const Point3d& pToInt, 
			       const AObject * originalLabel, 
			       AObject * finalLabel,
			       float brushSize, 
			       AimsData<AObject*> *volumeOfLabels,
			       const Point3df & voxelSize, 
			       const Point3df & vlOffset, 
			       aims::BucketMap<Void>::Bucket  & deltaModifications,
			       std::list< std::pair< Point3d, ChangesItem> > & changes,
			       bool replace,
			       bool mm ) ;
  private :
    const std::vector<Point3d>& brush( int size ) ;
  } ;

  
  class PaintActionSharedData : public Observable, public Observer
  {
  public:
    virtual ~PaintActionSharedData() ;
    static PaintActionSharedData* instance() ;
    void noMoreUndoable() ;
    
    bool replaceMode() const { return myReplaceMode ; }
    virtual void update (const Observable *observable, void *arg) ;
    
  private:
    friend class PaintAction ;

    PaintActionSharedData() ;
    static PaintActionSharedData * _instance ;
    
    // Attributes
    float myBrushSize ;
    bool myLineMode ;
    bool myReplaceMode ;
    bool myFollowingLinkedCursor ;
    bool myMmMode ;
    
    anatomist::PaintStrategy * myPainter ;
    
    bool myIsChangeValidated ;
    bool myPainting ;
    bool myValidRegion ;
    
    std::list< std::pair< Point3d, ChangesItem> > * myCurrentChanges ;
    
    anatomist::Bucket * myDeltaModifications ;
    anatomist::Bucket * myCurrentModifiedRegion ;
    anatomist::Bucket * myCursor;
    Point3df myCursorPos;
    bool myCursorShapeChanged;
    Referential *myCursorRef;
  };


  class PaintAction : public anatomist::Action
  {
  public:
    PaintAction() ;
    virtual ~PaintAction() ;
    
    virtual std::string name() const;
    
    // Action inputs
    void increaseBrushSize( ) ;
    void decreaseBrushSize( ) ;
    void setSize( float i ) ;
    float brushSize() ;
    enum PaintStrategy::PaintType paintType() ;
    
    void changeRegionTransparency( float alpha ) ;
    void lineOn() ;
    void lineOff() ;
    bool lineMode() { return _sharedData->myLineMode ; }
    
    void replaceOn( ) ;
    void replaceOff( ) ;
    bool replaceMode() { return _sharedData->myReplaceMode ; }
    
    void followingLinkedCursorOn( ) ;
    void followingLinkedCursorOff( ) ;
    bool followingLinkedCursorMode() { return _sharedData->myFollowingLinkedCursor ; }
    
    void brushToSquare( ) ;
    void brushToDisk( ) ;
    void brushToBall( ) ;
    // Beware, size doesn't matter in this mode.
    void brushToPoint( ) ;
    void brushToMm() ;
    bool mmMode() { return _sharedData->myMmMode ;}
    void brushToVoxel() ;
    
    
    void paintStart( int x, int y, int globalX, int globalY ) ;
    void paint( int x, int y, int globalX, int globalY ) ;
    
    void eraseStart( int x, int y, int globalX, int globalY ) ;
    void erase( int x, int y, int globalX, int globalY ) ;
    
    void clearRegion() ;
    void fill( int x, int y, int globalX, int globalY ) ;
    void fillRegion2D( const Point3d& seed, 
                       const Point3d& bmin,
                       const Point3d& bmax,
                       Point3d neighbour[],
                       AimsData<AObject*>& volumeOfLabels, 
                       AObject * final,
                       std::list< std::pair< Point3d, 
                                             ChangesItem> > & changes ) ;
    void validateChange( int x = 0, int y = 0, int globalX = 0,
                         int globalY = 0 ) ;
    void undo( ) ;
    bool undoable() { return RoiChangeProcessor::instance()->undoable() ; }
    void redo( ) ;
    bool redoable() {return RoiChangeProcessor::instance()->redoable() ; }

    void moveCursor( int x, int y, int globalX, int globalY );
    void updateCursor();
    void hideCursor();

    static Action * creator() ;
    
    void change( bool forward ) ;
    virtual QWidget * actionView( QWidget *) ;
    virtual bool viewableAction( ) const ;
/*     void refreshWindow() ; */
    
    void addObserver (Observer *observer) { _sharedData->addObserver(observer) ; }
    void deleteObserver (Observer *observer) { _sharedData->deleteObserver(observer) ;}
    void notifyObservers (void *arg=0) { _sharedData->notifyObservers(arg) ; }
    void notifyUnregisterObservers ()  { _sharedData->notifyUnregisterObservers() ; }
    bool hasChanged () const { return _sharedData->hasChanged() ; }
    void setChanged () { _sharedData->setChanged() ; }
    void changeCursor( bool cross) ;
    void copyPreviousSliceWholeSession( ) ;
    void copyPreviousSliceCurrentRegion( ) ;
    void copyNextSliceWholeSession( ) ;
    void copyNextSliceCurrentRegion( ) ;
    
  private:
    PaintActionSharedData * _sharedData ;
    std::list<AWindow3D*> myLinkedWindows ;
    
    void copySlice( bool wholeSession, int sliceIncrement ) ;
    
//     bool getCurrentRegion( ) ;
//     anatomist::AGraph *getGraph( ) ;
//     anatomist::AGraphObject * getGraphObject( ) ;
//     std::set<anatomist::AObject*> selectedObjectsInWindow() ; 
  };
}
#endif
