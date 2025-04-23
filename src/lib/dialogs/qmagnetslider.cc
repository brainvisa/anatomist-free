
#include <anatomist/dialogs/qmagnetslider.h>
#include <QMouseEvent>
#include <cmath>
#include <time.h>

using namespace anatomist;
using namespace std;


struct QMagnetSlider::Private
{
  Private()
    : double_click_time( 0.2 ),
      last_press_time( 0 ),
      default_value( 0 ),
      pressval( 0 ),
      mag_size( 20. ),
      min1( 0 ),
      max1( 1000 ),
      current_val( 500 ),
      released( false ),
      moved( false )
  {
  }

  float double_click_time;
  QPointF presspos;
  double last_press_time;
  set<float> magnets;
  float default_value;
  float pressval;
  float mag_size;
  float min1;
  float max1;
  float current_val;
  bool released;
  bool moved;
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
  connect( this, SIGNAL( valueChanged( int ) ),
           this, SLOT( valueChangedSlot( int ) ) );
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


void QMagnetSlider::setAbsRange( float min1, float max1 )
{
  if( d->min1 != min1 || d->max1 != max1 )
  {
    d->min1 = min1;
    d->max1 = max1;
    float value = absValue();
    d->current_val += 1.;  // to force change current
    blockSignals( true );
    setAbsValue( value );
    blockSignals( false );
  }
}


void QMagnetSlider::setAbsValue( float value )
{
  if( d->current_val == value )
    return;

  d->current_val = value;
  float r = d->max1 - d->min1;
  if( r == 0.f )
    r = 1.f;
  QSlider::setValue( int( std::round( ( value - d->min1 ) * 1000 / r ) ) );
  emit absValueChanged( value );
}


float QMagnetSlider::absValue() const
{
  return d->current_val;
}


void QMagnetSlider::resetDefault()
{
  setAbsValue( d->default_value );
}


void QMagnetSlider::mousePressEvent( QMouseEvent *event )
{
#if QT_VERSION >= 0x060000
  d->presspos = event->position();
#else
  d->presspos = event->localPos();
#endif

  d->pressval = d->current_val;

  struct timespec ts;
  timespec_get( &ts, TIME_UTC );
  double t = ts.tv_sec + ts.tv_nsec / 1000000000L;
  if( d->last_press_time == 0.
      || t - d->last_press_time > d->double_click_time )
  {
    d->last_press_time = t;
    d->released = false;
    d->moved = false;
  }

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

  d->moved = true;
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
    // could not avoid copy because reverse_iterator is not an iterator
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
  setAbsValue( nval );
  emit sliderMoved( objectName().toStdString() );
}


void QMagnetSlider::valueChangedSlot( int value )
{
  if( d->moved && !d->released )
    return;

  float relval = float( value - QSlider::minimum() )
    / ( QSlider::maximum() - QSlider::minimum() );
  float absval = relval * ( d->max1 - d->min1 ) + d->min1;
  setAbsValue( absval );
}


void QMagnetSlider::mouseReleaseEvent( QMouseEvent *event )
{
  QSlider::mouseReleaseEvent( event );
  float absval = absValue();
  emit sliderReleased( objectName().toStdString() );

  if( d->moved )
  {
    // moved: abort double-click check
    d->last_press_time = 0.;
    d->released = false;
    d->moved = false;
    return;
  }

  if( !d->released )
  {
    // one click so far.
    d->released = true;
    return;
  }

  struct timespec ts;
  timespec_get( &ts, TIME_UTC );
  double t = ts.tv_sec + ts.tv_nsec / 1000000000L;
  if( d->last_press_time != 0.
      && t - d->last_press_time < d->double_click_time )
    emit sliderDoubleClicked();

  d->last_press_time = 0.;
  d->released = false;
  d->moved = false;
}

