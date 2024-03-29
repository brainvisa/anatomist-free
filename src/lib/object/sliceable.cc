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

#include <anatomist/object/sliceable.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Referential.h>
#include <aims/resampling/quaternion.h>
#include <cartodata/volume/volume.h>
#include <aims/rgb/rgb.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;

namespace anatomist
{

  namespace internal
  {

    struct SliceInfo
    {
      Point3df	vertices[8];
      Point3df	normals[8];
      Point2df	texcoords[8];
      AImage	xim;
      Point3df	posbase;
    };

  }

}

using namespace anatomist::internal;


struct Sliceable::Private
{
  Private();

  mutable AimsVector<uint, 4>		polygons[2];
  mutable bool				refreshgeom;
  mutable map<string, SliceInfo>	slices;
};


Sliceable::Private::Private() : refreshgeom( true )
{
  polygons[0] = AimsVector<uint, 4>( 0, 1, 2, 3 );
  polygons[1] = AimsVector<uint, 4>( 4, 5, 6, 7 );
}


Sliceable::Sliceable() : d( new Sliceable::Private )
{
}


Sliceable::~Sliceable()
{
  delete d;
}


unsigned Sliceable::glNumVertex( const ViewState & state ) const
{
  const SliceViewState* st = state.sliceVS();
  if( !st || !st->wantslice || !st->orientation )
    return 0;	// not drawable in pure 3D
  return 8;
}


vector<float> Sliceable::glVoxelSize() const
{
  return vector<float>( 4, 1.f );
}


const GLfloat* Sliceable::glVertexArray( const ViewState & state ) const
{
  const SliceViewState* st = state.sliceVS();
  // cout << "Sliceable::glVertexArray, sl: " << st << endl;
  if( !st || !st->wantslice || !st->orientation )
    return 0;	// not drawable in pure 3D

  Sliceable::Private	*d = Sliceable::d;

  // cout << "drawable in 3D\n";
  if( d->refreshgeom )
    {
      d->slices.clear();
      d->refreshgeom = false;
    }
  string	id = viewStateID( glGEOMETRY, state );
  map<string, SliceInfo>::iterator	ii = d->slices.find( id );
  if( ii != d->slices.end() )
    {
      //cout << "vertex array cached " << id << endl;
      return (const GLfloat *) &ii->second.vertices;	// in cache
    }
  //cout << "vertex array not in cache: " << id << endl;

  // clear cache of older things
  d->slices.clear();

  SliceInfo	& si = d->slices[ id ];

  vector<float> vs = glVoxelSize();

  float			xm, ym;
  const Quaternion	& rot = *st->orientation;
  const anatomist::Referential     *ref = getReferential();
  const anatomist::Referential     *wref = st->winref;
  Transformation        *otrans = 0;
  if( ref && wref )
    otrans = theAnatomist->getTransformation( wref, ref );

  Point3df		dirx = rot.transformInverse( Point3df( 1, 0, 0 ) );
  Point3df		diry = rot.transformInverse( Point3df( 0, 1, 0 ) );
  Point3df		direction = rot.transformInverse( Point3df( 0, 0, -1 ) );
  const Point3df	& pos = st->position;
  AImage		& xim = si.xim;

  Point3df	gs;
  if( st->wingeom )
    {
      gs = st->wingeom->Size();

      Point4dl	dm = st->wingeom->DimMin();
      Point4dl	dM = st->wingeom->DimMax();
      Point3df	dz = rot.transformInverse( Point3df( 0, 0, 1 ) );
      float	z = dz.dot( pos );
      //cout << "z : " << z << endl;

      si.vertices[0] = rot.transformInverse(
        Point3df( gs[0] * ( -0.5F + dm[0] ), gs[1] * ( -0.5F + dm[1] ), z ) );
      si.vertices[1] = rot.transformInverse(
        Point3df( gs[0] * ( -0.5F + dm[0] ), gs[1] * ( -0.5F + dM[1] ), z ) );
      si.vertices[2] = rot.transformInverse(
        Point3df( gs[0] *  ( -0.5F + dM[0] ), gs[1] * ( -0.5F + dM[1] ), z ) );
      si.vertices[3] = rot.transformInverse(
        Point3df( gs[0] *  ( -0.5F + dM[0] ), gs[1] * ( -0.5F + dm[1] ), z ) );

      xim.width = dM[0] - dm[0]; // + 1;
      xim.height = dM[1] - dm[1]; // + 1;
    }
  else
    {
      cout << "warning : 2D object displayed without a window geometry\n";
      gs = Point3df( 1, 1, 1 );

      //cout << "gs : " << gs << endl;

      vector<float> min2d = glMin2D();
      vector<float> max2d = glMax2D();
      Point3df	lims = Point3df( vs[0] * ( 0.5 + max2d[0] ), 
				 vs[1] * ( 0.5 + max2d[1] ), 
				 vs[2] * ( 0.5 + max2d[2] ) );
      Point3df	mlim = Point3df( vs[0] * ( min2d[0] - 0.5 ), 
				 vs[1] * ( min2d[1] - 0.5 ), 
				 vs[2] * ( min2d[2] - 0.5 ) );

      /* Cutting plane: rotated by `rot', through `pos'
	 view: the cube [mlim, lims] must project on this plane inside the
	 viewing area
	 Cube projection: hexagon

	 In plane coordinates, clip the cube projection
	 -> project on rot( 1, 0, 0 ) for x-clipping
	 project on rot( 0, 1, 0 ) for y-clipping
      */
      float		xmax, xmin, ymax, ymin;
      Point3df	k = mlim - pos;
      float		l = dirx.dot( k );

      xmax = xmin = l;
      l = diry.dot( k );
      ymax = ymin = l;

      k = lims - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( mlim[0], lims[1], mlim[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( lims[0], lims[1], mlim[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( lims[0], mlim[1], mlim[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( mlim[0], mlim[1], lims[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( mlim[0], lims[1], lims[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      k = Point3df( lims[0], mlim[1], lims[2] ) - pos;
      l = dirx.dot( k );
      if( l < xmin )
	xmin = l;
      if( l > xmax )
	xmax = l;
      l = diry.dot( k );
      if( l < ymin )
	ymin = l;
      if( l > ymax )
	ymax = l;

      si.vertices[0] = pos + dirx * xmin + diry * ymin;
      si.vertices[1] = pos + dirx * xmin + diry * ymax;
      si.vertices[2] = pos + dirx * xmax + diry * ymax;
      si.vertices[3] = pos + dirx * xmax + diry * ymin;
      /* cout << "xmax etc: " << xmax << ", " << xmin << ", " << ymax << ", " 
         << ymin << ", " << gs[0] << ", " << gs[1] << endl; */

      xim.width = (int) ( ( xmax - xmin ) / gs[0] );
      xim.height = (int) ( ( ymax - ymin ) / gs[1] );
    }

  xim.depth = 32;
  unsigned w, h;
  w = 0x1 << (unsigned) ::ceil( ::log( xim.width ) / ::log(2) );
  h = 0x1 << (unsigned) ::ceil( ::log( xim.height ) / ::log(2) );
  xm = ((float) xim.width) / w;
  ym = ((float) xim.height) / h;
  xim.effectiveWidth = w;
  xim.effectiveHeight = h;

  // start from center of first voxel
  si.posbase = si.vertices[0] + dirx * gs[0] * 0.5F + diry * gs[1] * 0.5F;

  /* cout << "image dims : " << xim.width << " x " << xim.height << endl;
  cout << "corners : " << si.vertices[0] << ", " << si.vertices[1] << "," 
       << endl;
  cout << "          " << si.vertices[2] << ", " << si.vertices[3] << endl;
  cout << "first point : " << si.posbase << endl; */

  if( otrans )
  {
    // get back to object referential
    si.vertices[0] = otrans->transform( si.vertices[0] );
    si.vertices[1] = otrans->transform( si.vertices[1] );
    si.vertices[2] = otrans->transform( si.vertices[2] );
    si.vertices[3] = otrans->transform( si.vertices[3] );
  }
  si.vertices[4] = si.vertices[3];
  si.vertices[5] = si.vertices[2];
  si.vertices[6] = si.vertices[1];
  si.vertices[7] = si.vertices[0];

  si.normals[0] = direction;
  if( otrans )
    // get back to object referential
    direction = otrans->transform( direction )
        - otrans->transform( Point3df( 0, 0, 0 ) );
  si.normals[1] = direction;
  si.normals[2] = direction;
  si.normals[3] = direction;
  si.normals[4] = -direction;
  si.normals[5] = -direction;
  si.normals[6] = -direction;
  si.normals[7] = -direction;

  si.texcoords[0] = Point2df( 0, 0 );
  si.texcoords[1] = Point2df( 0, ym );
  si.texcoords[2] = Point2df( xm, ym );
  si.texcoords[3] = Point2df( xm, 0 );
  si.texcoords[4] = Point2df( xm, 0 );
  si.texcoords[5] = Point2df( xm, ym );
  si.texcoords[6] = Point2df( 0, ym );
  si.texcoords[7] = Point2df( 0, 0 );

  return (const GLfloat *) si.vertices;
}


VolumeRef<AimsRGBA> Sliceable::glBuildTexImage(
  const ViewState & state, unsigned tex, int dimx, int dimy, bool ) const
{
  if( !glVertexArray( state ) )
    return VolumeRef<AimsRGBA>();

  string                id = viewStateID( glGEOMETRY, state );
  SliceInfo             & si = d->slices[ id ];
  AImage                xim = si.xim;
  /* cout << "texture dim: " << xim.effectiveWidth << " x "
       << xim.effectiveHeight << "; " << xim.width
       << " x " << xim.height << ", id: " << id << endl; */
  if( dimx > 0 )
  {
    xim.effectiveWidth = dimx;
    xim.width = dimx;
  }
  if( dimy > 0 )
  {
    xim.effectiveHeight = dimy;
    xim.height = dimy;
  }

  unsigned              w = xim.effectiveWidth, h = xim.effectiveHeight;
  const SliceViewState  *st = state.sliceVS();

  VolumeRef<AimsRGBA> teximage( w, h );

  xim.data = (char *) &teximage( 0 );

  bool  retcode = update2DTexture( xim, si.posbase, *st, tex );
  if( retcode )
    return teximage;

  return VolumeRef<AimsRGBA>();
}


bool Sliceable::glMakeTexImage( const ViewState & state, 
                                const GLTexture & gltex, unsigned tex ) const
{
  // cout << "Sliceable::glMakeTexImage " << this << ": " << typeid( *this ).name() << endl;
  if( !glVertexArray( state ) )
    return false;

  string		id = viewStateID( glGEOMETRY, state );
  SliceInfo		& si = d->slices[ id ];
  AImage		& xim = si.xim;
  /* cout << "texture dim: " << xim.effectiveWidth << " x "
       << xim.effectiveHeight << "; " << xim.width 
       << " x " << xim.height << ", id: " << id << endl; */

  unsigned		w = xim.effectiveWidth, h = xim.effectiveHeight;
  const SliceViewState	*st = state.sliceVS();

  xim.data = new char[ ((long long)w) * h * 4 ];

  bool	retcode = update2DTexture( xim, si.posbase, *st, tex );

  if( retcode )
    {
      //	GL texture
      GLint	twidth;
      GLuint	tex = gltex.item();

      glBindTexture( GL_TEXTURE_2D, tex );
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
      do
	{
          glTexImage2D( GL_PROXY_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
                        GL_UNSIGNED_BYTE, (GLvoid*) xim.data );
          //cout << "proxy created " << w << " x " << h << "\n";
	  glGetTexLevelParameteriv( GL_PROXY_TEXTURE_2D, 0, 
				    GL_TEXTURE_WIDTH, &twidth );
          //cout << "twidth: " << twidth << endl;
	  if( twidth == 0 )	// too large
	    {
	      cerr << "Texture too large for OpenGL implementation. " 
                   << "Degrading image.\n";
	      // degraded image
	      unsigned	dxs = w * 2 - xim.effectiveWidth;

	      w /= 2;
	      h /= 2;
	      xim.effectiveWidth /= 2;
	      xim.effectiveHeight /= 2;

	      int	x, y;
	      unsigned	dxd = w - xim.effectiveWidth;
	      char	*ps = new char[ w * h * 4 ];
	      unsigned	*p = (unsigned *) ps, *q = (unsigned *) xim.data;

	      for( y=0; y<xim.effectiveHeight; ++y )
		{
		  for( x=0; x<xim.effectiveWidth; ++x )
		    {
		      *p++ = *q;
		      q += 2;
		    }
		  p += dxd;
		  q += dxs;
		}
	      delete[] xim.data;
	      xim.data = ps;
	    }
	} while( twidth == 0 && w > 0 && h > 0 );
      if( w > 0 && h > 0 )
        {
          // cout << "glTexImage2D " << w << " x " << h << endl;
          glTexImage2D( GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
                        GL_UNSIGNED_BYTE, (GLvoid*) xim.data );
          GLenum status = glGetError();
          if( status != GL_NO_ERROR )
          {
            cerr << "OpenGL error tex: " << gluErrorString(status) << endl;
            retcode = false;
          }
        }
      else
        {
          cerr << "OpenGL seems not to be able to display a texture at all. " 
               << "Nothing will be displayed." << endl;
          retcode = false;
        }
    }

  delete[] xim.data;
  return retcode;
}


const GLfloat *Sliceable::glNormalArray( const ViewState & state ) const
{
  if( !glVertexArray( state ) )
    return 0;

  SliceInfo		& si = d->slices[ viewStateID( glGEOMETRY, state ) ];
  return (GLfloat *) si.normals;
}


unsigned Sliceable::glDimTex( const ViewState &, unsigned ) const
{
  return 2;
}


unsigned Sliceable::glTexCoordSize( const ViewState & state, unsigned ) const
{
  const SliceViewState* st = state.sliceVS();
  if( !st || !st->wantslice || !st->orientation )
    return 0;	// not drawable in pure 3D
  return 8;
}


const GLfloat *Sliceable::glTexCoordArray( const ViewState & state, 
                                           unsigned ) const
{
  if( !glVertexArray( state ) )
    return 0;

  // cout << "Sliceable::glTexCoordArray\n";
  SliceInfo		& si = d->slices[ viewStateID( glGEOMETRY, state ) ];
  return (GLfloat *) si.texcoords;
}


unsigned Sliceable::glPolygonSize( const ViewState & ) const
{
  return 4;
}


unsigned Sliceable::glNumPolygon( const ViewState & state ) const
{
  const SliceViewState* st = state.sliceVS();
  if( !st || !st->wantslice || !st->orientation )
    return 0;	// not drawable in pure 3D
  return 2;
}


const GLuint* Sliceable::glPolygonArray( const ViewState & state ) const
{
  const SliceViewState* st = state.sliceVS();
  if( !st || !st->wantslice || !st->orientation )
    return 0;	// not drawable in pure 3D
  return (const GLuint *) d->polygons;
}


bool Sliceable::update2DTexture( AImage &, const Point3df &, 
                                 const SliceViewState &, unsigned ) const
{
  return false;
}


string Sliceable::viewStateID( glPart part,
                               const ViewState & state ) const
{
  // cout << "SliceableObject::viewStateID\n";
  const SliceViewState	*st = state.sliceVS();
  if( !st || !st->wantslice )
    return GLComponent::viewStateID( part, state );

  vector<float> timedims = state.timedims;
  vector<float> gmin = glMin2D(), gmax = glMax2D();
  unsigned i, n = std::min( timedims.size(), gmax.size() - 3 );
  timedims.resize( n );
  for( i=0; i<n; ++i )
  {
    if( timedims[i] < gmin[3 + i] )
      timedims[i] = gmin[3 + i];
    if( timedims[i] > gmax[3 + i] )
      timedims[i] = gmax[3 + i];
  }
  if( n < gmax.size() - 3 )
  {
    n = gmax.size() - 3;
    for( ; i<n; ++i )
      timedims.push_back( gmin[i + 3] );
  }

  string		s;
  static const int	nf = sizeof(float);
  static const int	ns4 = 4*sizeof(int);
  size_t p = 0;

  switch( part )
    {
    case glMATERIAL:
      return s;
    case glGEOMETRY:
    case glBODY:
      {
        if( st->wingeom )
          s.resize( 6 * nf + 2 * ns4 );
        else
          s.resize( 6 * nf );
        Point4df	o = st->orientation->vector();
        // level in plane: position .dot. plane normal
        // (inverse since the quaternion transforms to the texture space)
        Point3df normal = st->orientation->transformInverse(
          Point3df( 0, 0, 1 ) );
        float level = normal.dot( st->position );
        memcpy( &s[0], &o[0], 4*nf );
        memcpy( &s[4*nf], &level, nf );
        memcpy( &s[5*nf], &state.selectRenderMode, nf );
        if( st->wingeom )
        {
          memcpy( &s[6*nf], &st->wingeom->DimMin()[0], ns4 );
          memcpy( &s[6*nf+ns4], &st->wingeom->DimMax()[0], ns4 );
        }
      }
      break;
    case glGENERAL:
      {
        if( st->wingeom )
          s.resize( ( 7 + n ) * nf + 2 * ns4 );
        else
          s.resize( ( 7 + n ) * nf );
        (float &) s[0] = n;
        for( i=0; i<n; ++i )
          (float &) s[nf * ( i + 1 )] = timedims[i];
        p = nf * (n + 1 );
        Point4df        o = st->orientation->vector();
        // level in plane: position .dot. plane normal
        Point3df normal = st->orientation->transformInverse(
          Point3df( 0, 0, 1 ) );
        float level = normal.dot( st->position );
        memcpy( &s[p], &o[0], 4*nf );
        memcpy( &s[p + 4*nf], &level, nf );
        memcpy( &s[p + 5*nf], &state.selectRenderMode, nf );
        if( st->wingeom )
        {
          memcpy( &s[p + 6*nf], &st->wingeom->DimMin()[0], ns4 );
          memcpy( &s[p + 6*nf + ns4], &st->wingeom->DimMax()[0], ns4 );
        }
      }
      break;
    case glTEXIMAGE:
    case glTEXENV:
      {
        if( st->wingeom )
          s.resize( ( 6 + n ) * nf + 2 * ns4 );
        else
          s.resize( ( 6 + n ) * nf );
        (float &) s[0] = n;
        for( i=0; i<n; ++i )
          (float &) s[nf * ( i + 1 )] = timedims[i];
        p = nf * (n + 1 );
        Point4df	o = st->orientation->vector();
        // level in plane: position .dot. plane normal
        Point3df normal = st->orientation->transformInverse(
          Point3df( 0, 0, 1 ) );
        float level = normal.dot( st->position );
        memcpy( &s[p], &o[0], 4*nf );
        memcpy( &s[p + 4*nf], &level, nf );
        if( st->wingeom )
          {
            memcpy( &s[p + 5*nf], &st->wingeom->DimMin()[0], ns4 );
            memcpy( &s[p + 5*nf + ns4], &st->wingeom->DimMax()[0], ns4 );
          }
      }
      break;
    default:
      return s;
    }
  return s;
}


bool Sliceable::glAllowedTexRGBInterpolation( unsigned ) const
{
  return false;
}


VolumeRef<AimsRGBA> Sliceable::rgbaVolume( const SliceViewState* svs,
                                           int tex ) const
{
  Point3dl dims;
  Point3df dmin;
  vector<float> vvs(3, 1.);

  if( svs && svs->wingeom )
  {
    Point3df vs = svs->wingeom->Size();
    vvs[0] = vs[0];
    vvs[1] = vs[1];
    vvs[2] = vs[2];
    Point4dl dmm = svs->wingeom->DimMin();
    Point4dl	dm = svs->wingeom->DimMax() - dmm;
    dims[0] = dm[0];
    dims[1] = dm[1];
    dims[2] = dm[2];
    dmin = Point3df( dmm[0], dmm[1], dmm[2] );
  }
  else
  {
    // no geometry case
    vector<float>  dmm = glMin2D();
    vector<float> dmax2d = glMax2D();
    Point3df max2d = Point3df( dmax2d[0], dmax2d[1], dmax2d[2] )
      - Point3df( dmm[0], dmm[1], dmm[2] ) + Point3df( 1.F );
    vvs = glVoxelSize();
    dims = Point3dl( (int) rint( max2d[0] ), (int) rint( max2d[1] ),
                    (int) rint( max2d[2] ) );
    dmin = Point3df( dmm[0], dmm[1], dmm[2] );
  }

  VolumeRef<AimsRGBA> vol( new Volume<AimsRGBA>( dims[0], dims[1], dims[2] ) );
  vol->header().setProperty( "voxel_size", vvs );
  if( dmin[0] != 0 || dmin[1] != 0 || dmin[2] != 0 )
  {
    vector<string> refs(1);
    vector<vector<float> > trans(1);
    refs[0] = getReferential()->uuid().toString();
    vector<float> & t = trans[0];
    t.reserve( 16 );
    for( int i=0; i<16; ++i )
      t.push_back( 0. );
    t[0] = 1.;
    t[5] = 1.;
    t[10] = 1.;
    t[15] = 1.;
    t[3] = dmin[0] * vvs[0];
    t[7] = dmin[1] * vvs[1];
    t[11] = dmin[2] * vvs[2];
    vol->header().setProperty( "transformations", trans );
    vol->header().setProperty( "referentials", refs );
  }
  rgbaVolume( *vol, svs, tex );
  return vol;
}


void Sliceable::rgbaVolume( Volume<AimsRGBA> & vol,
                            const SliceViewState* slvs, int tex ) const
{
  AImage  aim;
  aim.width = vol.getSizeX();
  aim.height = vol.getSizeY();
  aim.depth = 32;
  aim.effectiveWidth = aim.width;
  aim.effectiveHeight = aim.height;

  Quaternion q( 0.F, 0.F, 0.F, 1.F );
  SliceViewState svs( 0, true, Point3df( 0.F ), &q );
  if( slvs )
  {
    svs.timedims = slvs->timedims;
    svs.window = slvs->window;
    svs.orientation = slvs->orientation;
    svs.winref = slvs->winref;
    svs.vieworientation = slvs->vieworientation;
    // don't copy wingeom since the volume has his own
  }

  Point4dl vmin( 0, 0, 0, 0 );

  if( vol.header().hasProperty( "transformations" ) )
  {
    string rid = getReferential()->uuid().toString();
    try
    {
      Object trans = vol.header().getProperty( "transformations" );
      Object refs = vol.header().getProperty( "referentials" );
      Object it1, it2;
      for( it1=trans->objectIterator(), it2=refs->objectIterator();
        it1->isValid() && it2->isValid(); it1->next(), it2->next() )
      {
        if( it2->currentValue()->getString() == rid )
          break;
      }
      if( it1->isValid() && it2->isValid() )
      {
        AffineTransformation3d tr( it1->currentValue() );
        anatomist::Referential *nref = new anatomist::Referential;
/*        nref->header().setProperty( "uuid",
          vol.header().getProperty( "referential" ) );*/
        Transformation *at = new Transformation( nref,
          const_cast<anatomist::Referential *>( getReferential() ) );
        at->motion() = tr;
        at->registerTrans();
        svs.winref = nref;
        Point3df p0 = tr.transform( Point3df( 0, 0, 0 ) );
        vmin = Point4dl( int( rint( p0[0] ) ), int( rint( p0[1] ) ),
                        int( rint( p0[2] ) ), 0 );
      }
    }
    catch( ... )
    {
    }
  }

  if( !svs.winref )
    svs.winref = getReferential();

  Point3df vs( 1.F, 1.F, 1.F );
  Object vvs = vol.header().getProperty( "voxel_size" );
  if( vvs )
  {
    Object i = vvs->objectIterator();
    if( i && i->isValid() )
    {
      vs[0] = (float) i->currentValue()->getScalar();
      i->next();
      if( i && i->isValid() )
      {
        vs[1] = (float) i->currentValue()->getScalar();
        i->next();
        if( i && i->isValid() )
          vs[2] = (float) i->currentValue()->getScalar();
      }
    }
  }
  Geometry geom( vs, vmin, Point4dl( vol.getSizeX() - 1,
                 vol.getSizeY() - 1, vol.getSizeZ() - 1, 0 ) );
  svs.wingeom = &geom;

  unsigned z, nz = vol.getSizeZ();

  for( z=0; z<nz; ++z )
  {
    aim.data = (char *) &vol( 0, 0, z );
    svs.position[2] = z * vs[2];
    update2DTexture( aim, Point3df( 0, 0, z * vs[2] ), svs, tex );
  }
}


// ---------

SliceableObject::SliceableObject() : AObject(), Sliceable()
{
}


SliceableObject::~SliceableObject()
{
}


const Material *SliceableObject::glMaterial() const
{
  return &material();
}


const AObjectPalette* SliceableObject::glPalette( unsigned ) const
{
  return getOrCreatePalette();
}


void SliceableObject::glSetChanged( glPart p, bool x ) const
{
  //cout << "SliceableObject::glSetChanged " << p << ", " << x << endl;
  GLComponent::glSetChanged( p, x );
  if( x )
    obsSetChanged( p );
}


void SliceableObject::glSetTexImageChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexImageChanged( x, tex );
  if( x )
    obsSetChanged( glTEXIMAGE_NUM + tex * 2 );
}


void SliceableObject::glSetTexEnvChanged( bool x, unsigned tex ) const
{
  GLComponent::glSetTexEnvChanged( x, tex );
  if( x )
    obsSetChanged( glTEXENV_NUM + tex * 2 );
}


vector<float> SliceableObject::glVoxelSize() const
{
  return voxelSize();
}


vector<float> SliceableObject::glMin2D() const
{
  vector<float> bmin, bmax;
  boundingBox2D( bmin, bmax );
  return bmin;
}


vector<float> SliceableObject::glMax2D() const
{
  vector<float> bmin, bmax;
  boundingBox2D( bmin, bmax );
  return bmax;
}


const anatomist::Referential* SliceableObject::getReferential() const
{
  return AObject::getReferential();
}


