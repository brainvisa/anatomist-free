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
#include <aims/data/data.h>
#include <aims/rgb/rgb.h>
#include <cartobase/object/object.h>

class QImage;

namespace anatomist
{

  class AObjectPalette
  {
  public:
    enum Palette1DMapping
    {
      FIRSTLINE,
      DIAGONAL
    };

    typedef AimsRGBA (*MixMethod)( const AimsData<AimsRGBA> & map1, 
				   const AimsData<AimsRGBA> *map2, 
				   unsigned x, unsigned y, 
				   const AObjectPalette & pal );

    AObjectPalette( carto::rc_ptr<APalette> pal );
    AObjectPalette( const AObjectPalette & x );
    virtual ~AObjectPalette();

    virtual AObjectPalette* clone() const;

    virtual AObjectPalette & operator = ( const AObjectPalette & x );

    carto::rc_ptr<APalette> refPalette() const { return( _refPal ); }
    carto::rc_ptr<APalette> refPalette2() const { return( _refPal2 ); }
    void setRefPalette( carto::rc_ptr<APalette> pal )
    { if( _refPal != pal ) { clearColors(); _refPal = pal; } }
    void setRefPalette2( carto::rc_ptr<APalette> pal ) { _refPal2 = pal; }
    const AimsData<AimsRGBA>* colors() const
    { if( !_colors ) const_cast<AObjectPalette *>(this)->fill();
    return _colors; }
    AimsData<AimsRGBA>* colors() { if( !_colors ) fill(); return _colors; }
    void create( unsigned dimx, unsigned dimy = 1, unsigned dimz = 1, 
		 unsigned dimt = 1 );
    virtual void fill();
    float min1() const { return( _min ); }
    float max1() const { return( _max ); }
    float min2() const { return( _min2 ); }
    float max2() const { return( _max2 ); }
    int palette1DMapping() const { return (_palette1DMapping) ; }
    std::string palette1DMappingName() const 
    { return (_palette1DMapping == 1 ? "Diagonal" : "FirstLine" ) ; }
    void setMin1( float x ) { _min = x; }
    void setMax1( float x ) { _max = x; }
    void setMin2( float x ) { _min2 = x; }
    void setMax2( float x ) { _max2 = x; }
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
    void setZeroCenteredAxis1( bool x ) { _zeroCentered1 = x; }
    void setZeroCenteredAxis2( bool x ) { _zeroCentered2 = x; }

    static AimsRGBA palette2DMixMethod( const AimsData<AimsRGBA> & map1, 
					const AimsData<AimsRGBA> *map2, 
					unsigned x, unsigned y, 
					const AObjectPalette & pal );
    static AimsRGBA linearMixMethod( const AimsData<AimsRGBA> & map1, 
				     const AimsData<AimsRGBA> *map2, 
				     unsigned x, unsigned y, 
				     const AObjectPalette & pal );
    static AimsRGBA geometricMixMethod( const AimsData<AimsRGBA> & map1, 
					const AimsData<AimsRGBA> *map2, 
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
    QImage* toQImage( int w = 0, int h = 0 ) const;

  protected:
    static std::map<std::string, MixMethod> defaultMixMethods();

    carto::rc_ptr<APalette> _refPal;
    AimsData<AimsRGBA>	*_colors;
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
