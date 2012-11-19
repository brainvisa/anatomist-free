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


#include <assert.h>
#include <anatomist/winhisto/histoWin.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/object/Object.h>
#include <anatomist/window/winFactory.h>
#include <aims/qtcompat/qhbox.h>
#include <aims/qtcompat/qvbox.h>
#include <qlayout.h>
#include <aims/qtcompat/qvbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcolor.h>
#include <qpen.h>
#include <qwt_plot.h>
#if QWT_VERSION >= 0x050000
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#endif
#include <iostream>

#include <stdio.h>


using namespace anatomist;
using namespace carto;
using namespace std;


set< unsigned >  QAHistogramWindow::_histCount;
string QAHistogramWindow::_baseTitle = "Histogram";
int QAHistogramWindow::_classType = registerClass();


struct QAHistogramWindow::Private
{
public:
#if QWT_VERSION >= 0x050000
  map< AObject *, QwtPlotCurve * > mcurve;
  map< QwtPlotCurve *, QRgb > pcol;
  QwtPlotMarker *pmark1;
#else
  map< AObject *, int > mcurve;
  map< int, QRgb > pcol;
  int pmark1;
#endif
};


QAHistogramWindow::QAHistogramWindow( QWidget *p, const char *name, 
                                      Object options, Qt::WFlags f ) 
  : QAWindow( p, name, options, f ), d( new Private )
{
  assert( theAnatomist );
  setCaption( "Histogram" );

  x_curve = 0;

  QHBox	*hb = new QHBox( this );
  hb->setSpacing( 5 );
  setCentralWidget( hb );

#if QWT_VERSION >= 0x050000
  graphic = new QwtPlot( hb );
#else
  graphic = new QwtPlot( hb, "histogram" );
#endif
  graphic->setAutoReplot( true );
  graphic->setFixedSize( (int)histogramWidth, (int)histogramHeight );
  graphic->setAxisTitle( QwtPlot::yLeft, "Opportunities" );
  graphic->setAxisTitle( QwtPlot::xBottom, "Gray levels" );

#if QWT_VERSION >= 0x050000
  d->pmark1 = new QwtPlotMarker;
  d->pmark1->attach( graphic );
  d->pmark1->setLinePen( QPen( QColor( 255, 255, 255 ) ) );
  d->pmark1->setLineStyle( QwtPlotMarker::VLine );
#else
  d->pmark1 = graphic->insertLineMarker( "", QwtPlot::xBottom );
  graphic->setMarkerLinePen( d->pmark1, QPen( QColor( 255, 255, 255 ) ) );
  graphic->setMarkerLineStyle( d->pmark1, QwtMarker::VLine );
#endif

  _position = Point3df( 0.0f, 0.0f, 0.0f );
  _time = 0.0f;

  setFixedSize( sizeHint() );
}


QAHistogramWindow::~QAHistogramWindow()
{
  delete x_curve;

  set< AObject * >::iterator it;
  for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
#if QWT_VERSION >= 0x050000
      //QwtPlotCurve	*crv = d->mcurve[ *it ];
      //crv->detach();
      //delete crv;
#else
      graphic->removeCurve( d->mcurve[ *it ] );
#endif
      d->pcol.erase( d->mcurve[ *it ] );
      d->mcurve.erase( *it );

      delete[] phisto[ *it ];
      phisto.erase( *it );
    }

  if ( _instNumber != -1 )
    _histCount.erase( _histCount.find( _instNumber ) );

  delete d;
}


int QAHistogramWindow::registerClass()
{
  int type = AWindowFactory::registerType(
    QT_TRANSLATE_NOOP( "ControlWindow", "Histogram" ), createHistogramWindow,
    true );

  return type;
}


AWindow *QAHistogramWindow::createHistogramWindow( void* dock, Object options )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f  = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( dock )
    f = 0;
  QAHistogramWindow *qapw = new QAHistogramWindow( dk, "Histogram", options, f );
  qapw->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return qapw;
}


void QAHistogramWindow::displayClickPoint()
{
}


void QAHistogramWindow::Draw( bool )
{
}


void QAHistogramWindow::registerObject( AObject *object, bool temp, int pos )
{
  if ( object->type() == AObject::VOLUME )
    {
      if ( _sobjects.find( object ) == _sobjects.end() )
	{
	  if ( object->hasTexture() )
	    {
	      QAWindow::registerObject( object, temp, pos );
#if QWT_VERSION >= 0x050000
	      QwtPlotCurve	*crv = new QwtPlotCurve( "" );
	      crv->attach( graphic );
	      d->mcurve[ object ] = crv;
	      d->pcol[ crv ] = newColor( graphic->itemList().count() );
#else
	      int index = graphic->insertCurve( "" );
	      d->mcurve[ object ] = index;
	      d->pcol[ index ] = newColor( index );
#endif
	      initX();
	    }
	  else
	    {
	      cerr << "Object : " << object->name()
		   << " cannot be displayed in window " 
		   << Title() << endl;
	    }
	}
    }
}

void QAHistogramWindow::unregisterObject( AObject *object )
{
  if( _sobjects.find( object ) == _sobjects.end() )
    return;
#if QWT_VERSION >= 0x050000
  QwtPlotCurve	*crv = d->mcurve[ object ];
  crv->detach();
  delete crv;
  graphic->replot();
#else
  graphic->removeCurve( d->mcurve[ object ] );
#endif
  d->pcol.erase( d->mcurve[ object ] );
  d->mcurve.erase( object );

  delete[] phisto[ object ];
  phisto.erase( object );

  QAWindow::unregisterObject( object );
  initX();
}


void QAHistogramWindow::initX()
{
  set< AObject * >::iterator it;

  if ( !_sobjects.empty() )
  {
    for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
      if ( !qaHisto.add( *it ) )
      {
        unregisterObject( *it );
        return; // unregisterObject() calls initX() again
      }
    }

    delete x_curve;
    x_curve = qaHisto.abscisse();

    for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
      if ( phisto.find( *it ) != phisto.end() )
        delete[] phisto[ *it ];

        phisto[ *it ] = qaHisto.doit( *it );
    }
  }
}


void QAHistogramWindow::refreshNow()
{
  set< AObject * >::iterator it;

  QAWindow::refreshNow();

  for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
#if QWT_VERSION >= 0x050000
      QwtPlotCurve	*crv = d->mcurve[ *it ];
      crv->setPen( QPen( QColor( d->pcol[ crv ] ) ) );
#if QWT_VERSION >= 0x060000
      QwtCPointerData *cp = new QwtCPointerData( x_curve, phisto[ *it ],
                                                 qaHisto.getDim() );
      crv->setData( cp );
#else
      crv->setData( x_curve, phisto[ *it ], qaHisto.getDim() );
#endif
#else
      int index = d->mcurve[ *it ];
      graphic->setCurvePen( index, QPen( QColor( d->pcol[ index ] ) ) );
      graphic->setCurveData( index, x_curve, phisto[ *it ], qaHisto.getDim() );
#endif
    }

  ResetRefreshFlag();
}


QRgb QAHistogramWindow::newColor( int index )
{
  int nmax = 64, ind = (int)index;
  int r, g, b;

  if ( ind > nmax )  ind = nmax;

  float nstep = (float)( ( nmax % 8 ) ? ( nmax / 8 + 1 ) : ( nmax / 8 ) );
  float val = (float)( ( ind % 8 ) ? ( ind / 8 + 1 ) : ( ind / 8 ) ) - 1.0f;

  r = (int)( 255.0f * (float)( ind & 1 ) * ( 1.0f - val / nstep ) );
  g = (int)( 255.0f * (float)( ( ind & 2 ) / 2 ) * ( 1.0f - val / nstep ) );
  b = (int)( 255.0f * (float)( ( ind & 4 ) / 4 ) * ( 1.0f - val / nstep ) );

  return qRgb( r, g, b );
}


const set<unsigned> & QAHistogramWindow::typeCount() const
{
  return( _histCount );
}


set<unsigned> & QAHistogramWindow::typeCount()
{
  return( _histCount );
}


const string & QAHistogramWindow::baseTitle() const
{
  return( _baseTitle );
}
