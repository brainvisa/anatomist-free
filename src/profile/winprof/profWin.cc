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
#include <anatomist/winprof/profWin.h>
#include <anatomist/winprof/profFactory.h>
#include <anatomist/winprof/profX.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/window/winFactory.h>
#include <anatomist/reference/Transformation.h>
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


set< unsigned >  QAProfileWindow::_profCount;
string QAProfileWindow::_baseTitle = "Profile";
int QAProfileWindow::_classType = registerClass();


struct QAProfileWindow::Private
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


QAProfileWindow::QAProfileWindow( QWidget *p, const char *name, 
                                  Object options, Qt::WFlags f ) 
  : QAWindow( p, name, options, f ), d( new Private )
{
  assert( theAnatomist );
  setCaption( "Profile" );

  x_curve = 0;
  pdim = 0;

  QHBox	*hb = new QHBox( this );
  hb->setSpacing( 5 );
  setCentralWidget( hb );
  QVBox *vb = new QVBox( hb );

  QVButtonGroup *bg = new QVButtonGroup( "Direction", vb );
  bt1 = new QRadioButton( "Along X", bg );
  bt2 = new QRadioButton( "Along Y", bg );
  bt3 = new QRadioButton( "Along Z", bg );
  bt4 = new QRadioButton( "Along T", bg );
  bt1->setChecked( true );
  bt1->setEnabled( false );
  bt2->setEnabled( false );
  bt3->setEnabled( false );
  bt4->setEnabled( false );
  bg->setFixedSize( bg->sizeHint() );
  connect( bg, SIGNAL( pressed( int ) ), this, SLOT( dirChange( int ) ) );

#if QWT_VERSION >= 0x050000
  graphic = new QwtPlot( hb );
#else
  graphic = new QwtPlot( hb, "profile" );
#endif
  graphic->setAutoReplot( true );
  graphic->setFixedSize( (int)profileWidth, (int)profileHeight );
  graphic->setAxisTitle( QwtPlot::yLeft, "Amplitude" );
  graphic->setAxisTitle( QwtPlot::xBottom, "Columns" );

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

  pDir = QAProfileWindow::alongX;
  profS = new QAProfileX();

  _position = Point3df( 0.0f, 0.0f, 0.0f );
  _time = 0.0f;
  pmin = Point4df( 1000.0f, 1000.0f, 1000.0f, 1000.0f );
  pmax = Point4df( 0.0f, 0.0f, 0.0f, 0.0f );

  setFixedSize( sizeHint() );
}


QAProfileWindow::~QAProfileWindow()
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

      delete[] pprof[ *it ];
      pprof.erase( *it );
    }

  if ( _instNumber != -1 )
    _profCount.erase( _profCount.find( _instNumber ) );

  delete d;
}


int QAProfileWindow::registerClass()
{
  int type = AWindowFactory::registerType( QT_TRANSLATE_NOOP( "ControlWindow", 
							      "Profile" ), 
					   createProfileWindow );

  return type;
}


AWindow *QAProfileWindow::createProfileWindow( void* dock, Object options )
{
  QWidget	*dk = (QWidget *) dock;
  Qt::WFlags	f  = Qt::WType_TopLevel | Qt::WDestructiveClose;
  if( dock )
    f = 0;
  QAProfileWindow *qapw = new QAProfileWindow( dk, "Profile", options, f );
  qapw->show();
  if( dk )
    dk->resize( dk->sizeHint() );
  return qapw;
}


void QAProfileWindow::displayClickPoint()
{
}


void QAProfileWindow::Draw( bool )
{
}


void QAProfileWindow::registerObject( AObject *object, bool temporaryObject,
                                      int pos )
{
  //cout << "QAProfileWindow::registerObject - " << object->name() << endl;
  if ( _sobjects.find( object ) == _sobjects.end() )
    {
      if ( object->hasTexture() )
	{
	  QAWindow::registerObject( object, temporaryObject, pos );
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

void QAProfileWindow::unregisterObject( AObject *object )
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

  delete[] pprof[ object ];
  pprof.erase( object );

  QAWindow::unregisterObject( object );
  initX();
}


void QAProfileWindow::dirChange( int btn )
{
  switch( btn )
    {
    case 0:
      {
	pDir = QAProfileWindow::alongX;
	graphic->setAxisTitle( QwtPlot::xBottom, "Columns" );
	break;
      }
    case 1:
      {
	pDir = QAProfileWindow::alongY;
	graphic->setAxisTitle( QwtPlot::xBottom, "Lines" );
	break;
      }
    case 2:
      {
	pDir = QAProfileWindow::alongZ;
	graphic->setAxisTitle( QwtPlot::xBottom, "Slices" );
	break;
      }
    case 3:
      {
	pDir = QAProfileWindow::alongT;
	graphic->setAxisTitle( QwtPlot::xBottom, "Time" );
	break;
      }
    }

  QAProfileFactory pfact;
  profS = pfact.create( pDir );

  delete x_curve;
  pdim = profS->size( pmin, pmax );
  x_curve = profS->abscisse( pmin, pdim );

  Refresh();
}


void QAProfileWindow::initX()
{
  pmin = Point4df( 1000.0f, 1000.0f, 1000.0f, 1000.0f );
  pmax = Point4df( 0.0f, 0.0f, 0.0f, 0.0f );

  set< AObject * >::iterator it;
  Point3df		bmin, bmax, bmin2, bmax2;
  Referential *ref = getReferential();

  if ( !_sobjects.empty() )
  {
    for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
      (*it)->boundingBox( bmin, bmax );
      Referential *oref = (*it)->getReferential();
      Transformation *tra = theAnatomist->getTransformation( ref, oref );
      
      if( tra )
      {
        tra->transformBoundingBox( bmin, bmax, 
                                   bmin2, bmax2 );
        bmin = bmin2;
        bmax = bmax2;
      }

      if ( bmin[0] < pmin[0] ) pmin[0] = bmin[0];
      if ( bmax[0] > pmax[0] ) pmax[0] = bmax[0];
      if ( bmin[1] < pmin[1] ) pmin[1] = bmin[1];
      if ( bmax[1] > pmax[1] ) pmax[1] = bmax[1];
      if ( bmin[2] < pmin[2] ) pmin[2] = bmin[2];
      if ( bmax[2] > pmax[2] ) pmax[2] = bmax[2];
      if ( (*it)->MinT() < pmin[3] ) pmin[3] = (*it)->MinT();
      if ( (*it)->MaxT() > pmax[3] ) pmax[3] = (*it)->MaxT();
    }

    bt1->setEnabled( ( pmax[0] > pmin[0] ) ? true : false );
    bt2->setEnabled( ( pmax[1] > pmin[1] ) ? true : false );
    bt3->setEnabled( ( pmax[2] > pmin[2] ) ? true : false );
    bt4->setEnabled( ( pmax[3] > pmin[3] ) ? true : false );

    delete x_curve;
    pdim = profS->size( pmin, pmax );
    x_curve = profS->abscisse( pmin, pdim );

    switch( pDir )
    {
    case QAProfileWindow::alongX:
      {
        if ( pmax[0] <= pmin[0] && bt2->isEnabled() )
          {
            bt2->setChecked( true );
            dirChange( 1 );
          }
        break;
      }
    case QAProfileWindow::alongY:
      {
        if ( pmax[1] <= pmin[1] && bt1->isEnabled() )
          {
            bt1->setChecked( true );
            dirChange( 0 );
          }
        break;
      }
    case QAProfileWindow::alongZ:
      {
        if ( pmax[2] <= pmin[2] && bt1->isEnabled() )
          {
            bt1->setChecked( true );
            dirChange( 0 );
          }
        break;
      }
    case QAProfileWindow::alongT:
      {
        if ( pmax[3] <= pmin[3] && bt1->isEnabled() )
          {
            bt1->setChecked( true );
            dirChange( 0 );
          }
        break;
      }
    }
  }
  else
  {
    pDir = QAProfileWindow::alongX;
    graphic->setAxisTitle( QwtPlot::xBottom, "Columns" );
    bt1->setChecked( true );
    bt1->setEnabled( false );
    bt2->setEnabled( false );
    bt3->setEnabled( false );
    bt4->setEnabled( false );
  }
}


void QAProfileWindow::refreshNow()
{
  set< AObject * >::iterator it;

  QAWindow::refreshNow();

  Point3df vs;
  Point3df thePos;
  Referential *ref = getReferential();

  Point4df incw;
  switch( pDir )
  {
    case alongX:
      incw = Point4df( 1, 0, 0, 0 );
      break;
    case alongY:
      incw = Point4df( 0, 1, 0, 0 );
      break;
    case alongZ:
      incw = Point4df( 0, 0, 1, 0 );
      break;
    case alongT:
      incw = Point4df( 0, 0, 0, 1 );
      break;
  }
  
  Point3df pos0 = Point3df( pmin[0], pmin[1], pmin[2] );
  float t0 = pmin[3];
  if( incw[0] == 0. )
    pos0[0] = _position[0];
  if( incw[1] == 0. )
    pos0[1] = _position[1];
  if( incw[2] == 0. )
    pos0[2] = _position[2];
  if( incw[3] == 0. )
    t0 = _time;

  for ( it=_sobjects.begin(); it!=_sobjects.end(); ++it )
    {
      Referential *oref = (*it)->getReferential();
      Transformation *tra = theAnatomist->getTransformation( ref, oref );
      if( tra )
        thePos = tra->transform( pos0 );
      else
        thePos = pos0;
      vs = (*it)->VoxelSize();
      Point4df increment;
      Point3df inc3;
      if( tra )
      {
        inc3 = tra->transform( Point3df( incw[0], incw[1], incw[2] ) ) 
          - tra->transform( Point3df( 0, 0, 0 ) );
        increment[0] = inc3[0];
        increment[1] = inc3[1];
        increment[2] = inc3[2];
        increment[3] = incw[3];
      }
      else
      {
        increment = incw;
      }
      /* thePos[0] /= vs[0];
      thePos[1] /= vs[1];
      thePos[2] /= vs[2]; */
      if ( pprof.find( *it ) != pprof.end() )  delete[] pprof[ *it ];
      //pprof[ *it ] = profS->doit( *it, thePos, _time, pmin, pdim );
      cout << "doit...\n";
      pprof[ *it ] = profS->doit( *it, thePos, _time, pmin, pdim, increment );
      cout << "done\n";
#if QWT_VERSION >= 0x060000
      QwtPlotCurve      *crv = d->mcurve[ *it ];
      crv->setPen( QPen( QColor( d->pcol[ crv ] ) ) );
      crv->setSamples( x_curve, pprof[ *it ], pdim );
#elif QWT_VERSION >= 0x050000
      QwtPlotCurve	*crv = d->mcurve[ *it ];
      crv->setPen( QPen( QColor( d->pcol[ crv ] ) ) );
      crv->setData( x_curve, pprof[ *it ], pdim );
#else
      int index = d->mcurve[ *it ];
      graphic->setCurvePen( index, QPen( QColor( d->pcol[ index ] ) ) );
      graphic->setCurveData( index, x_curve, pprof[ *it ], pdim );
#endif
    }

  double pos = 0.0;
  if ( !_sobjects.empty() )
    pos = profS->markerPos( thePos, _time, pmin );

#if QWT_VERSION >= 0x050000
  d->pmark1->setValue( pos, 0. );
#else
  graphic->setMarkerPos( d->pmark1, pos, 0.0 );
#endif

  ResetRefreshFlag();
}


QRgb QAProfileWindow::newColor( int index )
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


void QAProfileWindow::setDirection( Direction dir )
{
  switch( dir )
    {
    case QAProfileWindow::alongX:
      {
	if ( !bt1->isChecked() )  bt1->setChecked( true );
	dirChange( 0 );
	break;
      }
    case QAProfileWindow::alongY:
      {
	if ( !bt2->isChecked() )  bt2->setChecked( true );
	dirChange( 1 );
	break;
      }
    case QAProfileWindow::alongZ:
      {
	if ( !bt3->isChecked() )  bt3->setChecked( true );
	dirChange( 2 );
	break;
      }
    case QAProfileWindow::alongT:
      {
	if ( !bt4->isChecked() )  bt4->setChecked( true );
	dirChange( 3 );
	break;
      }
    }
}


const set<unsigned> & QAProfileWindow::typeCount() const
{
  return( _profCount );
}


set<unsigned> & QAProfileWindow::typeCount()
{
  return( _profCount );
}


const string & QAProfileWindow::baseTitle() const
{
  return( _baseTitle );
}
