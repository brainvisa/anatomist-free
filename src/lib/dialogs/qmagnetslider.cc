
#include <anatomist/dialogs/qmagnetslider.h>
#include <QMouseEvent>
#include <time.h>

using namespace anatomist;
using namespace std;


struct QMagnetSlider::Private
{
  Private()
    : double_click_time( 0.3 ),
      last_release_time( 0 ),
      default_value( 0 ),
      pressval( 0 ),
      mag_size( 20. ),
      min1( 0 ),
      max1( 1000 ),
      current_val( 500 )
  {
  }

  float double_click_time;
  QPointF presspos;
  double last_release_time;
  set<float> magnets;
  float default_value;
  float pressval;
  float mag_size;
  float min1;
  float max1;
  float current_val;
};


QMagnetSlider::QMagnetSlider( Qt::Orientation orientation,
                              QWidget *parent )
  : QSlider( orientation, parent ),
    d( new Private )
{
  setMinimum( 0 );
  setMaximum( 1000 );
  setValue( 500 );
  connect( this, SIGNAL( sliderDoubleClicked() ),
           this, SLOT( resetDefault() ) );
}


QMagnetSlider::~QMagnetSlider()
{
  delete d;
}


void QMagnetSlider::setMagnets( const std::set<float> & magnets )
{
  d->magnets = magnets;
}


void QMagnetSlider::setDefault( float value )
{
  d->default_value = value;
}


void QMagnetSlider::setRange( float min1, float max1 )
{
  d->min1 = min1;
  d->max1 = max1;
}


void QMagnetSlider::setValue( float value )
{
  d->current_val = value;
  float r = d->max1 - d->min1;
  if( r == 0.f )
    r = 1.f;
  QSlider::setValue( int( ( value - d->min1 ) * 1000 / r ) );
}


float QMagnetSlider::absValue() const
{
  return d->current_val;
}


void QMagnetSlider::resetDefault()
{
  setValue( d->default_value );
  emit absValueChanged( d->default_value );
}


void QMagnetSlider::mousePressEvent( QMouseEvent *event )
{
#if QT_VERSION >= 0x060000
  d->presspos = event->position();
#else
  d->presspos = event->localPos();
#endif

  d->pressval = d->current_val;
  QSlider::mousePressEvent( event );
  emit sliderPressed( objectName().toStdString() );
}


void QMagnetSlider::mouseMoveEvent( QMouseEvent *event )
{
  QSlider::mouseMoveEvent( event );

#if QT_VERSION >= 0x060000
  QPointF pos = event->position();
#else
  QPointF pos = event->localPos();
#endif

  float xdiff = pos.x() - d->presspos.x();
  float nval_i = xdiff / width();
  float nval = d->pressval + nval_i * ( d->max1 - d->min1 );
  float vrangem = std::min( d->pressval, nval );
  float vrangeM = std::max( d->pressval, nval );
  bool nval_set = false;
  list<float>::iterator m, e;
  list<float> rmag;
  if( xdiff < 0 )
  {
    // could not avoid copy because reverse_iterator is not a iterator
    rmag.insert( rmag.end(), d->magnets.begin(), d->magnets.end() );
    rmag.reverse();
  }
  else
    rmag.insert( rmag.end(), d->magnets.begin(), d->magnets.end() );
  for( m=rmag.begin(), e=rmag.end(); m!=e; ++m )
  {
    if( *m > vrangem && *m < vrangeM )
    {
      if( xdiff > 0 )
      {
        xdiff -= d->mag_size;
        nval_i = xdiff / width();
        float old_nval = nval;
        nval = d->pressval + nval_i * ( d->max1 - d->min1 );
        if( old_nval >= *m && nval <= *m )
        {
          nval = *m;
          nval_set = true;
          break;
        }
      }
      else
      {
        xdiff += d->mag_size;
        nval_i = xdiff / width();
        float old_nval = nval;
        nval = d->pressval + nval_i * ( d->max1 - d->min1 );
        if( old_nval <= *m && nval >= *m )
        {
          nval = *m;
          nval_set = true;
          break;
        }
      }
      vrangem = std::min( d->pressval, nval );
      vrangeM = std::max( d->pressval, nval );
    }
  }
  if( !nval_set )
  {
    nval_i = xdiff / width();
    nval = d->pressval + nval_i * ( d->max1 - d->min1 );
  }
  setValue( nval );
  emit absValueChanged( nval );
  emit sliderMoved( objectName().toStdString() );
}


void QMagnetSlider::mouseReleaseEvent( QMouseEvent *event )
{
  QSlider::mouseReleaseEvent( event );
  float absval = absValue();
  emit absValueChanged( absval );
  emit sliderReleased( objectName().toStdString() );

  struct timespec ts;
  timespec_get( &ts, TIME_UTC );
  double t = ts.tv_sec + ts.tv_nsec / 1000000000L;
  if( d->last_release_time != 0.
      && t - d->last_release_time < d->double_click_time )
  {
    emit sliderDoubleClicked();
    d->last_release_time = 0.;
  }
  else
    d->last_release_time = t;
}

