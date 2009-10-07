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

#include <anatomist/winperf/qfloatspinbox.h>
#include <qvalidator.h>
#include <math.h>


QFloatSpinBox::QFloatSpinBox( int pr, QWidget *p, const char *n ) :
#if QT_VERSION >= 0x040000
  QFloatSpinBoxBase( p )
#else
  QFloatSpinBoxBase( p, n )
#endif
{
  nDigit = pr;
  prec = (int)pow( 10, pr );

#if QT_VERSION >= 0x040000
  setRange( 0, 99 );
  setDecimals( 1 );
#else
  setValidator( new QDoubleValidator( 0.0, 99.0, 1, this ) );
  connect( this, SIGNAL( valueChanged( double ) ), 
           SLOT( valChange( double ) ) );
#endif
}


QFloatSpinBox::QFloatSpinBox( int min, int max, int pr, int st, QWidget *p,
			      const char *n ) :
#if QT_VERSION >= 0x040000
  QDoubleSpinBox( p )
#else
  QSpinBox( min, max, st, p, n )
#endif
{
  nDigit = pr;
  prec = (int)pow( 10, pr );

  double tmin = (double)min / (double)pow( 10, pr );
  double tmax = (double)max / (double)pow( 10, pr );

#if QT_VERSION >= 0x040000
  setRange( tmin, tmax );
  setDecimals( pr );
#else
  setValidator( new QDoubleValidator( tmin, tmax, pr, this ) );
  connect( this, SIGNAL( valueChanged( double ) ), 
           SLOT( valChange( double ) ) );
#endif
}


QFloatSpinBox::~QFloatSpinBox()
{
}


QString QFloatSpinBox::mapValueToText( int value )
{
  QString tmp = QString( "%1" ).arg( value % prec );
  QString dgt = tmp.rightJustify( nDigit, '0' );

  return QString( "%1.%2" ).arg( value / prec ).arg( dgt );
}


int QFloatSpinBox::mapTextToValue( bool * )
{
  return int( text().toFloat() * prec );
}


#if QT_VERSION >= 0x040000
void QFloatSpinBox::valChange( double value )
{
  emit valueChanged( (float)value );
}
#else
void QFloatSpinBox::valChange( int value )
{
  emit valueChanged( (float)value / (float)prec );
}
#endif

