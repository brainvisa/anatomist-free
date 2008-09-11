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

#include <anatomist/color/colortraits.h>

using namespace anatomist;

template <typename T> 
void ColorScalarPaletteTraits<T>::setup( const T & minit, const T & maxit )
{
  colors = palette->colors();

  unsigned	ncol0, ncol1;
  float		minc0, minc1, maxc0, maxc1;
  float		mini = (float) minit, maxi = (float) maxit;

  ncol0 = colors->dimX();
  ncol1 = colors->dimY();

  minc0 = palette->min1();
  maxc0 = palette->max1(); // colormap min & max
  minc1 = palette->min2();
  maxc1 = palette->max2(); // colormap min & max

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
      minv0 = mini + minc0 * (maxi - mini); // thresholds in image values
      maxv0 = mini + maxc0 * (maxi - mini);
      if( minv0 > maxv0 )
	{
	  float tmp = minv0;
	  minv0 = maxv0;
	  maxv0 = tmp;
	}
      minv1 = mini + minc1 * (maxi - mini);	// thresholds in image values
      maxv1 = mini + maxc1 * (maxi - mini);
      if( minv1 > maxv1 )
	{
	  float tmp = minv1;
	  minv1 = maxv1;
	  maxv1 = tmp;
	}
    }

  scale0 = ( (float) ncol0 ) / ( (maxi - mini) * (maxc0 - minc0) );
  scale1 = ( (float) ncol1 ) / ( (maxi - mini) * (maxc1 - minc1) );
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
  /*cout << "cmin0: " << cmin0 << ", cmax0: " << cmax0 << ", cmin1: " << cmin1 
       << ", cmax1: " << cmax1 << ", scale0: " << scale0 << ", scale1: " 
       << scale1 << endl;*/
}


template class ColorScalarPaletteTraits<int8_t>;
template class ColorScalarPaletteTraits<uint8_t>;
template class ColorScalarPaletteTraits<int16_t>;
template class ColorScalarPaletteTraits<uint16_t>;
template class ColorScalarPaletteTraits<int32_t>;
template class ColorScalarPaletteTraits<uint32_t>;
template class ColorScalarPaletteTraits<float>;
template class ColorScalarPaletteTraits<double>;
