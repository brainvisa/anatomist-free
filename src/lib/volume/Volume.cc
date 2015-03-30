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
#include <math.h>
#include <anatomist/window/glwidget.h>
#include <cartobase/config/cartobase_config.h>
#include <anatomist/volume/Volume.h>
#include <anatomist/object/objectmenu.h>
#include <anatomist/window/Window.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Geometry.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/misc/error.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/object/actions.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/color/colortraits.h>
#include <graph/tree/tree.h>
#include <aims/resampling/quaternion.h>
#include <aims/rgb/rgb.h>
#include <aims/data/pheader.h>
#include <aims/io/finder.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <cartobase/object/object.h>
#include <qapplication.h>
#include <qmessagebox.h>

//#define USE_TEXTURE_3D

using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace anatomist
{
  namespace internal
  {
    template<typename T> class VolumeTraits;
  }
}


namespace anatomist
{

  template<typename T> struct AVolume<T>::PrivateData
  {
    PrivateData( AVolume* vol );
    ~PrivateData();

    GenericObject		*attrib;
    internal::VolumeTraits<T>	traits;
  };

}

namespace anatomist
{

  namespace internal
  {

    // traits

    template<typename T> class VolumeScalarTraits
    {
    public:
      VolumeScalarTraits( AVolume<T> *vol );
      void adjustPalette();
      void setExtrema();
      T minTypedTexValue() const;
      T maxTypedTexValue() const;
      float mixedTexValue( unsigned x, unsigned y, unsigned z, 
                           unsigned t ) const;
      vector<float> texValue( unsigned x, unsigned y, unsigned z,
                              unsigned t ) const;

    private:
      AVolume<T>	*volume;
      T	mini;
      T	maxi;
    };


    template<typename T> class VolumeVectorTraits
    {
    public:
      VolumeVectorTraits( AVolume<T> *vol );
      void adjustPalette();
      void setExtrema();
      T minTypedTexValue() const;
      T maxTypedTexValue() const;
      float mixedTexValue( unsigned x, unsigned y, unsigned z, 
                           unsigned t ) const;
      vector<float> texValue( unsigned x, unsigned y, unsigned z,
                              unsigned t ) const;

    private:
      AVolume<T>	*volume;
      T		minT;
      T		maxT;
    };


    template<typename T> struct VolumeTraitsType
    {
      typedef VolumeScalarTraits<T> traitstype;
    };


    // high level traits
    template<typename T> class VolumeTraits
    {
    public:
      VolumeTraits( AVolume<T> *vol );
      void adjustPalette() { vttype.adjustPalette(); }
      void setExtrema() { vttype.setExtrema(); }
      T minTypedTexValue() const { return vttype.minTypedTexValue(); }
      T maxTypedTexValue() const { return vttype.maxTypedTexValue(); }
      float mixedTexValue( unsigned x, unsigned y, unsigned z,
                           unsigned t ) const
      { return vttype.mixedTexValue( x, y, z, t ); }
      vector<float> texValue( unsigned x, unsigned y, unsigned z,
                              unsigned t ) const
      { return vttype.texValue( x, y, z, t ); }

    private:
      typename VolumeTraitsType<T>::traitstype	vttype;
    };

    template<> struct VolumeTraitsType<AimsRGB>
    {
      typedef VolumeVectorTraits<AimsRGB> traitstype;
    };


    template<> struct VolumeTraitsType<AimsRGBA>
    {
      typedef VolumeVectorTraits<AimsRGBA> traitstype;
    };


    template<typename T, long D> struct VolumeTraitsType<AimsVector<T,D> >
    {
      typedef VolumeVectorTraits<AimsVector<T,D> > traitstype;
    };

  }

}


template<typename T>
AVolume<T>::PrivateData::PrivateData( AVolume<T>* vol ) 
  : attrib( 0 ), traits( vol )
{
}


template<typename T>
AVolume<T>::PrivateData::~PrivateData()
{
  delete attrib;
}


// ----

AVolumeBase::AVolumeBase() : SliceableObject(), PythonAObject()
{}

AVolumeBase::~AVolumeBase()
{}

void AVolumeBase::setShaderParameters(const Shader &shader, const ViewState & state) const
{
  shader.setShaderParameters(*this, state);
}


// ----

template <class T>
AVolume<T>::AVolume( const string & fname )
  : AVolumeBase(), 
    d( new PrivateData( this ) ), 
    _volume( new Volume<T> )
{
  _type = AObject::VOLUME;
  setFileName( fname );
  glAddTextures( 1 );
  glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  te.scaled = false;
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


template <class T>
AVolume<T>::AVolume( const AimsData<T> & aims )
  : AVolumeBase(), 
    d( new PrivateData( this ) ), 
    _volume( aims.volume() )
{
  _type = AObject::VOLUME;
  glAddTextures( 1 );
  glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


template <class T>
AVolume<T>::AVolume( rc_ptr<AimsData<T> > aims )
  : AVolumeBase(), 
    d( new PrivateData( this ) ), _volume( aims->volume() )
{
  _type = AObject::VOLUME;
  glAddTextures( 1 );
  glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


template <class T>
AVolume<T>::AVolume( rc_ptr<Volume<T> > aims )
  : AVolumeBase(),
    d( new PrivateData( this ) ), _volume( aims )
{
  _type = AObject::VOLUME;
  glAddTextures( 1 );
  glSetTexMode( glDECAL );
  TexExtrema  & te = glTexExtrema( 0 );
  te.min.push_back( 0 );
  te.max.push_back( 0 );
  te.minquant.push_back( 0 );
  te.maxquant.push_back( 0 );
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


template <typename T>
AObject* AVolume<T>::clone( bool shallow )
{
  AVolume<T>  *avol = 0;
  if( shallow )
    avol = new AVolume<T>( _volume );
  else
    avol = new AVolume<T>( rc_ptr<Volume<T> >( new Volume<T>( *_volume ) ) );
  avol->setFileName( fileName() );
  avol->SetExtrema();
  avol->adjustPalette();
  return avol;
}


template <class T>
AVolume<T>::~AVolume()
{
  cleanup();
  delete d;
}


static const float	c = 1. / ::sqrt( 2 );
static const float	eps = 1.e-5;
static const float      eps2 = 1.e-10;


template <class T>
bool AVolume<T>::update2DTexture( AImage & ximage, const Point3df & pos, 
                                  const SliceViewState & state, 
                                  unsigned /*tex*/ ) const
{
  /* cout << "AVolume<" << DataTypeCode<T>::name()
      << ">::update2DTexture, pos : " << pos << "\n"; */
  const Referential	*objref;
  Transformation	*tra = 0;
  bool			owntr = false;
  const Quaternion	& quat = *state.orientation;
  const Referential	*winref = state.winref;
  const Geometry	*wingeom = state.wingeom;
  float			time = state.time;

  objref = getReferential();
  if( winref && objref != winref )
    {
      tra = theAnatomist->getTransformation( winref, objref );
      /* cout << "refs : obj : " << objref << ", win : " << winref 
         << ", tr : " << tra << endl; */
    }

  Point3df	u = quat.apply( Point3df( 1, 0, 0 ) ),
    v = quat.apply( Point3df( 0, 1, 0 ) ), 
    w = quat.apply( Point3df( 0, 0, 1 ) );

  Quaternion	q;
  if( tra )
    q = tra->quaternion() * quat;
  else
    q = quat;

  if( time < 0 )
    time = 0;
  else if( time > _volume->getSizeT() - 1 )
    time = _volume->getSizeT() - 1;

  Point4df		vec = q.vector();
  Point3df		gs = wingeom->Size();
  // cout << "quaternion : " << vec << endl;
  vector<float> vs;
  _volume->header().getProperty( "voxel_size", vs );
  while( vs.size() < 3 )
    vs.push_back( 1. );

  if( ( vec - Point4df( 0, 0, 0, 1 ) ).norm2() < eps2 )	// ID : axial
  {
    // cout << "axial orientation\n";
    if( fabs( gs[0] - vs[0] ) <= eps
        && fabs( gs[1] - vs[1] ) <= eps )
    {
      Point3df	post( pos );
      if( tra )
        post = tra->transform( pos );
      //cout << "axial optimized\n";
      updateAxial( &ximage, post, time );
      return true;
    }
  }
  else if( ( vec - Point4df( c, 0, 0, c ) ).norm2() < eps2 )	// coronal
  {
    // cout << "coronal orientation\n";
    if( fabs( gs[0] - vs[0] ) <= eps
      && fabs( gs[1] - vs[2] ) <= eps )
    {
      Point3df	post( pos );
      if( tra )
        post = tra->transform( pos );
      // cout << "coronal optimized\n";
      updateCoronal( &ximage, post, time );
      return true;
    }
  }
  else if( ( vec - Point4df( -0.5, -0.5, -0.5, 0.5 ) ).norm2() < eps2 )	// sagittial
  {
    // cout << "sagittal orientation\n";
    if( fabs( gs[0] - vs[1] ) <= eps && fabs( gs[1] - vs[2] ) <= eps )
    {
      Point3df	post( pos );
      if( tra )
        post = tra->transform( pos );
      // cout << "sagittal optimized\n";
      updateSagittal( &ximage, post, time );
      return true;
    }
  }

  if( !tra )
  {
    tra = new Transformation( 0, 0 );
    owntr = true;
  }
  updateSlice( ximage, pos, time, tra, u, v, wingeom );
  if( owntr )
    delete tra;

  return true;
}


namespace
{

  bool _hasInterpolation( const PythonAObject *volume )
  {
    int       intp = true;
    const GenericObject *ph = volume->attributed();
    if( ph )
      try
      {
        Object o = ph->getProperty( "volumeInterpolation" );
        if( !o.isNull() )
          return bool( o->getScalar() );
      }
      catch( ... )
      {
      }
    theAnatomist->config()->getProperty( "volumeInterpolation", intp );
    return (bool) intp;
  }

}

template <class T>
void AVolume<T>::updateSlice( AImage & image, const Point3df & p0, float time, 
			      const Transformation* tra, const Point3df & inc, 
			      const Point3df & offset, 
			      const Geometry* wingeom ) const
{
  /* p0 is the center of the first voxel */

  /*cout << "AVolume::updateSlice, dims : " << image.width << " x " 
    << image.height << "\n";*/


  // interpolation flag: "Plus Proche Voisin" (nearest neighbour)
  bool	ppv = !_hasInterpolation( this );

  ColorTraits<T>	coltraits( getOrCreatePalette(), 
				   d->traits.minTypedTexValue(), 
				   d->traits.maxTypedTexValue() );
  T iempty = coltraits.neutralColor();
  AimsRGBA empty = coltraits.color( iempty );

  //	image
  Point3df	vs = VoxelSize(), gs = wingeom->Size();
  int		x, y;
  AimsRGBA	*pdat = (AimsRGBA *) image.data;
  T		val = 0;
  float		dx = _volume->getSizeX() - 1, dy = _volume->getSizeY() - 1, dz = _volume->getSizeZ() - 1;
  int		t = (int) time;

  // cout << "wingeom : " << gs[0] << ", " << gs[1] << ", " << gs[2] << endl;
  Point3df	pf = Transformation::transform( p0, tra, /*gs,*/ vs );
  Point3df	incd 
    = Transformation::transform( p0 + inc * gs[0], tra, /*gs,*/ vs ) - pf;
  Point3df	offsd 
    = Transformation::transform( p0 + offset * gs[1], tra, /*gs,*/ vs ) - pf;
  Point3df	pxd;
  Point3dl	pfi;
  float		wx, wy, wz, wx2, wy2;
  const T *fp = &_volume->at( 0 );
  const T *pim0 = fp + ( &_volume->at( 0, 0, 0, 1 ) - fp ) * t;
  const T *pim;
  long		dyi = &_volume->at( 0, 1 ) - fp;
  long		dzi = &_volume->at( 0, 0, 1 ) - fp;
  long		dyxi = dyi + 1;
  long		dzyi = dzi + dyi;
  long		dzxi = dzi + 1;
  long		dzyxi = dzyi + 1;
  long		offset_xim
    = (image.effectiveWidth - image.width) * ( image.depth / 32 );

  if( ppv )	// no interpolation
    {
      // advance 1/2 voxel for rounding float->int conversion
      // Point3df	p1 = p0 + inc * gs[0] * 0.5F + offset * gs[1] * 0.5F;
      Point3df	p1 = p0;
      pf = Transformation::transform( p1, tra, vs );

      for( y=0; y<image.height; ++y )
	{
	  pxd = pf;
	  for( x=0; x<image.width; ++x )
	    {
	      pfi = Point3dl( (int) rint( pf[0] ), (int) rint( pf[1] ), 
                             (int) rint( pf[2] ) );
	      if( pfi[0] < 0 || pfi[0] > dx || pfi[1] < 0 || pfi[1] > dy 
		  || pfi[2] < 0 || pfi[2] > dz )
		val = iempty;
	      else
		val = *( pim0 + dzi * pfi[2] + dyi * pfi[1] + pfi[0] );
	      
	      *pdat++ = coltraits.color( val );

	      pf += incd;
	    }
	  pf = pxd + offsd;
	  pdat += offset_xim;
	}
    }

  else		// interpolation
    {
      //cout << "fill image - interpolation\n";
      /*--dx;
      --dy;
      --dz;*/
      long	nextx, nexty, nextyx, nextz, nextzy, nextzx, nextzyx;
      bool	done;

      for( y=0; y<image.height; ++y )
        {
          pxd = pf;
          for( x=0; x<image.width; ++x )
            {
              done = false;
              pfi = Point3dl( (int) pf[0], (int) pf[1], (int) pf[2] );
              if( pf[0] < 0 || pf[1] < 0 || pf[2] < 0 )
                {
                  val = iempty;
                  done = true;
                }
              else
                {
                  nextx = 1;
                  nexty = dyi;
                  nextyx = dyxi;
                  nextz = dzi;
                  nextzx = dzxi;
                  nextzy = dzyi;
                  nextzyx = dzyxi;

                  if( pfi[0] >= dx )
                  {
                    if( pfi[0] > dx )
                      {
                        val = iempty;
                        done = true;
                      }
                    else
                      {
                        nextx = 0;
                        nextyx = dyi;
                        nextzx = dzi;
                        nextzyx = dzyi;
                      }
                  }
                  if( pfi[1] >= dy )
                  {
                    if( pfi[1] > dy )
                      {
                        val = iempty;
                        done = true;
                      }
                    else
                      {
                        nexty = 0;
                        nextyx = nextx;
                        nextzy = dzi;
                        nextzyx = dzi + nextx;
                      }
                  }
                  if( pfi[2] >= dz )
                  {
                    if( pfi[2] > dz )
                      {
                        val = iempty;
                        done = true;
                      }
                    else
                      {
                        nextz = 0;
                        nextzx = nextx;
                        nextzy = nexty;
                        nextzyx = nextyx;
                      }
                  }

                  if( !done )
                    {
                      pim = pim0 + dzi * pfi[2] + dyi * pfi[1] + pfi[0];
                      val = *pim;

                      wx = pf[0] - pfi[0];
                      wx2 = 1. - wx;
                      wy = pf[1] - pfi[1];
                      wy2 = 1. - wy;
                      wz = pf[2] - pfi[2];
                      val = (T)
                        ( ( ( ( wx <= 0 ? 0 : *(pim+nextx) * wx )
                              + val * wx2 ) * wy2
                            + ( wy <= 0 ? 0 :
                                ( ( wx <= 0 ? 0 : *(pim+nextyx) * wx )
                                  + *(pim+nexty) * wx2 ) * wy ) ) * (1. - wz)
                          + ( wz <= 0 ? 0 :
                              ( ( ( wx <= 0 ? 0 : *(pim+nextzx) * wx )
                                  + *(pim+nextz) * wx2 ) * wy2
                                + ( wy <= 0 ? 0 :
                                    ( ( wx <= 0 ? 0 : *(pim+nextzyx) * wx )
                                      + *(pim+nextzy) * wx2 ) * wy ) )
                              * wz ) );
                    }
                }

              *pdat++ = coltraits.color( val );
              pf += incd;
            }
          pf = pxd + offsd;
          pdat += offset_xim;
        }
    }
  //cout << "OK updateSlice\n";
}


namespace
{

  void fillBlack( AImage * im, const AimsRGBA & col )
  {
    int	y, w = im->width, wl = im->effectiveWidth - w, h = im->height, x;
    AimsRGBA  *buf = (AimsRGBA *) im->data;

    for( y=0; y<h; ++y )
      {
        for( x=0; x<w; ++x )
          *buf++ = col;
        buf += wl;
      }
  }

}

template <class T>
void AVolume<T>::updateAxial( AImage *ximage, const Point3df & pf0, 
			      float time ) const
{
  // cout << "UpdateAxial simple, pos : " << pf0 << " on " << name() << endl;
  long	dx, dxx, dy;		// Dimensions du volume
  long	dslice;			// Taille d'une coupe

  Point3df vs = VoxelSize();
  Point3df	p0 = Point3df( rint( pf0[0] / vs[0] ),
                               rint( pf0[1] / vs[1] ),
                               rint( pf0[2] / vs[2] ) );
  // cout << "p0: " << p0 << endl;

  ColorTraits<T>	coltraits( getOrCreatePalette(),
                                   d->traits.minTypedTexValue(), 
                                   d->traits.maxTypedTexValue() );
  AimsRGBA empty = coltraits.color( coltraits.neutralColor() );

  if( vs[2] == 0 )
  {
    fillBlack( ximage, empty );
    return;
  }
  int	zz = (int) p0[2];
  if ( zz >= _volume->getSizeZ() || zz < 0 )
  {
    fillBlack( ximage, empty );
    return;
  }

  dx = max( min( ximage->width, _volume->getSizeX() - int( p0[0] ) ), 0 )
    - min( max( 0, - int( p0[0] ) ), ximage->width );
  dy = max( min( ximage->height, _volume->getSizeY() - int( p0[1] ) ), 0 )
    - min( max( 0, - int( p0[1] ) ), ximage->height );
  const T *fp = &_volume->at( 0 );
  dxx = &_volume->at( 0, 1 ) - fp;

  dslice = &_volume->at( 0, 0, 1 ) - fp;

  // Initialisation des pointeurs utilises

  const T *ptrori = fp + ( &_volume->at( 0, 0, 0, 1 ) - fp ) * (int)time
    + dslice * zz ;
  AimsRGBA     *ptrpix = (AimsRGBA *) ximage->data;
  int	   offset_xim = (ximage->effectiveWidth - dx) * ( ximage->depth / 32 );
  if( p0[0] < 0 )
    ptrpix -= ( (int) p0[0] ) * ( ximage->depth / 32 );
  else if( p0[0] > 0 )
    ptrori += (int) p0[0];
  if( p0[1] < 0 )
    ptrpix -= ( (int) p0[1] ) * ximage->effectiveWidth
      * ( ximage->depth / 32 );
  else if( p0[1] > 0 )
    ptrori += ( (int) p0[1] ) * _volume->getSizeX();

  //	remove garbage at beginning of image
  AimsRGBA	*p = (AimsRGBA *) ximage->data;
  AimsRGBA	*pend = ( (AimsRGBA *) ximage->data ) 
    + ( (long long) ximage->effectiveWidth ) * ximage->height
    * ( ximage->depth / 32 );
  while( p < ptrpix )
    *p++ = empty;

  /* cout << "taille obj : " << dx << " x " << dy << endl;
  cout << "taille pix : " << ximage->width << " x " << ximage->height << endl;
  cout << "offset_xim : " << offset_xim << endl;
  cout << "depth : " << ximage->depth << endl; */

  int	x, y;

  // dump volume to image

  for( y=0; y<dy; ++y )
  {
    for( x=0; x<dx; ++x )
      *ptrpix++ = coltraits.color( *( ptrori + dxx * y + x ) );
    p = ptrpix + offset_xim;
    if( p > pend )
      p = pend;
    while( ptrpix < p )
      *ptrpix++ = empty;
  }

  // clear garbage at end of image
  while( ptrpix < pend )
    *ptrpix++ = empty;
}

template <class T>
void AVolume<T>::updateCoronal( AImage *ximage, const Point3df &pf0, 
				float time ) const
{
  // cout << "UpdateCoronal simple, p0 : " << pf0 << "\n";
  long	dx, dxx, dz;		// Dimensions du volume
  long	dline;			// Taille d'une ligne

  Point3df vs = VoxelSize();
  Point3df	p0 = Point3df( rint( pf0[0] / vs[0] ),
			       rint( pf0[1] / vs[1] ),
			       rint( pf0[2] / vs[2] ) );
  int	yy = (int) p0[1];

  ColorTraits<T>	coltraits( getOrCreatePalette(), 
				   d->traits.minTypedTexValue(), 
				   d->traits.maxTypedTexValue() );
  AimsRGBA empty = coltraits.color( coltraits.neutralColor() );

  if( time >= _volume->getSizeT() || yy >= _volume->getSizeY() || yy < 0 )
  {
    fillBlack( ximage, empty );
    return;
  }

  int ze = int( p0[2] ) + 1 - ximage->height;
  dx = max( min( ximage->width, _volume->getSizeX() - int( p0[0] ) ), 0 )
    - min( max( 0, - int( p0[0] ) ), ximage->width );
  dz = max( min( ximage->height, _volume->getSizeZ() - ze ), 0 )
    - min( max( 0, - ze ), ximage->height );
  const T* fp = &_volume->at( 0 );
  dxx = &_volume->at( 0, 0, 1 ) - fp;
  dline = &_volume->at( 0, 1 ) - fp;

  /* cout << "taille obj : " << dx << " x " << dz << endl;
  cout << "taille pix : " << ximage->width << " x " << ximage->height << endl;
  */

  // Initialisation des pointeurs utilises

  const T *ptrori = fp + ( &_volume->at( 0, 0, 0, 1 ) - fp ) * (int)time;
  AimsRGBA     *ptrpix = (AimsRGBA *)ximage->data;
  int	   offset_xim = (-ximage->effectiveWidth - dx) * ( ximage->depth / 32 );
  ptrpix += ximage->effectiveWidth * ( dz - 1 ) * ( ximage->depth / 32 );
  int		off_line = dx * ( ximage->depth / 32 );
  if( p0[0] < 0 )
    ptrpix -= ( (int) p0[0] ) * ( ximage->depth / 32 );
  else if( p0[0] > 0 )
    ptrori += (int) p0[0];
  if( p0[2] > _volume->getSizeZ() - 1 )
    ptrpix += ( (int) ( p0[2] - _volume->getSizeZ() + 1 ) )
      * ximage->effectiveWidth * ( ximage->depth / 32 );
  else if( p0[2] < _volume->getSizeZ() - 1 )
    ptrori += ( (int) ( _volume->getSizeZ() - p0[2] - 1 ) )
      * ( &_volume->at( 0, 0, 1 ) - fp );

  //	remove garbage at beginning of image
  AimsRGBA	*p = ptrpix + dx * ( ximage->depth / 32 );
  AimsRGBA	*p2 = ( (AimsRGBA *) ximage->data ) 
    + ( (long long) ximage->height ) * ximage->effectiveWidth
    * ( ximage->depth / 32 );
  while( p < p2 )
    *p++ = empty;

  int	x, z;

  // Boucles de deplacement dans le volume

  for( z=0; z<dz; ++z )
  {
    p2 = ptrpix;
    for( x=0; x<dx; ++x )
      *ptrpix++ = coltraits.color( *( ptrori + dline * yy + dxx * z + x ) );
    ptrpix += offset_xim;
    p = ptrpix + off_line;
    if( p < (AimsRGBA *) ximage->data )
      p = (AimsRGBA *) ximage->data;
    while( p < p2 )
      *p++ = empty;
  }
  // clear garbage at end of image
  p = (AimsRGBA *) ximage->data;
  while( p < p2 )
    *p++ = empty;
}

template <class T>
void AVolume<T>::updateSagittal( AImage *ximage, const Point3df & pf0, 
				 float time ) const
{
  // cout << "UpdateSagittal simple, pf0 : " << pf0 << "\n";
  long	dyy, dy, dz;		// Dimensions du volume
  const T *fp = &_volume->at( 0 );
  long	decY = &_volume->at( 0, 1 ) - fp;

  Point3df vs = VoxelSize();
  Point3df	p0 = Point3df( rint( pf0[0] / vs[0] ),
			       rint( pf0[1] / vs[1] ),
			       rint( pf0[2] / vs[2] ) );

  int	xx = (int) p0[0];

  ColorTraits<T>	coltraits( getOrCreatePalette(), 
				   d->traits.minTypedTexValue(), 
				   d->traits.maxTypedTexValue() );
  AimsRGBA empty = coltraits.color( coltraits.neutralColor() );

  if( xx >= _volume->getSizeX() || xx < 0 )
  {
    fillBlack( ximage, empty );
    return;
  }

  dy = max( min( ximage->width, _volume->getSizeY() - int( p0[1] ) ), 0 )
    - min( max( 0, - int( p0[1] ) ), ximage->width );
  dz = max( min( ximage->height, _volume->getSizeZ() - int( p0[2] ) ), 0 )
    - min( max( 0, - int( p0[2] ) ), ximage->height );
//   dyy = (int) _volume->getSizeY() * decY + &_volume->at( 0, 0, 1 ) - fp;
  dyy = &_volume->at( 0, 0, 1 ) - fp;

  /*
  cout << "taille obj : " << dy << " x " << dz << endl;
  cout << "taille pix : " << ximage->width << " x " << ximage->height << endl;
  */

  // Initialisation des pointeurs utilises

  const T *ptrori = fp + ( &_volume->at( 0, 0, 0, 1 ) - fp ) * (int)time;
  AimsRGBA     *ptrpix = (AimsRGBA *)ximage->data;
  int	   offset_xim = (ximage->effectiveWidth - dy) * ( ximage->depth / 32 );
  if( p0[1] < 0 )
    ptrpix -= ( (int) p0[1] ) * ( ximage->depth / 32 );
  else if( p0[1] > 0 )
    ptrori += (int) p0[1] * ( &_volume->at( 0, 1 ) - fp );
  if( p0[2] < 0 )
    ptrpix -= ( (int) p0[2] ) * ximage->effectiveWidth
      * ( ximage->depth / 32 );
  else if( p0[2] > 0 )
    ptrori += ( (int) p0[2] ) * ( &_volume->at( 0, 0, 1 ) - fp );

  //	remove garbage at beginning of image
  AimsRGBA	*p = (AimsRGBA *) ximage->data;
  AimsRGBA	*pend = ( (AimsRGBA *) ximage->data ) 
    + ximage->effectiveWidth * ximage->height * ( ximage->depth / 32 );
  while( p < ptrpix )
    *p++ = empty;

  int	y, z;

  // Boucles de deplacement dans le volume

  for( z=0; z<dz; ++z )
    {
      for( y=0; y<dy; ++y )
	*ptrpix++ = coltraits.color( *( ptrori + xx + dyy * z + decY * y ) );
      p = ptrpix + offset_xim;
      if( p > pend )
	p = pend;
      while( ptrpix < p )
	*ptrpix++ = empty;
    }

  // clear garbage at end of image
  AimsRGBA	*end_image = ( (AimsRGBA *) ximage->data ) 
    + ximage->effectiveWidth * ximage->height * ( ximage->depth / 32 );
  while( ptrpix < end_image )
    *ptrpix++ = empty;
}

template<class T> void AVolume<T>::adjustPalette()
{
  d->traits.adjustPalette();
}


template<class T> void AVolume<T>::SetExtrema()
{
  d->traits.setExtrema();

  GenericObject	*attrib = attributed();

  vector<float>	vec;
  vec.push_back( 0 );
  vec.push_back( 0 );
  vec.push_back( 0 );
  vec.push_back( 0 );
  attrib->setProperty( "boundingbox_min", vec );
  vec[0] = _volume->getSizeX();
  vec[1] = _volume->getSizeY();
  vec[2] = _volume->getSizeZ();
  vec[3] = _volume->getSizeT();
  attrib->setProperty( "boundingbox_max", vec );
}

template <class T> Point3df AVolume<T>::VoxelSize() const
{
  vector<float> vs( 3, 1. );
  _volume->header().getProperty( "voxel_size", vs );
  while( vs.size() < 3 )
    vs.push_back( 1. );
  return Point3df( vs[0], vs[1], vs[2] );
}


template <class T> void AVolume<T>::setVoxelSize( const Point3df & vs )
{
  vector<float> vvs( 4, 1. );
  vvs[0] = vs[0];
  vvs[1] = vs[1];
  vvs[2] = vs[2];
  _volume->header().setProperty( "voxel_size", vvs );
}


template<class T> 
float AVolume<T>::mixedTexValue( const Point3df & pos, float time ) const
{
  int x = (int) rint( pos[0] / VoxelSize()[0] );
  int y = (int) rint( pos[1] / VoxelSize()[1] );
  int z = (int) rint( pos[2] / VoxelSize()[2] );

  if( x >= MinX2D() && x <= MaxX2D() && y >= MinY2D() && 
      y <= MaxY2D() && z >= MinZ2D() && z <= MaxZ2D() )
  {
    if( time < MinT() )
      time = MinT();
    else if( time > MaxT() )
      time = MaxT();
    return d->traits.mixedTexValue( (unsigned) x, (unsigned) y,
                                    (unsigned) z, (unsigned) time );
  }
  else
    return 0;
}


template<class T> 
vector<float> AVolume<T>::texValues( const Point3df & pos, float time ) const
{
  int x = (int) rint( pos[0] / VoxelSize()[0] );
  int y = (int) rint( pos[1] / VoxelSize()[1] );
  int z = (int) rint( pos[2] / VoxelSize()[2] );

  if( x >= MinX2D() && x <= MaxX2D() && y >= MinY2D() &&
      y <= MaxY2D() && z >= MinZ2D() && z <= MaxZ2D() )
  {
    if( time < MinT() )
      time = MinT();
    else if( time > MaxT() )
      time = MaxT();
    return d->traits.texValue( (unsigned) x, (unsigned) y,
                               (unsigned) z, (unsigned) time );
  }
  else
    return vector<float>();
}



namespace anatomist
{
template<class T> std::string	AVolume<T>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<?>";
}

template<> std::string	AVolume<int8_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<int8_t>";
}

template<> std::string	AVolume<uint8_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<uint8_t>";
}

template<> std::string	AVolume<int16_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<int16_t>";
}

template<> std::string	AVolume<uint16_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<uint16_t>";
}

template<> std::string	AVolume<int32_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<int32_t>";
}

template<> std::string	AVolume<uint32_t>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<uint32_t>";
}

template<> std::string	AVolume<float>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<float>";
}

template<> std::string	AVolume<double>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<double>";
}

template<> std::string	AVolume<AimsRGB>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<AimsRGB>";
}

template<> std::string	AVolume<AimsRGBA>::objectFullTypeName(void) const
{
  return objectTypeName(_type) + "<AimsRGBA>";
}

}

template<class T> 
bool AVolume<T>::boundingBox( Point3df & bmin, Point3df & bmax ) const
{
  Point3df vs = VoxelSize();
  bmin = Point3df( -0.5 * vs[0], -0.5 * vs[1],
                   -0.5 * vs[2] );
  bmax = Point3df( (-0.5 + (float)_volume->getSizeX()) * vs[0],
		   (-0.5 + (float)_volume->getSizeY()) * vs[1],
		   (-0.5 + (float)_volume->getSizeZ()) * vs[2] );
  return true;
}


template<class T> 
GenericObject *AVolume<T>::attributed()
{
  if( _volume.get() )
  {
    if( !d->attrib )
      d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
  }
  else if( !d->attrib )
    d->attrib = new ValueObject<Dictionary>;
  return d->attrib;
}


template<class T> 
const GenericObject *AVolume<T>::attributed() const
{
  if( _volume.get() )
  {
    if( !d->attrib )
      d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
  }
  else if( !d->attrib )
    d->attrib = new ValueObject<Dictionary>;
  return d->attrib;
}


template<class T> 
bool AVolume<T>::reload( const string & filename )
{
  Finder f;
  if( !f.check( filename ) )
    return false;

  string dtype;
  const PythonHeader *h = dynamic_cast<const PythonHeader *>( f.header() );
  if( h )
    h->getProperty( "data_type", dtype );

  Reader<Volume<T> >	reader( filename );
  rc_ptr<Volume<T> > obj( reader.read() );
  if( !obj )
    return false;

  if( !dtype.empty() && dtype != DataTypeCode<T>::dataType() )
    QMessageBox::warning( 0,
      QObject::tr( "Voxel type was changed", "ControlWindow" ),
      QObject::tr( "The voxel type in file was changed since it was loaded "
        "the first time. It was ", "ControlWindow" )
      + DataTypeCode<T>::dataType().c_str()
      + QObject::tr( " and is now ", "ControlWindow" ) + dtype.c_str()
      + QObject::tr( ". The voxel type cannot change dynamically, so values "
        "may be truncated. You had probably better load the file as a new "
        "object.", "ControlWindow" ),
      QMessageBox::Ok, QMessageBox::Ok );

  _volume = obj;
  delete d->attrib;
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
  glSetTexImageChanged( true, 0 );
  glSetChanged( glBODY, true );
  SetExtrema();
  adjustPalette();
  return true;
}


template<class T>
bool AVolume<T>::save( const string & filename )
{
  storeHeaderOptions();
  Writer<Volume<T> >	w( filename );
  return w.write( *_volume );
}


template<class T>
bool AVolume<T>::printTalairachCoord( const Point3df & pos, 
				      const Referential* ref ) const
{
  vector<float>	origin;
  Point3df	rpos 
    = Transformation::transform( pos, ref, getReferential(), 
				 Point3df( 1, 1, 1 ), VoxelSize() );

  const PropertySet & ph = _volume->header();
  try
  {
    ph.getProperty( "origin", origin );
    if( origin.size() >= 3 )
    {
      bool	norm = false;
      try
      {
        norm = (bool) ph.getProperty( "spm_normalized" )->getScalar();
      }
      catch( ... )
      {
      }
      origin[0] = - ( rpos[0] - origin[0] ) * _volume->getSizeX();
      origin[1] = - ( rpos[1] - origin[1] ) * _volume->getSizeY();
      origin[2] = - ( rpos[2] - origin[2] ) * _volume->getSizeZ();
      if( norm )
        cout << "SPM Talairach (mm): ";
      else
        cout << "SPM (mm): ";
      cout << origin[0] << ", " << origin[1]
        << ", " << origin[2] << endl;
      return true;
    }
  }
  catch( exception & )
  {
  }

  return false;
}


template<typename T>
bool AVolume<T>::isTransparent() const
{
  return AObject::isTransparent();
}


template<typename T>
void AVolume<T>::setInternalsChanged()
{
  glSetTexImageChanged( true, 0 );
  glSetTexEnvChanged( true, 0 );
}


template <typename T>
void AVolume<T>::setVolume( carto::rc_ptr<AimsData<T> > vol )
{
  _volume = vol->volume();
  delete d->attrib;
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


template <typename T>
void AVolume<T>::setVolume( carto::rc_ptr<Volume<T> > vol )
{
  _volume = vol;
  delete d->attrib;
  d->attrib = new ReferenceObject<PropertySet>( _volume->header() );
}


// --

namespace anatomist
{

template<> bool AVolume<AimsRGBA>::isTransparent() const
{
  return true;
}

}

using namespace anatomist::internal;

// traits definitions

template<typename T> inline VolumeTraits<T>::VolumeTraits( AVolume<T> *vol )
  : vttype( vol )
{
}


template<typename T> inline 
VolumeScalarTraits<T>::VolumeScalarTraits( AVolume<T> *vol )
  : volume( vol )
{
}


template<typename T> inline 
VolumeVectorTraits<T>::VolumeVectorTraits( AVolume<T> *vol )
  : volume( vol )
{
}


template<typename T> 
void VolumeScalarTraits<T>::adjustPalette()
{
//   cout << "adjustPalette, mini: " << mini << ", maxi: " << maxi << endl;
  if( mini == maxi )
  {
    volume->getOrCreatePalette();
    AObjectPalette        *pal = volume->palette();
    pal->setMin1( 0 );
    pal->setMax1( 1. );

    return;
  }

  //	generate histogram
  unsigned long	histo[ 256 ];
  unsigned long	i, n = 0;
//   typename Volume<T>::const_iterator	iv, fv=volume->volume()->end();
  const float 
    limit = 0.99 * volume->volume()->getSizeX() * volume->volume()->getSizeY()
    * volume->volume()->getSizeZ() * volume->volume()->getSizeT();
  set<T>		vals;
  unsigned long 	nval = 0, maxval = 50, value;

  for( i=0; i<256; ++i )
    histo[ i ] = 0;

  double factor = 255. / (double) ( maxi - mini );

  /*
//   unsigned long nn = 0;
  for( iv=volume->volume()->begin(); iv!=fv; ++iv )
  {
//     if( nn % 10000000 == 0 )
//       cout << "n: " << nn << endl;
//     ++nn;
    const T & v = *iv;
    value = (unsigned long) ( ( v - mini ) * factor );
    if( value < 0 )
      value = 0;
    else if( value >= 256 )
      value = 255;
    ++histo[ value ];
    if( nval < maxval )
    {
      vals.insert( v );
      nval = vals.size();
    }
  }
  */

  unsigned long x, y, z, t, nx = volume->volume()->getSizeX(), ny = volume->volume()->getSizeY(), nz = volume->volume()->getSizeZ(), nt = volume->volume()->getSizeT();
  T *buf;
  for( t=0; t<nt; ++t )
    for( z=0; z<nz; ++z )
      for( y=0; y<ny; ++y )
  {
    buf = &volume->volume()->at( 0, y, z, t );
//     cout << "t: " << t << ", z: " << z << ", y: " << y << endl;
    for( x=0; x<nx; ++x, ++buf )
    {
    value = (unsigned long) ( ( (*buf) - mini ) * factor );
    if( value < 0 )
      value = 0;
    else if( value >= 256 )
      value = 255;
    ++histo[ value ];
    if( nval < maxval )
    {
      vals.insert( *buf );
      nval = vals.size();
    }
    }
  }

//   cout << "histo done\n";

  //	cummulated histogram
  for( i=0; i<256 && n<limit; ++i )
    n += histo[ i ];
  if( i == 1 )	// patch Denis 100300: si tout est sur la 1ere bande,
    i = 255;	// on desactive l'histo (vol. fonctionnel)
  else if( i == 256 )
    i = 255;
  else if( nval < maxval )
    i = 255;

   /* cout << "limit : " << limit << endl;
     cout << "coupure histo : " << ((float)i)/255 << endl; */
  /* cout << "min   : " << _mini << endl;
    cout << "max   : " << _maxi << endl;
    cout << "nc    : " << _objPal->NumberOfColors() << endl;
    cout << "scale : " << _scale << endl; */
  volume->getOrCreatePalette();
  AObjectPalette	*pal = volume->palette();
  pal->setMin1( 0 );
  pal->setMax1( ((float) i) / 255 );
}


template<typename T> inline 
void VolumeVectorTraits<T>::adjustPalette()
{
}


template<typename T> 
void VolumeScalarTraits<T>::setExtrema()
{
  //	clear NaN values found in some SPM volumes
  int	x, y, z, t;
  T	val;
  Volume<T>	& vol = *volume->volume();

  mini = vol( 0, 0, 0, 0 );
  maxi = vol( 0, 0, 0, 0 );

  if( numeric_limits<T>::has_infinity )
    for( long t=0, nt=vol.getSizeT(); t!=nt; ++t )
      for( long z=0, nz=vol.getSizeZ(); z!=nz; ++z )
        for( long y=0, ny=vol.getSizeY(); y!=ny; ++y )
        {
          T* pvol = &vol( 0, y, z, t );
          for( long x=0, nx=vol.getSizeX(); x!=nx; ++x, ++pvol )
          {
            val = *pvol;
            if( isnan( val ) )
            {
              *pvol = 0; // WARNING can't work on MMap/RO volumes !
              val = 0;
              if( x == 0 && y == 0 && z == 0 && t == 0 )
              {
                mini = 0;
                maxi = 0;
              }
            }
            if( val < mini )
              mini = val;
            if( val > maxi )
              maxi = val;
          }
        }
  else // no NaN, save the test time
    for( long t=0, nt=vol.getSizeT(); t!=nt; ++t )
      for( long z=0, nz=vol.getSizeZ(); z!=nz; ++z )
        for( long y=0, ny=vol.getSizeY(); y!=ny; ++y )
        {
          T* pvol = &vol( 0, y, z, t );
          for( long x=0, nx=vol.getSizeX(); x!=nx; ++x, ++pvol )
          {
            val = *pvol;
            if( val < mini )
              mini = val;
            if( val > maxi )
              maxi = val;
          }
        }

  GenericObject	*attrib = volume->attributed();

  attrib->setProperty( "texture_min", (float) mini );
  attrib->setProperty( "texture_max", (float) maxi );
  GLComponent::TexExtrema  & te = volume->glTexExtrema( 0 );
  if( te.min.empty() )
    te.min.push_back( mini );
  else
    te.min[0] = mini;
  if( te.max.empty() )
    te.max.push_back( maxi );
  else
    te.max[0] = maxi;
  if( te.minquant.empty() )
    te.minquant.push_back( mini );
  else
    te.minquant[0] = mini;
  if( te.maxquant.empty() )
    te.minquant.push_back( maxi );
  else
    te.maxquant[0] = maxi;
}


template<typename T> inline
void VolumeVectorTraits<T>::setExtrema()
{
  minT = 0;
  maxT = 255;
}


template<typename T> inline
T VolumeScalarTraits<T>::minTypedTexValue() const
{
  return mini;
}


template<typename T> inline
T VolumeScalarTraits<T>::maxTypedTexValue() const
{
  return maxi;
}


template<typename T> inline
T VolumeVectorTraits<T>::minTypedTexValue() const
{
  return minT;
}


template<typename T> inline
T VolumeVectorTraits<T>::maxTypedTexValue() const
{
  return maxT;
}


template<typename T> inline
float VolumeScalarTraits<T>::mixedTexValue( unsigned x, unsigned y, 
					    unsigned z, unsigned t ) const
{
  return (float) (*volume->volume())( x, y, z, t );
}

template<typename T> inline
vector<float> VolumeScalarTraits<T>::texValue( unsigned x, unsigned y,
                                               unsigned z, unsigned t ) const
{
  vector<float> v(1);
  v[0] = (float) (*volume->volume())( x, y, z, t );
  return v;
}

namespace anatomist
{
namespace internal
{

template<> inline
float 
VolumeVectorTraits<AimsRGB>::mixedTexValue( unsigned x, unsigned y, 
					    unsigned z, unsigned t ) const
{
  const AimsRGB	& rgb = (*volume->volume())( x, y, z, t );
  return ( ((float) rgb.red() ) + rgb.green() + rgb.blue() ) / 3;
}


template<> inline
vector<float>
VolumeVectorTraits<AimsRGB>::texValue( unsigned x, unsigned y,
                                       unsigned z, unsigned t ) const
{
  const AimsRGB	& rgb = (*volume->volume())( x, y, z, t );
  vector<float> v(3);
  v[0] = (float) rgb.red();
  v[1] = (float) rgb.green();
  v[2] = (float) rgb.blue();
  return v;
}


template<> inline
float 
VolumeVectorTraits<AimsRGBA>::mixedTexValue( unsigned x, unsigned y, 
					     unsigned z, unsigned t ) const
{
  const AimsRGBA	& rgb = (*volume->volume())( x, y, z, t );
  return ( ((float) rgb.red() ) + rgb.green() + rgb.blue() + 
	   rgb.alpha() ) / 4;
}


template<> inline
vector<float>
VolumeVectorTraits<AimsRGBA>::texValue( unsigned x, unsigned y,
                                        unsigned z, unsigned t ) const
{
  const AimsRGBA	& rgb = (*volume->volume())( x, y, z, t );
  vector<float> v(4);
  v[0] = (float) rgb.red();
  v[1] = (float) rgb.green();
  v[2] = (float) rgb.blue();
  v[3] = (float) rgb.alpha();
  return v;
}

}
}

template<typename T> inline
float VolumeVectorTraits<T>::mixedTexValue( unsigned x, unsigned y, 
					    unsigned z, unsigned t ) const
{
  const T			& val = (*volume)( x, y, z, t );
  float				mval = 0;
  typename T::const_iterator	it, et = val.end();
  unsigned			n = 0;

  for( it=val.begin(); it!=et; ++it )
    mval += (float) *it;

  return mval / n;
}


// instanciations

template class AVolume<int8_t>;
template class AVolume<uint8_t>;
template class AVolume<int16_t>;
template class AVolume<uint16_t>;
template class AVolume<int32_t>;
template class AVolume<uint32_t>;
template class AVolume<int64_t>;
template class AVolume<uint64_t>;
template class AVolume<float>;
template class AVolume<double>;
template class AVolume<AimsRGB>;
template class AVolume<AimsRGBA>;
