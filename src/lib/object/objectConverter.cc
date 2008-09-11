/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
 */

#include <anatomist/object/objectConverter.h>
#include <aims/bucket/bucket.h>
#include <anatomist/bucket/Bucket.h>
#include <aims/mesh/surface.h>
#include <anatomist/surface/triangulated.h>
#include <anatomist/volume/Volume.h>
#include <aims/mesh/texture.h>
#include <anatomist/surface/texture.h>
#include <anatomist/graph/Graph.h>

using namespace anatomist;
using namespace aims;
using namespace carto;

namespace anatomist
{

template<> AObject* 
ObjectConverter<BucketMap<Void> >::aims2ana( BucketMap<Void> *x )
{
  Bucket	*y = new Bucket;
  y->setBucket( *x );
  delete x;
  y->setGeomExtrema();

  return( y );
}


template<> AObject* 
ObjectConverter<BucketMap<Void> >::aims2ana( rc_ptr<BucketMap<Void> > x )
{
  Bucket	*y = new Bucket;
  y->setBucket( x );
  y->setGeomExtrema();

  return( y );
}


template<> BucketMap<Void>* 
ObjectConverter<BucketMap<Void> >::ana2aims( AObject *x )
{
  Bucket	*y = dynamic_cast<Bucket *>( x );
  if( !y )
    return( 0 );
  return( &y->bucket() );
}


template<> bool
ObjectConverter<BucketMap<Void> >::setAims( AObject* x,
                                            rc_ptr<BucketMap<Void> > y )
{
  Bucket *bk = dynamic_cast<Bucket *>( x );
  if( !bk )
    return false;
  bk->setBucket( y );
  return true;
}


template<> AObject*
ObjectConverter<AimsBucket<Void> >::aims2ana( AimsBucket<Void> *x )
{
  Bucket	*y = new Bucket;
  y->setBucket( *x );
  delete x;
  y->setGeomExtrema();

  return( y );
}


template<> AObject* 
ObjectConverter<AimsBucket<Void> >::aims2ana( rc_ptr<AimsBucket<Void> > x )
{
  Bucket	*y = new Bucket;
  rc_ptr<BucketMap<Void> > b( new BucketMap<Void>( *x ) );
  y->setBucket( b );
  y->setGeomExtrema();

  return( y );
}


template<> AObject* 
ObjectConverter<AimsSurfaceTriangle>::aims2ana( AimsSurfaceTriangle *x )
{
  ATriangulated	*y = new ATriangulated( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> AObject* 
ObjectConverter<AimsSurfaceTriangle>::aims2ana( rc_ptr<AimsSurfaceTriangle> x )
{
  ATriangulated	*y = new ATriangulated( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> bool
ObjectConverter<AimsSurfaceTriangle>::setAims( AObject* x,
                                               rc_ptr<AimsSurfaceTriangle> y )
{
  ATriangulated *ana = dynamic_cast<ATriangulated *>( x );
  if( !ana )
    return false;
  ana->setSurface( y );
  return true;
}


template<> AObject* 
ObjectConverter<AimsTimeSurface<2, Void> >::aims2ana( AimsTimeSurface<2, Void> 
                                                      *x )
{
  ASurface<2>	*y = new ASurface<2>( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> AObject* 
ObjectConverter<AimsTimeSurface<2, Void> >::aims2ana( 
    rc_ptr<AimsTimeSurface<2, Void> > x )
{
  ASurface<2>	*y = new ASurface<2>( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> bool
ObjectConverter<AimsTimeSurface<2, Void> >::setAims
( AObject* x, rc_ptr<AimsTimeSurface<2, Void> > y )
{
  ASurface<2> *ana = dynamic_cast<ASurface<2> *>( x );
  if( !ana )
    return false;
  ana->setSurface( y );
  return true;
}


template<> AObject* 
ObjectConverter<AimsSurfaceFacet>::aims2ana( AimsSurfaceFacet *x )
{
  ASurface<4>	*y = new ASurface<4>( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> AObject* 
ObjectConverter<AimsSurfaceFacet>::aims2ana( rc_ptr<AimsSurfaceFacet> x )
{
  ASurface<4>	*y = new ASurface<4>( "triangulation" );
  y->setSurface( x );

  return( y );
}


template<> bool
ObjectConverter<AimsSurfaceFacet>::setAims
( AObject* x, rc_ptr<AimsSurfaceFacet> y )
{
  ASurface<4> *ana = dynamic_cast<ASurface<4> *>( x );
  if( !ana )
    return false;
  ana->setSurface( y );
  return true;
}


template<> AimsSurfaceTriangle* // rc_ptr<AimsSurfaceTriangle> 
ObjectConverter<AimsSurfaceTriangle>::ana2aims( AObject *x )
{
  ATriangulated	*y = dynamic_cast<ATriangulated *>( x );
  if( !y )
    return( 0 );
  return( y->surface().get() ); // TODO: return the rc_ptr
}


template<> AimsTimeSurface<2, Void> * 
ObjectConverter<AimsTimeSurface<2, Void> >::ana2aims( AObject *x )
{
  ASurface<2>	*y = dynamic_cast<ASurface<2> *>( x );
  if( !y )
    return( 0 );
  return( y->surface().get() ); // TODO: return the rc_ptr
}


template<> AimsSurfaceFacet* 
ObjectConverter<AimsSurfaceFacet>::ana2aims( AObject *x )
{
  ASurface<4>	*y = dynamic_cast<ASurface<4> *>( x );
  if( !y )
    return( 0 );
  return( y->surface().get() ); // TODO: return the rc_ptr
}


template<> AimsData<int8_t>*
ObjectConverter<AimsData<int8_t> >::ana2aims( AObject *x )
{
  AVolume<int8_t>	*y = dynamic_cast<AVolume<int8_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<int8_t> >::setAims
( AObject* x, rc_ptr<AimsData<int8_t> > y )
{
  AVolume<int8_t> *ana = dynamic_cast<AVolume<int8_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<uint8_t>*
ObjectConverter<AimsData<uint8_t> >::ana2aims( AObject *x )
{
  AVolume<uint8_t>	*y = dynamic_cast<AVolume<uint8_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<uint8_t> >::setAims
( AObject* x, rc_ptr<AimsData<uint8_t> > y )
{
  AVolume<uint8_t> *ana = dynamic_cast<AVolume<uint8_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<int16_t>*
ObjectConverter<AimsData<int16_t> >::ana2aims( AObject *x )
{
  AVolume<int16_t>	*y = dynamic_cast<AVolume<int16_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<int16_t> >::setAims
( AObject* x, rc_ptr<AimsData<int16_t> > y )
{
  AVolume<int16_t> *ana = dynamic_cast<AVolume<int16_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<uint16_t>*
ObjectConverter<AimsData<uint16_t> >::ana2aims( AObject *x )
{
  AVolume<uint16_t>	*y = dynamic_cast<AVolume<uint16_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<uint16_t> >::setAims
( AObject* x, rc_ptr<AimsData<uint16_t> > y )
{
  AVolume<uint16_t> *ana = dynamic_cast<AVolume<uint16_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<int32_t>*
ObjectConverter<AimsData<int32_t> >::ana2aims( AObject *x )
{
  AVolume<int32_t>	*y = dynamic_cast<AVolume<int32_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<int32_t> >::setAims
( AObject* x, rc_ptr<AimsData<int32_t> > y )
{
  AVolume<int32_t> *ana = dynamic_cast<AVolume<int32_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<uint32_t>*
ObjectConverter<AimsData<uint32_t> >::ana2aims( AObject *x )
{
  AVolume<uint32_t>	*y = dynamic_cast<AVolume<uint32_t> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<uint32_t> >::setAims
( AObject* x, rc_ptr<AimsData<uint32_t> > y )
{
  AVolume<uint32_t> *ana = dynamic_cast<AVolume<uint32_t> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<float>*
ObjectConverter<AimsData<float> >::ana2aims( AObject *x )
{
  AVolume<float>	*y = dynamic_cast<AVolume<float> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<float> >::setAims
( AObject* x, rc_ptr<AimsData<float> > y )
{
  AVolume<float> *ana = dynamic_cast<AVolume<float> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<double>*
ObjectConverter<AimsData<double> >::ana2aims( AObject *x )
{
  AVolume<double>	*y = dynamic_cast<AVolume<double> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<double> >::setAims
( AObject* x, rc_ptr<AimsData<double> > y )
{
  AVolume<double> *ana = dynamic_cast<AVolume<double> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<AimsRGB>*
ObjectConverter<AimsData<AimsRGB> >::ana2aims( AObject *x )
{
  AVolume<AimsRGB>	*y = dynamic_cast<AVolume<AimsRGB> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<AimsRGB> >::setAims
( AObject* x, rc_ptr<AimsData<AimsRGB> > y )
{
  AVolume<AimsRGB> *ana = dynamic_cast<AVolume<AimsRGB> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> AimsData<AimsRGBA>*
ObjectConverter<AimsData<AimsRGBA> >::ana2aims( AObject *x )
{
  AVolume<AimsRGBA>	*y = dynamic_cast<AVolume<AimsRGBA> *>( x );
  if( !y )
    return( 0 );
  return( y->volume().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<AimsData<AimsRGBA> >::setAims
( AObject* x, rc_ptr<AimsData<AimsRGBA> > y )
{
  AVolume<AimsRGBA> *ana = dynamic_cast<AVolume<AimsRGBA> *>( x );
  if( !ana )
    return false;
  ana->setVolume( y );
  return true;
}


template<> TimeTexture<float>*
ObjectConverter<TimeTexture<float> >::ana2aims( AObject *x )
{
  ATexture     *y = dynamic_cast<ATexture *>( x );
  if( !y || y->dimTexture() != 1 )
    return 0;
  return( y->texture<float>().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<TimeTexture<float> >::setAims
( AObject* x, rc_ptr<TimeTexture<float> > y )
{
  ATexture *ana = dynamic_cast<ATexture *>( x );
  if( !ana )
    return false;
  ana->setTexture( y );
  return true;
}


template<> TimeTexture<Point2df>*
ObjectConverter<TimeTexture<Point2df> >::ana2aims( AObject *x )
{
  ATexture     *y = dynamic_cast<ATexture *>( x );
  if( !y || y->dimTexture() != 2 )
    return 0;
  return( y->texture<Point2df>().get() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<TimeTexture<Point2df> >::setAims
( AObject* x, rc_ptr<TimeTexture<Point2df> > y )
{
  ATexture *ana = dynamic_cast<ATexture *>( x );
  if( !ana )
    return false;
  ana->setTexture( y );
  return true;
}


template<> Graph*
ObjectConverter<Graph>::ana2aims( AObject *x )
{
  AGraph     *y = dynamic_cast<AGraph *>( x );
  if( !y )
    return 0;
  return( y->graph() ); // TODO: return the rc_ptr
}


template<> bool
ObjectConverter<Graph>::setAims
( AObject* x, rc_ptr<Graph> y )
{
  AGraph *ana = dynamic_cast<AGraph *>( x );
  if( !ana )
    return false;
  ana->setGraph( y );
  return true;
}

}



template class ObjectConverter<BucketMap<Void> >;
template class ObjectConverter<AimsBucket<Void> >;
template class ObjectConverter<AimsSurfaceTriangle>;
template class ObjectConverter<AimsTimeSurface<2, Void> >;
template class ObjectConverter<AimsSurfaceFacet>;
template class ObjectConverter<AimsData<int8_t> >;
template class ObjectConverter<AimsData<uint8_t> >;
template class ObjectConverter<AimsData<int16_t> >;
template class ObjectConverter<AimsData<uint16_t> >;
template class ObjectConverter<AimsData<int32_t> >;
template class ObjectConverter<AimsData<uint32_t> >;
template class ObjectConverter<AimsData<float> >;
template class ObjectConverter<AimsData<double> >;
template class ObjectConverter<AimsData<AimsRGB> >;
template class ObjectConverter<AimsData<AimsRGBA> >;
template class ObjectConverter<TimeTexture<float> >;
template class ObjectConverter<TimeTexture<Point2df> >;


