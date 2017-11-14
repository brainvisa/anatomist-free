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


#include <anatomist/mobject/MObject.h>

#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window/viewstate.h>
#include <float.h>

// uncomment this to enable debug output for update pattern
// #define ANA_DEBUG_UPDATE

using namespace anatomist;
using namespace aims;
using namespace std;


MObject::~MObject()
{
  /*	Children objects cannot be removed in MObject destructor
	because they have to be destroyed in the specific destructors of 
	derivated classes, which is called *before* ~MObject, 
	so no objects (and even no list) exist any longer now.
  */
  cleanup();
}


float MObject::MinT() const
{
  const_iterator i = begin(), j = end();
  float	mint, mint2;
  if( i == j ) return( 0 );
  mint = (*i)->MinT();
  for( ++i; i!=j; ++i )
    {
      mint2 = (*i)->MinT();
      if( mint2 < mint ) mint = mint2;
    }
  return( mint );
}


float MObject::MaxT() const
{
  const_iterator i = begin(), j = end();
  float	maxt, maxt2;
  if( i == j ) return( 0 );
  maxt = (*i)->MaxT();
  for( ++i; i!=j; ++i )
    {
      maxt2 = (*i)->MaxT();
      if( maxt2 > maxt ) maxt = maxt2;
    }
  return( maxt );
}


//

float MObject::MinX2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return m[0] / voxelSize()[0];
}


float MObject::MinY2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return m[1] / voxelSize()[1];
}


float MObject::MinZ2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return m[2] / voxelSize()[2];
}


float MObject::MaxX2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return M[0] / voxelSize()[0];
}


float MObject::MaxY2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return M[1] / voxelSize()[1];
}


float MObject::MaxZ2D() const
{
  vector<float> m, M;
  if( !boundingBox2D( m, M ) )
    return 0;
  return M[2] / voxelSize()[2];
}


//

void MObject::update( const Observable* o, void* arg )
{
  // cout << "MObject::update, this: " << this << endl;
  AObject::update( o, arg );
  const AObject	*ao = dynamic_cast<const AObject*>( o );
  if( ao )
    {
      updateSubObjectReferential( ao );
      // setChanged(); // TODO: don't always do that
      notifyObservers( this );
    }
}


void MObject::updateSubObjectReferential( const AObject* ao )
{
  if( ao->obsHasChanged( GLComponent::glREFERENTIAL ) )
    {
#ifdef ANA_DEBUG_UPDATE
      cout << "object " << ao->name() << " (" << ao 
           << ") has changed ref in MObject " << name() << " (" 
           << this << ")\n";
#endif
      const Referential 
        *r1 = getReferential(), *r2 = ao->previousReferential();
      if( obsHasChanged( GLComponent::glREFERENTIAL ) )
        r1 = previousReferential();
#ifdef ANA_DEBUG_UPDATE
      cout << "MObject ref: " << r1 << endl;
#endif
      ATransformSet	*ts = ATransformSet::instance();
      if( r1 )
        {
#ifdef ANA_DEBUG_UPDATE
          cout << "old ref: " << r2 << endl;
#endif
          if( r2 )
            ts->unregisterObserver( r1, r2, this );
        }
      if( obsHasChanged( GLComponent::glREFERENTIAL ) )
        r1 = getReferential();
      if( r1 )
        {
          r2 = ao->getReferential();
#ifdef ANA_DEBUG_UPDATE
          cout << "new ref: " << r2 << endl;
#endif
          if( r2 )
            {
#ifdef ANA_DEBUG_UPDATE
              cout << "register obs " << r1 << " - " << r2 << endl;
#endif
              ts->registerObserver( r1, r2, this );
            }
        }
      GLComponent	*glc = glAPI();
      if( glc )
        glc->glSetChanged( GLComponent::glREFERENTIAL );
      else
        obsSetChanged( GLComponent::glREFERENTIAL );
    }
}


void MObject::setContentChanged() const
{ 
  _contentHasChanged = true; 
  setChanged(); 
}


bool MObject::hasContentChanged() const
{ 
  return(_contentHasChanged); 
}


void MObject::clearHasChangedFlags() const
{
  AObject::clearHasChangedFlags();
  _contentHasChanged = false;
}


bool MObject::render( PrimList & prim, const ViewState & state )
{
  bool retcode = false;

  const_iterator i, j = end();
  for( i=begin(); i!=j; ++i )
    if( (*i)->Is2DObject()
        && (*i)->render( prim, state ) )
      retcode = true;

  return retcode;
}


bool MObject::renderingIsObserverDependent() const
{
  const_iterator i, e = end();
  for( i=begin(); i!=e; ++i )
    if( (*i)->renderingIsObserverDependent() )
      return true;
  return false;
}


void MObject::setPalette( const AObjectPalette & pal )
{
  AObject::setPalette( pal );

  iterator	i, f=end();
  for( i=begin(); i!=f; ++i )
    (*i)->setPalette( pal );
}


void MObject::SetMaterial( const Material & mat )
{
  AObject::SetMaterial( mat );

  iterator	i, f=end();
  for( i=begin(); i!=f; ++i )
    (*i)->SetMaterial( mat );
}


AObject* 
MObject::ObjectAt( float x, float y, float z, float t, float tol )
{
  //	By default, look into every child

  const_iterator	io, fo=end();
  AObject		*obj = 0;

  for( io=begin(); io!=end(); ++io )
    {
      obj = (*io)->ObjectAt( x, y, z, t, tol );
      if( obj ) return( this );	// found one
    }
  return( 0 );	// not found
}


bool MObject::Is2DObject()
{
  //	By default, look into every child

  const_iterator	io, fo=end();

  for( io=begin(); io!=end(); ++io )
    {
      if( (*io)->Is2DObject() )
	return( true );	// found one
    }
  return( false );	// not found
}


bool MObject::Is3DObject()
{
  //	By default, look into every child

  const_iterator	io, fo=end();

  for( io=begin(); io!=end(); ++io )
    {
      if( (*io)->Is3DObject() )
	return( true );	// found one
    }
  return( false );	// not found
}


bool MObject::isTransparent() const
{
  const_iterator	io, fo=end();

  for( io=begin(); io!=end(); ++io )
    {
      if( (*io)->isTransparent() )
	return true;	// found one
    }
  return false;	// not found
}


vector<float> MObject::voxelSize() const
{
  const_iterator	io, fo=end();
  vector<float>		vs, vs2;
  bool			first = true;
  unsigned i, n;

  for( io=begin(); io!=end(); ++io )
    if( (*io)->Is2DObject() )
    {
      if( first )
        vs = (*io)->voxelSize();
      else
      {
        vs2 = (*io)->voxelSize();
        for( i=0, n=std::min( vs.size(), vs2.size() ); i<n; ++i )
        {
          if( vs2[i] < vs[i] )
            vs[i] = vs2[i];
        }
        if( n < vs2.size() )
          vs.insert( vs.end(), vs2.begin() + n, vs2.end() );
      }
    }

  return( vs );
}


bool MObject::boundingBox( vector<float> & bmin, vector<float> & bmax ) const
{
  const_iterator	i, j = end();
  Referential		*r = getReferential(), *oref;
  Transformation	*tr;
  AObject		*o;
  vector<float>		pmin, pmax;
  bool			valid = false;
  unsigned k, n;
  bmin.resize( 4 );
  bmax.resize( 4 );

  for( i=begin(); i!=j; ++i )
  {
    o = *i;

    if( o->boundingBox( pmin, pmax ) )
    {
      if( r && ( oref = o->getReferential() )
          && ( tr = theAnatomist->getTransformation( oref, r ) ) )
      {
        Point3df ppmino, ppmaxo, pmino2, pmaxo2;
        ppmino = Point3df( pmin[0], pmin[1], pmin[2] );
        ppmaxo = Point3df( pmax[0], pmax[1], pmax[2] );
        tr->transformBoundingBox( ppmino, ppmaxo, pmino2, pmaxo2 );
        pmin[0] = pmino2[0];
        pmin[1] = pmino2[1];
        pmin[2] = pmino2[2];
      }
      if( !valid )
      {
        bmin = pmin;
        bmax = pmax;
      }
      else
      {
        for( k=0, n=std::min( pmin.size(), bmin.size() ); k<n; ++k )
        {
          if( pmin[k] < bmin[k] )
            bmin[k] = pmin[k];
          if( pmax[k] > bmax[k] )
          bmax[k] = pmax[k];
        }
        if( n < pmin.size() )
        {
          bmin.insert( bmin.end(), pmin.begin() + n, pmin.end() );
          bmax.insert( bmax.end(), pmax.begin() + n, pmax.end() );
        }
      }
      valid = true;
    }
  }

  return valid;
}


bool MObject::boundingBox2D( vector<float> & bmin, vector<float> & bmax ) const
{
  const_iterator        i, j = end();
  Referential           *r = getReferential(), *oref;
  Transformation        *tr;
  AObject               *o;
  vector<float>              pmin, pmax, pmino, pmaxo;
  bool                  valid = false;
  unsigned k, n;

  for( i=begin(); i!=j; ++i )
  {
    o = *i;

    if( o->boundingBox2D( pmin, pmax ) )
    {
      if( r && ( oref = o->getReferential() )
          && ( tr = theAnatomist->getTransformation( oref, r ) ) )
      {
        Point3df pmino, pmaxo, pmino2, pmaxo2;
        pmino = Point3df( pmin[0], pmin[1], pmin[2] );
        pmaxo = Point3df( pmax[0], pmax[1], pmax[2] );
        tr->transformBoundingBox( pmino, pmaxo, pmino2, pmaxo2 );
        pmin[0] = pmino2[0];
        pmin[1] = pmino2[1];
        pmin[2] = pmino2[2];
        pmax[0] = pmaxo2[0];
        pmax[1] = pmaxo2[1];
        pmax[2] = pmaxo2[2];
      }
      if( !valid )
      {
        bmin = pmin;
        bmax = pmax;
      }
      else
      {
        for( k=0, n=std::min(bmin.size(), pmin.size() ); k<n; ++k )
        {
          if( pmin[k] < bmin[k] )
            bmin[k] = pmin[k];
          if( pmax[k] > bmax[k] )
            bmax[k] = pmax[k];
        }
        if( bmin.size() > n )
        {
          pmin.insert( pmin.end(), bmin.begin() + n, bmin.end() );
          pmax.insert( pmax.end(), bmax.begin() + n, bmax.end() );
        }
      }
      valid = true;
    }
  }

  return valid;
}


MObject* MObject::mObjectAPI()
{
  return this;
}


const MObject* MObject::mObjectAPI() const
{
  return this;
}


void MObject::_insertObject( AObject* o )
{
  // cout << "MObject::_insertObject\n";
  Referential	*r1 = getReferential();
  if( r1 )
    {
      Referential	*r2 = o->getReferential();
      if( r2 )
        ATransformSet::instance()->registerObserver( r1, r2, this );
    }
  o->RegisterParent( this );
  if( o != referentialInheritance() )
    o->addObserver( this );
  _contentHasChanged = true;
  setChanged();
}


void MObject::_eraseObject( AObject* o )
{
  o->UnregisterParent( this );
  if( o != referentialInheritance() )
    o->deleteObserver( this );
  Referential	*r1 = getReferential();
  if( r1 )
    {
      Referential	*r2 = o->getReferential();
      if( r2 )
        ATransformSet::instance()->unregisterObserver( r1, r2, this );
    }
  _contentHasChanged = true;
  setChanged();
}


void MObject::clearReferentialInheritance()
{
  AObject    *o = referentialInheritance();
  if( !o )
    return;

  _referentialInheritance() = 0;
  iterator   i, e = end();
  for( i=begin(); i!=end(); ++i )
    if( (*i) == o )
      return; // don't unregister object if it is a child
  o->deleteObserver( this );
}


void MObject::setReferentialInheritance( AObject* p )
{
  if( !p )
  {
    setReferential( 0 );
    return;
  }

  Referential  *ref = p->getReferential();
  if( ref && ref->destroying() )
    ref = theAnatomist->centralReferential();
  if( p == referentialInheritance() && ref == _referential )
    return;

  Referential   *old = _referential, *r2;
  ATransformSet	*ts = ATransformSet::instance();
  iterator	io, eo = end();

  if( old && old != ref )
    // remove all observers for me
    for( io=begin(); io!=eo; ++io )
      {
        r2 = (*io)->getReferential();
        if( r2 )
          ts->unregisterObserver( old, r2, this );
      }
  
  AObject::setReferentialInheritance( p );
  
  if( ref && ref != old )
    // put back observers to all objects
    for( io=begin(); io!=eo; ++io )
      {
        r2 = (*io)->getReferential();
        /* cout << "observing trans change for " << (*io)->name() << " btw "
                << ref << " and " << r2
          << endl; */
        if( r2 )
          ts->registerObserver( ref, r2, this );
      }
}


void MObject::setReferential( Referential* ref )
{
  if( ref == _referential && !referentialInheritance() )
    return;
  
  Referential   *old = _referential, *r2;

  ATransformSet	*ts = ATransformSet::instance();
  iterator	io, eo = end();

  if( old && old != ref )
    // remove all observers for me
    for( io=begin(); io!=eo; ++io )
      {
        r2 = (*io)->getReferential();
        if( r2 )
          ts->unregisterObserver( old, r2, this );
      }

  AObject::setReferential( ref );

  if( ref && ref != old )
    // put back observers to all objects
    for( io=begin(); io!=eo; ++io )
      {
        r2 = (*io)->getReferential();
        /* cout << "observing trans change for " << (*io)->name() << " btw "
                << ref << " and " << r2
          << endl; */
        if( r2 )
          ts->registerObserver( ref, r2, this );
      }
}


bool MObject::shouldRemoveChildrenWithMe() const
{
  return false;
}


list<AObject *> MObject::generativeChildren() const
{
  list<AObject *> children;
  const_iterator i, e = end();
  for( i=begin(); i!=e; ++i )
    children.push_back( *i );
  return children;
}

