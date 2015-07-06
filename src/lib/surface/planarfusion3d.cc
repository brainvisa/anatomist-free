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
#include <anatomist/surface/planarfusion3d.h>
#include <anatomist/window/Window.h>
#include <anatomist/object/sliceable.h>
#include <anatomist/misc/error.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transformobserver.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/object/actions.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/window/glcaps.h>
#include <anatomist/control/qObjTree.h>
#include <anatomist/application/settings.h>
#include <aims/resampling/quaternion.h>
#include <graph/tree/tree.h>
#include <qpixmap.h>
#include <qtranslator.h>
#include <float.h>

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


int PlanarFusion3D::registerClass()
{
  int	type = registerObjectType( "PlanarFusion3D" );
  return type;
}


struct PlanarFusion3D::Private
{
  Private() 
    : xmin( 0 ), xmax( 0 ), ymin( 0 ), ymax( 0 ), refreshVTexture( true )
  {}

  float		xmin;
  float		xmax;
  float		ymin;
  float		ymax;
  Geometry	geometry;
  Quaternion	quat;
  Point3df	origin;
  mutable map<unsigned, vector<Point2df> >	vtexture;
  mutable bool					refreshVTexture;
};


PlanarFusion3D::PlanarFusion3D( const vector<AObject *> & obj ) 
  : GLObjectVector(), d( new PlanarFusion3D::Private )
{
  _type = classType();

  if( QObjectTree::TypeNames.find( _type ) == QObjectTree::TypeNames.end() )
    {
      string str = Settings::findResourceFile( "icons/list_planarfusion.xpm" );
      if( !QObjectTree::TypeIcons[ _type ].load( str.c_str() ) )
      {
        QObjectTree::TypeIcons.erase( _type );
        cerr << "Icon " << str.c_str() << " not found\n";
      }

      QObjectTree::TypeNames[ _type ] = "Planar fusion";
    }

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
  /* cout << "planarFusion: surfaces: " << surf.size() << ", sliceables: " 
     << vol.size() << endl; */

  if( !surf.empty() )
    setReferentialInheritance( *surf.begin() );
  for( io=surf.begin(), fo=surf.end(); io!=fo; ++io )
    insert( *io );
  for( io=vol.begin(), fo=vol.end(); io!=fo; ++io )
    insert( *io );

  glAddTextures( 1 );
  SetMaterial( mesh()->GetMaterial() );
}


PlanarFusion3D::~PlanarFusion3D()
{
  iterator	i = begin();
  erase( i );
  i = begin();
  erase( i );
  delete d;
}


int PlanarFusion3D::classType()
{
  static int	_classType = registerClass();
  return _classType;
}


const AObject* PlanarFusion3D::volume() const
{
  return *(++begin());
}


AObject* PlanarFusion3D::volume()
{
  return *(++begin());
}


const AObject* PlanarFusion3D::mesh() const
{
  return *begin();
}


AObject* PlanarFusion3D::mesh()
{
  return *begin();
}


GLComponent* PlanarFusion3D::glTexture( unsigned )
{
  return this;
}


const AObjectPalette* PlanarFusion3D::glPalette( unsigned ) const
{
  return volume()->palette();
}


const GLComponent* PlanarFusion3D::glTexture( unsigned ) const
{
  return this;
}


void PlanarFusion3D::update( const Observable* observable, void* arg )
{
  // cout << "PlanarFusion3D::update\n";

  AObject::update( observable, arg );

  const AObject	*obj = dynamic_cast<const AObject *>( observable );

  if( !obj )
    {
      const TransformationObserver 
        *to = dynamic_cast<const TransformationObserver *>( observable );
      if( !to )
        return;

      //cout << "Transformation changed in PlanarFusion3D\n";
      glSetTexImageChanged( true, 0 );
      obsSetChanged( GLComponent::glTEXIMAGE );
      d->refreshVTexture = true;
      glSetChanged( glBODY );
      notifyObservers((void*)this);
      return;
    }

  // cout << "obj name: " << obj->name() << endl;

  iterator	io = find( obj );
  if( io == end() )
    return;

  if( obj->hasReferenceChanged() )
    {
      // cout << "ref changed\n";
      glSetTexImageChanged( true, 0 );
      glSetChanged( glBODY );
      d->refreshVTexture = true;
    }

  const GLComponent	*tr = obj->glAPI();

  if( tr )
    {
      const Sliceable	*sl = tr->sliceableAPI();
      if( sl )
        {
          // cout << "volume changed\n";
          // volume changed
          // if the functional volume palette has changed
          if( obj->obsHasChanged( glTEXIMAGE ) )
            {
              // cout << "teximage changed\n";
              glSetTexImageChanged( true, 0 );
            }
        }
      else
        {
          // cout << "surface changed\n";
          // surface changed
          if( obj->obsHasChanged( glMATERIAL ) )
            glSetChanged( glMATERIAL );
          // if the volume reference has changed
          if( obj->obsHasChanged( glGEOMETRY ) )
            {
              // cout << "surface geometry changed\n";
              glSetChanged( glGEOMETRY );
              glSetTexImageChanged( true, 0 );
              d->refreshVTexture = true;
            }
        }
    }

  // cout << "updateSubObjectReferential\n";
  updateSubObjectReferential( obj );
  // cout << "planarFusion notify: " << this << endl;
  notifyObservers((void*)this);
  // cout << "planarFusion update done\n";
}


void  PlanarFusion3D::glSetTexImageChanged( bool x, unsigned ) const
{
  GLComponent::glSetTexImageChanged( x );
}


void PlanarFusion3D::glClearHasChangedFlags() const
{
  GLObjectVector::clearChanged();
  d->refreshVTexture = false;
}


Tree* PlanarFusion3D::optionTree() const
{
  static Tree*	_optionTree = 0;

  if( !_optionTree )
    {
      Tree	*t, *t2;
      _optionTree = new Tree( true, "option tree" );
      t = new Tree( true, QT_TRANSLATE_NOOP( "QSelectMenu", "File" ) );
      _optionTree->insert( t );
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

      t = new Tree( true, "Referential" );
      _optionTree->insert( t );
      t2 = new Tree( true, "Load" );
      t2->setProperty( "callback", &ObjectActions::referentialLoad );
      t->insert( t2 );
    }
  return( _optionTree );
}


unsigned PlanarFusion3D::glNumTextures() const
{
  return 1;
}


unsigned PlanarFusion3D::glNumTextures( const ViewState & ) const
{
  return 1;
}


unsigned PlanarFusion3D::glDimTex( const ViewState &, unsigned ) const
{
  return 2;
}


unsigned PlanarFusion3D::glTexCoordSize( const ViewState & state, 
                                         unsigned ) const
{
  refreshTexCoords( state );

  const AObject	*functional = volume();
  unsigned	t = (unsigned) (state.time / functional->TimeStep() );
  map<unsigned, vector<Point2df> >::const_iterator it = d->vtexture.find( t );

  if( it == d->vtexture.end() )
    return( 0 );
  else
    {
      //cout << "texCoordSize : " << (*it).second.size() << endl;
      return( (*it).second.size() );
    }
}


const GLfloat* PlanarFusion3D::glTexCoordArray( const ViewState & state, 
                                                unsigned ) const
{
  refreshTexCoords( state );

  const AObject	*functional = volume();
  unsigned	t = (unsigned) (state.time / functional->TimeStep() );
  map<unsigned, vector<Point2df> >::const_iterator it = d->vtexture.find( t );

  if( it == d->vtexture.end() )
    return( 0 );
  else
    return( (GLfloat *) &(*it).second[0] );
}


bool PlanarFusion3D::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  return mesh()->boundingBox( bmin, bmax );
}


bool PlanarFusion3D::refreshTexCoords( const ViewState & state ) const
{
  const AObject	*vol = volume();
  unsigned	time = (unsigned) (state.time / vol->TimeStep() );

  if( d->refreshVTexture )
    d->vtexture.clear();

  map<unsigned, vector<Point2df> >::const_iterator 
    it = d->vtexture.find( time );
  if( it != d->vtexture.end() )
    return true;	// in cache

  //cout << "PlanarFusion3D::refreshTexCoords\n";

  const GLComponent	*surf = ( *begin() )->glAPI();
  //cout << "surf: " << surf << endl;
  unsigned		nver = surf->glNumVertex( state );
  //cout << "vertices: " << nver << endl;

  if( nver == 0 )
    {
      // empty mesh
      d->refreshVTexture = false;
      glSetTexImageChanged( true, 0 );
      return false;
    }
  Point3df	n = ((const Point3df *) surf->glNormalArray( state ))[0], u, v;

  n.normalize();
  /* if( n[0] != 0 || n[1] != 0 )
    u = Point3df( -n[1], n[0], 0 );
  else
    u = Point3df( 0, -n[2], n[1] );
  u.normalize();
  v = Point3df( n[1] * u[2] - n[2] * u[1], 
		n[2] * u[0] - n[0] * u[2], 
		n[0] * u[1] - n[1] * u[0] );
  v.normalize();

  cout << "u: " << u << endl;
  cout << "v: " << v << endl;
  cout << "n: " << n << endl; */

  Quaternion	quat;

  /* vector<float>	rot( 12 );
  rot[0] = u[0];
  rot[1] = u[1];
  rot[2] = u[2];
  rot[3] = 0;
  rot[4] = v[0];
  rot[5] = v[1];
  rot[6] = v[2];
  rot[7] = 0;
  rot[8] = n[0];
  rot[9] = n[1];
  rot[10] = n[2];
  rot[11] = 0;
  quat.buildFromMatrix( &rot[0] ); */

  if( n[0] != 0 || n[1] != 0 )
    u = Point3df( -n[1], n[0], 0 );
  else
    u = Point3df( 0, 1, 0 );
  float	alpha = - ::acos( n[2] );
  quat.fromAxis( u, alpha );
  d->quat = quat;

  u = quat.apply( Point3df( 1, 0, 0 ) );
  v = quat.apply( Point3df( 0, 1, 0 ) );
  n = quat.apply( Point3df( 0, 0, 1 ) );

  /* cout << "u: " << u << endl;
  cout << "v: " << v << endl;
  cout << "n: " << n << endl; */

  // ---

  unsigned		i;

  if( nver == 0 )	// empty mesh
    return false;

  vector<Point2df>	& vtexture = d->vtexture[(unsigned) time];

  vtexture.erase( vtexture.begin(), vtexture.end() );
  
  //cout << "texturing mesh of " << nver << " vertices\n";
  vtexture.reserve( nver );

  const Point3df	*vert 
    = (const Point3df *) surf->glVertexArray( state );
  Point3df		pos = vert[0], pos2;
  float			x, y;

  d->xmin = FLT_MAX;
  d->xmax = -FLT_MAX;
  d->ymin = FLT_MAX;
  d->ymax = -FLT_MAX;

  for( i=0; i<nver; ++i )
    {
      x = vert[i][0] * u[0] + vert[i][1] * u[1] + vert[i][2] * u[2];
      y = vert[i][0] * v[0] + vert[i][1] * v[1] + vert[i][2] * v[2];
      if( x < d->xmin )
	d->xmin = x;
      if( x > d->xmax )
	d->xmax = x;
      if( y < d->ymin )
	d->ymin = y;
      if( y > d->ymax )
	d->ymax = y;
      vtexture.push_back( Point2df( x, y ) );
    }

  float	pl = pos.dot( n );
  pos = n * pl + u * d->xmin + v * d->ymin;
  d->origin = pos;

  // handle geometry and bounds

  Point3df	vs = vol->VoxelSize();

  // we must select geometry in the volume local referential
  // (code copied from Window3D::updateWindowGeometry() )
  Referential	*sref = mesh()->getReferential(), 
    *vref = vol->getReferential();
  Transformation	*tr = 0;
  Point3df		uv( 1, 0, 0 ), vv( 0, 1, 0), wv( 0, 0, 1 ), s;
  if( sref && vref )
    {
      tr = theAnatomist->getTransformation( vref, sref );
      if( tr )
	{
	  uv = tr->transform( uv ) - tr->transform( Point3df( 0, 0, 0 ) );
	  vv = tr->transform( vv ) - tr->transform( Point3df( 0, 0, 0 ) );
	  wv = tr->transform( wv ) - tr->transform( Point3df( 0, 0, 0 ) );
	}
    }
  uv = quat.apply( uv );
  vv = quat.apply( vv );
  wv = quat.apply( wv );
  s = Point3df( 1. / max( max( fabs( uv[0] / vs[0] ), fabs( uv[1] / vs[1] ) ), 
			  fabs( uv[2] / vs[2] ) ), 
		1. / max( max( fabs( vv[0] / vs[0] ), fabs( vv[1] / vs[1] ) ), 
			  fabs( vv[2] / vs[2] ) ), 
		1. / max( max( fabs( wv[0] / vs[0] ), fabs( wv[1] / vs[1] ) ), 
			  fabs( wv[2] / vs[2] ) ) );

  pos2 = Point3df( 1, 0, 0 ) * ( d->xmax - d->xmin ) 
    + Point3df( 0, 1, 0 ) * ( d->ymax - d->ymin );
  Point4dl	dmin( 0, 0, 0, 0 ), dmax( 0, 0, 0, (int) vol->MaxT() );
  /*dmin[0] = (int) ::ceil( pos[0] / s[0] );
  dmin[1] = (int) ::ceil( pos[1] / s[1] );
  dmin[2] = (int) ::ceil( pos[2] / s[2] );*/
  dmax[0] = (int) rint( pos2[0] / s[0] ) + dmin[0];
  dmax[1] = (int) rint( pos2[1] / s[1] ) + dmin[1];
  dmax[2] = (int) rint( pos2[2] / s[2] ) + dmin[2];
  d->geometry = Geometry( s, dmin, dmax );

  //cout << "geom: " << dmin << ", max: " << dmax << endl;
  int		xdim = dmax[0]+1, ydim = dmax[1]+1;
  unsigned	effectiveWidth = 0x1 << (unsigned) ::ceil( ::log( xdim ) 
                                                         / ::log(2) );
  unsigned	effectiveHeight 
    = 0x1 << (unsigned) ::ceil( ::log( ydim ) / ::log(2) );

  // apply tex coords rescaling

  float	xnorm = (1.-1e-5) / (d->xmax - d->xmin) 
    * xdim / effectiveWidth;
  float ynorm = (1.-1e-5) / (d->ymax - d->ymin) 
    * ydim / effectiveHeight;
  for( i=0; i<nver; ++i )
    vtexture[i] = Point2df( ( vtexture[i][0] - d->xmin ) * xnorm, 
			    ( vtexture[i][1] - d->ymin ) * ynorm );

  // ---

  d->refreshVTexture = false;
  glSetTexImageChanged( true, 0 );

  //cout << "refreshTexCoords done\n";
  return true;
}


VolumeRef<AimsRGBA> PlanarFusion3D::glBuildTexImage(
  const ViewState & state, unsigned tex, int dimx, int dimy,
  bool /*useTexScale*/ ) const
{
  refreshTexCoords( state );

  const GLComponent     *surf = mesh()->glAPI();
  if( surf->glNumVertex( state ) == 0 )
    return VolumeRef<AimsRGBA>();

  const Sliceable       *vol = volume()->glAPI()->sliceableAPI();

  AImage        image;
  int           xdim = d->geometry.DimMax()[0]+1;
  int           ydim = d->geometry.DimMax()[1]+1;
  if( dimx > 0 )
    xdim = dimx;
  if( dimy > 0 )
    ydim = dimy;
  image.width = xdim;
  image.height = ydim;
//   if( !useTexScale )
//   {
//     image.effectiveWidth = xdim;
//     image.effectiveHeight = ydim;
//   }
//   else
//   {
    image.effectiveWidth = 0x1 << (unsigned) ::ceil( ::log( image.width )
                                                  / ::log(2) );
    image.effectiveHeight = 0x1 << (unsigned) ::ceil( ::log( image.height )
                                                    / ::log(2) );
//   }
  image.depth = 32;

  VolumeRef<AimsRGBA> teximage( image.effectiveWidth, image.effectiveHeight );

  image.data = (char *) &teximage( 0 );

  SliceViewState        sst( state.time, true, d->origin, &d->quat,
                             getReferential(), &d->geometry );
  bool  retcode = vol->update2DTexture( image, d->origin, sst );

  if( retcode )
    return teximage;

  return VolumeRef<AimsRGBA>();
}


bool PlanarFusion3D::glMakeTexImage( const ViewState & state, 
                                     const GLTexture & gltex, 
                                     unsigned tex ) const
{
  // cout << "PlanarFusion3D::glMakeTexImage\n";

  refreshTexCoords( state );

  /* SliceViewState	sst( state.time, true, d->origin, &d->quat, 
                             getReferential(), &d->geometry );
    return volume()->glAPI()->glMakeTexImage( sst, gltex, tex ); */

  const GLComponent	*surf = mesh()->glAPI();
  if( surf->glNumVertex( state ) == 0 )
    return false;

  const Sliceable	*vol = volume()->glAPI()->sliceableAPI();

  AImage	image;
  int		xdim = d->geometry.DimMax()[0]+1;
  int		ydim = d->geometry.DimMax()[1]+1;
  image.width = xdim;
  image.height = ydim;
  image.effectiveWidth = 0x1 << (unsigned) ::ceil( ::log( image.width ) 
						 / ::log(2) );
  image.effectiveHeight = 0x1 << (unsigned) ::ceil( ::log( image.height ) 
						  / ::log(2) );
  image.depth = 32;
  image.data = new char[ image.effectiveWidth * image.effectiveHeight * 4 ];

  /* cout << "tex dims: " << image.width << " x " << image.height << endl;
     cout << image.effectiveWidth << " x " << image.effectiveHeight << endl; */

  // ---

  SliceViewState	sst( state.time, true, d->origin, &d->quat, 
                             getReferential(), &d->geometry );
  bool	retcode = vol->update2DTexture( image, d->origin, sst );

  if( retcode )
    {
      GLuint	texName = gltex.item();
      unsigned	texu = tex;

      GLCaps::glActiveTexture( GLCaps::textureID( texu ) );

      if( !texName )
        {
          GLenum status = glGetError();
          cerr << "OpenGL error: " << gluErrorString(status) << endl;
          retcode = false;
        }
      else
        {
          glBindTexture( GL_TEXTURE_2D, texName );
          glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
          glTexImage2D( GL_TEXTURE_2D, 0, 4, image.effectiveWidth, 
                        image.effectiveHeight, 0, GL_RGBA, 
                        GL_UNSIGNED_BYTE, (GLvoid*) image.data );

          GLenum status = glGetError();
          if( status != GL_NO_ERROR )
            {
              cerr << "OpenGL error tex: " << gluErrorString(status) << endl;
              retcode = false;
            }
        }
    }

  delete[] image.data;

  return retcode;
}


GLComponent::glTextureMode PlanarFusion3D::glTexMode( unsigned tex ) const
{
  return volume()->glAPI()->glTexMode( tex );
}


float PlanarFusion3D::glTexRate( unsigned tex ) const
{
  return volume()->glAPI()->glTexRate( tex );
}


GLComponent::glTextureFiltering 
PlanarFusion3D::glTexFiltering( unsigned tex ) const
{
  return volume()->glAPI()->glTexFiltering( tex );
}


GLComponent::glAutoTexturingMode 
PlanarFusion3D::glAutoTexMode( unsigned tex ) const
{
  return volume()->glAPI()->glAutoTexMode( tex );
}


const float *PlanarFusion3D::glAutoTexParams( unsigned coord, 
                                              unsigned tex ) const
{
  return volume()->glAPI()->glAutoTexParams( coord, tex );
}


void PlanarFusion3D::glSetAutoTexParams( const float* params, unsigned coord, 
                                         unsigned tex )
{
  volume()->glAPI()->glSetAutoTexParams( params, coord, tex );
}


AObject* PlanarFusion3D::fallbackReferentialInheritance() const
{
  return *begin();
}


