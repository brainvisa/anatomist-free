#ifndef ANA_COLOR_MINIPALETTE_H
#define ANA_COLOR_MINIPALETTE_H

#include <anatomist/dialogs/qclickgraphicsview.h>
#include <anatomist/object/Object.h>
#include <anatomist/observer/Observer.h>


class QSlider;


namespace anatomist
{

  /** MiniPaletteGraphics is an element which draws a palette in a
      GraphicsView scene. It is used by MiniPaletteWidget, but can be used
      alone in a QGraphicsView.

      It provides a small sized palette widget which can be used to display
      the palette.

      The palette view displayes the palette assigned to an object, and the
      view may be zoomed to a given values range.
  */
  class MiniPaletteGraphics : public Observer
  {
  public:
    /**
      Parameters:
      graphicsview:
          the existing graphic view where the palette should be drawn
      object:
          object to display or edit the palette for
      width:
          width of the display in the graphics view. -10000 (default) means
          whole scene width.
      height:
          height of the display in the graphics view. -10000 (default) means
          whole scene height.
      left:
          left position of the display in the graphics view. -10000 (default)
          means centered in scene.
      top:
          top position of the display in the graphics view. -10000 (default)
          means centered in scene.
      with_view: bool
          if false, the palette view will not be displayed.
    */
    MiniPaletteGraphics( QGraphicsView *graphicsview, AObject *object = 0,
                         int dim = 0,
                         float width = -10000, float height = -10000,
                         float left = -10000, float top = -10000,
                         bool with_view = true );
    virtual ~MiniPaletteGraphics();

    AObject *getObject();
    /// set or change the observed object
    void setObject( AObject *obj, int dim = 0 );
    void setRange( float min1, float max1 );
    void updateDisplay();
    void resize( float x, float y, float w, float h );
    float width() const;
    float height() const;
    float top() const;
    float left() const;
    void clear();
    void update( const Observable *observable, void *arg );
    float min1() const;
    float max1() const;
    int observedDimension() const;

  private:
    struct Private;

    void _drawPaletteInGraphicsView();
    std::string _format( float num ) const;
    QGraphicsSimpleTextItem* _textGraphicsItem(
      const std::string & text, float xpos, float ypos, float xmax,
      float hardmax=-1, QGraphicsItem *parentitem=0 ) const;

    Private *d;
  };


  /** MiniPaletteWidget.

    It provides a small sized palette widget which can be used both to display
    the palette in a GUI, and to edit the palette range (optionally).

    The palette view displayes the palette assigned to an object, and the view
    may be zoomed to a given values range.

    The palatte may be zoomed in/out using the mouse wheel. This action will
    not change the palette settings, but only the view displayed.

    Edition is possible if enabled, either using the allow_edit constructor
    parameter, or using the method :meth:`allow_edit`.

    Edition is triggered in 2 modes:

    - if ``click_to_edit`` is True (the default), a click on the palette will
      open the editor mode.
    - otherwise a mouse hover will open it, and it will be closed when the
      mouse leaves the editor, without the need for a user click.

    The edition mode opens a popup frameless widget, with sliders.
    \see MiniPaletteWidgetTranscient.
  */
  class MiniPaletteWidget : public QWidget
  {
    Q_OBJECT

  public:
    /**
        Parameters:

        object: AObject or null
            object to display or edit the palette for
        dim: int
            texture dimension of the object
        allow_edit: bool
            if True, an editor will popup, either by clicking on the widget, or
            by "hovering" it if ``click_to_edit`` is False.
        self_parent: bool
            if true, the edit parent (see edit_parent) will be the
            MiniPaletteWidget, this.
        edit_parent: QWidget or 0
            the parent widget passed to the editor widget, if edition is
            allowed. If in self_parent mode, this valus is not used.
        click_to_edit: bool
            if False, the edition widget will popup as soon as the mouse cursor
            passes over the palette widget, without clicking.
            If True, only a user click will open the editor window.
        auto_range: bool
            For edition mode, allow the auto-zoom mode when palette range is
            modified.
      with_view: bool
          if false, the palette view will not be displayed.
    */
    MiniPaletteWidget( AObject *object = 0, int dim = 0,
                       bool allow_edit = true,
                       bool self_parent = true, QWidget *edit_parent = 0,
                       bool click_to_edit = true, bool auto_range = false,
                       bool with_view = true );
    virtual ~MiniPaletteWidget();
    AObject *getObject();
    /** set or change the observed object, dim is the texture dimension
        observed */
    void setObject( AObject *obj, int dim = 0 );
    /** Enalbes or disable the edition capabilities

    Parameters
    ----------
    allow: bool
        if True, an editor will popup, either by clicking on the widget, or
        by "hovering" it if ``click_to_edit`` is False.
    self_parent: bool
        if true, the edit parent (see edit_parent) will be the
        MiniPaletteWidget, this.
    edit_parent: QWidget  or 0
        the parent widget passed to the editor widget, if edition is
        allowed. If in self_parent mode, this valus is not used.
    */
    void allowEdit( bool allow, bool self_parent = true,
                    QWidget *edit_parent = 0 );
    /// set the view range in object values
    void setRange( float min1, float max1 );
    /// redraws the palette view
    void updateDisplay();
    void clear();
    /// pops up the editor, if the edition is allowed
    void showEditor();
    void hideEditor();
    MiniPaletteGraphics *miniPaletteGraphics();
    QGraphicsView *graphicsView();
    int observedDimension() const;

  signals:
    /** signal emitted when the zoom range has changed (after a mouse wheel
        event, typically)
    */
    void rangeChanged( float, float );
    /** signal emitted when the palete view is clicked, and ``click_to_edit``
        mode is disabled.
    */
    void paletteClicked();

  protected:
    void resizeEvent( QResizeEvent *event );
    void focusInEvent( QFocusEvent *event );
    void focusOutEvent( QFocusEvent *event );
#if QT_VERSION >= 0x060000
    void enterEvent( QEnterEvent *event );
#else
    void enterEvent( QEvent *event );
#endif
    void leaveEvent( QEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void wheelEvent( QWheelEvent *event );

  protected slots:
    void gvReleased( QMouseEvent *event );
    void editorClosed();

  private:
    struct Private;

    Private *d;
  };


  /** Mini palette editor widget.

    MiniPaletteWidgetEdit is part of the MiniPaletteWidget
    infrastructure and in most cases will not be used directly.

    However a GUI may incorporate the editor widget.

    It is normally used within MiniPaletteWidgetTranscient, itself
    used in the edition mode of MiniPaletteWidget. In turn,
    MiniPaletteWidgetEdit contains a non-editable
    MiniPaletteWidget object.

    The editor thus presents a palette view, plus 2 sliders to set the min and
    max range of the palette. The view may be zoomed using the mouse wheel (see
    MiniPaletteWidget), and it can also use an automatic zoom mode, if
    ``auto_range=true`` is passed to the constructor, or set_auto_range()
    is called. In auto range mode, the zoom range is adapted after each user
    interaction on sliders (after the mouse is released).
  */
  class MiniPaletteWidgetEdit : public QWidget, public Observer
  {
    Q_OBJECT

  public:
    MiniPaletteWidgetEdit( AObject *object = 0, int dim = 0,
                           bool auto_range = false, bool with_view = true,
                           bool allow_no_palette = false );
    virtual ~MiniPaletteWidgetEdit();

    /// set or change the observed object
    void setObject( AObject *obj, int dim = 0 );
    AObject* getObject();
    /// redraws the palette and sliders values
    void updateDisplay();
    virtual void update( const Observable *observable, void *arg );
    MiniPaletteWidget *miniPaletteWidget();
    QSlider *minSlider();
    QSlider *maxSlider();
    int observedDimension() const;

  public slots:
    /// auto-range function
    void adjustRange();
    /// allows or disables the auto-zoom mode
    void setAutoRange( bool auto_range );
    void minChanged( float value );
    void maxChanged( float value );
    void setRange( float rmin, float rmax );
    void selectPalette();
    void setPalette( const std::string & palname );
    void gvMoved( QMouseEvent *event );
    void clearAutoBtn();

  private:
    struct Private;

    Private *d;
  };


  /**
    The transcient palette editor widget features a
    MiniPaletteWidgetEdit which shows up upon given conditions (see
    MiniPaletteWidget) and closes when the editor widget loses focus.

    More precisely, if opened by a click, a complete focus loss is needed to
    close the window (which is generally triggered by another user action like
    a click at some other place or a keyboard focus change, using <tab> for
    instance).

    If not opened by a click, the widget will close as soon as the mouse
    pointer leaves the widget surface, or when the focus is lost, thus not
    requiring a click or keyboard user action.
  */
  class MiniPaletteWidgetTranscient : public QWidget
  {
    Q_OBJECT

  public:
    MiniPaletteWidgetTranscient( AObject *object = 0,
                                 int dim = 0,
                                 MiniPaletteWidget* pw = 0,
                                 QWidget *parent = 0,
                                 bool opened_by_click = false,
                                 bool auto_range = false );
    virtual ~MiniPaletteWidgetTranscient();
    /** Repositions / resizes the widget to superpose on its
    MiniPaletteWidget
    */
    void reposition();
    MiniPaletteWidgetEdit* editor();

  public slots:
    void closeIfFinished();
    void sliderPressed();
    void sliderReleased();

  signals:
    /// signal emitted when the editor widget closes
    void editorClosed();

  protected:
    void leaveEvent( QEvent *event );
    void focusOutEvent( QFocusEvent *event );
#if QT_VERSION >= 0x060000
    void enterEvent( QEnterEvent *event );
#else
    void enterEvent( QEvent *event );
#endif
    void focusInEvent( QFocusEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void closeEvent( QCloseEvent *event );

  private:
    struct Private;

    Private *d;
  };

}

#endif
