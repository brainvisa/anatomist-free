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


#ifndef ANA_COLOR_OBJECTPALETTE_H
#define ANA_COLOR_OBJECTPALETTE_H

#include <anatomist/color/palette.h>
#include <cartodata/volume/volume.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>

class QImage;

namespace anatomist
{
  class AObject;

  class AObjectPalette
  {
  public:
    enum Palette1DMapping
    {
      FIRSTLINE,
      DIAGONAL
    };

    typedef AimsRGBA (*MixMethod)(
      const carto::Volume<AimsRGBA> & map1,
      const carto::Volume<AimsRGBA> *map2,
      unsigned x, unsigned y,
      const AObjectPalette & pal );

    AObjectPalette( carto::rc_ptr<APalette> pal );
    AObjectPalette( const AObjectPalette & x );
    virtual ~AObjectPalette();

    virtual AObjectPalette* clone() const;

    virtual AObjectPalette & operator = ( const AObjectPalette & x );

    carto::rc_ptr<APalette> refPalette( int dim = 0 ) const
    { if( dim == 1 ) return _refPal2; else return( _refPal ); }
    carto::rc_ptr<APalette> refPalette2() const { return( _refPal2 ); }
    void setRefPalette( int dim, carto::rc_ptr<APalette> pal )
    { if( dim == 1 ) setRefPalette2( pal );
      else setRefPalette1( pal ); }
    void setRefPalette( carto::rc_ptr<APalette> pal )
    { setRefPalette1( pal ); } // FIXME to be removed
    void setRefPalette1( carto::rc_ptr<APalette> pal )
    { if( _refPal != pal ) { clearColors(); _refPal = pal; } }
    void setRefPalette2( carto::rc_ptr<APalette> pal ) { _refPal2 = pal; }
    const carto::Volume<AimsRGBA>* colors() const
    { if( !_colors.get() ) const_cast<AObjectPalette *>(this)->fill();
      return _colors.get(); }
    carto::Volume<AimsRGBA>* colors()
    { if( !_colors.get() ) fill(); return _colors.get(); }
    void create( unsigned dimx, unsigned dimy = 1, unsigned dimz = 1, 
                 unsigned dimt = 1 );
    virtual void fill();
    float min1() const { return( _min ); }
    float max1() const { return( _max ); }
    float min2() const { return( _min2 ); }
    float max2() const { return( _max2 ); }
    float min( int dim = 0 ) const { return dim == 1 ? min2() : min1(); }
    float max( int dim = 0 ) const { return dim == 1 ? max2() : max1(); }
    int palette1DMapping() const { return (_palette1DMapping) ; }
    std::string palette1DMappingName() const 
    { return (_palette1DMapping == 1 ? "Diagonal" : "FirstLine" ) ; }
    void setMin1( float x )
    { _min = x; if( isnan( x ) || isinf( x ) ) _min = 0; }
    void setMax1( float x )
    { _max = x; if( isnan( x ) || isinf( x ) ) _max = 0; }
    void setMin2( float x ) { _min2 = x; }
    void setMax2( float x ) { _max2 = x; }
    void setMin( int dim, float x )
    { if( dim == 1 ) setMin2( x ); else setMin1( x ); }
    void setMax( int dim, float x )
    { if( dim == 1 ) setMax2( x ); else setMax1( x ); }

    /// get the absolute min for a given object
    float absMin1( const AObject * obj ) const;
    /// get the absolute max for a given object
    float absMax1( const AObject * obj ) const;
    /// get the absolute min for a given object
    float absMin2( const AObject * obj ) const;
    /// get the absolute max for a given object
    float absMax2( const AObject * obj ) const;
    float absMin( int dim, const AObject* obj ) const
    { return dim == 1 ? absMin2( obj ) : absMin1( obj ); }
    float absMax( int dim, const AObject* obj ) const
    { return dim == 1 ? absMax2( obj ) : absMax1( obj ); }
    /// set the min from an absolute value for a given object
    void setAbsMin1( const AObject * obj, float x );
    /// set the max from an absolute value for a given object
    void setAbsMax1( const AObject * obj, float x );
    /// set the min from an absolute value for a given object
    void setAbsMin2( const AObject * obj, float x );
    /// set the max from an absolute value for a given object
    void setAbsMax2( const AObject * obj, float x );
    void setAbsMin( int dim, const AObject *obj, float x )
    { if( dim == 1 ) setAbsMin2( obj, x ); else setAbsMin1( obj, x ); }
    void setAbsMax( int dim, const AObject *obj, float x )
    { if( dim == 1 ) setAbsMax2( obj, x ); else setAbsMax1( obj, x ); }
    float relValue1( const AObject * obj, float absval ) const;
    float relValue2( const AObject * obj, float absval ) const;
    float relValue( int dim, const AObject* obj, float absval ) const
    { return dim == 1 ? relValue2( obj, absval ) : relValue1( obj, absval ); }
    float absValue1( const AObject * obj, float relval ) const;
    float absValue2( const AObject * obj, float relval ) const;
    float absValue( int dim, const AObject* obj, float relval ) const
    { return dim == 1 ? absValue2( obj, relval ) : absValue1( obj, relval ); }

    void setPalette1DMapping( Palette1DMapping palette1DMapping )
    { _palette1DMapping = palette1DMapping ; }
    void setPalette1DMappingName( std::string palette1DMappingName ) 
    { _palette1DMapping = ( palette1DMappingName == "FirstLine" ? 
                            FIRSTLINE : DIAGONAL ) ; }
    std::string mixMethodName() const { return( _mixMethodName ); }
    void setMixMethod( const std::string & name );
    float linearMixFactor() const { return( _linMixFactor ); }
    void setLinearMixFactor( float x ) { _linMixFactor = x; }
    bool is2dMode() const { return _mode2d; }
    void set2dMode( bool x ) { _mode2d = x; }
    bool isTransparent() const { return _transp; }
    AimsRGBA normColor( float x, float y = 0 ) const;
    AimsRGBA normColor( const Point2df & pos ) const;
    bool zeroCenteredAxis1() const { return _zeroCentered1; }
    bool zeroCenteredAxis2() const { return _zeroCentered2; }
    bool zeroCenteredAxis( int dim = 0 ) const
    { return dim == 1 ? zeroCenteredAxis2() : zeroCenteredAxis1(); }
    void setZeroCenteredAxis1( bool x ) { _zeroCentered1 = x; }
    void setZeroCenteredAxis2( bool x ) { _zeroCentered2 = x; }
    void setZeroCenteredAxis( int dim, bool x )
    { if( dim == 1 ) setZeroCenteredAxis2( x ); else setZeroCenteredAxis1( x );
    }

    static AimsRGBA palette2DMixMethod( const carto::Volume<AimsRGBA> & map1,
                                        const carto::Volume<AimsRGBA> *map2,
                                        unsigned x, unsigned y,
                                        const AObjectPalette & pal );
    static AimsRGBA linearMixMethod( const carto::Volume<AimsRGBA> & map1,
                                     const carto::Volume<AimsRGBA> *map2,
                                     unsigned x, unsigned y,
                                     const AObjectPalette & pal );
    static AimsRGBA geometricMixMethod( const carto::Volume<AimsRGBA> & map1,
                                        const carto::Volume<AimsRGBA> *map2,
                                        unsigned x, unsigned y,
                                        const AObjectPalette & pal );
    bool set( const carto::GenericObject & );
    carto::Object genericDescription() const;
    /** Maximum size of the internal palette image.
        0 means unused: no palette image is needed.
       -1 means no limit: may grow as large as the reference palette image
       -2 means unchanged
     */
    int maxSizeX() const { return _maxSizeX; }
    int maxSizeY() const { return _maxSizeY; }
    void setMaxSize( int maxsizex, int maxsizey );
    /** Maximum size of the OpenGL palette image, after scaling.
        0 means unused: no GL palette image is needed.
       -1 means no limit: may grow as large as the reference palette image
       -2 means unchanged
     */
    int glMaxSizeX() const { return _glMaxSizeX; }
    int glMaxSizeY() const { return _glMaxSizeY; }
    void glSetMaxSize( int glmaxsizex, int glmaxsizey );
    void clearColors();
    /// if pal colors size is compatible, just copy it
    void copyColors( const AObjectPalette & pal );
    /// if pal colors size is compatible, just copy it; otherwise call fill()
    void copyOrFillColors( const AObjectPalette & pal );

    static std::map<std::string, MixMethod>	mixMethods;
    /** Get the palette image in a QImage.

        The image takes into account the palette min/max settings.

        Extra optional parameters allow to scale the image. They are not
        interpreted the same way as min/max bounds (which determine where on
        the palette image the object extrema are mapped), but at the contrary,
        allow to zoom the palette view on specific object values bounds.
    */
    QImage* toQImage( int w = 0, int h = 0, float min1 = 0., float max1 = 1.,
                      float min2 = 0., float max2 = 1. ) const;
    carto::rc_ptr<carto::Volume<AimsRGBA> >
      toVolume( int w = 0, int h = 0, bool scaled = true ) const;
    /** relative bounds of the min/max values.
        It corresponds to reordered min, max, taking into account the
        zeroCenteredAxis() option, where bounds are symmetric.
    */
    std::pair<float, float> relBounds( int dim = 0 ) const;

  protected:
    static std::map<std::string, MixMethod> defaultMixMethods();

    carto::rc_ptr<APalette> _refPal;
    carto::VolumeRef<AimsRGBA>	_colors;
    float		_min;
    float		_max;
    carto::rc_ptr<APalette> _refPal2;
    float		_min2;
    float		_max2;
    MixMethod		_mixMethod;
    std::string		_mixMethodName;
    float		_linMixFactor;
    Palette1DMapping	_palette1DMapping;
    bool		_mode2d;
    bool		_transp;
    int                 _maxSizeX;
    int                 _maxSizeY;
    int                 _glMaxSizeX;
    int                 _glMaxSizeY;
    bool                _zeroCentered1;
    bool                _zeroCentered2;
  };

}


#endif
