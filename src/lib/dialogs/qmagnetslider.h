
#ifndef ANATOMIST_DIALOGS_QMAGNETSLIDER
#define ANATOMIST_DIALOGS_QMAGNETSLIDER

#include <QSlider>
#include <string>
#include <set>


namespace anatomist
{

  /** Specialized slider with "magnet" values, used for
    MiniPaletteWidgetEdit.

    It features float min/max values (typically matching an AObject texture
    values),
    magnets which mark some given significant values, and emits signals when
    the slider is moved.

    The values range can be changed afterwards.
  */
  class QMagnetSlider: public QSlider
  {
    Q_OBJECT

  public:
    QMagnetSlider( Qt::Orientation orientation = Qt::Vertical,
                   QWidget *parent = nullptr );
    virtual ~QMagnetSlider();

    /** Magnets are "attractive" values, where the mouse must be moved
        further to pass them when moving the slider.
    */
    void setMagnets( const std::set<float> & magnets );
    void setDefault( float value );
    void setRange( float min1, float max1 );
    void setValue( float value );
    float absValue() const;

  public slots:
    void resetDefault();

  signals:
    // signal emitted when the value changes, in object texture value scale
    void absValueChanged( float );
    // signal emitted when the slider is pressed
    void sliderPressed( const std::string & );
    // signal emitted when the slider is moved
    void sliderMoved( const std::string & );
    // signal emitted when the slider is released
    void sliderReleased( const std::string & );
    // signal emitted when the slider is double-clicked
    void sliderDoubleClicked();

  protected:
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );

  private:
    struct Private;

    Private *d;
  };


}

#endif
