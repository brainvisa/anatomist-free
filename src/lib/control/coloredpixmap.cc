/* Copyright (c) 2007 CEA
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

#include <anatomist/control/coloredpixmap.h>
#include <anatomist/application/settings.h>
#include <qimage.h>
#include <qpixmapcache.h>
#include <map>

using namespace anatomist;
using namespace std;
#include <iostream>

QPixmap ColoredPixmapCache::coloredPixmap( const QColor & rgb, unsigned size,
                                           const std::string & colbgfilename,
                                           const std::string & fgfilename )
{
  QString key = QString( colbgfilename.c_str() ) + "-"
      + QString( fgfilename.c_str() ) + "-" + QString::number( size ) +
      "-" + QString( "%1" ).arg( rgb.rgb(), 0, 16 );
  /* cout << "key for " << colbgfilename << " / " << fgfilename << " : "
      << key << endl; */
  QPixmap *cpix1 = QPixmapCache::find( key );
  if( cpix1 )
    return *cpix1;
  static map<string, QImage>  imgs;
  QPixmap cpix( size, size );

  map<string, QImage>::iterator i = imgs.find( colbgfilename ), e = imgs.end();
  QImage  *bg1;
  if( i == e )
  {
    bg1 = &imgs[ colbgfilename ];
    bg1->load( (Settings::globalPath() + "/icons/" + colbgfilename).c_str() );
  }
  else
    bg1 = &i->second;

  QImage *fg1;
  i = imgs.find( fgfilename );
  if( i == e )
  {
    fg1 = &imgs[ fgfilename ];
    fg1->load( (Settings::globalPath() + "/icons/" + fgfilename).c_str() );
  }
  else
    fg1 = &i->second;

  if( bg1->isNull() )
  {
    cpix.fill( rgb );
  }
  else
  {
    if( fg1->isNull() )
      cout << "problem: " << fgfilename << " is null\n";
#if QT_VERSION >= 0x040000
    QImage bg = bg1->scaled( size, size, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation );
    QImage fg = fg1->scaled( size, size, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation );
#else
    QImage bg = bg1->scale( size, size, QImage::ScaleMin );
    QImage fg = fg1->scale( size, size, QImage::ScaleMin );
#endif
    QImage  cim( bg.width(), bg.height(), 32 );
    cim.setAlphaBuffer( true );
    float red = rgb.red() / 255., green = rgb.green() / 255.,
                        blue = rgb.blue() / 255.;
    int x, y, w = cim.width(), h = cim.height();
    for( y=0; y<h; ++y )
      for( x=0; x<w; ++x )
      {
        QRgb c = fg.pixel( x, y ), d = bg.pixel( x, y );
        float al = qAlpha( c ) / 255.;
        int r = int( qRed(c) * al + qRed(d) * red * (1. - al) );
        int g = int( qGreen(c) * al + qGreen(d) * green * (1. - al) );
        int b = int( qBlue(c) * al + qBlue(d) * blue * (1. - al) );
        QRgb e = qRgba( r, g, b, max( int(al * 255), qAlpha( d ) ) );
        cim.setPixel( x, y, e );
      }
    cpix = cim;
  }
  QPixmapCache::insert( key, cpix );
  return cpix;
}


