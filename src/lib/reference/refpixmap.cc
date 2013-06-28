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


#include <anatomist/reference/refpixmap.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/control/coloredpixmap.h>
#include <anatomist/application/Anatomist.h>
#include <aims/rgb/rgb.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <map>

#include <qapplication.h>

using namespace anatomist;
using namespace std;

QPixmap ReferencePixmap::referencePixmap( const Referential* ref,
                                            bool ownref,
                                            unsigned RefPixSize )
{
  if( theAnatomist->destroying() )
    return QPixmap();
  static map<QRgb, QPixmap> refpixs0;
  static map<QRgb, QPixmap> refpixs1;
  static QPixmap    noref;

  if( !ref )
  {
//     if( noref.isNull() )
//     {
//       QBitmap       bmp( 1, 1 );
//       noref = QPixmap( 1, 1 );
//       noref.fill( Qt::color0 );
//       bmp.fill( Qt::color0 );
//       noref.setMask( bmp );
//     }
    return noref;
  }

  AimsRGB   col = ref->Color();
  QColor    qcol( col.red(), col.green(), col.blue() );
  if( RefPixSize <= 12 )
  {
    if( ownref )
      return ColoredPixmapCache::coloredPixmap( qcol, RefPixSize,
                                                "referential-small.png",
                                                "referential-small-fg.png" );
    else
      return ColoredPixmapCache::coloredPixmap
          ( qcol, RefPixSize, "referential-weak-small.png",
            "referential-weak-small-fg.png" );
  }
  return ColoredPixmapCache::coloredPixmap( qcol, RefPixSize,
                                            "referential.png",
                                            "referential-fg.png" );
  /*
  map<QRgb, QPixmap>::iterator ip;
  QPixmap   *p = 0;
  if( ownref )
    {
      ip = refpixs1.find( qcol );
      if( ip != refpixs1.end() )
        return ip->second;
      p = &refpixs1[ qcol ];
    }
  else
    {
      ip = refpixs0.find( qcol );
      if( ip != refpixs0.end() )
        return ip->second;
      p = &refpixs0[ qcol ];
    }

  QPixmap       & pix = *p;
  QBitmap       bmp;

  bmp.resize( RefPixSize, RefPixSize );
  pix.resize( RefPixSize, RefPixSize );
  bmp.fill( Qt::color0 );
  pix.fill( QColor( col.red(), col.green(), col.blue() ) );

  QPainter      pen( &bmp );

  pen.setPen( Qt::color1 );
  pen.setBrush( Qt::color1 );
  pen.drawEllipse( 0, 0, RefPixSize, RefPixSize );
  pen.end();

  pix.setMask( bmp );

  pen.begin( &pix );
  pen.setPen( Qt::black );
  pen.setBrush( QBrush( Qt::NoBrush ) );
  pen.drawEllipse( 0, 0, RefPixSize, RefPixSize );
  if( !ownref )
  {
    const int c = 3;
    pen.setPen( Qt::white );
    pen.setBrush( Qt::white );
    pen.drawEllipse( RefPixSize/2-c, RefPixSize/2-c, c*2, c*2 );
  }
  pen.end();

  return pix;
  */
}


