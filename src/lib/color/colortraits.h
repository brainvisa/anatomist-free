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
                              const T & maxi, const T & mini2,
                              const T & maxi2, bool insideBounds = false );
    ColorScalarPaletteTraits( const AObjectPalette* pal, const T & mini,
                              const T & maxi, bool insideBounds = false );
    AimsRGBA color( const T & ) const;
    void setup( const T & mini, const T & maxi,
                const T & mini2, const T & maxi2, bool insideBounds = false );
    void setup1D( int dim, const T & mini, const T & maxi,
                  bool insideBounds = false );
    T neutralColor() const;
    void paletteCoords( double val0, double val1, int & px, int & py ) const;
    void paletteCoord( int dim, double val0, int & px ) const;
    void paletteCoord0( double val0, int & px ) const;
    void paletteCoord1( double val0, int & px ) const;

  private:
    const AObjectPalette	*palette;
    const carto::Volume<AimsRGBA>	*colors;
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

    float			scalen0;
    float			scalen1;
    float			decaln0;
    float			decaln1;
    int				cminn0;
    int				cminn1;
    int				cmaxn0;
    int				cmaxn1;

  };


  template<typename T> class ColorNoPaletteTraits
  {
  public:
    ColorNoPaletteTraits( const AObjectPalette*, const T & mini,
                          const T & maxi, bool insideBounds = false );
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
    /** mini / maxi are min and max indexes used to address the colormap
    */
    ColorTraits( const AObjectPalette*, const T & mini, const T & maxi,
                 const T & mini2, const T & maxi2,
                 bool insideBounds = false );
    ColorTraits( const AObjectPalette*, const T & mini, const T & maxi,
                 bool insideBounds = false );
    AimsRGBA color( const T & ) const;
    /// returns a black / transparent / zero color
    T neutralColor() const;
    void paletteCoords( double val0, double val1, int & px, int & py ) const;
    void paletteCoord( int dim, double val0, int & px ) const;
    void paletteCoord0( double val0, int & px ) const;
    void paletteCoord1( double val0, int & px ) const;


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
                               const T & maxi, const T & mini2,
                               const T & maxi2, bool insideBounds )
    : _traitstype( pal, mini, maxi, mini2, maxi2, insideBounds )
  {
  }


  template<typename T> inline
  ColorTraits<T>::ColorTraits( const AObjectPalette* pal, const T & mini,
                               const T & maxi, bool insideBounds )
    : _traitstype( pal, mini, maxi, insideBounds )
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

  template <typename T> inline
  void ColorTraits<T>::paletteCoords( double val0, double val1,
                                      int & px, int & py ) const
  {
    _traitstype.paletteCoords( val0, val1, px, py );

  }

  template <typename T> inline
  void ColorTraits<T>::paletteCoord( int dim, double val0, int & px ) const
  {
    _traitstype.paletteCoord( dim, val0, px );
  }


  template <typename T> inline
  void ColorTraits<T>::paletteCoord0( double val0, int & px ) const
  {
    _traitstype.paletteCoord0( val0, px );
  }


  template <typename T> inline
  void ColorTraits<T>::paletteCoord1( double val0, int & px ) const
  {
    _traitstype.paletteCoord1( val0, px );
  }

  // ---

  template<typename T> inline
  ColorNoPaletteTraits<T>::ColorNoPaletteTraits( const AObjectPalette*,
                                                 const T &, const T &, bool )
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
                                                         const T & maxi,
                                                         const T & mini2,
                                                         const T & maxi2,
                                                         bool insideBounds )
    : palette( pal )
  {
    setup( mini, maxi, mini2, maxi2, insideBounds );
  }


  template <typename T> inline
  ColorScalarPaletteTraits<T>::ColorScalarPaletteTraits( const AObjectPalette
                                                         * pal,
                                                         const T & mini,
                                                         const T & maxi,
                                                         bool insideBounds )
    : palette( pal )
  {
    setup( mini, maxi, mini, maxi, insideBounds );
  }


  template <typename T> inline
  void ColorScalarPaletteTraits<T>::paletteCoords(
    double val0, double val1, int & ival0, int & ival1 ) const
  {
    paletteCoord0( val0, ival0 );
    paletteCoord1( val0, ival0 );
  }


  template <typename T> inline
  void ColorScalarPaletteTraits<T>::paletteCoord(
    int dim, double val0, int & ival0 ) const
  {
    if( dim != 0 )
      paletteCoord1( val0, ival0 );
    else
      paletteCoord0( val0, ival0 );
  }


  template <typename T> inline
  void ColorScalarPaletteTraits<T>::paletteCoord0(
    double val0, int & ival0 ) const
  {
    double fval0;

    if( !palette->zeroCenteredAxis1() )
    {
      // Comparisons are written this way to accommodate NaN and Inf
      fval0 = scale0 * val0 + decal0;

      if( fval0 > cmin0 && fval0 < cmax0 )
      {
        ival0 = static_cast<int>( fval0 );
      }
      else if( fval0 <= cmin0 )
        ival0 = cmin0;
      else if( fval0 >= cmax0 )
        ival0 = cmax0;
      else
        ival0 = cmin0;
    }
    else
    {
      if( val0 >= 0 )
      {
        fval0 = scale0 * val0 + decal0;
        if( fval0 > cmin0 && fval0 < cmax0 )
          ival0 = static_cast<int>( fval0 );
        else if( fval0 <= cmin0 )
          ival0 = cmin0;
        else if( fval0 >= cmax0 )
          ival0 = cmax0;
        else
          ival0 = cmin0;
      }
      else
      {
        fval0 = scalen0 * val0 + decaln0;
        if( fval0 > cminn0 && fval0 < cmaxn0 )
          ival0 = static_cast<int>( fval0 );
        else if( fval0 <= cminn0 )
          ival0 = cminn0;
        else if( fval0 >= cmaxn0 )
          ival0 = cmaxn0;
        else
          ival0 = cminn0;
      }
    }
  }


  template <typename T> inline
  void ColorScalarPaletteTraits<T>::paletteCoord1(
    double val0, int & ival0 ) const
  {
    if( ( !palette->is2dMode()
          && palette->palette1DMapping() == AObjectPalette::FIRSTLINE )
        || colors->getSizeY() == 1 )
      ival0 = 0 ;
    else
    {
      double fval0;

      if( !palette->zeroCenteredAxis2() )
      {
        // Comparisons are written this way to accommodate NaN and Inf
        fval0 = scale1 * val0 + decal1;

        if( fval0 > cmin1 && fval0 < cmax1 )
        {
          ival0 = static_cast<int>( fval0 );
        }
        else if( fval0 <= cmin1 )
          ival0 = cmin1;
        else if( fval0 >= cmax1 )
          ival0 = cmax1;
        else
          ival0 = cmin1;
      }
      else
      {
        if( val0 >= 0 )
        {
          fval0 = scale1 * val0 + decal1;
          if( fval0 > cmin1 && fval0 < cmax1 )
            ival0 = static_cast<int>( fval0 );
          else if( fval0 <= cmin1 )
            ival0 = cmin1;
          else if( fval0 >= cmax1 )
            ival0 = cmax1;
          else
            ival0 = cmin1;
        }
        else
        {
          fval0 = scalen1 * val0 + decaln1;
          if( fval0 > cminn1 && fval0 < cmaxn1 )
            ival0 = static_cast<int>( fval0 );
          else if( fval0 <= cminn1 )
            ival0 = cminn1;
          else if( fval0 >= cmaxn1 )
            ival0 = cmaxn1;
          else
            ival0 = cminn1;
        }
      }
    }
  }


  template <typename T> inline
  AimsRGBA ColorScalarPaletteTraits<T>::color( const T & in ) const
  {
    int ival0;
    paletteCoord0( in, ival0 );

    return colors->at( ival0, 0 );
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

    int ival0;

    paletteCoord0( in.red(), ival0 );
    col[0] = colors->at( ival0, 0 )[0];

    paletteCoord0( in.green(), ival0 );
    col[1] = colors->at( ival0, 0 )[1];

    paletteCoord0( in.blue(), ival0 );
    col[2] = colors->at( ival0, 0 )[2];

    double val = static_cast<double>( std::sqrt( in.red() * in.red()
                                         + in.green() * in.green()
                                         + in.blue() * in.blue() ) );
    paletteCoord0( val, ival0 );
    col[3] = colors->at( ival0, 0 )[3];

    return col;
  }


  template <> inline
  AimsRGBA ColorScalarPaletteTraits<AimsRGBA>::color( const AimsRGBA & in )
      const
  {
    AimsRGBA col;

    int ival0;

    paletteCoord0( in.red(), ival0 );
    col[0] = colors->at( ival0, 0 )[0];

    paletteCoord0( in.green(), ival0 );
    col[1] = colors->at( ival0, 0 )[1];

    paletteCoord0( in.blue(), ival0 );
    col[2] = colors->at( ival0, 0 )[2];

    paletteCoord0( in.alpha(), ival0 );
    col[3] = colors->at( ival0, 0 )[3];

    return col;
  }

}

#endif
