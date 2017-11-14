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


#include <cstdlib>
#include <anatomist/mobject/Fusion3D.h>
#include <anatomist/window/Window.h>
#include <anatomist/object/sliceable.h>
#include <anatomist/misc/error.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/object/actions.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window/viewstate.h>
#include <graph/tree/tree.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/application/Anatomist.h>
#include <aims/mesh/mesh_rasterization_d.h>
#include <aims/bucket/bucket.h>
#include <aims/mesh/surfaceOperation.h>
#include <qtranslator.h>
#include <cfloat>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


Tree*	Fusion3D::_optionTree = 0;

struct Fusion3D::Private
{
  //	texture coord vector
  mutable map<string, vector< vector<float> > > vtexture;
  mutable bool refreshVTexture;
};


Fusion3D::Fusion3D( const vector<AObject *> & obj )
  : GLObjectVector(), d( new Private )
{
  using carto::shared_ptr;

  // cout << "Fusion3D::Fusion3D " << this << endl;
  vector<AObject *>::const_iterator	io, fo=obj.end();
  vector<AObject *>			surf, vol;
  GLComponent				*tr;

  //	sort objects: surfaces first, volumes last
  for( io=obj.begin(); io!=fo; ++io )
  {
    tr = (*io)->glAPI();
    if( tr && tr->sliceableAPI() )
      vol.push_back( *io );
    else
      surf.push_back( *io );
  }

  _nsurf = surf.size();
  for( io=surf.begin(), fo=surf.end(); io!=fo; ++io )
    insert( shared_ptr<AObject>( shared_ptr<AObject>::Strong, *io ) );
  for( io=vol.begin(), fo=vol.end(); io!=fo; ++io )
    insert( shared_ptr<AObject>( shared_ptr<AObject>::Strong, *io ) );

  _type = AObject::FUSION3D;

  //setTexRate( 1. );
  //setTexMode( GEOMETRICAL );
  _method = POINT_TO_POINT;
  _submethod = MEAN;
  _depth = 5.0;
  _step = 2.5;
  d->refreshVTexture = true;
  setReferentialInheritance( *begin() );
  unsigned	ntex = vol.size();
  glAddTextures( ntex );
  TexExtrema  & te = GLComponent::glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  setReferentialInheritance( *begin() );
}


Fusion3D::~Fusion3D()
{
  cleanupObserver();
  cleanup();
  delete d;
}


Fusion3D::iterator Fusion3D::firstVolume()
{
  unsigned	i;
  iterator	io;

  for( i=0, io=begin(); i<_nsurf; ++io, ++i ) {}
  return( io );
}


Fusion3D::const_iterator Fusion3D::firstVolume() const
{
  unsigned		i;
  const_iterator	io;

  for( i=0, io=begin(); i<_nsurf; ++io, ++i ) {}
  return( io );
}


bool Fusion3D::Is2DObject()
{
  return (*begin())->Is2DObject();
}


void Fusion3D::setMethod( Method method )
{
  if( _method != method )
    {
      _method = method;
      d->refreshVTexture = true;
      glSetChanged( glBODY );
    }
}


void Fusion3D::setSubMethod( SubMethod submethod )
{
  if( _submethod != submethod )
    {
      _submethod = submethod;
      d->refreshVTexture = true;
      glSetChanged( glBODY );
    }
}


void Fusion3D::setDepth( float depth )
{
  if( _depth != depth )
    {
      _depth = depth;
      d->refreshVTexture = true;
      glSetChanged( glBODY );
    }
}


void Fusion3D::setStep( float step )
{
  if( _step != step )
    {
      _step = step;
      d->refreshVTexture = true;
      glSetChanged( glBODY );
    }
}


void Fusion3D::update( const Observable* observable, void* arg )
{
  // cout << "Fusion3D::update\n";

  AObject::update( observable, arg );
  const AObject	*obj = dynamic_cast<const AObject *>( observable );

  if( !obj )
    {
      const TransformationObserver 
        *to = dynamic_cast<const TransformationObserver *>( observable );
      if( !to )
        return;

      // cout << "Transformation changed in Fusion3D\n";
      unsigned	i, n = glNumTextures();
      for( i=0; i<n; ++i )
        glSetTexImageChanged( true, i );
      d->refreshVTexture = true;
      glSetChanged( glBODY );
      notifyObservers((void*)this);
      return;
    }

  iterator	io, eo = end();
  unsigned	num = 0;
  bool          isvol = false;
  for( io=begin(); io!=eo && obj!=*io; ++io, ++num )
    if( io == firstVolume() )
      isvol = true;
  if( io == eo )
    return;
  if( io == firstVolume() )
    isvol = true;

  const GLComponent	*tr = obj->glAPI();

  if( tr )
    {
      if( isvol )
        {
          // volume changed
          if( obj->obsHasChanged( glTEXIMAGE ) )
            {
              // cout << "teximage " << num - _nsurf << " changed\n";
              glSetTexImageChanged( true, num - _nsurf );
            }
          if( obj->obsHasChanged( glTEXENV ) )
            {
              // cout << "texenv " << num - _nsurf << " changed\n";
              glSetTexEnvChanged( true, num - _nsurf );
            }
          // if the functional volume reference has changed
          if( obj->obsHasChanged( glREFERENTIAL ) 
              || obj->obsHasChanged( glBODY ) )
            {
              // cout << "ref/body " << num - _nsurf << " changed\n";
              d->refreshVTexture = true;
              glSetChanged( glBODY );
            }
        }
      else
        {
          // surface changed
          if( obj->obsHasChanged( glMATERIAL ) )
            glSetChanged( glMATERIAL );
          if( obj->obsHasChanged( glREFERENTIAL ) 
              || obj->obsHasChanged( glGEOMETRY ) )
            {
              // cout << "mesh geometry changed\n";
              glSetChanged( glGEOMETRY );
              d->refreshVTexture = true;
            }
        }
    }
  updateSubObjectReferential( obj );
  notifyObservers((void*)this);
}


void Fusion3D::refreshVTexture( const ViewState & s ) const
{
  /*
  cout << "Fusion3D::refreshVTexture\n";
  cout << "haschanged: " << glHasChanged( glGENERAL ) << ", texchanged: " 
       << glTexImageChanged() << endl;
  */

  if( d->refreshVTexture )
    d->vtexture.clear();

  const GLComponent	*surf = (*begin())->glAPI();
  unsigned		nver = surf->glNumVertex( s );
  string		id = viewStateID( glBODY, s );

  vector< vector<float> >	& vtexture = d->vtexture[id];

  if( !d->refreshVTexture && !vtexture.empty() )
    return;

  // deletion of texture values vectors
  vtexture.erase( vtexture.begin(), vtexture.end() );

  // cout << "texturing mesh of " << nver << " vertices\n";
  unsigned i, n = glNumTextures();
  vtexture.reserve( n );
  for( i=0; i<n; ++i )
    {
      vtexture.push_back( vector<float>() );
      vtexture[i].reserve( nver );

      // compute texture values according to the current method
      switch( _method )
        {
        case POINT_TO_POINT:
        case INSIDE_POINT_TO_POINT:
        case OUTSIDE_POINT_TO_POINT:
          {
            refreshVTextureWithPointToPoint( s, i );
            break;
          }
        case LINE_TO_POINT:
          {
            refreshVTextureWithLineToPoint( s, i );
            break;
          }
        case INSIDE_LINE_TO_POINT:
          {
            refreshVTextureWithInsideLineToPoint( s, i );
            break;
          }
        case OUTSIDE_LINE_TO_POINT:
          {
            refreshVTextureWithOutsideLineToPoint( s, i );
            break;
          }
        case SPHERE_TO_POINT:
          {
            refreshVTextureWithSphereToPoint( s, i );
            break;
          }
        default:
          {
            AError("Fusion3D: bad fusion method");
            break;
          }
        }
    }

  d->refreshVTexture = false;
}


bool Fusion3D::render( PrimList & prim, const ViewState & state )
{
  theAnatomist->setCursor( Anatomist::Working );
  bool x = GLObjectVector::render( prim, state );
  theAnatomist->setCursor( Anatomist::Normal );
  return x;
}


void Fusion3D::glClearHasChangedFlags() const
{
  GLObjectVector::glClearHasChangedFlags();
  d->refreshVTexture = false;
}


const AObject* Fusion3D::volume( unsigned n ) const
{
  const_iterator	v = firstVolume();
  unsigned		i;
  for( i=0; i<n; ++i )
    ++v;
  return *v;
}


void Fusion3D::refreshVTextureWithPointToPoint( const ViewState & s, 
                                                unsigned tex ) const
{
  // cout << "refreshVTextureWithPointToPoint, tex: " << tex << endl;
  //  volume scale
  float min;
  float max;
  float	scale;

  // get surface vector of vertex
  const AObject		*osurf = *begin();
  const GLComponent	*surf = osurf->glAPI();
  const GLfloat		*vver = surf->glVertexArray( s );
  const GLfloat		*vnor = surf->glNormalArray( s );
  unsigned		nver = surf->glNumVertex( s );
  string		id = viewStateID( glBODY, s );
  vector<float>		& vtexture = d->vtexture[id][ tex ];

  const AObject		*functional = volume( tex );
  const GLComponent     *glf = functional->glAPI();
  Transformation	*trans 
    = theAnatomist->getTransformation( osurf->getReferential(), 
                                       functional->getReferential() );

  vtexture.clear();
  vtexture.reserve( nver );

  // computation of texture values
  const GLfloat	*itver, *verend = vver + 3*nver, *itnor;
  float		value;
  Point3df	pos, ver, nor;
  vector<float> vpos( 3, 0.f );
  vpos.insert( vpos.end(), s.timedims.begin(), s.timedims.end() );

  min = FLT_MAX;
  max = -FLT_MAX;

  /* cout << "Fusion3D min: " << min << ", max: " << max << ", scale: "
     << scale << endl; */
  float	depth = _depth;
  if( _method == POINT_TO_POINT )
    depth = 0;
  else if( _method == INSIDE_POINT_TO_POINT )
    depth = -depth;

  for( itnor=vnor, itver=vver; itver!=verend; itver+=3, itnor+=3 )
  {
    ver = Point3df( *itver, *(itver+1), *(itver+2) );
    if( vnor )
    {
      nor = Point3df( *itnor, *(itnor+1), *(itnor+2) );
      pos = ver + depth*nor;
    }
    else
      pos = ver;

    if( trans )
      pos = trans->transform( pos );

    vpos[0] = pos[0];
    vpos[1] = pos[1];
    vpos[2] = pos[2];
    value = functional->mixedTexValue( vpos );

    if( value < min )
      min = value;
    if( value > max )
      max = value;

    vtexture.push_back( value );
  }

  const TexExtrema & fte = glf->glTexExtrema( tex );
  float qmin = fte.minquant[0], qmax = fte.maxquant[0];
  float fmin = 0., fmax = 1.;
  // keep same qmin/qmax as the volume
  {
    if( qmin == qmax )
      scale = 1.;
    else
      scale = ( fmax - fmin ) / ( qmax - qmin );
    vector<float>::iterator itt, ett = vtexture.end();
    for( itt=vtexture.begin(); itt!=ett; ++itt )
      *itt = ( *itt - qmin ) * scale + fmin;
  }

  TexExtrema  & te 
    = const_cast<Fusion3D *>( this )->GLComponent::glTexExtrema( 0 );
  te.minquant[0] = qmin;
  te.maxquant[0] = qmax;
  te.min[0] = fmin;
  te.max[0] = fmax;
  te.scaled = true;
}


namespace
{
  pair<float,int> _effectiveStep( const Point3df & vs, float estep, 
                                  float depth )
  {
    // computation of the effective step
    float vox = vs.item(0);
    float voy = vs.item(1);
    float voz = vs.item(2);
    float minvoxsize;
    int i = 1;

    /*try to prevent oversampling of functional volume*/
    if(vox<voy) minvoxsize = vox;
    else minvoxsize = voy;
    if(voz<minvoxsize) minvoxsize = voz;
    if( estep < minvoxsize / 2 )
      estep = minvoxsize / 2;

    if( estep >= depth )
      {
        if( depth <= minvoxsize / 2 )
          i = 0;
        else
          estep = depth;
      }
    else
      {
        i = int( depth / estep );
        estep= depth / i;
      }
    //cout << "Fusion3D effectiveStep: " << estep << ", " << i << endl;
    return pair<float,int>( estep, i );
  }
}


void Fusion3D::refreshVTextureWithLineToPoint( const ViewState & s, 
                                               unsigned tex ) const
{
  pair<float,int>	es = _effectiveStep( (*firstVolume())->VoxelSize(), 
                                             _step, _depth );

  refreshLineTexture( -_depth, _depth, s, tex );
}


void Fusion3D::refreshLineTexture( float start_deth, float stop_depth,
                                   const ViewState & s, unsigned tex ) const
{
  // get volume scale
  float min; 
  float max;
  float	scale;

  // get surface vector of vertex
  const AObject		*osurf = *begin();
  const GLComponent	*surf = osurf->glAPI();
  const GLfloat		*vver = surf->glVertexArray( s );
  unsigned		nver = surf->glNumVertex( s );
  const GLfloat		*vnor = surf->glNormalArray( s );
  string		id = viewStateID( glBODY, s );
  vector<float>		& vtexture = d->vtexture[id][ tex ];

  const AObject		*functional = volume( tex );
  const GLComponent     *glf = functional->glAPI();
  Transformation	*trans 
    = theAnatomist->getTransformation( osurf->getReferential(), 
				       functional->getReferential() );

  // computation of texture values
  const GLfloat	*itver, *verend = vver + 3*nver;
  const GLfloat	*itnor;
  Point3df	ver, cver;
  Point3df	nor;
  float		cvalue;
  float 	value;
  unsigned	nvalue;
  int		j;

  min = FLT_MAX;
  max = -FLT_MAX;

  map<float, unsigned> histo;
  map<float, unsigned>::iterator ih, eh=histo.end();

  Point3df vs = functional->VoxelSize();
  BucketMap<Void>::Bucket line;
  BucketMap<Void>::Bucket::iterator iline, eline = line.end();
  float length = fabs( stop_depth - start_deth );
  int nbElem;
  vector<float> vpos( 3, 0.f );
  vpos.insert( vpos.end(), s.timedims.begin(), s.timedims.end() );

  for( itnor=vnor, itver=vver; itver!=verend; itver+=3, itnor+=3 )
    {
      ver = Point3df( *itver, *(itver+1), *(itver+2) );
      if( vnor )
        nor = Point3df( *itnor, *(itnor+1), *(itnor+2) );
      else
        nor = Point3df( 0, 0, 1 ); // FIXME

      cver = ver + nor * start_deth;
      if( trans )
      {
        // work in volume space
        cver = trans->transform( cver );
        nor = trans->transform( nor ) - trans->transform( Point3df( 0, 0, 0 ) );
      }
      nor[0] /= vs[0];
      nor[1] /= vs[1];
      nor[2] /= vs[2];
      nor.norm();

      switch( _submethod )
      {
      case MIN:
        value = 1.0;
        break;
      case MEDIAN:
        histo.clear();
        break;
      default:
        value = 0.0;
        break;
      }
      nvalue = 0;

      line.clear();
      SurfaceManip::rasterizeLine(
        Point3df( cver[0] / vs[0], cver[1] / vs[1], cver[2] / vs[2] ),
        nor, length, line, 0 );
      nbElem = line.size();

      for( iline=line.begin(); iline!=eline; ++iline )
      {
        cver = Point3df( iline->first[0] * vs[0], iline->first[1] * vs[1],
                         iline->first[2] * vs[2] );

        vpos[0] = cver[0];
        vpos[1] = cver[1];
        vpos[2] = cver[2];
        cvalue = functional->mixedTexValue( vpos );

        switch( _submethod )
        {
        case MAX:
          if (value < cvalue)
            value = cvalue;
          break;
        case MIN:
          if (value > cvalue)
            value = cvalue;
          break;
        case MEAN:
          value += cvalue;
          break;
        case CORRECTED_MEAN:
        case ENHANCED_MEAN:
          if( cvalue != 0.0 )
          {
            value += cvalue;
            ++nvalue;
          }
          break;
        case ABSMAX:
          if( fabs( value ) <= fabs( cvalue ) )
          {
            value = cvalue;
          }
          break;
        case MEDIAN:
          ++histo[ cvalue ];
          break;
        }
      }

      switch( _submethod )
      {
      case MEAN:
        value /= (float) nbElem;
        break;
      case CORRECTED_MEAN:
        if( nvalue > 0 )
          value /= (float) nvalue;
        break;
      case ENHANCED_MEAN:
        if( nvalue > 0 )
          value = enhancedMean( value, nvalue, nbElem );
        break;
      case MEDIAN:
        nvalue = 0;
        for( ih=histo.begin(); ih!=eh; ++ih )
          if( ih->second > nvalue )
          {
            nvalue = ih->second;
            value = ih->first;
          }
      default:
        break;
      }
      if( value < min )
        min = value;
      if( value > max )
        max = value;

      vtexture.push_back(value);
    }

  const TexExtrema & fte = glf->glTexExtrema( tex );
  float qmin = fte.minquant[0], qmax = fte.maxquant[0];
  float fmin = 0., fmax = 1.;
  // keep same qmin/qmax as the volume
  {
    if( qmin == qmax )
      scale = 1.;
    else
      scale = ( fmax - fmin ) / ( qmax - qmin );
    vector<float>::iterator itt, ett = vtexture.end();
    for( itt=vtexture.begin(); itt!=ett; ++itt )
      *itt = ( *itt - qmin ) * scale + fmin;
  }

  TexExtrema  & te
    = const_cast<Fusion3D *>( this )->GLComponent::glTexExtrema( 0 );
  te.minquant[0] = qmin;
  te.maxquant[0] = qmax;
  te.min[0] = fmin;
  te.max[0] = fmax;
  te.scaled = true;
}


void Fusion3D::refreshVTextureWithInsideLineToPoint( const ViewState & s, 
                                                     unsigned tex ) const
{
  pair<float,int>	es = _effectiveStep( (*firstVolume())->VoxelSize(), 
                                             _step, _depth );

  refreshLineTexture( -_depth, 0, s, tex );
}


void Fusion3D::refreshVTextureWithOutsideLineToPoint( const ViewState & s, 
                                                      unsigned tex ) const
{
  pair<float,int>	es = _effectiveStep( (*firstVolume())->VoxelSize(), 
                                             _step, _depth );

  refreshLineTexture( 0, _depth, s, tex );
}


void Fusion3D::refreshVTextureWithSphereToPoint( const ViewState & s, 
                                                 unsigned tex ) const
{
  // get volume scale
  float min = FLT_MAX;
  float max = -FLT_MAX;
  float	scale;

  // get surface vector of vertex
  const AObject		*osurf = *begin();
  const GLComponent	*surf = osurf->glAPI();
  const GLfloat		*vver = surf->glVertexArray( s );
  const GLfloat		*vnor = surf->glNormalArray( s );
  unsigned		nver = surf->glNumVertex( s );
  string		id = viewStateID( glBODY, s );
  vector<float>		& vtexture = d->vtexture[id][ tex ];

  const AObject		*functional = volume( tex );
  const GLComponent     *glf = functional->glAPI();
  Transformation	*trans 
    = theAnatomist->getTransformation( osurf->getReferential(), 
				       functional->getReferential() );

  Point3df		vs = functional->VoxelSize();
  pair<float,int>	es = _effectiveStep( vs, _step, _depth );
  int	i = es.second;
  float	estep = es.first;

  // computation of the sphere elements
  list<Point3df> lelem;
  Point3df elem;
  int j,k,l;
  for (j=-i;j<=i;++j) for (k=-i;k<=i;++k) for (l=-i;l<=i;++l)
    {
      elem.item(0) = j*estep;
      elem.item(1) = k*estep;
      elem.item(2) = l*estep;
      if (norm(elem) <= _depth)
	{
	  lelem.push_back( elem );
	}
    }

  int nbElem = lelem.size();

  // computation of texture values

  list<Point3df>::iterator	itelem;
  const GLfloat			*itver, *itnor;
  const GLfloat			*verend = vver + 3*nver;
  Point3df			ver, cver;
  Point3df			nor;
  float				cvalue;
  float 			value;
  unsigned			nvalue;

  map<float, unsigned> histo;
  map<float, unsigned>::iterator ih, eh=histo.end();
  vector<float> vpos( 3, 0.f );
  vpos.insert( vpos.end(), s.timedims.begin(), s.timedims.end() );

  for( itnor=vnor, itver=vver; itver!=verend; itver+=3, itnor+=3 )
    {
      ver = Point3df( *itver, *(itver+1), *(itver+2) );

      switch( _submethod )
      {
      case MIN:
        value = 1.0;
        break;
      case MEDIAN:
        histo.clear();
        break;
      default:
        value = 0.0;
        break;
      }
      nvalue = 0;

      for( itelem=lelem.begin(); itelem!=lelem.end(); ++itelem )
      {
        cver = ver + *itelem;

        if( trans )
          cver = trans->transform( cver );

        vpos[0] = cver[0];
        vpos[1] = cver[1];
        vpos[2] = cver[2];
        cvalue = functional->mixedTexValue( vpos );

        switch( _submethod )
        {
        case MAX:
          if (value < cvalue)
            value = cvalue;
          break;
        case MIN:
          if (value > cvalue)
            value = cvalue;
          break;
        case MEAN:
          value += cvalue;
          break;
        case CORRECTED_MEAN:
        case ENHANCED_MEAN:
          if (cvalue != 0.0)
          {
            value += cvalue;
            ++nvalue;
          }
          break;
        case ABSMAX:
          if( fabs( value ) <= fabs( cvalue ) )
          {
            value = cvalue;
          }
          break;
        case MEDIAN:
          ++histo[ cvalue ];
          break;
        }
      }

      switch( _submethod )
      {
      case MEAN:
        value /= (float) nbElem;
        break;
      case CORRECTED_MEAN:
        if( nvalue > 0 )
          value /= (float) nvalue;
        break;
      case ENHANCED_MEAN:
        if( nvalue > 0 )
          value = enhancedMean( value, nvalue, nbElem );
        break;
      case MEDIAN:
        nvalue = 0;
        for( ih=histo.begin(); ih!=eh; ++ih )
          if( ih->second > nvalue )
          {
            nvalue = ih->second;
            value = ih->first;
          }
      default:
        break;
      }

      if( value < min )
        min = value;
      if( value > max )
        max = value;

      vtexture.push_back(value);
    }

  const TexExtrema & fte = glf->glTexExtrema( tex );
  float qmin = fte.minquant[0], qmax = fte.maxquant[0];
  float fmin = 0., fmax = 1.;
  // keep same qmin/qmax as the volume
  {
    if( qmin == qmax )
      scale = 1.;
    else
      scale = ( fmax - fmin ) / ( qmax - qmin );
    vector<float>::iterator itt, ett = vtexture.end();
    for( itt=vtexture.begin(); itt!=ett; ++itt )
      *itt = ( *itt - qmin ) * scale + fmin;
  }

  TexExtrema  & te
    = const_cast<Fusion3D *>( this )->GLComponent::glTexExtrema( 0 );
  te.minquant[0] = qmin;
  te.maxquant[0] = qmax;
  te.min[0] = fmin;
  te.max[0] = fmax;
  te.scaled = true;
}


Tree* Fusion3D::optionTree() const
{
  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Save" ) );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                              "Rename object" ) );
      t2->setProperty( "callback", &ObjectActions::renameObject );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                              "Save as textured mesh" ) );
      t2->setProperty( "callback", &ObjectActions::saveStatic );
      t->insert( t2 );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu",
                                              "Export texture" ) );
      t2->setProperty( "callback", &ObjectActions::saveTexture );
      t->insert( t2 );
      t2 = new Tree( true, 
                     QT_TRANSLATE_NOOP( "QSelectMenu", 
                                        "Extract texture as new object" ) );
      t2->setProperty( "callback", &ObjectActions::extractTexture );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Color" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Texturing" ) );
      t2->setProperty( "callback", &ObjectActions::textureControl );
      t->insert( t2 );

      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );

      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "Fusion" ) );
      _optionTree->insert( t );
      t2 = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", 
					      "Control 3D fusion" ) );
      t2->setProperty( "callback", &ObjectActions::fusion3DControl );
      t->insert( t2 );
    }
  return( _optionTree );
}


unsigned Fusion3D::glTexCoordSize( const ViewState & s, unsigned ) const
{
  refreshVTexture( s );

  string		id = viewStateID( glBODY, s );
  map<string, vector<vector<float> > >::const_iterator 
    it = d->vtexture.find( id );

  if( it == d->vtexture.end() )
    return 0;
  else
    return( it->second.begin()->size() );
}


const GLfloat* Fusion3D::glTexCoordArray( const ViewState & s, 
                                          unsigned tex ) const
{
  refreshVTexture( s );

  string		id = viewStateID( glBODY, s );
  map<string, vector<vector<float> > >::const_iterator 
    it = d->vtexture.find( id );

  if( it == d->vtexture.end() )
    return 0;
  else
    return( &it->second[ tex ][0] );
}


bool Fusion3D::boundingBox( vector<float> & bmin, vector<float> & bmax ) const
{
  unsigned		i;
  const_iterator	is, es = end();
  vector<float>		pmin, pmax;
  bool			valid = false;
  unsigned j, n;

  for( i=0, is=begin(); i<_nsurf; ++i )
    if( (*is)->boundingBox( pmin, pmax ) )
    {
      if( valid )
      {
        for( j=0, n=std::min( pmax.size(), bmax.size() ); j<n; ++j )
        {
          // TODO take transformation into account
          if( pmin[j] < bmin[j] )
            bmin[j] = pmin[j];
          if( pmax[j] > bmax[j] )
            bmax[j] = pmax[j];
        }
        if( n < pmax.size() )
        {
          bmin.insert( bmin.end(), pmin.begin() + n, pmin.end() );
          bmax.insert( bmax.end(), pmax.begin() + n, pmax.end() );
        }
      }
      else
      {
        bmin = pmin;
        bmax = pmax;
      }
      valid = true;
    }

  // textures can extend time dimensions (only)
  if( valid )
    for( ; is!=es; ++is )
    if( (*is)->boundingBox( pmin, pmax ) )
    {
      for( j=3, n=std::min( pmax.size(), bmax.size() ); j<n; ++j )
      {
        if( pmin[j] < bmin[j] )
          bmin[j] = pmin[j];
        if( pmax[j] > bmax[j] )
          bmax[j] = pmax[j];
      }
      if( n < pmax.size() )
      {
        bmin.insert( bmin.end(), pmin.begin() + n, pmin.end() );
        bmax.insert( bmax.end(), pmax.begin() + n, pmax.end() );
      }
    }

  return valid;
}


const GLComponent* Fusion3D::glTexture( unsigned ) const
{
  //return (*firstVolume())->glAPI();
  return glAPI();
}


GLComponent* Fusion3D::glTexture( unsigned )
{
  //return (*firstVolume())->glAPI();
  return glAPI();
}


unsigned Fusion3D::glNumTextures() const
{
  return GLComponent::glNumTextures();
}


unsigned Fusion3D::glNumTextures( const ViewState & ) const
{
  return glNumTextures();
}


unsigned Fusion3D::glDimTex( const ViewState &, unsigned ) const
{
  return 1;
}


const AObjectPalette* Fusion3D::glPalette( unsigned tex ) const
{
  // cout << "Fusion3D::glPalette, " << tex << ", this: " << this << endl;
  return volume( tex )->glAPI()->glPalette();
}


void Fusion3D::glGarbageCollector( int nkept )
{
  GLComponent::glGarbageCollector( nkept );

  GLComponent	*c = glGeometry();
  if( c )
    c->glGarbageCollector( nkept );
}


GLPrimitives Fusion3D::glTexNameGLL( const ViewState & s, unsigned tex ) const
{
  /* cout << "Fusion3D::glTexNameGLL " << tex << " " 
     << glTexImageChanged( tex ) << endl; */
  return GLComponent::glTexNameGLL( s, tex );
}


GLPrimitives Fusion3D::glTexEnvGLL( const ViewState & s, unsigned tex ) const
{
  return GLComponent::glTexEnvGLL( s, tex );
}


GLComponent::glTextureMode Fusion3D::glTexMode( unsigned tex ) const
{
  return GLComponent::glTexMode( tex );
  // return volume( tex )->glAPI()->glTexMode( 0 );
}


void Fusion3D::glSetTexMode( glTextureMode mode, unsigned tex )
{
  GLComponent::glSetTexMode( mode, tex  );
}


float Fusion3D::glTexRate( unsigned tex ) const
{
  return GLComponent::glTexRate( tex );
  // return volume( tex )->glAPI()->glTexRate( 0 );
}


void Fusion3D::glSetTexRate( float rate, unsigned tex )
{
  GLComponent::glSetTexRate( rate, tex  );
}


GLComponent::glTextureFiltering Fusion3D::glTexFiltering( unsigned tex ) const
{
  return GLComponent::glTexFiltering( tex );
  // return volume( tex )->glAPI()->glTexFiltering( 0 );
}


void Fusion3D::glSetTexFiltering( GLComponent::glTextureFiltering x, 
                                  unsigned tex )
{
  GLComponent::glSetTexFiltering( x, tex );
}


GLComponent::glAutoTexturingMode Fusion3D::glAutoTexMode( unsigned tex ) const
{
  return GLComponent::glAutoTexMode( tex );
  // return volume( tex )->glAPI()->glAutoTexMode( 0 );
}


void Fusion3D::glSetAutoTexMode( GLComponent::glAutoTexturingMode mode, 
                                 unsigned tex )
{
  GLComponent::glSetAutoTexMode( mode, tex );
}


const float *Fusion3D:: glAutoTexParams( unsigned coord, unsigned tex ) const
{
  return GLComponent::glAutoTexParams( coord, tex );
}


void Fusion3D:: glSetAutoTexParams( const float* params, unsigned coord, 
                                    unsigned tex )
{
  GLComponent::glSetAutoTexParams( params, coord, tex );
}


void Fusion3D::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    MObject::obsSetChanged( glTEXIMAGE_NUM + 2 * tex );
}


void Fusion3D::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexEnvChanged( x, tex );
  if( x )
    MObject::obsSetChanged( glTEXENV_NUM + 2 * tex );
}


std::string Fusion3D::viewStateID( glPart part, const ViewState & state ) const
{
  // cout << "Fusion3D::viewStateID " << part << ", this: " << this << endl;
  switch( part )
  {
  case glGENERAL:
  case glBODY:
    {
      string s;
      vector<float> td = state.timedims;
      vector<float> bbmin, bbmax;
      size_t i, n = td.size();
      if( boundingBox( bbmin, bbmax ) )
      {
        n = std::min( n, bbmax.size() - 3 );
        for( i=0; i<n; ++i )
        {
          if( td[i] < bbmin[i + 3] )
            td[i] = bbmin[i + 3];
          if( td[i] > bbmax[i + 3] )
            td[i] = bbmax[i + 3];
        }
        if( n < bbmax.size() - 3 )
        {
          n = bbmax.size() - 3;
          for( ; i<n; ++i )
            td.push_back( bbmin[i + 3] );
        }
      }
      else
      {
        n = 1;
        td[0] = 0.f;
      }
      s.resize( sizeof(float) * ( n + 1 ) );
      (float &) s[0] = n;
      for( i=0; i<n; ++i )
        (float &) s[sizeof(float) * ( i + 1 )] = td[i];
      return s + GLMObject::viewStateID( part, state );
    }
  case glTEXIMAGE:
  case glTEXENV:
    return GLMObject::viewStateID( glBODY, state );
  default:
    return GLMObject::viewStateID( part, state );
  }
}


const Fusion3D::TexExtrema & Fusion3D::glTexExtrema( unsigned tex ) const
{
  ViewState	vs( 0 ); // FIXME: always time 0
  refreshVTexture( vs );
  return GLComponent::glTexExtrema( tex );
}


Fusion3D::TexExtrema & Fusion3D::glTexExtrema( unsigned tex )
{
  ViewState	vs( 0 ); // FIXME: always time 0
  refreshVTexture( vs );
  return GLComponent::glTexExtrema( tex );
}


