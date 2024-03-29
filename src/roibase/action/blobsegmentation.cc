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
in( const VolumeRef<T>& o, Point3df p,
    const Point3df & offset )
{
  p -= offset;
  vector<int> osize = o.getSize();
  if ( p[0] < 0 || p[0] > osize[0] - 1 ||
       p[1] < 0 || p[1] > osize[1] - 1 ||
       p[2] < 0 || p[2] > osize[2] - 1 )
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
  Bucket * currentRegion = RoiChangeProcessor::instance()->getCurrentRegion(view()->aWindow() ) ;
  if( !currentRegion )
    return ;

  list< pair< Point3d, ChangesItem> > * currentChanges = new list< pair< Point3d, ChangesItem> > ;
  RoiChangeProcessor::instance()->setRedoable( false ) ;
  AGraph * g = RoiChangeProcessor::instance()->getGraph( view()->aWindow() ) ;
  if (!g) return ;

  VolumeRef<AObject*>& labels = g->volumeOfLabels( 0 ) ;
  vector<float> bmin, bmax, vs;
  g->boundingBox2D( bmin, bmax );
  vs = g->voxelSize();
  vector<int> dims( 3 );
  dims[0] = int( rint( ( bmax[0] - bmin[0] ) / vs[0] ) );
  dims[1] = int( rint( ( bmax[1] - bmin[1] ) / vs[1] ) );
  dims[2] = int( rint( ( bmax[2] - bmin[2] ) / vs[2] ) );
  if( labels.getSizeX() != dims[0]
      || labels.getSizeY() != dims[1]
      || labels.getSizeZ() != dims[2] )
  {
    g->clearLabelsVolume();
    g->setLabelsVolumeDimension( dims[0], dims[1], dims[2] );
  }

  AGraphObject * grao = RoiChangeProcessor::instance()->getGraphObject( view()->aWindow() ) ;
  grao->attributed()->setProperty("modified", true) ;

  VolumeRef<AObject*> volOfLabels( g->volumeOfLabels( 0 ) ) ;
  AWindow3D * win = dynamic_cast<AWindow3D*>( view()->aWindow() ) ;
  if( !win )
    {
      cerr << "warning: RoiBlobSegmentationAction operating on wrong view type\n" ;
      return;
    }
  
  Referential* winRef = win->getReferential() ;
  Referential* buckRef = currentRegion->getReferential() ;
  Point3df pos ;
  vector<float> vpos = win->getFullPosition();
  //cout << "Time = " << time << endl ;

  if( win->positionFromCursor( x, y, pos ) )
    {
      Point3df normalVector( win->sliceQuaternion().transformInverse(
        Point3df(0., 0., 1.) ) );

      //     cout << "Normal Vector : " << normalVector << endl ;
      normalVector *= normalVector.dot( pos - win->getPosition() ) ;
      pos = pos - normalVector ;

      AObject * image = RoiManagementActionSharedData::instance()->
	getObjectByName( AObject::VOLUME, 
                         RoiManagementActionSharedData::instance()->
                         currentImage() );
      if( !image )
        return;

      vector<float> vs = image->voxelSize();
      float volOfElt = vs[0] * vs[1] * vs[2] ;
      _blobVolume = RoiLevelSetActionSharedData::instance()->maxSize()  ;
      if( _blobVolume <= 0 )
      {
        vector<float> vs = labels.getVoxelSize();
        _blobVolume = labels.getSizeX() * labels.getSizeY() * labels.getSizeZ()
          * vs[0] * vs[1] * vs[2];
      }

      float percentageOfMax = RoiLevelSetActionSharedData::instance()->percentageOfMaximum() / 100. ;
      float minimum = 10000000. ;
      float maximum = -10000000. ;
      float val ;

      Transformation * transf = theAnatomist->getTransformation( winRef, buckRef ) ;
      Point3df p ;
      if ( transf )
        p = transf->transform( pos );
      else
        p = pos;
      vpos[0] = p[0];
      vpos[1] = p[1];
      vpos[2] = p[2];

      vector<float> bmin, bmax;
      g->boundingBox2D( bmin, bmax );

      Point3df vlOffset( bmin[0] / vs[0] + 0.5,
                          bmin[1] / vs[1] + 0.5,
                          bmin[2] / vs[2] + 0.5 );

      Point3d posInt( static_cast<int>( p[0] / vs[0] + 0.5 ),
                      static_cast<int>( p[1] / vs[1] + 0.5 ), static_cast<int>( p[2] / vs[2] + 0.5 ) ) ;
      set<Point3d, PointLess> added ;
      added.insert(posInt) ;
      Connectivity connec(0, 0, Connectivity::CONNECTIVITY_6_XYZ) ;
      multimap<float, Point3d> front ;
      multimap<float, Point3d, More> rfront ;
      val = image->mixedTexValue( vpos ) ;
      if( _blobType == BLOB ){
        rfront.insert( pair<float, Point3d>( val, posInt ) ) ;
      }else{
        front.insert( pair<float, Point3d>( val, posInt ) );
      }
      Point3d pCurr, neigh ;
      float threshold = val;

      VolumeRef<short> finalMap( volOfLabels.getSize() );
      finalMap.setVoxelSize( volOfLabels.getVoxelSize() );
      finalMap.at(posInt) = 1 ;

      Point3d pMax, pMin ;

      while( ( ( _blobType == BLOB && !rfront.empty() /*&& minimum > percentageOfMax * maximum*/ ) ||
                ( _blobType != BLOB && !front.empty() /*&& maximum < percentageOfMax * minimum*/ ) )
            && (added.size() * volOfElt < _blobVolume || _blobVolume < 0 ) )
      {
        if( _blobType == BLOB )
        {
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

        vpos[0] = pCurr[0] * vs[0];
        vpos[1] = pCurr[1] * vs[1];
        vpos[2] = pCurr[2] * vs[2];
        val = image->mixedTexValue( vpos );

        if( val > maximum ){
          maximum = val ;
          pMax = pCurr ;
        }
        if( val < minimum ){
          minimum = val ;
          pMin = pCurr ;
        }

        //cout << "maximum = " << maximum << "\tminimum = " << minimum << endl ;
        for( int n = 0 ; n < connec.nbNeighbors() ; ++n )
        {
          neigh = pCurr + connec.xyzOffset(n) ;
          if( in( volOfLabels, Point3df(neigh[0], neigh[1], neigh[2]),
                  vlOffset ) )
          {
            vpos[0] = neigh[0] * vs[0];
            vpos[1] = neigh[1] * vs[1];
            vpos[2] = neigh[2] * vs[2];
            float val = image->mixedTexValue( vpos );

            if( _blobType == BLOB )
            {
              if( val > threshold && !finalMap.at(neigh) )
              {
                finalMap.at(neigh) = 1 ;
                if( PaintActionSharedData::instance()->replaceMode() ||
                    volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ),
                                  (unsigned) rint( neigh[1] - vlOffset[1] ),
                                  (unsigned) rint( neigh[2] - vlOffset[2] ) ) == 0 )
                  added.insert(neigh) ;

                rfront.insert( pair<float, Point3d>( val, neigh ) );
              }
            }
            else
            {
              if( val < threshold && !finalMap.at(neigh) )
              {
                finalMap.at(neigh) = 1 ;
                if( PaintActionSharedData::instance()->replaceMode() ||
                    volOfLabels( (unsigned) rint( neigh[0] - vlOffset[0] ),
                                  (unsigned) rint( neigh[1] - vlOffset[1] ),
                                  (unsigned) rint( neigh[2] - vlOffset[2] ) ) == 0 )
                  added.insert(neigh) ;

                front.insert( pair<float, Point3d>( val, neigh ) );
              }
            }
          }
        }
      }

      if( ((!front.empty() || !rfront.empty()) && _blobVolume > 0. )||
          ( _blobType == BLOB
            && ( minimum <= percentageOfMax * maximum ||
                  volOfLabels( (unsigned) rint( pMax[0] - vlOffset[0] ),
                              (unsigned) rint( pMax[1] - vlOffset[1] ),
                              (unsigned) rint( pMax[2] - vlOffset[2] ) )
                    != 0 ) ) ||
          ( _blobType != BLOB
            && ( maximum >= percentageOfMax * minimum ||
                  volOfLabels( (unsigned) rint( pMin[0] - vlOffset[0] ),
                              (unsigned) rint( pMin[1] - vlOffset[1] ),
                              (unsigned) rint( pMin[2] - vlOffset[2] ) )
                    != 0 ) ) )
      {
        cerr << "Sorry, you should click nearer from the chosen blob or increase blob size" << endl ;
        return ;
      }

      if( _blobVolume > 0. )
      {
        set<Point3d, PointLess> frontSet ;
        for( set<Point3d, PointLess>::iterator itA = added.begin() ; itA != added.end() ; ++itA )
        {
          for( int n = 0 ; n < connec.nbNeighbors() ; ++n )
          {
            Point3d neigh = *itA + connec.xyzOffset(n) ;
            if( in( finalMap, Point3df(neigh[0], neigh[1], neigh[2]),
                    vlOffset ) )
              if( !finalMap.at( neigh ) )
                if( frontSet.find(neigh) == frontSet.end() )
                {
                  vpos[0] = neigh[0] * vs[0];
                  vpos[1] = neigh[1] * vs[1];
                  vpos[2] = neigh[2] * vs[2];
                  val = image->mixedTexValue( vpos );
                  if( _blobType == BLOB && val > maximum * percentageOfMax )
                    rfront.insert( pair<float, Point3d>( val, neigh ) );
                  else if( val < minimum * percentageOfMax )
                    front.insert( pair<float, Point3d>( val, neigh ) );

                  frontSet.insert( neigh );
                }
          }
        }

        //cout << "volume = " << added.size() * volOfElt << endl ;
        while( added.size() * volOfElt < _blobVolume
                && ( ( _blobType == BLOB && !rfront.empty() )
                    || ( _blobType != BLOB && !front.empty() ) ) )
        {
          if( _blobType == BLOB )
          {
            if( rfront.empty() )
              cerr << "map should not be empty !" << endl ;
            pCurr = rfront.begin()->second ;
            //cout << "RFront size : " << rfront.size() << endl ;
            rfront.erase( rfront.begin() ) ;
          }
          else
          {
            if( front.empty() )
              cerr << "map should not be empty !" << endl ;
            pCurr = front.begin()->second ;
            //cout << "Front size : " << front.size() << endl ;
            front.erase( front.begin() ) ;
          }

          vpos[0] = pCurr[0] * vs[0];
          vpos[1] = pCurr[1] * vs[1];
          vpos[2] = pCurr[2] * vs[2];
          val = image->mixedTexValue( vpos ) ;
          if( val > maximum )
            maximum = val ;
          if( val < minimum )
            minimum = val ;

          // cout << "maximum = " << maximum << "\tminimum = " << minimum << endl ;

          finalMap.at( pCurr ) = 1 ;
          added.insert( pCurr ) ;

          for( int n = 0 ; n < connec.nbNeighbors() ; ++n )
          {
            neigh = pCurr + connec.xyzOffset(n) ;

            if( in( volOfLabels, Point3df(neigh[0], neigh[1], neigh[2]),
                    vlOffset) )
              if( (PaintActionSharedData::instance()->replaceMode() ||
                    volOfLabels.at( (unsigned) rint( neigh[0] - vlOffset[0] ),
                                    (unsigned) rint( neigh[1] - vlOffset[1] ),
                                    (unsigned) rint( neigh[2] - vlOffset[2] ) )
                        == 0)
                    && !finalMap.at(neigh) )
              {
                vpos[0] = neigh[0] * vs[0];
                vpos[1] = neigh[1] * vs[1];
                vpos[2] = neigh[2] * vs[2];
                val = image->mixedTexValue( vpos );
                if( _blobType == BLOB && val > maximum * percentageOfMax )
                  rfront.insert( pair<float, Point3d>( val, neigh ) ) ;
                else if( val < minimum * percentageOfMax )
                  front.insert( pair<float, Point3d>( val, neigh ) ) ;
              }
          }
        }
      }
      for( set<Point3d>::iterator it = added.begin() ; it != added.end() ; ++it )
      {
        ChangesItem item ;
        item.before
          = volOfLabels.at( (unsigned) rint( (*it)[0] - vlOffset[0] ),
                            (unsigned) rint( (*it)[1] - vlOffset[1] ),
                            (unsigned) rint( (*it)[2] - vlOffset[2] ) );
        item.after = grao;

        currentChanges->push_back(pair<Point3d, ChangesItem>( *it, item ) );
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

Action*
RoiBlobSegmentationAction::creator()
{
  return  new RoiBlobSegmentationAction( ) ;
}
