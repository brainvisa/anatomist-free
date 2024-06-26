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


#include <anatomist/color/palette.h>
#include <anatomist/gradientwidget/gradient.h>


using namespace anatomist;
using namespace std;


APalette:: APalette( const string & name, unsigned dimx, unsigned dimy, 
                     unsigned dimz, unsigned dimt )
  : Volume<AimsRGBA>( dimx, dimy, dimz, dimt ), _name( name ),
    _transp( false )
{
}


APalette::~APalette()
{
}


void APalette::update()
{
  fillFromGradients();
  unsigned x, y, z, t, dx = getSizeX(), dy = getSizeY(), dz = getSizeZ(),
    dt = getSizeT();
  _transp = false;
  for( t=0; t<dt && !_transp; ++t )
    for( z=0; z<dz && !_transp; ++z )
      for( y=0; y<dy && !_transp; ++y )
        for( x=0; x<dx; ++x )
          if( this->at( x, y, z, t ).alpha() != 255 )
          {
            _transp = true;
            break;
          }
}


bool APalette::hasGradients() const
{
  return header().hasProperty( "palette_gradients" );
}


void APalette::fillFromGradients()
{
  string grad_def;
  try
  {
    grad_def = header().getProperty( "palette_gradients" )->getString();
  }
  catch( ... )
  {
    return;
  }

  string grad_mode;
  try
  {
    grad_mode = header().getProperty( "palette_gradients_mode" )->getString();
  }
  catch( ... )
  {
  }

  bool hsv = ( grad_mode == "HSV" );

  Gradient grad( hsv );
  grad.fromString( grad_def );

  unsigned i, psize = getSizeX();
  vector<QRgb> buf( psize );

  grad.fillGradient( &buf[0], psize, 0, true );
  for( i=0; i<psize; ++i )
  {
    QRgb & rgb = buf[i];
    (*this)( i ) = AimsRGBA( qRed( rgb ), qGreen( rgb ), qBlue( rgb ),
                             qAlpha( rgb ));
  }
}

