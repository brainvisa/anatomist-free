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

#include <anatomist/action/roichangeprocessor.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/Graph.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/misc/error.h>

using namespace anatomist ;
using namespace aims ;
using namespace std;
  
RoiChangeProcessor* RoiChangeProcessor::_instance = 0 ;

RoiChangeProcessor* RoiChangeProcessor::instance()
{
  if( _instance == 0 )
    _instance = new RoiChangeProcessor ;
  return _instance ;
}

RoiChangeProcessor::~RoiChangeProcessor()
{  
  list< pair< Point3d, ChangesItem> >* temp, *temp2 ;
  std::map< AGraph*, std::list< list< pair< Point3d, ChangesItem> >* > >::iterator
    iter( myChangesToUndo.begin() ),last(myChangesToUndo.end() ) ;
  while ( iter != last ){
    while ( !iter->second.empty() ){
      temp = iter->second.front() ;
      iter->second.pop_front() ;
      if( temp )
	delete temp ;
    }
  }
  
  iter = myChangesToRedo.begin() ;
  last = myChangesToRedo.end() ;
  while ( iter != last ){
    while ( !iter->second.empty() ){
      temp2 = iter->second.front() ;
      iter->second.pop_front() ;
      if( temp2 )
	delete temp2 ;
    }
    ++iter ;
  }
  notifyUnregisterObservers() ;
}

void 
RoiChangeProcessor::change( bool forward )
{
  set<AGraphObject *> modifiedObjList ;
  list<pair< Point3d, ChangesItem > >::const_iterator 
    changesIter( (*myCurrentChanges).begin( ) ), 
    changesLast( (*myCurrentChanges).end( ) ) ;
  map<Bucket *, ChangedBucketsItem > changedBuckets ;
  set<Bucket *> erasedbck;

  BucketMap<Void>::Bucket::iterator found/*, current*/ ;
  while ( changesIter != changesLast ){
    Bucket		*bk = 0;
    AGraphObject * moa = dynamic_cast<AGraphObject*>( changesIter->second.after ) ;
    AGraphObject * mob = dynamic_cast<AGraphObject*>( changesIter->second.before ) ;

    if( ( forward ? mob : moa ) != 0 ){
      AGraphObject::iterator	ic, ec ( forward ? mob->end() : moa->end() ) ;

      for( ic = ( forward ? mob->begin() : moa->begin() ) ; ic!=ec; ++ic )
	if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	  break;

      if ( bk != 0 ) {
	found = bk->bucket()[0].find( changesIter->first ) ;
	if ( found != bk->bucket()[0].end() ){
	  bk->bucket()[0].erase( found ) ;
	  bk->setBucketChanged() ;
	}
        erasedbck.insert( bk );
      }
    }
    if( ( forward ? moa : mob ) != 0 ){
      map<Bucket *, ChangedBucketsItem >::iterator cbFound ;
      ChangedBucketsItem item ;

      AGraphObject::iterator	ic, 
	ec ( ( forward ? moa->end() : mob->end() ) ) ;

      for( ic = ( forward ? moa->begin() : mob->begin() ) ; ic!=ec; ++ic )
	if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	  break;

      if ( bk != 0 )
	{
	  //bk->bucket()[0].insert( pair<> )
	  bk->bucket()[0][changesIter->first] ;
	  bk->setBucketChanged() ;
	  
	  if ( ( cbFound = changedBuckets.find(bk) ) == changedBuckets.end() )
	    {
	      item.min[0] = item.max[0] = float((changesIter->first)[0]) ; 
	      item.min[1] = item.max[1] = float((changesIter->first)[1]) ; 
	      item.min[2] = item.max[2] = float((changesIter->first)[2]) ; 
	      changedBuckets[bk] = item ;
	    }
	  else 
	    {
	      if ( (changesIter->first)[0] < cbFound->second.min[0] )
		cbFound->second.min[0] = (float)(changesIter->first)[0] ;
	      else if ( (changesIter->first)[0] > cbFound->second.max[0] ) 
		cbFound->second.max[0] = (float)(changesIter->first)[0] ;

	      if ( (changesIter->first)[1] < cbFound->second.min[1] ) 
		cbFound->second.min[1] = (float)(changesIter->first)[1] ;
	      else if ( (changesIter->first)[1] > cbFound->second.max[1] ) 
		cbFound->second.max[1] = (float)(changesIter->first)[1] ;

	      if ( (changesIter->first)[2] < cbFound->second.min[2] ) 
		cbFound->second.min[2] = (float)(changesIter->first)[2] ;
	      else if ( (changesIter->first)[2] > cbFound->second.max[2] ) 
		cbFound->second.max[2] = (float)(changesIter->first)[2] ;
	    }
	}
    }

    if( moa || mob ){
      AGraph * graph = getGraph( 0 ) ;
      if( graph )
      {
	graph->volumeOfLabels( 0 )( changesIter->first )
            = ( forward ? moa : mob ) ;
	graph->setContentChanged() ;
      }
      if( moa )
	modifiedObjList.insert( moa ) ;
      if( mob )
	modifiedObjList.insert( mob ) ;
    }
    ++changesIter ;
  }

  map<Bucket *, ChangedBucketsItem>::iterator iSet(changedBuckets.begin()), 
                                              lSet(changedBuckets.end()) ;
  while( iSet != lSet ){
    iSet->first->setSubBucketGeomExtrema( iSet->second.min, iSet->second.max ) ;
    cout << "notify change for " << iSet->first->name() << endl;
    if( !iSet->first->hasChanged() )
      cout << "not changed...\n";
    iSet->first->setGeomExtrema();
    iSet->first->notifyObservers( this );
    ++iSet ;
  }

  set<Bucket *>::iterator ieb, eeb = erasedbck.end();
  for( ieb=erasedbck.begin(); ieb!=eeb; ++ieb )
    (*ieb)->notifyObservers( this );

  set<AGraphObject*>::const_iterator objIter(modifiedObjList.begin()), 
    objLast(modifiedObjList.end()) ;
  //set<AWindow3D*> winList ;
  while(objIter != objLast) {
    (*objIter)->attributed()->setProperty( "modified", true );
    (*objIter)->notifyObservers( this );
    /*set<AWindow*> subWinList = (*objIter)->WinList() ;
    set<AWindow*>::const_iterator sWinIter(subWinList.begin()), 
      sWinLast(subWinList.end()) ;
    while( sWinIter != sWinLast ){
      if( dynamic_cast<AWindow3D*>(*sWinIter) )
	winList.insert( dynamic_cast<AWindow3D*>(*sWinIter) ) ;

      ++sWinIter ;
    }
    */
    ++objIter ;
  }

  /*
  set<AWindow3D*>::iterator iter( winList.begin() ), 
    last( winList.end() ) ;

  while ( iter != last ){
    (*iter)->Refresh() ;
    //cout << "Undo : " << (*iter)->Title()<< " refreshed" << endl ;
    ++iter ;
  }
  */

}

void 
RoiChangeProcessor::undo()
{  
  if ( undoable() )
    {
      AGraph * g = getGraph( 0 ) ;
      if(!g) 
	return ;
      myCurrentChanges = myChangesToUndo[g].front() ;
      myChangesToUndo[g].pop_front() ;
      change( false ) ;
      myChangesToRedo[g].push_front( myCurrentChanges ) ;
      myCurrentChanges = 0 ;
      
      setChanged() ;
      notifyObservers() ;
    }
}

void
RoiChangeProcessor::redo()
{
  if ( redoable() )
    {
      AGraph * g = getGraph( 0 ) ;
      if(!g) 
	return ;
      myCurrentChanges = myChangesToRedo[g].front() ;
      myChangesToRedo[g].pop_front() ;
      change( true ) ;
      myChangesToUndo[g].push_front( myCurrentChanges ) ;
      myCurrentChanges = 0 ;
      
      setChanged() ;
      notifyObservers() ;
    }
}

void 
RoiChangeProcessor::applyChange( list< pair< Point3d, ChangesItem> > * c )
{
  myCurrentChanges = c ;
  AGraph * g = getGraph( 0 )  ;
  if(!g) 
    return ;
  // Undo and redo stack management
  if ( ! myCurrentChanges->empty() ){
    change( true ) ;
    myChangesToUndo[g].push_front( myCurrentChanges ) ;
    if( myChangesToUndo[g].size() > (unsigned long) myMaxNumberOfUndo )
      myChangesToUndo[g].pop_back() ;
    myCurrentChanges = 0 ;
    setUndoable( true ) ;
  }

  setRedoable( false ) ;
  setChanged() ;
  notifyObservers() ;
}

void 
RoiChangeProcessor::setUndoable( bool undoable )
{
  if( !undoable ){
    AGraph * g = getGraph( 0 ) ;
    if(!g) 
      return ;
    list< pair< Point3d, ChangesItem> > * temp ;
    while ( !myChangesToUndo[g].empty() ){
      temp = myChangesToUndo[g].front() ;
      myChangesToUndo[g].pop_front() ;
      if( temp )
	delete temp ;
    }
  }
//   setChanged() ;
//   notifyObservers() ;
}

void
RoiChangeProcessor::setRedoable( bool redoable )
{
  if( !redoable ){
    AGraph * g = getGraph(0) ;
    if(!g) 
      return ;
    list< pair< Point3d, ChangesItem> > * temp ;
    while ( !myChangesToRedo[g].empty() ){
      temp = myChangesToRedo[g].front() ;
      myChangesToRedo[g].pop_front() ;
      if( temp )
	delete temp ;
    }
  }
//   setChanged() ;
//   notifyObservers() ;
}

void 
RoiChangeProcessor::noMoreUndoable()
{
  list< pair< Point3d, ChangesItem> > * temp ;
  AGraph * g = getGraph( 0 ) ;
  if(!g) 
    return ;

  while ( !myChangesToUndo[g].empty() ){
    temp = myChangesToUndo[g].front() ;
    myChangesToUndo[g].pop_front() ;
    if(temp)
      delete temp ;
  }

  while ( !myChangesToRedo[g].empty() ){
    temp = myChangesToRedo[g].front() ;
    myChangesToRedo[g].pop_front() ;
    if(temp)
      delete temp ;
  }
  setChanged() ;
  notifyObservers() ;
}

set<AObject*>
RoiChangeProcessor::selectedObjectsInWindow( AWindow * win )
{
  set<AObject*> selObjsInWindow ;

  if( win == 0 )
  {
    map<unsigned, set<AObject *> >::const_iterator 
      groupIter(SelectFactory::factory()->selected().begin() ), 
      groupLast(SelectFactory::factory()->selected().end() ) ;
    while( groupIter != groupLast )
      {
        selObjsInWindow.insert( groupIter->second.begin(),
                                groupIter->second.end() ) ;
        ++groupIter ;
      }
    return selObjsInWindow;
  }

  map<unsigned, set<AObject *> >::const_iterator 
    found( SelectFactory::factory()->selected().find( win->Group() ) ) ;

  if( found == SelectFactory::factory()->selected().end() )
    return selObjsInWindow;

  set<AObject*> winObjects = win->Objects();

  set<AObject*>::iterator 
    iterSel( found->second.begin() ), lastSel( found->second.end() ), 
    iterInWin( winObjects.begin() ), lastInWin( winObjects.end() ) ;

  while (iterSel != lastSel && iterInWin != lastInWin) 
    if (*iterSel < *iterInWin) 
      ++iterSel;
    else if (*iterInWin < *iterSel) 
      ++iterInWin;
    else {
      selObjsInWindow.insert( selObjsInWindow.end(), *iterSel ) ;
      ++iterSel;
      ++iterInWin;
    }
  return selObjsInWindow ;
}


Bucket * 
RoiChangeProcessor::getCurrentRegion( AWindow * win )
{
  updateCurrentRegion( win ) ;
  return myCurrentRegion ;
}

bool 
RoiChangeProcessor::updateCurrentRegion( AWindow * win )
{
  //cerr << "PaintAction::getCurrentRegion : entering" << endl ;
  myCurrentRegion = 0 ;
  
  unsigned bucketCount = 0, graphObjectCount = 0 ;
  
  set<AObject*> selInWin = selectedObjectsInWindow( win ) ;
  set<AObject*>::iterator iterSelInWin(selInWin.begin()), lastSelInWin(selInWin.end()),
    groupBucketFound, groupGraphObjectFound ;
  while( iterSelInWin != lastSelInWin )
    {
      if( (*iterSelInWin)->type() == AObject::BUCKET ){
	groupBucketFound = iterSelInWin ;
	++bucketCount ;
      }
      if( (*iterSelInWin)->type() == AObject::GRAPHOBJECT ){
	groupGraphObjectFound = iterSelInWin ;
	++graphObjectCount ;
      }
      ++iterSelInWin ;
    }

  if( bucketCount != 1 )
    if( graphObjectCount != 1 )
      {
// 	AWarning("Sorry, there should be one and only one region selected to "
// 		 "be painted.") ;
// 	cout << " Graph Object Count " << graphObjectCount << endl ;
	return false ;
      }
    else
      {
	AGraphObject * grao = dynamic_cast<AGraphObject *>( *groupGraphObjectFound ) ;
	Bucket		*bk = 0;
	if( grao != 0 ){
	  AGraphObject::iterator	ic, ec = grao->end();
	  
	  for( ic=grao->begin(); ic!=ec; ++ic )
	    if( ( bk = dynamic_cast<Bucket *>( *ic ) ) )
	      break;
	}
	if( bk )
	  myCurrentRegion = bk ;
	else
	  return false ;
      }
  else
    if(myCurrentRegion)
      myCurrentRegion = dynamic_cast<Bucket * > (*groupBucketFound ) ;


//   cout << "Bucket Name : " << myCurrentModifiedRegion->name() 
//        << "\t id : " << myCurrentModifiedRegion->id() << " Vox Size : " 
//        << myCurrentModifiedRegion->VoxelSize() << endl ;
  
  // It will never be a null pointer, but verify anyway
  if ( myCurrentRegion == 0 )
    return false ; 
  //cerr << "PaintAction::getCurrentRegion : exiting" << endl ;

  return true ;
}

AGraphObject *
RoiChangeProcessor::getGraphObject( AWindow * win )
{
  //cerr << "PaintAction::getGraph : entering" << endl ;

  if( ! updateCurrentRegion( win ) )
    return 0 ;

  AObject::ParentList parents = myCurrentRegion->Parents() ;
  AObject::ParentList::iterator
      iter( parents.begin() ), last( parents.end() ) ;
  while ( iter != last )
  {
    if( (*iter)->type() == AObject::GRAPHOBJECT ) 
      break ;
    ++iter ;
  }

  if( iter != last )
    return dynamic_cast<AGraphObject*>( *iter ) ;
  return 0;
}

AGraph *
RoiChangeProcessor::getGraph( AWindow * win )
{
  AGraphObject * grao = getGraphObject( win ) ;
  if( grao == 0 )
    return 0 ;

  AObject::ParentList parents = grao->Parents() ;
  AObject::ParentList::iterator
      iter( parents.begin() ), last( parents.end() ) ;

  while ( iter != last )
  {
    if( (*iter)->type() == AObject::GRAPH ) 
      break ;
    ++iter ;
  }

  if( iter != last )
    return dynamic_cast<AGraph*>( *iter ) ;
  return 0;
}
