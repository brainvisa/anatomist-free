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

#ifndef ROI_CHANGE_PROCESSOR
#define ROI_CHANGE_PROCESSOR

#include <aims/bucket/bucket.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/vector/vector.h>
#include <stack>

namespace anatomist
{ 
  class AObject ;
  
  struct ChangesItem{
    AObject * before ;
    AObject * after ;
  } ;
  
  struct ChangedBucketsItem {
    Point3df min ; 
    Point3df max ;
  } ;
  
  class RoiChangeProcessor : public Observable {
  public:
    ~RoiChangeProcessor( ) ;
    static RoiChangeProcessor* instance() ;
    
    void applyChange( std::list<std::pair<Point3d, ChangesItem> > * change ) ;
    void undo() ;
    void redo() ;
    
    void setUndoable( bool undoable ) ;
    bool undoable() 
      { 
	AGraph * graph = getGraph(0) ;
	if( graph == 0 ){
	  //cout << "Undoable :: graph nul" << endl ;
	  return false ;
	}
	return !( myChangesToUndo[graph].empty() ) ; 
      }
    void setRedoable( bool redoable ) ;
    bool redoable() { 
	AGraph * graph = getGraph(0) ;
	if( graph == 0 ){
	  //cout << "Redoable :: graph nul" << endl ;
	  return false ;
	}
      return !( myChangesToRedo[graph].empty() ) ; 
    }
    void noMoreUndoable() ;
    Bucket * getCurrentRegion( AWindow* ) ;
    anatomist::AGraph *getGraph( AWindow* ) ;
    anatomist::AGraphObject * getGraphObject( AWindow* ) ;
    
  private:
    void change( bool forward ) ;
    RoiChangeProcessor( int maxNumberOfUndo = 20 ) : myUndoable(false), myRedoable(false), 
						     myMaxNumberOfUndo(maxNumberOfUndo), myCurrentChanges(0), 
      myCurrentGraph(0) {}
    
    bool updateCurrentRegion( AWindow * ) ;
    std::set<anatomist::AObject*> selectedObjectsInWindow( AWindow* ) ; 
   
    static RoiChangeProcessor * _instance ;
    
    bool myUndoable ;
    bool myRedoable ;
    int myMaxNumberOfUndo ;
    std::map< AGraph*, std::list< std::list< std::pair< Point3d, anatomist::ChangesItem> >* > > 
    myChangesToUndo ;
    std::map< AGraph*, std::list< std::list< std::pair< Point3d, anatomist::ChangesItem> >* > > 
    myChangesToRedo ;
    std::list< std::pair< Point3d, anatomist::ChangesItem> > * myCurrentChanges ;
    AGraph * myCurrentGraph ;
    Bucket * myCurrentRegion ;
  } ;
}

#endif
