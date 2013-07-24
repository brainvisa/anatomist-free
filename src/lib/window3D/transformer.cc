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


#include <anatomist/window3D/transformer.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidget.h>
#include <anatomist/selection/selectFactory.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/window3D/boxviewslice.h>
#include <anatomist/surface/surface.h>
#include <aims/mesh/surfacegen.h>
#include <aims/mesh/surfaceOperation.h>
#include <iostream>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


struct Transformer::Private
{
  rc_ptr<BoxViewSlice> box1;
  rc_ptr<BoxViewSlice> box2;
};


struct TranslaterAction::Private
{
  rc_ptr<BoxViewSlice> box1;
  rc_ptr<BoxViewSlice> box2;
};


namespace
{

  rc_ptr<AObject> trieder( float colr, float colg, float colb, float cola )
  {
    AimsTimeSurface<2,Void> *cross = new AimsTimeSurface<2,Void>;
    vector<Point3df> & vert = (*cross)[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = (*cross)[0].polygon();
    vert.reserve(4);
    poly.reserve(3);
    vert.push_back( Point3df( 0, 0, 0 ) );
    vert.push_back( Point3df( 100, 0, 0 ) );
    vert.push_back( Point3df( 0, 100, 0 ) );
    vert.push_back( Point3df( 0, 0, 100 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 1 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 2 ) );
    poly.push_back( AimsVector<uint32_t,2>( 0, 3 ) );
    ASurface<2>* amesh = new ASurface<2>;
    rc_ptr<AObject> rmesh( amesh );
    amesh->setSurface( cross );
    Material & mat = amesh->GetMaterial();
    mat.SetDiffuse( colr, colg, colb, cola );
    mat.setRenderProperty( Material::Ghost, 1 );
    mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    mat.setLineWidth( 2. );
    amesh->SetMaterial( mat );
    amesh->setName( theAnatomist->makeObjectName( "trieder" ) );
    theAnatomist->registerObject( amesh, false );
    theAnatomist->releaseObject( amesh );
    return rmesh;
  }

  void initBoxes( BoxViewSlice & box1, BoxViewSlice & box2 )
  {
    box2.setCubeColor( 0., 1., 0.5, 1. );
    box2.setPlaneColor( 0.2, 0.6, 0.2, 1. );
    box1.enablePlane( false );
    box2.enablePlane( false );
    box1.enableText( false );
    box2.enableText( false );
    box1.addObject( trieder( 1., 0.5, 0., 1. ) );
    box2.addObject( trieder( 0.2, 0.6, 0.2, 1. ) );
  }


  void updateAxisWithCircles( AimsTimeSurface<2,Void> &mesh,
                              const Point3df & p0, const Quaternion & rotation,
                              float radius, float circlespacing )
  {
    Point3df axis = rotation.axis();
//     float angle = rotation.angle();
    mesh[0].vertex().clear();
    mesh[0].polygon().clear();
    mesh[0].normal().clear();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    AimsTimeSurface<2,Void> *mesh2
      = SurfaceGenerator::circle_wireframe( p0, radius, 20, axis, startdir );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    mesh2 = SurfaceGenerator::circle_wireframe( p0 + axis * circlespacing,
                                                radius, 20, axis, startdir );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    mesh2 = SurfaceGenerator::circle_wireframe( p0 - axis * circlespacing,
                                                radius, 20, axis, startdir );
    SurfaceManip::meshMerge( mesh, *mesh2 );
    delete mesh2;
    // add axis
    vector<Point3df> & vert = mesh[0].vertex();
    vector<AimsVector<uint32_t,2> > & poly = mesh[0].polygon();
    poly.push_back( AimsVector<uint32_t,2>( vert.size(), vert.size()+1 ) );
    vert.push_back( p0 - axis * circlespacing * 2 );
    vert.push_back( p0 + axis * circlespacing * 2 );
  }


  rc_ptr<AObject> axisWithCircles( const Point3df & p0,
                                   const Quaternion & rotation,
                                   float radius, float circlespacing,
                                   float colr, float colg,
                                   float colb, float cola )
  {
    Point3df axis = rotation.axis();
    float angle = rotation.angle();
    Point3df startdir = rotation.transformInverse( Point3df( 1, 0, 0 ) );
    AimsTimeSurface<2,Void> *mesh = new AimsTimeSurface<2,Void>;
    updateAxisWithCircles( *mesh, p0, rotation, radius, circlespacing );

    ASurface<2>* amesh = new ASurface<2>;
    rc_ptr<AObject> rmesh( amesh );
    amesh->setSurface( mesh );
    Material & mat = amesh->GetMaterial();
    mat.SetDiffuse( colr, colg, colb, cola );
    mat.setRenderProperty( Material::Ghost, 1 );
    mat.setRenderProperty( Material::RenderMode, Material::Wireframe );
    mat.setLineWidth( 2. );
    amesh->SetMaterial( mat );
    amesh->setName( theAnatomist->makeObjectName( "trieder" ) );
    theAnatomist->registerObject( amesh, false );
    theAnatomist->releaseObject( amesh );
    return rmesh;
  }

}


Transformer::Transformer() : Trackball(), d( new Private )
{
  d->box1.reset( new BoxViewSlice( this ) );
  d->box2.reset( new BoxViewSlice( this ) );
  initBoxes( *d->box1, *d->box2 );
}


Transformer::Transformer( const Transformer & a ) : Trackball( a )
{
}


Transformer::~Transformer()
{
  delete d;
}


Action* Transformer::creator()
{
  return( new Transformer );
}


string Transformer::name() const
{
  return( "Transformer" );
}


void Transformer::beginTrackball( int x, int y, int globalX, int globalY )
{
  Trackball::beginTrackball( x, y, globalX, globalY );
  const std::map<unsigned, set<AObject *> > 
    & sel = SelectFactory::factory()->selected();
  map<unsigned, set<AObject *> >::const_iterator 
    is = sel.find( view()->aWindow()->Group() );
  if( is == sel.end() )
    return;

  const set<AObject *>			& obj = is->second;
  set<AObject *>			nobj, iobj, cobj;
  set<AObject *>::const_iterator	io, eo = obj.end();
  Referential	*ref, *cref = theAnatomist->centralReferential();
  Transformation			*t;

  _trans.clear();
  _itrans.clear();

  for( io=obj.begin(); io!=eo; ++io )
  {
    ref = (*io)->getReferential();
    t = theAnatomist->getTransformation( ref, cref );
    if( t && !t->isGenerated() )
    {
      cobj.insert( *io );
      _trans[ t ] = *t;
    }
    else
    {
      t = theAnatomist->getTransformation( cref, ref );
      if( t && !t->isGenerated() )
      {
        iobj.insert( *io );
        _itrans[ t ] = *t;
      }
      else
        nobj.insert( *io );
    }
  }

  if( !nobj.empty() )
  {
    set<AWindow *> wins;
    AssignReferentialCommand	*com = new AssignReferentialCommand( 0, nobj, 
                                                                    wins );
    theProcessor->execute( com );
    ref = com->ref();
    cobj.insert( nobj.begin(), nobj.end() );
    float		matvec[4][3];
    matvec[0][0] = 0;
    matvec[0][1] = 0;
    matvec[0][2] = 0;
    matvec[1][0] = 1;
    matvec[1][1] = 0;
    matvec[1][2] = 0;
    matvec[2][0] = 0;
    matvec[2][1] = 1;
    matvec[2][2] = 0;
    matvec[3][0] = 0;
    matvec[3][1] = 0;
    matvec[3][2] = 1;
    LoadTransformationCommand	*tc 
      = new LoadTransformationCommand( matvec, ref, cref );
    theProcessor->execute( tc );
    _trans[ tc->trans() ] = *tc->trans();
  }

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( w && d->box2->additionalObjects().size() <= 1 )
  {
    rc_ptr<AObject> axis = axisWithCircles( w->rotationCenter(), Quaternion(),
                                            70, 100, 0.8, 0.3, 0.2, 1. );
    d->box2->addObject( axis );
  }

  d->box1->setObjectsReferential( ref );
  d->box2->setObjectsReferential( cref );
  d->box1->beginTrackball( x, y );
  d->box2->beginTrackball( x, y );
}


Quaternion Transformer::rotation( int x, int y )
{
  if( _beginx < 0 || _beginy < 0 )
    return( Quaternion( 0, 0, 0, 1 ) );

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "Transformer operating on wrong view type -- error\n";
      return( Quaternion( 0, 0, 0, 1 ) );
    }

  float	dimx = w->width();
  float	dimy = w->height();

  Quaternion	q 
    = w->quaternion().inverse() * 
    initQuaternion( ( 2. * _beginx - dimx ) / dimx,
		    ( dimy - 2. * _beginy ) / dimy,
		    ( 2. * x - dimx ) / dimx,
		    ( dimy - 2. * y ) / dimy ).inverse() 
    * w->quaternion();

  Point4df	vec = q.vector();
  // we must invert Z since we're transforming to an indirect referential
  float zfac = w->invertedZ() ? 1 : -1;
  q = Quaternion( zfac * vec[0], zfac * vec[1], -vec[2], vec[3] );

  //cout << "rotation axis : " << q.axis() << ", angle : " 
  //     << q.angle() * 180. / M_PI << endl;
  return q;
}


void Transformer::moveTrackball( int x, int y, int, int )
{
  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
  {
    cerr << "Transformer operating on wrong view type -- error\n";
    return;
  }

  Quaternion	q = rotation( x, y );
  q.norm();

  Transformation	t( 0, 0 );
  t.setQuaternion( q.inverse() );
  Point3df t0 = w->rotationCenter();
  t0 -= t.transform( t0 ); // (I-R) t0
  t.SetTranslation( 0, t0[0] );
  t.SetTranslation( 1, t0[1] );
  t.SetTranslation( 2, t0[2] );

  map<Transformation*, Transformation>::iterator	it, et = _trans.end();
  for( it=_trans.begin(); it!=et; ++it )
  {
    it->first->unregisterTrans();
    *it->first = t;
    *it->first *= it->second;
    it->first->registerTrans();
  }

  if( !_itrans.empty() )
  {
    t.invert();
    for( it=_itrans.begin(), et=_itrans.end(); it!=et; ++it )
    {
      it->first->unregisterTrans();
      *it->first = t;
      *it->first *= it->second;
      it->first->registerTrans();
    }
  }


  list<rc_ptr<AObject> > & addobj = d->box2->additionalObjects();
  ASurface<2> *axis = dynamic_cast<ASurface<2> *>( addobj.rbegin()->get() );
  if( axis )
  {
    updateAxisWithCircles( *axis->surface(), w->rotationCenter(), q, 70, 100 );
    axis->glSetChanged( GLComponent::glGEOMETRY );
  }
//   d->box1->moveTrackball( x, y );
//   d->box2->moveTrackball( x, y );
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w3 )
    w3->refreshNow();
}


void Transformer::endTrackball( int x, int y, int globx, int globy )
{
  d->box1->endTrackball( x, y );
  d->box2->endTrackball( x, y );
  Trackball::endTrackball( x, y, globx, globy );
}

// ------------------


TranslaterAction::TranslaterAction() : Action(), d( new Private )
{
  d->box1.reset( new BoxViewSlice( this ) );
  d->box2.reset( new BoxViewSlice( this ) );
  initBoxes( *d->box1, *d->box2 );
}


TranslaterAction::~TranslaterAction()
{
  delete d;
}


TranslaterAction::TranslaterAction( const TranslaterAction & a ) 
  : Action( a )
{
}


Action* TranslaterAction::creator()
{
  return( new TranslaterAction );
}


string TranslaterAction::name() const
{
  return( "TranslaterAction" );
}


void TranslaterAction::begin( int x, int y, int, int )
{
  const std::map<unsigned, set<AObject *> > 
    & sel = SelectFactory::factory()->selected();
  map<unsigned, set<AObject *> >::const_iterator 
    is = sel.find( view()->aWindow()->Group() );
  if( is == sel.end() )
    return;

  const set<AObject *>			& obj = is->second;
  set<AObject *>			nobj, iobj, cobj;
  set<AObject *>::const_iterator	io, eo = obj.end();
  Referential	*ref, *cref = theAnatomist->centralReferential();
  Transformation			*t;

  _trans.clear();
  _itrans.clear();
  _started = true;
  _beginx = x;
  _beginy = y;

  for( io=obj.begin(); io!=eo; ++io )
  {
    ref = (*io)->getReferential();
    t = theAnatomist->getTransformation( ref, cref );
    if( t && !t->isGenerated() )
    {
      cobj.insert( *io );
      _trans[ t ] = *t;
    }
    else
    {
      t = theAnatomist->getTransformation( cref, ref );
      if( t && !t->isGenerated() )
      {
        iobj.insert( *io );
        _itrans[ t ] = *t;
      }
      else
        nobj.insert( *io );
    }
  }

  if( !nobj.empty() )
  {
    set<AWindow *> wins;
    AssignReferentialCommand	*com = new AssignReferentialCommand( 0, nobj, 
                                                                    wins );
    theProcessor->execute( com );
    ref = com->ref();
    cobj.insert( nobj.begin(), nobj.end() );
    float		matvec[4][3];
    matvec[0][0] = 0;
    matvec[0][1] = 0;
    matvec[0][2] = 0;
    matvec[1][0] = 1;
    matvec[1][1] = 0;
    matvec[1][2] = 0;
    matvec[2][0] = 0;
    matvec[2][1] = 1;
    matvec[2][2] = 0;
    matvec[3][0] = 0;
    matvec[3][1] = 0;
    matvec[3][2] = 1;
    LoadTransformationCommand	*tc 
      = new LoadTransformationCommand( matvec, ref, cref );
    theProcessor->execute( tc );
    _trans[ tc->trans() ] = *tc->trans();
  }

  d->box1->setObjectsReferential( ref );
  d->box2->setObjectsReferential( cref );
  d->box1->beginTrackball( x, y );
  d->box2->beginTrackball( x, y );
}


void TranslaterAction::move( int x, int y, int, int )
{
  if( !_started )
  {
    cerr << "error: translation not started (BUG)\n";
    return;
  }

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
  {
    cerr << "Translate3DAction operating on wrong view type -- error\n";
    return;
  }

  // cout << "translater move\n";
  float	mx = _beginx - x;
  float	my = y - _beginy;
  Point3df	sz = w->windowBoundingMax() - w->windowBoundingMin();
  float oratio = float(w->width()) / w->height() / sz[0] * sz[1];
  if( oratio <= 1 )
  {
    mx *= sz[0] / w->width();
    my *= sz[1] / w->height() / oratio;
  }
  else
  {
    mx *= sz[0] / w->width() * oratio;
    my *= sz[1] / w->height();
  }
  mx /= w->zoom();
  my /= w->zoom();
  Point3df	p = w->quaternion().inverse().apply( Point3df( mx, my, 0 ) );
  Transformation	t( 0, 0 );
  float zfac = w->invertedZ() ? -1 : 1;
  t.SetTranslation( 0, zfac * p[0] );
  t.SetTranslation( 1, zfac * p[1] );
  t.SetTranslation( 2, p[2] );

  map<Transformation*, Transformation>::iterator	it, et = _trans.end();
  for( it=_trans.begin(); it!=et; ++it )
  {
    it->first->unregisterTrans();
    *it->first = t;
    *it->first *= it->second;
    it->first->registerTrans();
  }

  if( !_itrans.empty() )
  {
    t.invert();
    for( it=_itrans.begin(), et=_itrans.end(); it!=et; ++it )
    {
      it->first->unregisterTrans();
      it->first->setGenerated( true );
      *it->first = t;
      *it->first *= it->second;
      it->first->registerTrans();
    }
  }

//   d->box1->moveTrackball( x, y );
//   d->box2->moveTrackball( x, y );
  AWindow3D    *w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( w3 )
    w3->refreshNow();
  // cout << "translation done\n";
}


void TranslaterAction::end( int x, int y, int, int )
{
  d->box1->endTrackball( x, y );
  d->box2->endTrackball( x, y );
  _started = false;
}


// ----------------------


PlanarTransformer::PlanarTransformer() : Transformer()
{
}


PlanarTransformer::PlanarTransformer( const PlanarTransformer & a ) 
  : Transformer( a )
{
}


PlanarTransformer::~PlanarTransformer()
{
}


Action* PlanarTransformer::creator()
{
  return( new PlanarTransformer );
}


string PlanarTransformer::name() const
{
  return( "PlanarTransformer" );
}


Quaternion PlanarTransformer::rotation( int x, int y )
{
  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );

  if( !w )
    {
      cerr << "PlanarTransformer operating on wrong view type -- error\n";
      return( Quaternion( 0, 0, 0, 1 ) );
    }

  AWindow3D	*w3 = dynamic_cast<AWindow3D *>( view()->aWindow() );
  if( !w3 )
    return Quaternion( 0, 0, 0, 1 );

  Point3df	axis = w3->sliceQuaternion().apply( Point3df( 0, 0, -1 ) );
  float	dimx = w->width();
  float	dimy = w->height();

  // compute an intuitive trackball-compatible rotation angle
  Point3df	axis2 = axis;
  axis2[2] *= -1;
  axis2 = w->quaternion().apply( axis2 ).normalize();
  float x1 = ( 2. * _beginx - dimx ) / dimx;
  float y1 = ( dimy - 2. * _beginy ) / dimy;
  float x2 = ( 2. * x - dimx ) / dimx;
  float y2 = ( dimy - 2. * y ) / dimy;
  //    coords on a sphere (initial, current)
  Point3df	a( x1, y1, tbProj2Sphere( 0.8, x1, y1 ) );
  Point3df	b( x2, y2, tbProj2Sphere( 0.8, x2, y2 ) );
  Point3df	c(0,0,0);// = w->rotationCenter();
  //    referential with 1st point (a-c, v2, axis2)
  Point3df	v3 = crossed( a - c, axis2 ).normalize();
  a = crossed( axis2, v3 ).normalize();
  //    b in this ref (only 2D coords are needed)
  b = Point3df( b.dot( a ), b.dot( v3 ), 0 );
  //    get the angle
  float	r = sqrt( b[0] * b[0] + b[1] * b[1] );
  float	angle = acos( b[0] / r );
  if( b[1] > 0 )
    angle *= -1;

  Quaternion	q;
  q.fromAxis( axis, angle );

  return q;
}


// -------------------


ResizerAction::ResizerAction() : TranslaterAction()
{
}


ResizerAction::~ResizerAction()
{
}


ResizerAction::ResizerAction( const ResizerAction & a ) 
  : TranslaterAction( a )
{
}


Action* ResizerAction::creator()
{
  return( new ResizerAction );
}


string ResizerAction::name() const
{
  return( "ResizerAction" );
}


void ResizerAction::move( int /* x */, int y, int, int )
{
  if( !_started )
    {
      cerr << "error: resize not started (BUG)\n";
      return;
    }

  GLWidgetManager * w = dynamic_cast<GLWidgetManager *>( view() );
  if( !w )
    {
      cerr << "ResizeAction operating on wrong view type -- error\n";
      return;
    }

  int	m = _beginy - y;
  float	zfac = exp( 0.01 * m );
  Point3df t0 = w->rotationCenter();

  Transformation	t( 0, 0 );
  t.SetRotation( 0, 0, zfac );
  t.SetRotation( 1, 1, zfac );
  t.SetRotation( 2, 2, zfac );
  t.SetTranslation( 0, t0[0] * ( 1 - zfac ) );
  t.SetTranslation( 1, t0[1] * ( 1 - zfac ) );
  t.SetTranslation( 2, t0[2] * ( 1 - zfac ) );

  map<Transformation*, Transformation>::iterator	it, et = _trans.end();
  for( it=_trans.begin(); it!=et; ++it )
    {
      it->first->unregisterTrans();
      *it->first = t;
      *it->first *= it->second;
      it->first->registerTrans();
    }

  if( !_itrans.empty() )
    {
      t.invert();
      for( it=_itrans.begin(), et=_itrans.end(); it!=et; ++it )
	{
	  it->first->unregisterTrans();
	  it->first->setGenerated( true );
	  *it->first = t;
	  *it->first *= it->second;
	  it->first->registerTrans();
	}
    }

  // cout << "scaling done\n";
}


