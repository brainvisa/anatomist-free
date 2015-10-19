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

using namespace anatomist;
using std::swap;


template <typename T>
void ColorScalarPaletteTraits<T>::setup( const T & minit, const T & maxit )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0, minc1, maxc0, maxc1;
  float		mini = static_cast<float>( minit ),
    maxi = static_cast<float>( maxit );

  ncol0 = colors->dimX();
  ncol1 = colors->dimY();

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

  if( mini == maxi )
    {
      maxi = mini + 1.;
      minv0 = mini;
      maxv0 = maxi;
      minv1 = mini;
      maxv1 = maxi;
    }
  else
    {
      // thresholds in image values
      minv0 = float( mini + minc0 * (double(maxi) - mini) );
      maxv0 = float( mini + maxc0 * (double(maxi) - mini) );
      if( minv0 > maxv0 )
        swap(minv0, maxv0);
      // thresholds in image values
      minv1 = float( mini + minc1 * (double(maxi) - mini) );
      maxv1 = float( mini + maxc1 * (double(maxi) - mini) );
      if( minv1 > maxv1 )
        swap(minv1, maxv1);
    }

  scale0 = float( ( static_cast<double>( ncol0 ) )
    / ( (double(maxi) - mini) * (maxc0 - minc0) ) );
  scale1 = float( ( static_cast<double>( ncol1 ) )
    / ( (double(maxi) - mini) * (maxc1 - minc1) ) );
  decal0 = float( - ( mini / (double(maxi) - mini) + minc0 ) * ncol0
    / (maxc0 - minc0) );
  decal1 = float( - ( mini / (double(maxi) - mini) + minc1 ) * ncol1
    / (maxc1 - minc1) );
  cmin0 = 0;
  cmin1 = 0;
  cmax0 = colors->dimX() - 1;
  cmax1 = colors->dimY() - 1;

  if( scale0 < 0 )
    {
      cmin0 = cmax0;
      cmax0 = 0;
    }
  if( scale1 < 0 )
    {
      cmin1 = cmax1;
      cmax1 = 0;
    }

  /*
  std::cout << "cmin0: " << cmin0 << ", cmax0: " << cmax0 << ", cmin1: " << cmin1
            << ", cmax1: " << cmax1 << ", scale0: " << scale0 << ", scale1: "
            << scale1 << std::endl; */
}


namespace anatomist
{

template <>
void ColorScalarPaletteTraits<AimsRGB>::setup( const AimsRGB &,
                                               const AimsRGB & )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0, minc1, maxc0, maxc1;
  float		mini = 0.F, maxi = 255.F;

  ncol0 = colors->dimX();
  ncol1 = colors->dimY();

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
  cmax0 = colors->dimX() - 1;
  cmax1 = colors->dimY() - 1;

  if( scale0 < 0 )
  {
    cmin0 = cmax0;
    cmax0 = 0;
  }
  if( scale1 < 0 )
  {
    cmin1 = cmax1;
    cmax1 = 0;
  }

}


template <>
void ColorScalarPaletteTraits<AimsRGBA>::setup( const AimsRGBA &,
    const AimsRGBA & )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0, minc1, maxc0, maxc1;
  float		mini = 0.F, maxi = 255.F;

  ncol0 = colors->dimX();
  ncol1 = colors->dimY();

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
  cmax0 = colors->dimX() - 1;
  cmax1 = colors->dimY() - 1;

  if( scale0 < 0 )
  {
    cmin0 = cmax0;
    cmax0 = 0;
  }
  if( scale1 < 0 )
  {
    cmin1 = cmax1;
    cmax1 = 0;
  }

}

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
