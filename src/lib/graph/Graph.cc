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


#include <anatomist/control/graphParams.h>

#include <anatomist/graph/Graph.h>
#include <anatomist/graph/GraphObject.h>
#include <anatomist/graph/qgraphproperties.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/bucket/Bucket.h>
#include <anatomist/object/actions.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/application/syntax.h>
#include <graph/tree/tree.h>
#include <anatomist/object/objectConverter.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/color/objectPalette.h>

#include <aims/resampling/standardreferentials.h>
#include <graph/graph/graph.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/fileutil.h>
#include <graph/graph/greader.h>
#include <graph/graph/gwriter.h>
#include <aims/io/writer.h>
#include <aims/def/path.h>
#include <aims/bucket/bucket.h>
#include <cartobase/type/limits.h>
#ifdef __APPLE__
// template exportation bug on MacOS 10
#include <aims/io/aimsGraphR_d.h>
#else
#include <aims/io/aimsGraphR.h>
#endif
#include <aims/io/aimsGraphW.h>
#include <aims/io/finder.h>
#include <aims/graph/graphmanip.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

bool	(*AGraph::specialColorFunc)( AGraph* ag, AGraphObject* go, 
  				     Material & mat ) 
  = QGraphParam::labelColorsActivated() 
  ? &GraphParams::recolorLabelledGraph : 0;



//

struct AGraph::Private
{
  Private();

  ///	volume of labels
  std::map<int, AimsData<AObject *> >	*labelsVol;
  float					minX;
  float					minY;
  float					minZ;
  float					maxX;
  float					maxY;
  float					maxZ;
  Point3df				voxelSize;
  Point3d				labelDim;
  AGraph::ColorMode			colormode;
  bool					recolorrecurs;
  string				colorproperty;
  int                                   colorpropmask;
  rc_ptr<Graph>                         graph;
  set<shared_ptr<AObject> >             data;
};


AGraph::Private::Private()
  : labelsVol( 0 ), 
    minX( 0 ), minY( 0 ), minZ( 0 ), maxX( 0 ), maxY( 0 ), maxZ( 0 ), 
    colormode( Normal ), 
    //colormode( PropertyMap ), 
    recolorrecurs( false ), 
    colorproperty( "size" ), colorpropmask( 1 )
{
}


namespace
{

  void files2Obj( AGraph* ag, int subobj );

}


AGraph::AGraph( Graph *dataGraph, const string & filename, bool init,
                const Point3d& labelDimension )
  : MObject(), AttributedAObject(), d( new Private )
{
  d->graph.reset( dataGraph );
  initialize( filename, init, labelDimension );
}


AGraph::AGraph( rc_ptr<Graph> dataGraph, const string & filename, bool init,
                const Point3d& labelDimension )
  : MObject(), AttributedAObject(), d( new Private )
{
  d->graph = dataGraph;
  initialize( filename, init, labelDimension );
}


void AGraph::initialize( const string & filename, bool init,
                         const Point3d& labelDimension )
{
#ifdef ANA_DEBUG
  cout << "AGraph: create " << this << ", current objects number: " 
       << theAnatomist->getObjects().size() << endl;
#endif

  d->labelDim = labelDimension ;
  _type = AObject::GRAPH;
  if( filename.empty() || filename == "" )
  {
    string f;
    if( d->graph->getProperty( "aims_reader_filename", f ) )
      setFileName( f );
  }
  else
    setFileName( filename );
  _material.SetDiffuse( 0., 0.5, 1., 1. );

  glAddTextures( 1 );
  GLComponent::TexExtrema & ex = glTexExtrema();
  ex.min.push_back( 0.F );
  ex.max.push_back( 1.F );
  ex.minquant.push_back( 0.F );
  ex.maxquant.push_back( 1.F );

  if ( init )
  {
    // force ref so my children inherit it
    setReferential( theAnatomist->centralReferential() );
    int mask = 1;
    d->graph->getProperty( "aims_reader_loaded_objects", mask );
    mask |= 1;
    files2Obj( this, mask );
    SetMaterial( _material );	// propagate
  }

  if( d->colormode != Normal )
    {
      updateExtrema();
      recolor();
    }

#ifdef ANA_DEBUG
  cout << "AGraph done. #obj (appli): "
       << theAnatomist->getObjects().size() << endl;
  cout << "  graph children: " << size() << endl;
#endif
}

AGraph::~AGraph()
{
#ifdef ANA_DEBUG
  cout << "~AGraph " << this << ": before cleanup. #obj: " 
       << theAnatomist->getObjects().size() << endl;
#endif

  cleanup();

  iterator		io, fo=end();
  set<AObject *>	toDelete;

  // clear relations
  const set<Edge *>            & edges = d->graph->edges();
  set<Edge *>::const_iterator  ie, ee = edges.end();
  shared_ptr<AObject>          o;

  for( ie=edges.begin(); ie!=ee; ++ie )
    if( (*ie)->getProperty( "ana_object", o ) )
    {
      o->UnregisterParent( this );
      (*ie)->removeProperty( "ana_object" );
    }

  for( io=begin(); io!=fo; ++io )
  {
    (*io)->UnregisterParent( this );
    (*io)->deleteObserver((Observer*)this);
  }

  delete d->labelsVol;
  d->labelsVol = 0;

  delete d;

#ifdef ANA_DEBUG
  cout << "~AGraph done. #obj: " << theAnatomist->getObjects().size() << endl;
#endif
}


AObject* AGraph::clone( bool shallow )
{
  AGraph *g = 0;
  if( shallow )
    g = new AGraph( d->graph, fileName() );
  else
  {
    Graph *g3 = d->graph.get(), *g2 = new Graph( g3->getSyntax() );
    g2->extract( *g3, g3->begin(), g3->end() );
    g = new AGraph( g3, fileName() );
  }
  return g;
}


bool AGraph::render( PrimList &, const ViewState & )
{
  // do _nothing_ !
  return true;
}


namespace
{

  template<class T> static bool insertElem( Process & p, const string &,
                                            Finder & )
  {
    AimsGraphReader::PostProcessor & pp = (AimsGraphReader::PostProcessor &) p;
    AimsGraphReader::ElementInfo	& info = pp.elementInfo();
    rc_ptr<T>		aimso( ((AimsGraphReader::ObjectWrapper<T> *) 
			  info.object)->data );
    /* cout << "insertItem : " << info.attribute << ": "
    << DataTypeCode<T>::name() << endl; */
    info.element->setProperty( info.attribute, aimso );
    AObject	*obj = ObjectConverter<T>::aims2ana( aimso );
    if( !obj )
      {
        cerr << "could not build AObject from type " 
             << info.object->objectType() << " / " << info.object->dataType() 
             << endl;
        return false;
      }

    shared_ptr<AObject>	go1;
    AGraphObject	*go = 0;	// (init to avoid warning)
    AGraph		*ag = 0;
    info.graph->getProperty( "ana_object", go1 );
    ag = (AGraph *) go1.get();

    if( !info.element->getProperty( "ana_object", go1 ) 
        || !(go = dynamic_cast<AGraphObject *>( go1.get() ) ) )
    {
      // cout << "no AGraphObject\n";
      string	name = "Unnamed";
      go = new AGraphObject( info.element );
      info.element->getProperty( "name", name );
      go->setName( theAnatomist->makeObjectName( name ) );

      ag->insert( go, info.element );
      go->setReferentialInheritance( ag );
      theAnatomist->registerObject( go, false );
      theAnatomist->releaseObject( go );
      // cout << "AGraphObject created\n";
    }
    else if( ag && go->parents().find( ag ) == go->parents().end() )
      ag->insert( go, info.element );
    obj->setReferentialInheritance( ag );

    info.element->setProperty( info.attribute + "_ana", obj );

    map<AObject *, string>			objmap;
    info.element->getProperty( "objects_map", objmap );

    obj->setName( theAnatomist->makeObjectName( info.attribute ) );
    go->insert( obj );
    theAnatomist->registerObject( obj, false );
    theAnatomist->releaseObject( obj );

    string	id = info.attribute;
    if( id.find( "aims_" ) == 0 )
      id.erase( 0, 5 );
    objmap[ obj ] = id;
    info.element->setProperty( "objects_map", objmap );

    // cout << "insertElem done\n";
    return( true );
  }


  void updateBucket( GraphObject* ao, rc_ptr<GraphElementTable> mgec )
  {
    if( ao->hasProperty( "agraph" ) )
      ao->removeProperty( "agraph" );
    if( !mgec.get() )
      return;

    GraphElementTable::const_iterator	im = mgec->find( ao->getSyntax() );
    if( im != mgec->end() )
      {
        DataTypeCode<BucketMap<Void> >	dtc;
        map<string,GraphElementCode>::const_iterator 
          ig, eg = im->second.end();
        AObject	*ana = 0;
        Bucket	*bck;

        for( ig=im->second.begin(); ig!=eg; ++ig )
          if( ig->second.objectType == dtc.objectType() 
              && ig->second.dataType == dtc.dataType() 
              && ao->getProperty( ig->second.attribute + "_ana", ana ) )
            {
              bck = dynamic_cast<Bucket *>( ana );
              if( bck )
                bck->setGeomExtrema();
            }
      }
  }


  template <typename T>
  void convElement( GraphObject* go, AimsGraphReader::PostProcessor & pp,
                    const string & attribute )
  {
    typedef rc_ptr<T> graphPtr;
    T *pmesh2 = NULL;
    try
    {
      graphPtr rcptrMesh2;
      go->getProperty( attribute, rcptrMesh2 );

      if ( rcptrMesh2.count() != 2 )
        throw runtime_error( "Internal error while post-processing graph "
            "in Anatomist: object rc_ptr should be referenced only once" );
      go->removeProperty( attribute );
      pmesh2 = rcptrMesh2.release();
    }
    catch( exception & )
    {
      // cout << e.what() << endl;
    }
    if ( pmesh2 )
    {
      pp.elementInfo().element = go;
      pp.elementInfo().attribute = attribute;
      pp.elementInfo().object =
          new AimsGraphReader::ObjectWrapper<T>( pmesh2 );
      Finder f;
      insertElem<T>( pp, "", f );
    }
  }


  template <typename T>
  void reconvElement( GraphObject* go, AimsGraphReader::PostProcessor & pp,
                      const string & attribute )
  {
    shared_ptr<AObject> ago;
    if( go->getProperty( "ana_object", ago ) )
    {
      if( !ago.get() || !theAnatomist->hasObject( ago.get() )
        || static_cast<AGraphObject *>( ago.get() )->attributed() != go )
      {
        cout << "remove obsolete ana_object\n";
        go->removeProperty( "ana_object" );
        ago.reset( shared_ptr<AObject>::Strong, 0 );
      }
    }

    typedef rc_ptr<T> elemPtr;
    elemPtr raims;
    AObject *obj = 0;
    go->getProperty( attribute, raims );
    if( raims )
    {
      if( !go->getProperty( attribute + "_ana", obj ) || !obj
        || !theAnatomist->hasObject( obj ) )
      {
        // no anatomist element yet: create it
        pp.elementInfo().element = go;
        pp.elementInfo().attribute = attribute;
        pp.elementInfo().object =
            new AimsGraphReader::ObjectWrapper<T>( raims.get() );
        Finder f;
        insertElem<T>( pp, "", f );
      }
      else
      {
        // check if aims object has changed
        rc_ptr<T> aims = ObjectConverter<T>::ana2aims( obj );
        if( aims != raims )
        {
          ObjectConverter<T>::setAims( obj, raims );
        }
      }
    }
    else if( go->getProperty( attribute + "_ana", obj ) )
    {
      if( obj && theAnatomist->hasObject( obj ) )
      {
        // cout << "obsolete AObject " << obj->name() << endl;
        if( ago.get() )
        {
          MObject *mo = dynamic_cast<MObject *>( ago.get() );
          if( mo )
          {
            cout << "remove obsolete graph sub-object " << obj->name()
                << " (" << obj->objectFullTypeName() << ")" << endl;
            mo->erase( obj );
            if( mo->size() == 0 )
            {
              cout << "remove empty AGraphObject.\n";
              go->removeProperty( "ana_object" );
              AObject * agr = 0;
              pp.elementInfo().graph->getProperty( "ana_object", agr );
              AGraph *ag = static_cast<AGraph *>( agr );
              AGraph::iterator ia = ag->find( mo );
              ag->erase( ia );
            }
          }
        }
        // else cout << "was not in a AGraphObject\n";
      }
      /* else
        cout << "obsolete [deleted] object " << obj << endl; */
      go->removeProperty( attribute + "_ana" );
    }
  }


  void rescan_element( AGraph* ag, GraphObject* go,
                       AimsGraphReader::PostProcessor & pp )
  {
    rc_ptr<GraphElementTable>   get;
    Graph *g = ag->graph();
    if( !g->getProperty( "aims_objects_table", get ) || !get )
      return; // no table: do nothing
    GraphElementTable::const_iterator iget = get->find( go->getSyntax() );
    if( iget == get->end() )
      return;
    map<string, GraphElementCode>::const_iterator
        igec, egec = iget->second.end();
    string  attribute;

    for( igec=iget->second.begin(); igec!=egec; ++igec )
    {
      attribute = igec->second.attribute;
      // cout << "rescan_element, attribute: " << attribute << endl;
      const GraphElementCode & gec = igec->second;
      if( gec.objectType
          == DataTypeCode<AimsTimeSurface<2,Void> >::objectType()
          && gec.dataType
          == DataTypeCode<AimsTimeSurface<2,Void> >::dataType() )
        reconvElement<AimsTimeSurface<2,Void> >( go, pp, attribute );
      else if( gec.objectType
               == DataTypeCode<AimsTimeSurface<3,Void> >::objectType()
               && gec.dataType
               == DataTypeCode<AimsTimeSurface<3,Void> >::dataType() )
        reconvElement<AimsTimeSurface<3,Void> >( go, pp, attribute );
      else if( gec.objectType
               == DataTypeCode<BucketMap<Void> >::objectType()
               && gec.dataType
               == DataTypeCode<BucketMap<Void> >::dataType() )
        reconvElement<BucketMap<Void> >( go, pp, attribute );
    }
  }


  void files2obj_element( AGraph* ag, AimsGraphReader & /*agr*/,
                          GraphObject* go,
                          AimsGraphReader::PostProcessor & pp )
  {
    rc_ptr<GraphElementTable>   get;
    Graph *g = ag->graph();
    if( !g->getProperty( "aims_objects_table", get ) || !get )
      return; // no table: do nothing
    GraphElementTable::const_iterator iget = get->find( go->getSyntax() );
    if( iget == get->end() )
      return;
    map<string, GraphElementCode>::const_iterator
        igec, egec = iget->second.end();
    string  attribute;

    for( igec=iget->second.begin(); igec!=egec; ++igec )
    {
      attribute = igec->second.attribute;
      // cout << "files2obj_element, attribute: " << attribute << endl;
      const GraphElementCode & gec = igec->second;
      if( gec.objectType
          == DataTypeCode<AimsTimeSurface<2,Void> >::objectType()
          && gec.dataType
          == DataTypeCode<AimsTimeSurface<2,Void> >::dataType() )
        convElement<AimsTimeSurface<2,Void> >( go, pp, attribute );
      else if( gec.objectType
               == DataTypeCode<AimsTimeSurface<3,Void> >::objectType()
               && gec.dataType
               == DataTypeCode<AimsTimeSurface<3,Void> >::dataType() )
        convElement<AimsTimeSurface<3,Void> >( go, pp, attribute );
      else if( gec.objectType
               == DataTypeCode<BucketMap<Void> >::objectType()
               && gec.dataType
               == DataTypeCode<BucketMap<Void> >::dataType() )
        convElement<BucketMap<Void> >( go, pp, attribute );
    }
  }


  void files2Obj( AGraph* ag, int subobj )
  {
    // cout << "Entering file2Obj " << endl ;

    vector<float>	vs;
    Graph		*g = ag->graph();
    if( g->getProperty( "voxel_size", vs ) && vs.size() >= 3 )
      ag->setVoxelSize( Point3df( vs[0], vs[1], vs[2] ) );
    else
      ag->setVoxelSize( Point3df( 1., 1., 1. ) );

    g->setProperty( "ana_object",
                    shared_ptr<AObject>( shared_ptr<AObject>::Weak, ag ) );

    /* if there are volumes in the graph, load all (nodes + relations) to
       enable conversion to buckets */
    if( g->hasProperty( "type.global.vol") )
    {
      int loaded = 0;
      g->getProperty( "aims_reader_loaded_objects", loaded );
      subobj |= (~loaded & 3);
    }

    const set<Edge *>           & edges = g->edges();
    set<Edge *>::const_iterator ie, ee = edges.end();
    try
      {
        // cout << "read " << ag->fileName() << endl;
        AimsGraphReader	agr( ag->fileName() );
        agr.readElements( *g, subobj );
        // cout << "graph elements read.\n";

        // transform to buckets
        GraphManip::volume2Buckets( *g );

        AimsGraphReader::PostProcessor pp;
        pp.elementInfo().graph = g;
        if( subobj & 1 )
        {
          // Change Aims attributes into Anatomist objects
          for( Graph::iterator itNode = g->begin();
                itNode != g->end(); ++itNode )
            files2obj_element( ag, agr, *itNode, pp );
        }
        if( subobj & 2 )
        {
          for( ie=edges.begin(); ie!=ee; ++ie )
            files2obj_element( ag, agr, *ie, pp );
        }
      }
    catch( exception & e )
      {
        cerr << e.what() << endl;
      }

    // provide access to Graph from GraphObjects
    Graph::iterator		iv, ev = g->end();
    if( subobj & 1 )
    {
      for( iv=g->begin(); iv!=ev; ++iv )
      {
        if( (*iv)->hasProperty( "ana_object" ) )
          (*iv)->setProperty( "agraph", ag );
      }
    }
    if( subobj & 2 )
    {
      for( ie=edges.begin(); ie!=ee; ++ie )
        if( (*ie)->hasProperty( "ana_object" ) )
          (*ie)->setProperty( "agraph", ag );
    }

    // cleanup
    rc_ptr<GraphElementTable>	mgec;
    g->getProperty( "aims_objects_table", mgec );
    if( subobj & 1 )
      for( iv=g->begin(); iv!=ev; ++iv )
        updateBucket( *iv, mgec );
    // cout << "updating buckets\n";
    if( subobj & 2 )
      for( ie=edges.begin(); ie!=ee; ++ie )
        updateBucket( *ie, mgec );

    // cout << "OK, setting extrema\n";
    ag->setGeomExtrema();
    ag->setChanged();
  }

}


const rc_ptr<map<string, vector<int> > > AGraph::objAttColors() const
{
  rc_ptr<map<string, vector<int> > >	cols;
  if( !d->graph->getProperty( "object_attributes_colors", cols ) )
    cols = rc_ptr<map<string, vector<int> > >( new map<string, vector<int> > );
  return cols;
}


rc_ptr<map<string, vector<int> > > AGraph::objAttColors()
{
  rc_ptr<map<string, vector<int> > >	cols;
  if( !d->graph->getProperty( "object_attributes_colors", cols ) )
    {
      cols 
	= rc_ptr<map<string, vector<int> > >( new map<string, vector<int> > );
      d->graph->setProperty( "object_attributes_colors", cols );
    }
  return cols;
}


AObject* AGraph::ObjectAt( float x, float y, float z, float t, float tol )
{
  //cout << "AGraph::ObjectAt( " << x << ", " << y << ", " << z << " )\n";
  float		rx, ry, rz;
  float		mx = d->minX, my = d->minY, mz = d->minZ;
  float		Mx = d->maxX, My = d->maxY, Mz = d->maxZ;

  AObject	*obj;

  if( tol < 0 ) tol *= -1;
  rx = ((float) d->labelDim[ 0 ]) / (Mx - mx + 1);
  ry = ((float) d->labelDim[ 1 ]) / (My - my + 1);
  rz = ((float) d->labelDim[ 2 ]) / (Mz - mz + 1);

  int	a = (int) ( (x/d->voxelSize.item( 0 ) - mx) * rx + 0.5 );
  int	b = (int) ( (y/d->voxelSize.item( 1 ) - my) * ry + 0.5 );
  int	c = (int) ( (z/d->voxelSize.item( 2 ) - mz) * rz + 0.5 );
  int	dd = (int) t;

  AimsData<AObject *>	*pvol;

  /*cout << "coord ds vol label : " << a << ", " << b << ", " << c << endl;
  cout << "mins : " << mx << ", " << my << ", " << mz << endl;
  cout << "maxs : " << Mx << ", " << My << ", " << Mz << endl;
  cout << "r    : " << rx << ", " << ry << ", " << rz << endl;*/

  if( dd < MinT() || dd > MaxT() )
    return( 0 );	// time does not match

  if( !d->labelsVol )
    {
      cout << "creating volume of labels\n";
      d->labelsVol = new map<int, AimsData<AObject *> >;
      pvol = &( (*d->labelsVol)[ dd ] 
         	= AimsData<AObject *>( d->labelDim[ 0 ],
				       d->labelDim[ 1 ],
				       d->labelDim[ 2 ] ) );
      fillVol( *pvol, dd, mx, my, mz, Mx, My, Mz );
    }
  else if( d->labelsVol->find( dd ) == d->labelsVol->end() )
    {
      pvol = &( (*d->labelsVol)[ dd ] 
		= AimsData<AObject *>( d->labelDim[ 0 ], d->labelDim[ 1 ], 
				       d->labelDim[ 2 ] ) );
      fillVol( *pvol, dd , mx, my, mz, Mx, My, Mz);
    }
  else
    pvol = &(*d->labelsVol)[ dd ];

  if( a >= 0 && b >= 0 && c >= 0 && a < (int) d->labelDim[ 0 ] &&
                                    b < (int) d->labelDim[ 1 ] &&
                                    c < (int) d->labelDim[ 2 ] )
    {
      obj = (*pvol)( a, b, c );
      if( obj )
	return( obj );
    }
  /*else
    cout << "out of bounds\n";*/

  if( tol == 0 )
    return( 0 );

  // if not found at once, check in the neighbourhood
  int	sa = (int) ( ((x+tol)/d->voxelSize[0] - mx) * rx + 0.5 );
  int	sb = (int) ( ((y+tol)/d->voxelSize[1] - my) * ry + 0.5 );
  int	sc = (int) ( ((z+tol)/d->voxelSize[2] - mz) * rz + 0.5 );
  int	ea = (int) ( ((x-tol)/d->voxelSize[0] - mx) * rx);
  int	eb = (int) ( ((y-tol)/d->voxelSize[1] - my) * ry);
  int	ec = (int) ( ((z-tol)/d->voxelSize[2] - mz) * rz);
  int	xx, yy, zz;

  if( sa >= (int) d->labelDim[ 0 ] ) sa = d->labelDim[ 0 ] - 1;
  if( sb >= (int) d->labelDim[ 1 ] ) sb = d->labelDim[ 1 ] - 1;
  if( sc >= (int) d->labelDim[ 2 ] ) sc = d->labelDim[ 2 ] - 1;
  if( ea < 0 ) ea = 0;
  if( eb < 0 ) eb = 0;
  if( ec < 0 ) ec = 0;

  /*cout << "search cube : " << ea << " <= x < " << sa << endl;
  cout << "              " << eb << " <= y < " << sb << endl;
  cout << "              " << ec << " <= z < " << sc << endl;*/

  map<AObject *, unsigned>	counts;
  map<AObject *, unsigned>::const_iterator	ic, fc=counts.end();
  unsigned			maxc = 0;

  //	search in the cube
  for( zz=ec; zz<=sc; ++zz )
    for( yy=eb; yy<=sb; ++yy )
      for( xx=ea; xx<=sa; ++xx )
	{
	  obj = (*pvol)( xx, yy, zz );
	  if( obj )
	    ++counts[ obj ];
	}
  if( counts.empty() )
  return( 0 );	// really not found...

  //	find obj with max count
  obj = 0;
  for( ic=counts.begin(); ic!=fc; ++ic )
    if( (*ic).second > maxc )
      {
        maxc = (*ic).second;
	obj = (*ic).first;
      }
  assert( obj );
  return( obj );
}


//

AObject* AGraph::LoadGraph( const char* filename )
{
  AGraph  *ag = 0;
  Graph   *gr = 0;

  try
    {
      int mask = 0;
      if( GraphParams::graphParams()->loadRelations )
        mask = -1;
      Reader<Graph>	grd( filename );
      gr = grd.read( 0, 0, mask );

      // cout << "read graph OK\n";
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }

  if( gr )
    ag = new AGraph( gr, filename );

  return ag;
}


void AGraph::fillVol( AimsData<AObject *> & vol, int t, float mx, float my, 
		      float mz, float Mx, float My, float Mz )
{
  /*cout << "Filling volume of labels ( " << vol.dimX() << ", " 
    << vol.dimY() << ", " << vol.dimZ() << " )..." << flush;*/
  const_iterator			io, fo = end();
  AGraphObject				*go;
  AGraphObject::const_iterator		is, fs;
  Bucket				*bck;
  BucketMap<Void>::Bucket::const_iterator	ib, fb;
  float					rx, ry, rz;
  AimsData<AObject*>::iterator		iv, fv=vol.end();

  rx = ((float) d->labelDim[ 0 ]) / (Mx - mx + 1);
  ry = ((float) d->labelDim[ 1 ]) / (My - my + 1);
  rz = ((float) d->labelDim[ 2 ]) / (Mz - mz + 1);

  //cout << "rx : " << rx << ", ry : " << ry << ", rz : " << rz << endl;

  //float	xx, yy, zz;

  //	vide le volume
  for( iv=vol.begin(); iv!=fv; ++iv )
    (*iv) = 0;
  /* for( unsigned iz=0; iz<d->labelDim[ 2 ]; ++iz )
    for( unsigned iy=0; iy<d->labelDim[ 1 ]; ++iy )
      for( unsigned ix=0; ix<d->labelDim[ 0 ]; ++ix )
        vol( ix, iy, iz ) = 0;
  cout << &vol( 0, 0, 0 ) << " , " 
       << &vol( d->labelDim[ 0 ]-1, d->labelDim[ 1 ]-1, d->labelDim[ 2 ]-1 ) << endl;
  cout << vol.begin() << " , " << vol.end() << endl;
  cout << "dims vol : " << vol.dimX() << " X " << vol.dimY() << " X " 
       << vol.dimZ() << " X " << vol.dimT() << "; bord : " 
       << vol.borderWidth() << endl;
  cout << "volume vid�. " << flush; */

  Point3d	l;

  for( io = begin(); io!=fo; ++io )
    {
      go = dynamic_cast<AGraphObject *>( *io );
      assert( go );
      for( is=go->begin(), fs=go->end(); is!=fs; ++is )
	{
	  bck = dynamic_cast<Bucket *>( *is );
	  if( bck )
	    {
	      BucketMap<Void>::const_iterator ibi 
		= bck->bucket().find( t );

	      if( ibi != bck->bucket().end() )
		for( ib=(*ibi).second.begin(), fb=(*ibi).second.end(); 
		     ib!=fb; ++ib )
		  {
		    const Point3d & loc = ib->first;
		    /*xx = (unsigned) ( (loc[0]-mx) * rx);
		    yy = (unsigned) ( (loc[1]-my) * ry);
		    zz = (unsigned) ( (loc[2]-mz) * rz);
		    cout << loc[0] << "->" << xx << " , " << loc[1] << "->" 
		    << yy << " , " << loc[2] << "->" << zz << endl;*/
		    l[0] = (short) ( (loc[0]-mx) * rx);
		    l[1] = (short) ( (loc[1]-my) * ry);
		    l[2] = (short) ( (loc[2]-mz) * rz);
		    if( l[0] < 0 || l[1] < 0 || l[2] < 0 
			|| l[0] >= d->labelDim[0] || l[1] >= d->labelDim[1] 
			|| l[2] >= d->labelDim[2] )
		      {
			string name, label;
			go->attributed()->getProperty( "name", name );
			go->attributed()->getProperty( "label", label );
			cerr << "Bucket outside bounding box: node name: " 
			     << name << ", label: " << label << ", bucket: "
			     << (*is)->name() << " loc: " << loc << endl;
		      }
		    else
		      vol( l ) = go;
		  }
	    }
	}
    }
  //cout << " OK\n";
}


void AGraph::loadSubObjects( int mask )
{
  Graph         &g = *graph();
  int           loaded = 0;

  g.getProperty( "aims_reader_loaded_objects", loaded );
  if( ( loaded & mask ) != mask )
  {
    if( !g.hasProperty( "aims_reader_filename" ) )
      g.setProperty( "aims_reader_filename", fileName() );
    files2Obj( this, (~loaded) & mask );
  }
}


bool AGraph::save( const string & filename )
{
  Graph		&g = *graph();
  bool		loadrels = false;
  string	reader;
  int		loaded = 0;

  bool l = g.getProperty( "aims_reader_loaded_objects", loaded );
  if( l && loaded != 3 )
    {
      if( !g.getProperty( "aims_reader_filename", reader ) )
        reader = fileName();
      cout << "Graph " << reader << " has not been loaded entirely. " 
           << "Loading missing elements...\n";
      // cout << reader << endl;
      files2Obj( this, (~loaded) & 3 );
      loadrels = true;
    }

  bool	filechanged = fileName() != filename;
  setFileName( filename );

  if( filechanged )
    g.setProperty( "filename_base", string( "*" ) );

  // using the generic Writer does not allow to save only changed objects
  // but allows to save model graphs, and use the correct syntax check set
  Writer<Graph> gw( FileUtil::removeExtension( filename ) + ".arg" );
  gw.write( *d->graph );

  /*
  //	save sub-objects if needed
  try
    {
      saveSubObjects( filechanged );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      // try to save the structure anyway before throwing exception
      GraphWriter	gw( FileUtil::removeExtension( filename ) + ".arg",
                            SyntaxRepository::exportedSyntax() );
      gw << *d->graph;
      throw;
    }

  GraphWriter	gw( FileUtil::removeExtension( filename ) + ".arg", 
                    SyntaxRepository::exportedSyntax() );
  gw << *d->graph;
  */

  if( loadrels )
    notifyObservers( this );

  return true;
}


template<class T> static bool extractElem( Process & p, const string &, 
					   Finder & )
{
  AimsGraphWriter		& pp = (AimsGraphWriter &) p;
  AimsGraphWriter::ElementInfo	& info = pp.elementInfo();
  shared_ptr<AObject>	go1;
  AGraphObject	*go;

  if( !info.element->getProperty( "ana_object", go1 ) 
      || !(go = dynamic_cast<AGraphObject *>( go1.get() ) ) )
    return( false );	// AGraphObject not found

  // retreive correct AObject in graph object container
  // and Aims object in AObject

  AObject			*obj;
  rc_ptr<T>	aimso;
  if( info.element->getProperty( info.attribute + "_ana", obj ) )
    aimso = ObjectConverter<T>::ana2aims( obj );
  else
  {
    rc_ptr<T> raimso;
    if( !info.element->getProperty( info.attribute, raimso ) )
      return false;
    aimso = raimso;
  }

  if( !aimso )
    return( false );

  info.object = new AimsGraphWriter::ObjectWrapper<T>( aimso.get() );

  return( true );
}


void AGraph::saveSubObjects( bool filechanged )
{
  AimsGraphWriter	aw( fileName() );
  aw.registerProcessType( "Segments", "VOID",
                          &extractElem<AimsTimeSurface<2, Void> > );
  aw.registerProcessType( "Mesh", "VOID", &extractElem<AimsSurfaceTriangle> );
  aw.registerProcessType( "Bucket", "VOID", &extractElem<BucketMap<Void> > );

  if( QGraphParam::autoSaveDirectory() )
    {
      string	dname 
        = FileUtil::removeExtension( FileUtil::basename( fileName() ) ) 
        + ".data";
      if( !filechanged )
        {
          string	fnb;
          graph()->getProperty( "filename_base", fnb );
          if( fnb != dname && fnb != "*" )
            filechanged = true;
        }
      graph()->setProperty( "filename_base", dname );
    }

  bool	savemod = QGraphParam::saveOnlyModified();
  if( filechanged )
    savemod = false;
  aw.writeElements( *d->graph, (AimsGraphWriter::SavingMode)
                    QGraphParam::savingMode(), 
                    (AimsGraphWriter::SavingMode) 
                    QGraphParam::savingMode(), 
                    savemod );
}


void AGraph::SetMaterial( const Material & mat )
{
  // cout << "AGraph::SetMaterial, this: " << this << endl;
  AObject::SetMaterial( mat );

  iterator	io, fo=end();
  Material	mat2 = mat;
  bool		changed = false;
  AGraphObject	*go;
  GraphParams	*gp = GraphParams::graphParams();

  gp->allowRescanHierarchies( true );
  for( io=begin(); io!=fo; ++io )
  {
    go = dynamic_cast<AGraphObject *>( *io );
    if( go )
    {
      if( specialColorFunc )
      {
        changed = specialColorFunc( this, go, mat2 );
      }
      if( changed )
      {
        go->SetMaterial( mat2 );
        mat2 = mat;
      }
      else
        go->SetMaterialOrDefault( this, mat2 );
    }
    gp->allowRescanHierarchies( false );
  }

  const set<Edge *> & edges = graph()->edges();
  set<Edge *>::const_iterator ie, ee = edges.end();
  shared_ptr<AObject> ao;

  for( ie=edges.begin(); ie!=ee; ++ie )
    if( (*ie)->getProperty( "ana_object", ao ) )
    {
      go = dynamic_cast<AGraphObject *>( ao.get() );
      if( go )
      {
        if( specialColorFunc )
        {
          changed = specialColorFunc( this, go, mat2 );
        }
        if( changed )
        {
          go->SetMaterial( mat2 );
          mat2 = mat;
        }
        else
          go->SetMaterialOrDefault( this, mat2 );
      }
      gp->allowRescanHierarchies( false );
    }

  gp->allowRescanHierarchies( true );
  setChanged();
  // cout << "AGraph::SetMaterial done: " << endl;
}


void AGraph::setVoxelSize( const Point3df & vs )
{
  d->voxelSize = vs;
  vector<float>	vvs;

  vvs.push_back( vs[0] );
  vvs.push_back( vs[1] );
  vvs.push_back( vs[2] );

  d->graph->setProperty( "voxel_size", vvs );
}


void AGraph::setGeomExtrema()
{
  vector<int>	bmin, bmax;
  float		minx = d->minX, miny = d->minY, minz = d->minZ;
  float		maxx = d->maxX, maxy = d->maxY, maxz = d->maxZ;

  if( d->graph->getProperty( "boundingbox_min", bmin ) && bmin.size() >= 3
      && d->graph->getProperty( "boundingbox_max", bmax ) && bmax.size() >= 3 )
    {
      d->minX = bmin[0];
      d->minY = bmin[1];
      d->minZ = bmin[2];
      d->maxX = bmax[0];
      d->maxY = bmax[1];
      d->maxZ = bmax[2];
      cout << "bounding box found : " << d->minX << ", " << d->minY << ", " 
	   << d->minZ << endl;
      cout << "                     " << d->maxX << ", " << d->maxY << ", " 
	   << d->maxZ << endl;
    }
  else
    {
      Point3df	pmin, pmax, vs = VoxelSize();
      if( MObject::boundingBox( pmin, pmax ) )
	{
	  d->minX = pmin[0] / vs[0];
	  d->minY = pmin[1] / vs[1];
	  d->minZ = pmin[2] / vs[2];
	  d->maxX = pmax[0] / vs[0];
	  d->maxY = pmax[1] / vs[1];
	  d->maxZ = pmax[2] / vs[2];
	  cout << "bounding box created : " << d->minX << ", " << d->minY << ", " 
	       << d->minZ << endl;
	  cout << "                       " << d->maxX << ", " << d->maxY << ", " 
	       << d->maxZ << endl;
	}
      else
	{
	  cout << "can't determine a bounding box - taking 'standard' values" 
	       << endl;
	  d->minX = 0;
	  d->minY = 0;
	  d->minZ = 0;
	  d->maxX = 255;
	  d->maxY = 255;
	  d->maxZ = 255;
	}
    }

  if( d->labelsVol && ( minx != d->minX || miny != d->minY || minz != d->minZ 
                      || maxx != d->maxX || maxy != d->maxY || maxz != d->maxZ
                      ) )
    clearLabelsVolume();
}


void AGraph::clearLabelsVolume()
{
  if( d->labelsVol )
    d->labelsVol->clear();
}


void AGraph::setLabelsVolumeDimension( unsigned dx, unsigned dy, unsigned dz )
{
  setLabelsVolumeDimension( Point3d( dx, dy, dz ) );
}


void AGraph::setLabelsVolumeDimension( const Point3d & vd )
{
  if ( vd == d->labelDim )
    return ;

  d->labelDim = vd;

  if( d->labelsVol )
    clearLabelsVolume() ;
}


Point3d AGraph::labelsVolumeDimension() const
{
  return( d->labelDim );
}


bool AGraph::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  bmin = Point3df( d->minX * d->voxelSize.item( 0 ), d->minY * d->voxelSize.item( 1 ), 
		   d->minZ * d->voxelSize.item( 2 ) );
  bmax = Point3df( d->maxX * d->voxelSize.item( 0 ), d->maxY * d->voxelSize.item( 1 ), 
		   d->maxZ * d->voxelSize.item( 2 ) );
  return( true );
}


AimsData<AObject *>& 
AGraph::volumeOfLabels( int t )
{
  if( d->labelsVol == 0 )
    d->labelsVol = new map<int, AimsData<AObject *> >;

  if( t > MaxT() )
    t = (int) MaxT();

  map<int, AimsData<AObject *> >::iterator found( d->labelsVol->find( t ) );
  if (found == d->labelsVol->end() )
    {
      cout << "Label Volume Dimension : "  << d->labelDim[0] << ", " 
           << d->labelDim[1] << ", " << d->labelDim[2] << endl ;
      AimsData<AObject *>	& vol = (*d->labelsVol)[t];
      vol = AimsData<AObject * >( d->labelDim[0], d->labelDim[1], 
                                  d->labelDim[2] );
      float	mx = d->minX, my = d->minY, mz = d->minZ;
      float	Mx = d->maxX, My = d->maxY, Mz = d->maxZ;
      fillVol( vol, t, mx, my, mz, Mx, My, Mz );
    }
  return (*d->labelsVol)[t] ;
}


Graph* AGraph::graph() const
{
  return d->graph.get();
}


void AGraph::setGraph( rc_ptr<Graph> g )
{
  d->graph = g;
  int mask = 3, loaded = 0;
  g->getProperty( "aims_reader_loaded_objects", loaded );
  if( ( loaded & mask ) != mask )
  {
    if( !g->hasProperty( "aims_reader_filename" ) )
      g->setProperty( "aims_reader_filename", fileName() );
    files2Obj( this, (~loaded) & mask );
  }
  setInternalsChanged();
}


float AGraph::MinX2D() const
{
  return d->minX;
}


float AGraph::MinY2D() const
{
  return d->minY;
}


float AGraph::MinZ2D() const
{
  return d->minZ;
}


float AGraph::MaxX2D() const
{
  return d->maxX;
}


float AGraph::MaxY2D() const
{
  return d->maxY;
}


float AGraph::MaxZ2D() const
{
  return d->maxZ;
}


GenericObject* AGraph::attributed()
{
  return d->graph.get();
}


const GenericObject* AGraph::attributed() const
{
  return d->graph.get();
}


Point3df  AGraph::VoxelSize() const
{
  return d->voxelSize;
}


void AGraph::createDefaultPalette( const string & name  )
{
  if( name.empty() )
    MObject::createDefaultPalette( "Blue-Red" );
  else
    MObject::createDefaultPalette( name );
}


GLComponent* AGraph::glAPI()
{
  return this;
}


const GLComponent* AGraph::glAPI() const
{
  return this;
}


const AObjectPalette* AGraph::glPalette( unsigned ) const
{
  return palette();
}


AGraph::ColorMode AGraph::colorMode() const
{
  return d->colormode;
}


void AGraph::setColorMode( ColorMode m, bool update )
{
  if( d->colormode != m )
    {
      d->colormode = m;
      if( update )
      {
        if( m != Normal )
          updateExtrema();
        recolor();
      }
    }
}


int AGraph::colorPropertyMask() const
{
  return d->colorpropmask;
}


void AGraph::setColorPropertyMask( int x, bool update )
{
  if( d->colorpropmask != x )
  {
    d->colorpropmask = x;
    if( update )
    {
      updateExtrema();
      recolor();
    }
  }
}


void AGraph::updateColors(void)
{
    updateExtrema();
    recolor();
}




namespace
{

  bool graphObjectValue( const GenericObject & o, const string & prop, 
                         double & val )
  {
    Object	p;
    try
      {
        if( !(p = o.getProperty( prop ) ) )
          return false;
        if( !p.isNull() )
          {
            val = p->getScalar();
            return true;
          }
      }
    catch( ... )
      {
      }
    return false;
  }


  void recolorElement( AObject* ao, GenericObject* gen, const string & prop,
                       float fmin, float fmax, float scale, unsigned ncol,
                       const AimsData<AimsRGBA> & cols, float galpha )
  {
    double  val;
    int     ind;
    if( graphObjectValue( *gen, prop, val ) )
    {
      // change material according to palette
      if( val <= std::min( fmin, fmax ) )
        ind = 0;
      else if( val >= std::max( fmin, fmax ) )
        ind = ncol - 1;
      else
        ind = (int) ( ( val - fmin ) * scale );
      //cout << "val : " << val << ", ind : " << ind << endl;
      Material & mat = ao->GetMaterial();
      /* if( _val0Col && val == 0 )
      mat.SetDiffuse( _p0red, _p0green, _p0blue, mat.Diffuse( 3 ) );
      else
      { */

      const AimsRGBA        & rgb = cols( ind );
      mat.SetDiffuse( (float) rgb.red()   / 255,
                       (float) rgb.green() / 255,
                       (float) rgb.blue()  / 255,
                       galpha * (float) rgb.alpha()  / 255 );
      //}
      ao->SetMaterial( mat );
    }
    else
    {
      // no value: show in black (error code for dead items)
      Material & mat = ao->GetMaterial();
      mat.SetDiffuse( 0, 0, 0, //_novalRed, _novalGreen, _novalBlue,
                      mat.Diffuse( 3 ) );
      ao->SetMaterial( mat );
    }
  }


  void recolorProperty( AGraph & g, const string & prop )
  {
    Graph               *graph = g.graph();
    Graph::iterator	i, e = graph->end();
    Object		p;
    double		minval = numeric_limits<double>::max(),
      maxval = -numeric_limits<double>::max();
    g.getOrCreatePalette();
    AObjectPalette	*objpal = g.palette();
    AimsData<AimsRGBA>	*cols = objpal->colors();
    float		cmin = objpal->min1(), cmax = objpal->max1();
    float		scale, fmin, fmax;
    unsigned		ncol = cols->dimX();
    shared_ptr<AObject>	ao;
    GLComponent::TexExtrema & ex = g.glTexExtrema();

    minval = ex.minquant[0];
    maxval = ex.maxquant[0];

    if( cmin == cmax )
      {
        cmin = 0;
        cmax = 1;
      }

    if( minval == maxval )
      scale = 1.;
    else
      scale = ((float) ncol) / ( ( cmax - cmin ) * ( maxval - minval ) );
    fmin = minval + cmin * ( maxval - minval );
    fmax = minval + cmax * ( maxval - minval );
    float galpha = g.GetMaterial().Diffuse(3);

    bool  colelem = g.colorPropertyMask() & 1;
    for( i=graph->begin(); i!=e; ++i )
      if( (*i)->getProperty( "ana_object", ao ) )
      {
        if( colelem ) // node
          recolorElement( ao.get(), *i, prop, fmin, fmax, scale, ncol, *cols,
                          galpha );
        else
        {
          // no value: show in black (error code for dead items)
          Material & mat = ao->GetMaterial();
          mat.SetDiffuse( 0, 0, 0, //_novalRed, _novalGreen, _novalBlue,
                          mat.Diffuse( 3 ) );
          ao->SetMaterial( mat );
        }
      }

    colelem = g.colorPropertyMask() & 2;
    const set<Edge *> & edges = graph->edges();
    set<Edge *>::const_iterator ie, ee = edges.end();
    for( ie=edges.begin(); ie!=ee; ++ie )
      if( (*ie)->getProperty( "ana_object", ao ) )
      {
        if( colelem ) // relation
          recolorElement( ao.get(), *ie, prop, fmin, fmax, scale, ncol, *cols,
                          galpha );
        else
        {
          // no value: show in black (error code for dead items)
          Material & mat = ao->GetMaterial();
          mat.SetDiffuse( 0, 0, 0, //_novalRed, _novalGreen, _novalBlue,
                          mat.Diffuse( 3 ) );
          ao->SetMaterial( mat );
        }
      }
    g.setChanged();
  }

}


void AGraph::recolor()
{
  if( d->recolorrecurs )
    return;
  d->recolorrecurs = true;

  switch( d->colormode )
    {
    case Normal:
      SetMaterial( GetMaterial() );
      break;
    case PropertyMap:
      recolorProperty( *this, d->colorproperty );
      break;
    default:
      cerr << "unknown color mode for graph\n";
      break;
    }

  d->recolorrecurs = false;
}


void AGraph::setPalette( const AObjectPalette & pal )
{
  if( palette() )
    {
      MObject::setPalette( pal );
      recolor();
    }
  else
    MObject::setPalette( pal );
}


string AGraph::colorProperty() const
{
  return d->colorproperty;
}


void AGraph::updateExtrema()
{
  iterator		i, e = end();
  double		val;
  AttributedAObject	*aao;
  double		minval = numeric_limits<double>::max(), 
    maxval = -numeric_limits<double>::max();

  if( d->colorpropmask & 1 )
    for( i=begin(); i!=e; ++i )
      if( ( aao = dynamic_cast<AttributedAObject *>( *i ) ) 
          && aao->attributed()->hasProperty( "ana_object" ) 
          && graphObjectValue( *aao->attributed(), d->colorproperty, val ) )
        {
          if( val > maxval )
            maxval = val;
          if( val < minval )
            minval = val;
        }
  if( d->colorpropmask & 2 )
  {
    const set<Edge *>  & edges = graph()->edges();
    set<Edge *>::const_iterator ie, ee = edges.end();
    for( ie=edges.begin(); ie!=ee; ++ie )
      if( (*ie)->hasProperty( "ana_object" )
          && graphObjectValue( **ie, d->colorproperty, val ) )
    {
      if( val > maxval )
        maxval = val;
      if( val < minval )
        minval = val;
    }
  }
  if( minval >= maxval )
    {
      minval = 0;
      maxval = 1.;
    }

  GLComponent::TexExtrema & ex = glTexExtrema();
  ex.minquant[0] = (float) minval;
  ex.maxquant[0] = (float) maxval;
}


void AGraph::setColorProperty( const string & prop, bool update )
{
  if( prop != d->colorproperty )
    {
      d->colorproperty = prop;
      if( update )
      {
        updateExtrema();
        recolor();
      }
    }
}

//

size_t AGraph::size() const
{
  return( d->data.size() );
}


MObject::iterator AGraph::begin()
{
  return iterator( new AGraphIterator( d->data.begin() ) );
}


MObject::const_iterator AGraph::begin() const
{
  return const_iterator( new const_AGraphIterator( d->data.begin() ) );
}


MObject::iterator AGraph::end()
{
  return( iterator( new AGraphIterator( d->data.end() ) ) );
}


MObject::const_iterator AGraph::end() const
{
  return( const_iterator( new const_AGraphIterator( d->data.end() ) ) );
}


void AGraph::insert( AObject* obj )
{
  insert( obj, "fold" );
}


void AGraph::insert( AObject* obj, std::string syntacticAtt )
{
  Vertex	*vert = d->graph->addVertex( syntacticAtt );
  d->data.insert( carto::shared_ptr<AObject>(
                  carto::shared_ptr<AObject>::WeakShared, obj ) );
  vert->setProperty( "ana_object", carto::shared_ptr<AObject>(
                     carto::shared_ptr<AObject>::Weak, obj ) );
  obj->RegisterParent( this );
  obj->addObserver( this );
  _insertObject( obj );
  _contentHasChanged = true;
  setChanged();
}


void AGraph::insert( AObject* obj, GenericObject* vert )
{
  d->data.insert( carto::shared_ptr<AObject>(
                  carto::shared_ptr<AObject>::WeakShared, obj ) );
  vert->setProperty( "ana_object", carto::shared_ptr<AObject>(
                     carto::shared_ptr<AObject>::Weak, obj ) );
  obj->RegisterParent( this );
  obj->addObserver( this );
  _insertObject( obj );
  _contentHasChanged = true;
  setChanged();
}


void AGraph::insert( const carto::shared_ptr<AObject> & obj,
                     std::string syntacticAtt )
{
  Vertex	*vert = d->graph->addVertex( syntacticAtt );
  d->data.insert( obj );
  vert->setProperty( "ana_object", carto::shared_ptr<AObject>(
                     carto::shared_ptr<AObject>::Weak, obj.get() ) );
  obj->RegisterParent( this );
  obj->addObserver( this );
  _insertObject( obj.get() );
  _contentHasChanged = true;
  setChanged();
}


MObject::const_iterator AGraph::find( const AObject* obj ) const
{
  datatype::const_iterator io
      = d->data.find( shared_ptr<AObject>( shared_ptr<AObject>::Weak,
                      const_cast<AObject *>( obj ) ) );
  return const_iterator( new const_AGraphIterator( io ) );
}


void AGraph::erase( MObject::iterator & i )
{
    //	ajouter un test du type d'iterateur avant le casting brutal
  datatype::iterator & it = ((AGraphIterator&)i.subIterator())._dataIt;
  _eraseObject( *i );
  AttributedAObject *ao = dynamic_cast<AttributedAObject *>( it->get() );
  if( ao )
  {
    GenericObject *a = ao->attributed();
    if( a )
      a->removeProperty( "ana_object" );
  }
  d->data.erase( it );
}


void AGraph::copyAttributes( const string & oldatt, const string & newatt,
                             bool removeold, bool dorels )
{
  Graph *g = graph();
  Graph::iterator i, e = g->end();
  Object  prop;

  for( i=g->begin(); i!=e; ++i )
    try
    {
      prop = (*i)->getProperty( oldatt );
      (*i)->setProperty( newatt, prop );
      if( removeold )
        (*i)->removeProperty( oldatt );
    }
    catch( ... )
    {
    }

  if( dorels )
  {
    const set<Edge *> &edg = g->edges();
    set<Edge *>::const_iterator ie, ee = edg.end();
    for( ie=edg.begin(); ie!=ee; ++ie )
      try
      {
        prop = (*ie)->getProperty( oldatt );
        (*ie)->setProperty( newatt, prop );
        if( removeold )
          (*ie)->removeProperty( oldatt );
      }
      catch( ... )
      {
      }
  }

  if( newatt == "label" )
    setLabelProperty( "label" );
  else if( newatt == "name" )
    setLabelProperty( "name" );
  else
  {
    setChanged();
    _contentHasChanged = true;
    notifyObservers( this );
  }
}


string AGraph::labelProperty( bool allowDefault ) const
{
  string prop;
  if( ( !graph() || !graph()->getProperty( "label_property", prop ) )
         && allowDefault )
    prop = GraphParams::graphParams()->attribute;
  return prop;
}


void AGraph::setLabelProperty( const string & prop )
{
  if( prop.empty() )
    graph()->removeProperty( "label_property" );
  else
    graph()->setProperty( "label_property", prop );
  setChanged();
  _contentHasChanged = true;
  recolor();
  notifyObservers( this );
}


void AGraph::updateAfterAimsChange()
{
  Graph *g = graph();
  // check for removed elements
  set<shared_ptr<AObject> >::iterator   id, id2, ed = d->data.end();
  AGraphObject *ago;
  GenericObject *go;
  bool removed;
  for( id=d->data.begin(); id!=ed; )
  {
    removed = false;
    ago = dynamic_cast<AGraphObject *>( id->get() );
    if( ago )
    {
      go = ago->attributed();
      if( !g->hasVertex( static_cast<Vertex *>( go ) )
        && !g->hasEdge( static_cast<Edge *>( go ) ) )
      {
        cout << "Vertex/Edge removed\n";
        id2 = id;
        ++id;
        removed = true;
        iterator ia( new AGraphIterator( id2 ) );
        erase( ia );
      }
    }
    else
      cerr << "bizarre: a AGraph child is not a AGraphObject\n";
    if( !removed )
      ++id;
  }

  // check for changed/added elements
  int loaded = 0;
  bool l = g->getProperty( "aims_reader_loaded_objects", loaded );
  if( !l )
    loaded = 3;
  AimsGraphReader::PostProcessor pp;
  pp.elementInfo().graph = g;
  if( loaded & 1 )
  {
    Graph::iterator iv, ev = g->end();
    for( iv=g->begin(); iv!=ev; ++iv )
      rescan_element( this, *iv, pp );
  }
  if( loaded & 2 )
  {
    Graph::ESet::iterator ie, ee = g->edges().end();
    for( ie=g->edges().begin(); ie!=ee; ++ie )
      rescan_element( this, *ie, pp );
  }
  updateColors();
  clearLabelsVolume();
  _contentHasChanged = true;
  setChanged();
}


void AGraph::setHeaderOptions()
{
  AObject::setHeaderOptions();

  Motion m = GraphManip::talairach( *d->graph );
  if( !m.isIdentity() )
  {
    GenericObject *go = attributed();
    vector<string> refs;
    refs.push_back( StandardReferentials::acPcReferential() );
    go->setProperty( "referentials", refs );
    vector< vector<float> > mot;
    mot.push_back( m.toVector() );
    go->setProperty( "transformations", mot );
  }
}


// compilation of some Volume classes on Aims types

#include <cartodata/volume/volume_d.h>

template class carto::Volume< AObject * >;

#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( AGraph * )


