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

#include <anatomist/color/colortraits.h>

#include <algorithm>

using std::swap;

namespace anatomist
{

template <typename T>
void ColorScalarPaletteTraits<T>::setup( const T & minit, const T & maxit,
                                         const T & minit1, const T & maxit1,
                                         float min1, float max1,
                                         float min2, float max2 )
{
  setup1D( 0, minit, maxit, min1, max1 );
  setup1D( 1, minit1, maxit1, min2, max2 );
}


template <typename T>
void ColorScalarPaletteTraits<T>::setup1D( int dim,
                                           const T & minit, const T & maxit,
                                           float min, float max )
{
  colors = palette->colors();

  float  scale;
  float  decal;
  int    cmin;
  int    cmax;
  float  scalen;
  float  decaln;
  int    cminn;
  int    cmaxn;

  unsigned	ncol0;
  float		mini = static_cast<float>( minit ),
    maxi = static_cast<float>( maxit );
  float rmin = palette->min( dim );
  float rmax = palette->max( dim );

  ncol0 = colors->getSize()[dim];

  if( rmin == rmax )
  {
    if( rmin == 1. )
      rmin = 0.;
    else
      rmax = 1.;
  }

  if( mini == maxi )
  {
    maxi = mini + 1.;
  }

  // in tex rel space, y = ax + b with:
  float a = ( max - min ) / ( maxi - mini );
  float b = min - a * mini;
  std::cout << "rmin/max: " << rmin << ", " << rmax << ", mini/maxi: " << mini << ", " << maxi << ", min/max: " << min << ", " << max << ", ncol: " << ncol0 << ", dim: " << dim << std::endl;
  std::cout << "a: " << a << ", b: " << b << std::endl;

  if( !palette->zeroCenteredAxis( dim ) )
  {
    // in cmap space, z = scale * x + decal with:
    float e = ncol0 / ( rmax - rmin );
    scale = e * a;
    decal = e * ( b - rmin );
    cmin = 0;
    cmax = ncol0 - 1;
    std::cout << "e: " << e << ", c: " << scale << ", d: " << decal << std::endl;
    scalen = 1.;
    decaln = 0.;
    cminn = 0.;
    cmaxn = 0.;
  }
  else
  {
    // float amax = std::max( std::abs( rmin ), std::abs( rmax ) );
    float e = ncol0 / ( ( rmax - rmin ) * 2 );
    scale = e * a;
    decal = ncol0 / 2  + e * ( b - rmin );
    cmin = ncol0 / 2;
    cmax = ncol0 - 1;
    std::cout << "Z e: " << e << ", c: " << scale << ", d: " << decal << std::endl;
    scalen = scale;
    decaln = e * ( b + rmax );
    cminn = 0;
    cmaxn = ncol0 / 2 - 1;
  }

#if 0
    // thresholds in texture (coords) values
    if( !palette->zeroCenteredAxis( dim ) )
    {
      minv = float( mini + minc0 * (double(maxi) - mini) );
      maxv = float( mini + maxc0 * (double(maxi) - mini) );
      if( minv > maxv )
        swap(minv, maxv);
    }
    else
    {
      maxi = std::max( std::abs( maxi ), std::abs( mini) );
      mini = -maxi;
      minv = float( minc0 * double(maxi) );
      maxv = float( std::abs( maxc0 ) * double(maxi) );
      if( maxc0 < 0 )
        minv *= -1;
    }
  }

  if( !palette->zeroCenteredAxis( dim ) )
  {
    scale = float( ( static_cast<double>( ncol0 ) )
      / ( (double(maxi) - mini) * (maxc0 - minc0) ) );
    decal = float( - ( mini / (double(maxi) - mini) + minc0 ) * ncol0
      / (maxc0 - minc0) );
    cmin = 0;
    cmax = ncol0 - 1;
  }
  else
  {
    scale = float( ( static_cast<double>( ncol0 ) / 2 )
      / ( double( maxv ) - double( minv ) ) ) * ( maxc0 >= 0 ? 1 : -1 );
    scalen = scale;
    if( maxc0 < 0 )
    {
      decal = - double( scale ) * double( maxv );
      decaln = ncol0 / 2 + scalen * double( minv );

      cmin = 0;
      cmax = ncol0 / 2;
      cminn = ncol0 / 2;
      cmaxn = ncol0 - 1;
    }
    else
    {
      decal = ncol0 / 2 - double( scale ) * double( minv );

      decaln = double( scalen ) * double( maxv );

      cmin = ncol0 / 2;
      cmax = ncol0 - 1;
      cminn = 0;
      cmaxn = ncol0 / 2;
    }
  }
#endif

  if( dim != 1 )
  {
    scale0 = scale;
    scalen0 = scalen;
    decal0 = decal;
    decaln0 = decaln;
    cmin0 = cmin;
    cminn0 = cminn;
    cmax0 = cmax;
    cmaxn0 = cmaxn;
    int valmi, valma, valz;
    paletteCoord0(mini, valmi);
    paletteCoord0(maxi, valma);
    paletteCoord0(0, valz);
    std::cout << "colortaits " << mini << ": " << valmi << ", " << maxi << ": " << valma << ", zero: " << valz << std::endl;
  }
  else
  {
    scale1 = scale;
    scalen1 = scalen;
    decal1 = decal;
    decaln1 = decaln;
    cmin1 = cmin;
    cminn1 = cminn;
    cmax1 = cmax;
    cmaxn1 = cmaxn;
  }
}


template <>
void ColorScalarPaletteTraits<AimsRGB>::setup( const AimsRGB &,
                                               const AimsRGB &,
                                               const AimsRGB &,
                                               const AimsRGB &,
                                               float min1, float max1,
                                               float min2, float max2 )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0 = 0.f, minc1 = 0.f, maxc0 = 1.f, maxc1 = 1.f;
  float		mini = 0.f, maxi = 255.f;

  ncol0 = colors->getSizeX();
  ncol1 = colors->getSizeY();

//   if( !insideBounds )
  {
    minc0 = palette->min1();
    maxc0 = palette->max1(); // colormap min & max
    if( minc0 == maxc0 )
    {
      if( minc0 == 1. )
        minc0 = 0.;
      else
        maxc0 = 1.;
    }
    minc1 = palette->min2();
    maxc1 = palette->max2(); // colormap min & max
    if( minc1 == maxc1 )
    {
      if( minc1 == 1. )
        minc1 = 0.;
      else
        maxc1 = 1.;
    }
  }

  scale0 = ( static_cast<float>( ncol0 ) ) / ( (maxi - mini) * (maxc0 - minc0) );
  scale1 = ( static_cast<float>( ncol1 ) ) / ( (maxi - mini) * (maxc1 - minc1) );
  decal0 = - ( mini / (maxi - mini) + minc0 ) * ncol0 / (maxc0 - minc0);
  decal1 = - ( mini / (maxi - mini) + minc1 ) * ncol1 / (maxc1 - minc1);
  cmin0 = 0;
  cmin1 = 0;
  cmax0 = colors->getSizeX() - 1;
  cmax1 = colors->getSizeY() - 1;
}


template <>
void ColorScalarPaletteTraits<AimsRGBA>::setup( const AimsRGBA &,
    const AimsRGBA &, const AimsRGBA &, const AimsRGBA &,
    float min1, float max1, float min2, float max2 )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0 = 0.f, minc1 = 0.f, maxc0 = 1.f, maxc1 = 1.f;
  float		mini = 0.f, maxi = 255.f;

  ncol0 = colors->getSizeX();
  ncol1 = colors->getSizeY();

//   if( !insideBounds )
  {
    minc0 = palette->min1();
    maxc0 = palette->max1(); // colormap min & max
    if( minc0 == maxc0 )
    {
      if( minc0 == 1. )
        minc0 = 0.;
      else
        maxc0 = 1.;
    }
    minc1 = palette->min2();
    maxc1 = palette->max2(); // colormap min & max
    if( minc1 == maxc1 )
    {
      if( minc1 == 1. )
        minc1 = 0.;
      else
        maxc1 = 1.;
    }
  }

  scale0 = ( static_cast<float>( ncol0 ) ) / ( (maxi - mini) * (maxc0 - minc0) );
  scale1 = ( static_cast<float>( ncol1 ) ) / ( (maxi - mini) * (maxc1 - minc1) );
  decal0 = - ( mini / (maxi - mini) + minc0 ) * ncol0 / (maxc0 - minc0);
  decal1 = - ( mini / (maxi - mini) + minc1 ) * ncol1 / (maxc1 - minc1);
  cmin0 = 0;
  cmin1 = 0;
  cmax0 = colors->getSizeX() - 1;
  cmax1 = colors->getSizeY() - 1;
}


template class ColorScalarPaletteTraits<int8_t>;
template class ColorScalarPaletteTraits<uint8_t>;
template class ColorScalarPaletteTraits<int16_t>;
template class ColorScalarPaletteTraits<uint16_t>;
template class ColorScalarPaletteTraits<int32_t>;
template class ColorScalarPaletteTraits<uint32_t>;
template class ColorScalarPaletteTraits<int64_t>;
template class ColorScalarPaletteTraits<uint64_t>;
template class ColorScalarPaletteTraits<float>;
template class ColorScalarPaletteTraits<double>;

} // namespace anatomist
