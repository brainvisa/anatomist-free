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

#include <anatomist/surface/cutmesh.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Referential.h>
#include <graph/tree/tree.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/fusion/defFusionMethods.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/surface/tesselatedmesh.h>
#include <anatomist/object/actions.h>
#include <aims/mesh/surfaceOperation.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <qpixmap.h>
#include <qtranslator.h>

#define USE_TESSELATION

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


int CutMesh::registerClass()
{
  int	type = registerObjectType( "CutMesh" );
  return type;
}


struct CutMesh::Private
{
  Private() : meshchanged( true ), meshcopied( false ), hascutmesh( true ),
    haspolygon( true ), hasplane( true ), inplanarmeshindex(1), texindex( 1 ),
    cutmeshindex( 2 ), planarcutindex( 3 ),  polygonindex( 3 ),
    planeindex( 4 ), planarfusionindex( 5 ), otherplanarfusionindex( 6 )
  {}

  bool	meshchanged;
  bool	meshcopied;
  bool	hascutmesh;
  bool	haspolygon;
  bool	hasplane;
  int   inplanarmeshindex;
  int   texindex;
  int   cutmeshindex;
  int   planarcutindex;
  int	polygonindex;
  int	planeindex;
  int	planarfusionindex;
  int   otherplanarfusionindex;
};


CutMesh::CutMesh( const vector<AObject *> & obj ) 
  : ObjectVector(), SelfSliceable(), d( new CutMesh::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_cutmesh.xpm" );
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ _type ] = "Cut Mesh";
    }

  vector<AObject *>::const_iterator	io, fo=obj.end();
  vector<AObject *>			surf, vol;

  //	sort objects: surfaces first, volumes last
  for( io=obj.begin(); io!=fo; ++io )
  {
    if( dynamic_cast<ATriangulated *>( *io ) )
      surf.push_back( *io );
    else if( dynamic_cast<CutMesh *>( *io ) )
    {
      CutMesh *cm = static_cast<CutMesh *>( *io );
      AObject *ao = cm->volume();
      vol.push_back( ao );
      MObject::iterator ic, ec = cm->end();
      int     i, n = 0;
      for( ic=cm->begin(); ic!=ec && *ic!=ao; ++ic, ++n ) {}
      ao = cm->cutMesh();
      if( ao )
      {
        for( ic=cm->begin(); ic!=ec && *ic!=ao; ++ic ) {}
        for( i=0; i<n; ++i, ++ic )
          if( dynamic_cast<ATriangulated *>( *ic ) )
            surf.push_back( *ic );
      }
      ao = cm->planarMesh();
      if( ao )
      {
        TesselatedMesh *tm = dynamic_cast<TesselatedMesh *>( ao );
        if( tm )
          surf.push_back( tm->tesselatedMesh() );
        else
          surf.push_back( ao );
      }
    }
    else
      vol.push_back( *io );
  }

  unsigned      i, n, nmesh = surf.size(), nplanar = 0;
  d->texindex = nmesh;
  d->cutmeshindex = d->texindex + vol.size();
  d->polygonindex = d->cutmeshindex + nmesh;
  ATriangulated *atr;
  for( i=0; i<nmesh; ++i )
  {
    atr = dynamic_cast<ATriangulated *>( surf[i] );
    if( !atr || !atr->isPlanar() )
      insert( surf[i] );
  }
  d->inplanarmeshindex = size();
  nplanar = nmesh - d->inplanarmeshindex;
  if( nplanar != 0 )
    for( i=0; i<nmesh; ++i )
    {
      atr = dynamic_cast<ATriangulated *>( surf[i] );
      if( atr && atr->isPlanar() )
        insert( surf[i] );
    }
  for( i=0, n=vol.size(); i<n; ++i )
    insert( vol[i] );

  AObject	*s = *surf.begin();
  AObject	*v = *vol.begin();

  setReferentialInheritance( s );

  // cut mesh
  ATriangulated	*cm = 0;
  for( i=0; i<nmesh; ++i )
  {
    cm = new ATriangulated( "" );
    cm->setName( theAnatomist->makeObjectName( "CutSubMesh" ) );
    theAnatomist->registerObject( cm, false );
    cm->setSurface( new AimsSurfaceTriangle );
    insert( cm );
    cm->setReferentialInheritance( s );
    cm->SetMaterial( s->GetMaterial() );
  }
  d->planarcutindex = d->inplanarmeshindex + d->cutmeshindex;

  // border polygon
  ASurface<2>	*bp = new ASurface<2>( "" );
  bp->setName( theAnatomist->makeObjectName( "BorderPolygon" ) );
  theAnatomist->registerObject( bp, false );
  bp->setSurface( new AimsTimeSurface<2,Void> );
  Material	& polmat = bp->GetMaterial();
  //polmat.setRenderProperty( Material::RenderLighting, 0 );
  polmat.SetDiffuse( 1., 1., 0., 1. );
  bp->SetMaterial( polmat );
  insert( bp );
  bp->setReferentialInheritance( s );

#ifdef USE_TESSELATION
  // tesselation
  FusionTesselationMethod tessmet;
  vector<AObject *> tessobj( 1 );
  tessobj[0] = bp;
  TesselatedMesh *tessm
    = static_cast<TesselatedMesh *>( tessmet.fusion( tessobj ) );
  tessm->setName( theAnatomist->makeObjectName( "TesselatedPolygon" ) );
  theAnatomist->registerObject( tessm, false );
  insert( tessm );
  tessm->setReferentialInheritance( s ); // should be bp
  d->planeindex = d->polygonindex + 1;

  Point3df  bmin, bmax;
  s->boundingBox( bmin, bmax );
  setOffsetSilent( Point3df( ( bmin[0] + bmax[0] ) / 2,
                             ( bmin[1] + bmax[1] ) / 2,
                             ( bmin[2] + bmax[2] ) / 2 ) );
  cut();

  // planar fusion
  PlanarFusion3dMethod      fm;
  vector<AObject *> sobj( 2 );
  sobj[0] = tessm; //->tesselatedMesh();
  sobj[1] = v;
  AObject   *fus = fm.fusion( sobj );
  fus->setName( theAnatomist->makeObjectName( "PlanarFusion3D" ) );
  theAnatomist->registerObject( fus, false );
  insert( fus );
  fus->setReferentialInheritance( s ); // should be tessm but update issue
  d->planarfusionindex = d->planeindex + 1;

#else
  if( true /* theAnatomist->userLevel() >= 3 */ )
    {
      // planar mesh
      ATriangulated	*pm = new ATriangulated( "" );
      pm->setName( theAnatomist->makeObjectName( "PlanarMesh" ) );
      theAnatomist->registerObject( pm, false );
      pm->setSurface( new AimsSurfaceTriangle );
      insert( pm );
      d->planeindex = d->polygonindex + 1;
      d->planarfusionindex = d->planeindex + 1;
      pm->setReferentialInheritance( s );

      Point3df	bmin, bmax;
      s->boundingBox( bmin, bmax );
      setOffsetSilent( Point3df( ( bmin[0] + bmax[0] ) / 2, 
                                 ( bmin[1] + bmax[1] ) / 2, 
                                 ( bmin[2] + bmax[2] ) / 2 ) );
      cut();

      // planar fusion
      PlanarFusion3dMethod	fm;
      vector<AObject *>	sobj( 2 );
      sobj[0] = pm;
      sobj[1] = v;
      AObject	*fus = fm.fusion( sobj );
      fus->setName( theAnatomist->makeObjectName( "PlanarFusion3D" ) );
      theAnatomist->registerObject( fus, false );
      insert( fus );
      fus->setReferentialInheritance( s ); // should be pm but update issue
    }
  else // no plane
    {
      d->hasplane = false;

      // render the cut mesh double-sided since it will be open
      Material	& cmmat = cm->GetMaterial();
      cmmat.setRenderProperty( Material::RenderFaceCulling, 0 );

      d->planeindex = -1;
      d->planarfusionindex = -1;

      Point3df	bmin, bmax;
      s->boundingBox( bmin, bmax );
      setOffsetSilent( Point3df( ( bmin[0] + bmax[0] ) / 2, 
                                 ( bmin[1] + bmax[1] ) / 2, 
                                 ( bmin[2] + bmax[2] ) / 2 ) );
      cut();
    }
#endif
  d->otherplanarfusionindex = size();
  if( nplanar > 0 )
  {
    iterator  io;
    for( i=0, io=begin(); i<(unsigned)d->planarcutindex; ++i, ++io ) {}
    for( i=0; i<nplanar; ++i, ++io )
    {
      ATriangulated     *pm = static_cast<ATriangulated *>( *io );
      PlanarFusion3dMethod      fm;
      vector<AObject *> sobj( 2 );
      sobj[0] = pm;
      sobj[1] = v;
      AObject   *fus = fm.fusion( sobj );
      fus->setName( theAnatomist->makeObjectName( "PlanarFusion3D" ) );
      theAnatomist->registerObject( fus, false );
      insert( fus );
      fus->setReferentialInheritance( s );
    }
  }
}


CutMesh::~CutMesh()
{
  AObject	*o = planarFusion();
  iterator	i;
  if( o )
    {
      i = find( o );
      erase( i );
      if( o->Parents().empty() )
        delete o;
    }
  o = planarMesh();
  if( o )
    {
      i = find( o );
      erase( i );
      if( o->Parents().empty() )
        delete o;
    }
  o = borderPolygon();
  if( o )
    {
      i = find( o );
      erase( i );
      if( o->Parents().empty() )
        delete o;
    }
  o = cutMesh();
  if( o )
    {
      i = find( o );
      erase( i );
      if( o->Parents().empty() )
        delete o;
    }
  if( d->meshcopied )
    {
      o = mesh();
      i = find( o );
      erase( i );
      if( o->Parents().empty() )
        delete o;
    }
  delete d;
}


int CutMesh::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


const AObject* CutMesh::volume() const
{
  MObject::const_iterator io;
  int                     i;
  for( io=begin(), i=0; i<d->texindex; ++i, ++io ) {}
  return *io;
}


AObject* CutMesh::volume()
{
  MObject::iterator io;
  int               i;
  for( io=begin(), i=0; i<d->texindex; ++i, ++io ) {}
  return *io;
}


const AObject* CutMesh::mesh() const
{
  return *begin();
}


AObject* CutMesh::mesh()
{
  return *begin();
}


const AObject* CutMesh::cutMesh() const
{
  MObject::const_iterator io;
  int                     i;
  for( io=begin(), i=0; i<d->cutmeshindex; ++i, ++io ) {}
  return *io;
}


AObject* CutMesh::cutMesh()
{
  MObject::iterator io;
  int               i;
  for( io=begin(), i=0; i<d->cutmeshindex; ++i, ++io ) {}
  return *io;
}


const AObject* CutMesh::borderPolygon() const
{
  if( !d->haspolygon )
    return 0;
  const_iterator	ii = begin();
  int			i;
  for( i=0; i<d->polygonindex; ++i )
    ++ii;
  return *ii;
}


AObject* CutMesh::borderPolygon()
{
  if( !d->haspolygon )
    return 0;
  iterator	ii = begin();
  int		i;
  for( i=0; i<d->polygonindex; ++i )
    ++ii;
  return *ii;
}


const AObject* CutMesh::planarMesh() const
{
  if( !d->hasplane )
    return 0;
  const_iterator	ii = begin();
  int			i;
  for( i=0; i<d->planeindex; ++i )
    ++ii;
  return *ii;
}


AObject* CutMesh::planarMesh()
{
  if( !d->hasplane )
    return 0;
  iterator	ii = begin();
  int		i;
  for( i=0; i<d->planeindex; ++i )
    ++ii;
  return *ii;
}


const AObject* CutMesh::planarFusion() const
{
  if( !d->hasplane )
    return 0;
  const_iterator	ii = begin();
  int			i;
  for( i=0; i<d->planarfusionindex; ++i )
    ++ii;
  return *ii;
}


AObject* CutMesh::planarFusion()
{
  if( !d->hasplane )
    return 0;
  iterator	ii = begin();
  int		i;
  for( i=0; i<d->planarfusionindex; ++i )
    ++ii;
  return *ii;
}


bool CutMesh::render( PrimList & prim, const ViewState & state )
{
//   cout << "CutMesh::render, this: " << this << endl;
  /* ATriangulated	*cm = (ATriangulated *) cutMesh();
  cout << "cutmesh: " << cm->glNumVertex( state ) << " vertices, "
    << cm->glNumPolygon( state ) << " polygons\n"; */

  if( d->meshchanged )
    updateCut();

  iterator  io = begin(), eo = end();
  int       i = 0;
  if( d->hascutmesh )
  {
    for( ; i<d->cutmeshindex; ++io, ++i ) {}
    for( ; i<d->planarcutindex; ++io, ++i )
      (*io)->render( prim, state );
  }
  if( d->hasplane )
  {
    for( ; i<d->planarfusionindex; ++io, ++i ) {}
    (*io)->render( prim, state );
  }
  for( ; i<d->otherplanarfusionindex; ++io, ++i ) {}
  for( ; i<d->otherplanarfusionindex + d->texindex - d->inplanarmeshindex;
       ++io, ++i )
    (*io)->render( prim, state );

  clearHasChangedFlags();
  return true;
}


void CutMesh::update( const Observable* observable, void* arg )
{
  // cout << "CutMesh::update " << this << " " << size() << endl;

  AObject::update( observable, arg );
  const AObject	*obj = dynamic_cast<const AObject *>( observable );
  if( obj )
    {
      // cout << "object: " << obj << endl;
      iterator  io, eo = end();
      int       i = 0;
      for( io=begin(); io!=eo && *io!=obj; ++io, ++i ) {}
      if( io == eo )
        return;
      if( i < d->texindex )
        {
          // cout << "mesh\n";
          const GLComponent	*surf = obj->glAPI();
          if( surf && obj->obsHasChanged( GLComponent::glGEOMETRY ) )
            {
              // cout << "geom changed\n";
              d->meshchanged = true;
              obsSetChanged( GLComponent::glGEOMETRY );
            }
        }
      else if( i < d->texindex + 1 ) // TODO: count textures
      {
        // cout << "tex\n";
        obsSetChanged( GLComponent::glTEXIMAGE );
      }
      else if( d->hascutmesh && i < d->cutmeshindex + d->texindex )
        {
          // cout << "cutMesh\n";
          if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
            obsSetChanged( GLComponent::glMATERIAL );
        }
      else if( d->hasplane && i >= d->planarfusionindex )
        {
          // cout << "planarFusion\n";
          if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
            obsSetChanged( GLComponent::glMATERIAL );
          if( obj->obsHasChanged( GLComponent::glTEXIMAGE ) )
            obsSetChanged( GLComponent::glTEXIMAGE );
          if( obj->obsHasChanged( GLComponent::glTEXENV ) )
            obsSetChanged( GLComponent::glTEXENV );
        }
      else if( d->haspolygon && i == d->polygonindex )
        {
          // cout << "borderPolygon\n";
          if( obj->obsHasChanged( GLComponent::glMATERIAL ) )
            obsSetChanged( GLComponent::glMATERIAL );
        }
      // cout << "updateSubObjectReferential cutmesh\n";
      updateSubObjectReferential( obj );
      // cout << "updateSubObjectReferential done\n";
    }
  else
    {
      const TransformationObserver 
        *to = dynamic_cast<const TransformationObserver *>( observable );
      if( !to )
        return;

      // cout << "Transformation changed in PlanarFusion3D\n";
      obsSetChanged( GLComponent::glTEXIMAGE );
    }

  // cout << "notify cutmesh\n";
  notifyObservers( (void*) this );
  // cout << "cutmesh update done\n";
}


Tree* CutMesh::optionTree() const
{
  static Tree*	_optionTree = 0;

  if( !_optionTree )
    {
      //Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );

      Tree *t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      Tree *t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );

      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


bool CutMesh::boundingBox( vector<float> & bmin, vector<float> & bmax ) const
{
  bool ok = mesh()->boundingBox( bmin, bmax );
  MObject::const_iterator io;
  unsigned i, j, n;
  for( io=begin(), j=0; j<d->texindex; ++j, ++io ) {}
  for( ; j<d->cutmeshindex; ++j, ++io )
  {
    vector<float> obmin, obmax;
    bool ook = (*io)->boundingBox( obmin, obmax );
    ok &= ook;
    if( ook )
    {
      for( i=3, n=obmax.size(); i<n; ++i )
      {
        if( bmin.size() <= i )
        {
          bmin.push_back( obmin[i] );
          bmax.push_back( obmax[i] );
        }
        else
        {
          if( obmin[i] < bmin[i] )
            bmin[i] = obmin[i];
          if( obmax[i] > bmax[i] )
            bmax[i] = obmax[i];
        }
      }
    }
  }
  return ok;
}


void CutMesh::clearHasChangedFlags() const
{
  d->meshchanged = false;
  MObject::clearHasChangedFlags();
}


void CutMesh::cut()
{
  // cout << "CutMesh::cut...\n";
  // # ATriangulated		*p = (ATriangulated *) planarMesh();
  ASurface<2>		*border = (ASurface<2> *) borderPolygon();

  vector<const AimsSurfaceTriangle *> insurf;
  insurf.reserve( d->texindex );
  int               i;
  MObject::iterator io;
  // cout << "cutMesh " << d->texindex << " meshes\n";
  for( i=0, io=begin(); i<d->texindex; ++i, ++io )
  {
    insurf.push_back( static_cast<const ATriangulated *>(
      *io )->surface().get() );
  }
  unsigned  nmesh = d->texindex;
  vector<rc_ptr<AimsSurfaceTriangle> > cut( nmesh );
  for( ; i<d->cutmeshindex; ++i, ++io ) {}
  AimsTimeSurface<2,Void>	*pol = 0;
#ifndef USE_TESSELATION
  AimsSurfaceTriangle		*ps = 0;
#endif
  try
  {
#ifndef USE_TESSELATION
    if( p )
      {
        ps = new AimsSurfaceTriangle;
        if( border )
          {
            pol = new AimsTimeSurface<2,Void>;
            SurfaceManip::cutMesh( insurf, plane(), cut, *ps, *pol );
          }
        else
          SurfaceManip::cutMesh( insurf, plane(), cut, *ps );
      }
    else
#endif
    if( border || nmesh != 0 )
      {
        pol = new AimsTimeSurface<2,Void>;
        SurfaceManip::cutMesh( insurf, plane(), cut, *pol );
      }
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
  }
  for( i=0; i<(int)nmesh; ++i, ++io )
  {
    static_cast<ATriangulated *>(*io)->setSurface( cut[i].get() );
    cut[i].release();
    // (*io)->notifyObservers( this );
  }
#ifndef USE_TESSELATION
  if( p )
  {
    p->setSurface( ps );
    // p->notifyObservers( this );
  }
#endif
  if( border )
  {
    border->setSurface( pol );
    Point4df plan = plane();
    vector<float> normal( 3 );
    normal[0] = plan[0];
    normal[1] = plan[1];
    normal[2] = plan[2];
    border->attributed()->setProperty( "normal", normal );
    // border->notifyObservers( this );
  }
  else
    delete pol;
  setGeomExtrema();
  // cout << "CutMesh::cut done\n";
}


void CutMesh::updateCut()
{
  cut();
  iterator  io = begin();
  int       i = 0, n0 = 0, n1 = 0;
  if( d->hascutmesh )
  {
    n0 = d->cutmeshindex;
    n1 = d->cutmeshindex + d->texindex;
    for( ; i<n0; ++i, ++io ) {}
    for( ; i<n1; ++i, ++io )
      (*io)->notifyObservers( this );
  }
  if( d->haspolygon )
  {
    n0 = d->polygonindex;
    for( ; i<n0; ++i, ++io ) {}
    (*io)->notifyObservers( this );
  }
  if( d->hasplane )
  {
    n0 = d->planeindex;
    for( ; i<n0; ++i, ++io ) {}
    (*io)->notifyObservers( this );
  }
  /*  AObject	*o = cutMesh();
  if( o )
    o->notifyObservers( this );
  o = planarMesh();
  if( o )
    o->notifyObservers( this );
  o = borderPolygon();
  if( o )
    o->notifyObservers( this );
  */
}


void CutMesh::sliceChanged()
{
  d->meshchanged = true;
  setChanged();
  updateCut();
}


void CutMesh::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );
  AObject	*cm = cutMesh();
  if( cm )
    cm->SetMaterial( mat );
}


Material & CutMesh::GetMaterial()
{
  AObject	*cm = cutMesh();
  if( cm )
    return cm->GetMaterial();
  return AObject::GetMaterial();
}


AObject* CutMesh::fallbackReferentialInheritance() const
{
  return const_cast<AObject *>( mesh() );
}


list<AObject *> CutMesh::generativeChildren() const
{
  list<AObject *> children;
  const_iterator io, e = end();
  int i = 0;
  for( io=begin(); i<d->cutmeshindex; ++i, ++io )
    children.push_back( *io );
  return children;
}


Object CutMesh::makeHeaderOptions() const
{
  Object opts = ObjectVector::makeHeaderOptions();
  makeSliceHeaderOptions( opts );
  return opts;
}


void CutMesh::setProperties( Object options )
{
  ObjectVector::setProperties( options );
  setSliceProperties( options );
}

