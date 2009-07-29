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


#ifndef ANA_COLOR_OBJECTPALETTE_H
#define ANA_COLOR_OBJECTPALETTE_H

#include <anatomist/color/palette.h>
#include <aims/data/data.h>
#include <cartobase/object/object.h>

class AimsRGBA;

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
    void setRefPalette( carto::rc_ptr<APalette> pal ) { _refPal = pal; }
    void setRefPalette2( carto::rc_ptr<APalette> pal ) { _refPal2 = pal; }
    const AimsData<AimsRGBA>* colors() const { return( _colors ); }
    AimsData<AimsRGBA>* colors() { return( _colors ); }
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

    static std::map<std::string, MixMethod>	mixMethods;

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
  };

}


#endif
