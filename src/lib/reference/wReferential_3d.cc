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

#include <anatomist/reference/wReferential_3d.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/surface/surface.h>
#include <aims/points_distribution/points_distribution.h>
#include <aims/mesh/surfacegen.h>
#include <cartodata/volume/volume.h>
#include <algorithm>
#include <cfloat>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{
  using anatomist::Transformation;

  inline double frand()
  {
    return ((rand()-(RAND_MAX/2))/(RAND_MAX/2.));
  }


  map<Referential *, set<Referential *> > *sort_trans(
    const set<Transformation *> & all_trans,
    const set<Referential *> & exclude_refs )
  {
    map<Referential *, set<Referential *> > *strans
      = new map<Referential *, set<Referential *> >;
    set<Transformation *>::const_iterator it, et = all_trans.end();
    set<Referential *>::const_iterator included = exclude_refs.end();

    for( it=all_trans.begin(); it!=et; ++it )
    {
      Transformation *t = *it;
      Referential *s = t->source();
      Referential *d = t->destination();
      if( exclude_refs.find( s ) != included
          || exclude_refs.find( d ) != included )
          continue; // skip central ref
      if( ! t->isGenerated() )
      {
        set<Referential *> & ss = (*strans)[s];
        ss.insert( d );
        set<Referential *> & sd = (*strans)[d];
        sd.insert( s );
      }
    }

    return strans;
  }


  map<Referential*, Point2df> *plot_one_connected_component(
    const map<Referential *, set<Referential*> > & strans, Referential* r0 )
  {
    map<Referential*, Point2df> *rpos = new map<Referential*, Point2df>;
    (*rpos)[ r0 ] = Point2df(0.);
    set<Referential *> todo;
    todo.insert( r0 );
    set<Referential *> done;
    map<Referential *, set<Referential*> >::const_iterator
      it, et = strans.end();
    map<Referential*, Point2df>::iterator irp, erp = rpos->end();
    set<Referential *>::const_iterator not_done = done.end();

    while( !todo.empty() )
    {
      set<Referential *>::iterator f = todo.begin();
      Referential *r1 = *f;
      todo.erase( f );

      done.insert( r1 );
      Point2df &pos = (*rpos)[ r1 ];
      it = strans.find( r1 );
      if( it == et )
        continue;
      const set<Referential*> & tref = it->second;
      size_t i = 0, nr = tref.size();
      set<Referential*>::const_iterator ir, er = tref.end();

      for( ir=tref.begin(); ir!=er; ++ir, ++i )
      {
        Referential *r = *ir;
        irp = rpos->find( r );
        if( irp != erp )
        {
          Point2df & p1 = irp->second;
          Point2df direc = (p1 - pos) / 3.;
          if( direc.norm() > 0.33 )
          {
            pos += direc;
            p1 -= direc;
          }
        }
        else
        {
          double rang = frand() * M_PI * 2.;
          Point2df p1 = pos + Point2df( cos( M_PI * 2. * i / nr + rang ),
                                        sin( M_PI * 2. * i / nr + rang ) );
          (*rpos)[r] = p1;
        }
        if( done.find( r ) == not_done )
          todo.insert( r );
      }
    }

    return rpos;
  }


  void normalize_coords( vector<Point2df> & pos, Point2df offset,
                         const Point2df & bbox_min, const Point2df & bbox_max )
  {
    Point2df scales( bbox_max[0] - bbox_min[0], bbox_max[1] - bbox_min[1] );
    if( scales[0] == 0. )
      scales[0] = 1.;
    else
      scales[0] = 1. / scales[0];
    if( scales[1] == 0. )
      scales[1] = 1.;
    else
      scales[1] = 1. / scales[1];
    offset = Point2df( offset[0] / scales[0] - bbox_min[0],
                       offset[1] / scales[1] - bbox_min[1] );

    vector<Point2df>::iterator ip, ep = pos.end();
    for( ip=pos.begin(); ip!=ep; ++ip )
    {
      *ip += offset;
      (*ip)[0] *= scales[0];
      (*ip)[1] *= scales[1];
    }
  }


  Point2d find_place( VolumeRef<uint8_t> used_spc )
  {
    short x, y, dx = used_spc.getSizeX(), dy = used_spc.getSizeY();
    for( y=0; y<dy; ++y )
      for( x=0; x<dx; ++x )
        if( used_spc->at( x, y ) == 0 )
          return Point2d( x, y );
    return Point2d( dx, 0 );
  }


  VolumeRef<uint8_t> store_place( VolumeRef<uint8_t> used_spc, Point2d xy )
  {
    int sz = std::max( xy[0] + 1, xy[1] + 1 );
    int dx = used_spc->getSizeX(), dy = used_spc->getSizeY();

    if( dx < sz || dy < sz )
    {
      VolumeRef<uint8_t> used_spc2( sz, sz );
      used_spc2->fill( 0 );
      int xx, yy;
      for( yy=0; yy<dy; ++yy )
        for( xx=0; xx<dx; ++xx )
          used_spc2->at( xx, yy ) = used_spc->at( xx, yy );
      used_spc = used_spc2;
    }
    used_spc->at( xy[0], xy[1] ) = 1;
    return used_spc;
  }


  map<Referential*, Point2df> *plot_all_connected_components(
    set<Referential *> *exclude_refs_p = 0 )
  {
    set<Referential *> real_excl;
    if( !exclude_refs_p )
    {
      // by default, exclude the central ref
      exclude_refs_p = &real_excl;
      real_excl.insert( theAnatomist->centralReferential() );
    }
    set<Referential *> & exclude_refs = *exclude_refs_p;

    map<Referential *, set<Referential*> > *strans
      = sort_trans( ATransformSet::instance()->allTransformations(),
                    exclude_refs );
    Point2df bounds_max( 0. );
    set<Referential *> done = exclude_refs;
    int x = 0;
    int y = 0;
    VolumeRef<uint8_t> used_spc( 1, 1 );
    set<Referential *>::const_iterator
      ir, er = theAnatomist->getReferentials().end();
    set<Referential *>::iterator not_done = done.end();
    map<Referential*, Point2df> *allpos = 0;
    map<Referential*, Point2df>::iterator ic, ec;

    for( ir=theAnatomist->getReferentials().begin(); ir!=er; ++ir )
    {
      Referential *r0 = *ir;
      if( done.find( r0 ) == not_done )
      {
        map<Referential*, Point2df> *cc = plot_one_connected_component(
          *strans, r0 );
        vector<Point2df> pts( cc->size() );
        Point2df bb_min( FLT_MAX ), bb_max( -FLT_MAX );
        int i = 0;
        for( ic=cc->begin(), ec=cc->end(); ic!=ec; ++ic, ++i )
        {
          done.insert( ic->first );
          Point2df &pos = ic->second;
          if( pos[0] < bb_min[0] )
            bb_min[0] = pos[0];
          if( pos[0] > bb_max[0] )
            bb_max[0] = pos[0];
          if( pos[1] < bb_min[1] )
            bb_min[1] = pos[1];
          if( pos[1] > bb_max[1] )
            bb_max[1] = pos[1];
          pts[i] = pos;
        }
        Point2d xy = find_place( used_spc );
        Point2df offset( xy[0] * 2., xy[1] * 2. );
        normalize_coords( pts, offset, bb_min, bb_max );
        used_spc = store_place( used_spc, xy );
        if( !allpos )
        {
          bounds_max = Point2df( 2., 2. );
          allpos = cc;
        }
        else
        {
          for( ic=cc->begin(); ic!=ec; ++ic )
            (*allpos)[ ic->first ] = ic->second;
          bounds_max[0] = std::max( bounds_max[0], xy[0] * 2.f + 2.f );
          bounds_max[1] = std::max( bounds_max[1], xy[1] * 2.f + 2.f );
          delete cc;
        }
      }
    }
    delete strans;

    for( ic=allpos->begin(), ec=allpos->end(); ic!=ec; ++ic )
    {
      Point2df & pos = ic->second;
      float sdx = bounds_max[0];
      if( sdx == 0. )
        sdx = 1.;
      float sdy = bounds_max[1];
      if( sdy == 0. )
        sdy = 1.;
      float scalex = M_PI * 1.9 / sdx;
      float scaley = M_PI * 0.8 / sdy;
      pos[0] = pos[0] * scalex;
      pos[1] = pos[1] * scaley - M_PI * 0.4;
    }

    return allpos;
  }


  vector<Point3df> *coords_to_sphere( const vector<Point2df> & points )
  {
    int i, n = points.size();
    vector<Point3df> *pts = new vector<Point3df>( n );
    for( i=0; i<n; ++i )
    {
      float lon = points[i][0];
      float lat = points[i][1];
      float r = cos( lat );
      (*pts)[i] = Point3df( r * cos( lon ),
                            r * sin( lon ),
                            sin( lat ) );
    }
    return pts;
  }


  vector<Point3df> *get_sphere_pos(
    unsigned nrefs, const Point3df & center = Point3df( 0., 0., 0. ),
    float radius = 100.,
    const map<unsigned, set<unsigned> > & links
      = map<unsigned, set<unsigned> >() )
  {
    vector<Point3df> *pos = new vector<Point3df>;
    PointsDistribution pdist;
    pdist.set_links( links );
    if( nrefs != 0 )
    {
      pos = pdist.distribute( nrefs - 1 );
      pos->insert( pos->begin(), Point3df( -0.2, 0., -0.2 ) );
    }
    else
    {
      Referential *cent = theAnatomist->centralReferential();
      map<Referential*, Point2df> *init_ref_pos
        = plot_all_connected_components();
      unsigned i, np = init_ref_pos->size();
      vector<Point2df> pts2d( np );
      map<Referential*, Point2df>::iterator ir, er = init_ref_pos->end();
      for( i=0, ir=init_ref_pos->begin(); ir!=er; ++ir, ++i )
        pts2d[i] = ir->second;
      delete init_ref_pos;
      vector<Point3df> *init_pts = coords_to_sphere( pts2d );
      pos = pdist.distribute( *init_pts );
      pos->insert( pos->begin(), Point3df( -0.2, 0., -0.2 ) );
      delete init_pts;
    }
    vector<Point3df>::iterator ip, ep = pos->end();
    for( ip=pos->begin(); ip!=ep; ++ip )
    {
      *ip *= radius;
      *ip -= center;
    }

    return pos;
  }


  vector<Point3df> *get_flat_pos(
    const Point3df & center = Point3df( 0., 0., 0. ),
    float radius = 100.,
    const map<unsigned, set<unsigned> > & links
      = map<unsigned, set<unsigned> >() )
  {
    set<Referential *> exclude_refs;
    map<Referential*, Point2df> *init_ref_pos
      = plot_all_connected_components( &exclude_refs );
    unsigned i, np = init_ref_pos->size();
    vector<Point3df> pts3d( np );
    map<Referential*, Point2df>::iterator ir, er = init_ref_pos->end();

    for( i=0, ir=init_ref_pos->begin(); ir!=er; ++ir, ++i )
      pts3d[i] = Point3df( ir->second[0], 0., ir->second[1] );
    delete init_ref_pos;

    PointsDistribution pdist( new PointsDistribution::CoulombAndRestoringForce,
                              new PointsDistribution::MoveConstraints );
    pdist.set_links( links );
    vector<Point3df> *pos = pdist.distribute( pts3d );

    vector<Point3df>::iterator ip, ep = pos->end();
    for( ip=pos->begin(); ip!=ep; ++ip )
    {
      /* y axis gets a random coord to make use of 3D and disambiguate
         crossing transformations */
      Point3df & p = *ip;
      p[0] = p[0] * radius - center[0];
      p[1] = frand() * radius - center[1];
      p[2] = p[2] * radius - center[2];
    }
    return pos;
  }


  class RefMesh : public ASurface<3>
  {
  public:
    RefMesh() : ASurface<3>(), referential( 0 ) {}
    virtual ~RefMesh() {}

    Referential *referential;
  };


  class TransMesh : public ASurface<3>
  {
  public:
    TransMesh() : ASurface<3>(), transformation( 0 ) {}
    virtual ~TransMesh() {}

    Transformation *transformation;
  };


  rc_ptr<AObject> createRefMesh( const Point3df & pos, Referential *ref )
  {
    Point3df center( 0., 0., 0. );
    float radius = 10.;
    AimsSurfaceTriangle *mesh;
    if( ref == theAnatomist->centralReferential() )
      mesh = SurfaceGenerator::icosahedron( pos, radius );
    else
      mesh = SurfaceGenerator::sphere( pos, radius, 200 );
    RefMesh *amesh = new RefMesh;
    rc_ptr<AObject> aobj( amesh );
    amesh->setSurface( rc_ptr<AimsSurfaceTriangle>( mesh ) );
    theAnatomist->registerObject( amesh, false );
    theAnatomist->releaseObject( amesh );
    AimsRGB col = ref->Color();
    Material & mat = amesh->GetMaterial();
    mat.SetDiffuse( float(col[0]) / 255., float(col[1]) / 255.,
                    float(col[2]) / 255., 1. );
    amesh->SetMaterial( mat );
    if( ref->header().hasProperty( "name" ) )
    {
      string name = ref->header().getProperty( "name" )->getString();
      amesh->setName( theAnatomist->makeObjectName( name ) );
    }
    amesh->attributed()->setProperty( "position", pos );
    amesh->attributed()->setProperty( "radius", radius );
    amesh->referential = ref;
    return aobj;
  }


  rc_ptr<AObject> createTransMesh(
    Transformation *tr,
    map<Referential *, rc_ptr<AObject> > & arefs )
  {
    RefMesh *source = static_cast<RefMesh *>( arefs[ tr->source() ].get() );
    RefMesh *dest = static_cast<RefMesh *>( arefs[ tr->destination() ].get() );
    Point3df spos, dpos;
    source->attributed()->getProperty( "position", spos );
    float sradius = source->attributed()->getProperty( "radius" )->getScalar();
    dest->attributed()->getProperty( "position", dpos );
    float dradius = dest->attributed()->getProperty( "radius" )->getScalar();
    Point3df direc = dpos - spos;
    direc.normalize();
    spos = spos + direc * sradius;
    dpos = dpos - direc * dradius;
    TransMesh *atr = new TransMesh;
    rc_ptr<AObject> aobj( atr );

    AimsSurfaceTriangle *mesh;
    Material & mat = atr->GetMaterial();
    if( !tr->isGenerated() )
    {
      mesh = SurfaceGenerator::arrow( dpos, spos, 1., 3., 10, 0.2 );
      mat.SetDiffuse( 0.8, 0.6, 0.4, 1. );
    }
    else
    {
      mesh = SurfaceGenerator::cylinder( dpos, spos, 1., 1., 4, false, false );
      mat.SetDiffuse( 0.8, 0.8, 0.8, 0.2 );
    }
    theAnatomist->registerObject( atr, false );
    theAnatomist->releaseObject( atr );
    atr->setSurface( mesh );
    atr->transformation = tr;
    mat.setRenderProperty( Material::SelectableMode,
                           Material::AlwaysSelectable );
    atr->SetMaterial( mat );
    return aobj;
  }

}

// ---

RefWindow::RefWindow() : AWindow3D(), _view_mode( Flat )
{
}


RefWindow::~RefWindow()
{
}


void RefWindow::updateReferentialView()
{
  vector<Referential *> excluded;
  set<Referential *> excluded_set;
  if( _view_mode == Sphere )
    excluded.push_back( theAnatomist->centralReferential() );
  excluded_set.insert( excluded.begin(), excluded.end() );
  set<Referential *>::iterator included = excluded_set.end();
  vector<Referential *> refs = excluded;
  refs.reserve( theAnatomist->getReferentials().size() );
  set<Referential *>::const_iterator
    ir, er = theAnatomist->getReferentials().end();
  for( ir=theAnatomist->getReferentials().begin(); ir!=er; ++ir )
    if( excluded_set.find( *ir ) == included )
      refs.push_back( *ir );

  const set<Transformation *> & trans
    = ATransformSet::instance()->allTransformations();
  map<unsigned, set<unsigned> > links;
  vector<Referential *> irefs;
  irefs.reserve( refs.size() - excluded.size() );
  irefs.insert( irefs.end(), refs.begin() + excluded.size(), refs.end() );
  set<Transformation *>::const_iterator it, et = trans.end();
  Transformation *tr;
  Referential *s, *d;

  for( it=trans.begin(); it!=et; ++it )
  {
    tr = *it;
    if( !tr->isGenerated() )
    {
      s = tr->source();
      d = tr->destination();
      if( excluded_set.find( s ) != included
          || excluded_set.find( d ) != included )
        continue; // skip central ref
      unsigned si = std::find( irefs.begin(), irefs.end(), s ) - irefs.begin();
      unsigned di = std::find( irefs.begin(), irefs.end(), d ) - irefs.begin();
      links[ min(si, di) ].insert( max(si, di) );
    }
  }

  vector<Point3df> *refpos;
  if( _view_mode = Flat )
    refpos = get_flat_pos( Point3df( 0., 0., 0. ), 150., links );
  else
    refpos = get_sphere_pos( 0, Point3df( 0., 0., 0. ), 100., links );

  map<Referential *, rc_ptr<AObject> > arefs;
  vector<Referential *>::iterator irf, erf = refs.end();
  unsigned i = 0;

  for( irf=refs.begin(); irf!=erf; ++irf, ++i )
    arefs[ *irf ] = createRefMesh( (*refpos)[i], *irf );

  delete refpos;

  map<Transformation *, rc_ptr<AObject> > atrans;
  map<Referential *, set<Referential *> > tr_set;
  map<Referential *, set<Referential *> >::iterator not_done = tr_set.end();

  for( it=trans.begin(); it!=et; ++it )
  {
    tr = *it;
    if( !tr->isGenerated() )
    {
      atrans[ tr ] = createTransMesh( tr, arefs );
      s = tr->source();
      d = tr->destination();
      tr_set[s].insert( d );
      tr_set[d].insert( s );
    }
  }

  // invisible trans
  for( it=trans.begin(); it!=et; ++it )
  {
    s = tr->source();
    d = tr->destination();
    if( tr_set.find( s ) == not_done )
    {
      set<Referential *> & sr = tr_set[s];
      if( sr.find( d ) == sr.end() )
      {
        atrans[ tr ] = createTransMesh( tr, arefs );
        tr_set[s].insert( d );
        tr_set[d].insert( s );
      }
    }
  }

  // remove former objects
  set<AObject *> current = Objects();
  set<AObject *>::iterator io, eo = current.end();
  for( io=current.begin(); io!=eo; ++io )
    if( dynamic_cast<RefMesh *>( *io ) || dynamic_cast<TransMesh *>( *io ) )
      unregisterObject( *io );

  // add new ones
  map<Referential *, rc_ptr<AObject> >::iterator ior, eor = arefs.end();
  for( ior=arefs.begin(); ior!=eor; ++ior )
    registerObject( ior->second.get() );
  map<Transformation *, rc_ptr<AObject> >::iterator iot, eot = atrans.end();
  for( iot=atrans.begin(); iot!=eot; ++iot )
    registerObject( iot->second.get() );

  _referentials = arefs;
  _transformations = atrans;
}


void RefWindow::setSphereView()
{
  _view_mode = Sphere;
  updateReferentialView();
}


void RefWindow::setFlatView()
{
  _view_mode = Flat;
  updateReferentialView();
}

