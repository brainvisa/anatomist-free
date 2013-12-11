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


#ifndef ANA_COLOR_COLORTRAITS_H
#define ANA_COLOR_COLORTRAITS_H

#include <anatomist/color/objectPalette.h>
#include <aims/def/general.h>
#include <aims/vector/vector.h>
#include <aims/rgb/rgb.h>

namespace anatomist
{

  template<typename T> class ColorScalarPaletteTraits
  {
  public:
    ColorScalarPaletteTraits( const AObjectPalette* pal, const T & mini,
                              const T & maxi );
    AimsRGBA color( const T & ) const;
    void setup( const T & mini, const T & maxi );
    T neutralColor() const;

  private:
    const AObjectPalette	*palette;
    const AimsData<AimsRGBA>	*colors;
    float			scale0;
    float			scale1;
    float			decal0;
    float			decal1;
    int				cmin0;
    int				cmin1;
    int				cmax0;
    int				cmax1;
    float			minv0;
    float			maxv0;
    float			minv1;
    float			maxv1;
  };


  template<typename T> class ColorNoPaletteTraits
  {
  public:
    ColorNoPaletteTraits( const AObjectPalette*, const T & mini,
                          const T & maxi );
    AimsRGBA color( const T & ) const;
    T neutralColor() const;
  };

  namespace internal
  {

    /// switch to the right color traits type
    template<typename T> struct ColorTraitsType
    {
      typedef ColorScalarPaletteTraits<T> traitstype;
    };

  }

  /// Converter value -> RGBA (high-level)
  template<typename T> class ColorTraits
  {
  public:
    ColorTraits( const AObjectPalette*, const T & mini, const T & maxi );
    AimsRGBA color( const T & ) const;
    /// returns a black / transparent / zero color
    T neutralColor() const;

  private:
    typename internal::ColorTraitsType<T>::traitstype	_traitstype;
  };


  //	implementation specializations

  namespace internal
  {

/*    template<> struct ColorTraitsType<AimsRGB>
    {
      typedef ColorNoPaletteTraits<AimsRGB> traitstype;
    };

    template<> struct ColorTraitsType<AimsRGBA>
    {
      typedef ColorNoPaletteTraits<AimsRGBA> traitstype;
    };
*/

  }

  template<typename T> inline
  ColorTraits<T>::ColorTraits( const AObjectPalette* pal, const T & mini,
                               const T & maxi )
    : _traitstype( pal, mini, maxi )
  {
  }

  template <typename T> inline
  AimsRGBA ColorTraits<T>::color( const T & in ) const
  {
    return _traitstype.color( in );
  }

  template <typename T> inline
  T ColorTraits<T>::neutralColor() const
  {
    return _traitstype.neutralColor();
  }

  // ---

  template<typename T> inline
  ColorNoPaletteTraits<T>::ColorNoPaletteTraits( const AObjectPalette*,
                                                 const T &, const T & )
  {
  }

  template <typename T> inline
  AimsRGBA ColorNoPaletteTraits<T>::color( const T & in ) const
  {
    // this assumes AimsRGBA::AimsRGBA(const T&) is defined on T
    return in;
  }

  template <typename T> inline
  T ColorNoPaletteTraits<T>::neutralColor() const
  {
    return 0;
  }

  template <> inline
  AimsRGBA ColorNoPaletteTraits<AimsRGBA>::neutralColor() const
  {
    // transparent color
    return AimsRGBA( 0, 0, 0, 0 );
  }

  // ---

  template <typename T> inline
  ColorScalarPaletteTraits<T>::ColorScalarPaletteTraits( const AObjectPalette
                                                         * pal,
                                                         const T & mini,
                                                         const T & maxi )
    : palette( pal )
  {
    setup( mini, maxi );
  }

  template <typename T> inline
  AimsRGBA ColorScalarPaletteTraits<T>::color( const T & in ) const
  {
    int ival0, ival1;
    float val = static_cast<float>( in );

    // Comparisons are written this way to accommodate NaN and Inf
    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    return (*colors)( ival0, ival1 );
  }

  template <typename T> inline
  T ColorScalarPaletteTraits<T>::neutralColor() const
  {
    return 0;
  }

  template <> inline
  AimsRGBA ColorScalarPaletteTraits<AimsRGB>::color( const AimsRGB & in ) const
  {
    AimsRGBA col;

    int ival0, ival1;
    float val = static_cast<float>( in.red() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[0] = (*colors)( ival0, ival1 )[0];

    val = static_cast<float>( in.green() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[1] = (*colors)( ival0, ival1 )[1];

    val = static_cast<float>( in.blue() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[2] = (*colors)( ival0, ival1 )[2];

    val = static_cast<float>( std::sqrt( in.red() * in.red()
                                         + in.green() * in.green()
                                         + in.blue() * in.blue() ) );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[3] = (*colors)( ival0, ival1 )[3];

    return col;
  }


  template <> inline
  AimsRGBA ColorScalarPaletteTraits<AimsRGBA>::color( const AimsRGBA & in )
      const
  {
    AimsRGBA col;

    int ival0, ival1;
    float val = static_cast<float>( in.red() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[0] = (*colors)( ival0, ival1 )[0];

    val = static_cast<float>( in.green() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[1] = (*colors)( ival0, ival1 )[1];

    val = static_cast<float>( in.blue() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[2] = (*colors)( ival0, ival1 )[2];

    val = static_cast<float>( in.alpha() );

    if( val >= minv0 && val < maxv0 )
      ival0 = static_cast<int>( scale0 * val + decal0 );
    else if( val <= minv0 )
      ival0 = cmin0;
    else if( val >= maxv0 )
      ival0 = cmax0;
    else
      return neutralColor();

    if( palette->palette1DMapping() == AObjectPalette::FIRSTLINE ||
        colors->dimY() == 1 )
      ival1 = 0 ;
    else
      {
        if( val >= minv1 && val < maxv1 )
          ival1 = static_cast<int>( scale1 * val + decal1 );
        else if( val <= minv1 )
          ival1 = cmin1;
        else if( val >= maxv1 )
          ival1 = cmax1;
        else
          return neutralColor();
      }

    col[3] = (*colors)( ival0, ival1 )[3];

    return col;
  }

}

#endif
