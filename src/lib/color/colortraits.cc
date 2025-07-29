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
                                         bool insideBounds )
{
  setup1D( 0, minit, maxit, insideBounds );
  setup1D( 1, minit1, maxit1, insideBounds );
}


template <typename T>
void ColorScalarPaletteTraits<T>::setup1D( int dim,
                                           const T & minit, const T & maxit,
                                           bool insideBounds )
{
  colors = palette->colors();

  float  scale;
  float  decal;
  int    cmin;
  int    cmax;
  float  minv;
  float  maxv;
  float  scalen;
  float  decaln;
  int    cminn;
  int    cmaxn;

  unsigned	ncol0;
  float		minc0 = 0.f, maxc0 = 1.f;
  float		mini = static_cast<float>( minit ),
    maxi = static_cast<float>( maxit );

  ncol0 = colors->getSize()[dim];

  if( !insideBounds )
  {
    if( dim == 1 )
    {
      minc0 = palette->min2();
      maxc0 = palette->max2(); // colormap min & max
    }
    else
    {
      minc0 = palette->min1();
      maxc0 = palette->max1(); // colormap min & max
    }
  }
  else if( palette->zeroCenteredAxis( dim ) )
    minc0 = palette->min( dim );

  if( minc0 == maxc0 )
  {
    if( minc0 == 1. )
      minc0 = 0.;
    else
      maxc0 = 1.;
  }

  if( mini == maxi )
  {
    maxi = mini + 1.;
    minv = mini;
    maxv = maxi;
  }
  else
  {
    // thresholds in image values
    if( ( dim == 0 && !palette->zeroCenteredAxis1() )
        || ( dim == 1 && !palette->zeroCenteredAxis2() ) )
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

  if( ( dim == 0 && !palette->zeroCenteredAxis1() )
      || ( dim == 1 && !palette->zeroCenteredAxis2() ) )
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
    minv0 = minv;
    maxv0 = maxv;
    std::cout << "ColorScalarPaletteTraits scale0: " << scale0 << ", " << decal0 << std::endl;
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
    minv1 = minv;
    maxv1 = maxv;
  }
}


template <>
void ColorScalarPaletteTraits<AimsRGB>::setup( const AimsRGB &,
                                               const AimsRGB &,
                                               const AimsRGB &,
                                               const AimsRGB &,
                                               bool insideBounds )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0 = 0.f, minc1 = 0.f, maxc0 = 1.f, maxc1 = 1.f;
  float		mini = 0.f, maxi = 255.f;

  ncol0 = colors->getSizeX();
  ncol1 = colors->getSizeY();

  if( !insideBounds )
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

  minv0 = mini + minc0 * (maxi - mini); // thresholds in image values
  maxv0 = mini + maxc0 * (maxi - mini);
  if( minv0 > maxv0 )
    swap(minv0, maxv0);
  minv1 = mini + minc1 * (maxi - mini);	// thresholds in image values
  maxv1 = mini + maxc1 * (maxi - mini);
  if( minv1 > maxv1 )
    swap(minv1, maxv1);

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
    const AimsRGBA &, const AimsRGBA &, const AimsRGBA &, bool insideBounds )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0 = 0.f, minc1 = 0.f, maxc0 = 1.f, maxc1 = 1.f;
  float		mini = 0.f, maxi = 255.f;

  ncol0 = colors->getSizeX();
  ncol1 = colors->getSizeY();

  if( !insideBounds )
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

  minv0 = mini + minc0 * (maxi - mini); // thresholds in image values
  maxv0 = mini + maxc0 * (maxi - mini);
  if( minv0 > maxv0 )
    swap(minv0, maxv0);
  minv1 = mini + minc1 * (maxi - mini);	// thresholds in image values
  maxv1 = mini + maxc1 * (maxi - mini);
  if( minv1 > maxv1 )
    swap(minv1, maxv1);

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
