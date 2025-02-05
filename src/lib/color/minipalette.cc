
#include <anatomist/color/minipalette.h>
#include <anatomist/color/objectPalette.h>
#include <anatomist/color/paletteList.h>
#include <anatomist/color/paletteselectwidget.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/dialogs/qmagnetslider.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/smart/weakptr.h>
#include <cartobase/config/paths.h>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QPushButton>
#include <QDialog>
#include <QTimer>


using namespace anatomist;
using namespace carto;
using namespace std;


struct MiniPaletteGraphics::Private
{
  Private( QGraphicsView *graphicsview, float width, float height,
           float left, float top )
    : aobj( 0 ), width( width ), height( height ),
      left( left ), top( top ), min1( 0. ), max1( 1. ),
      graphicsview( graphicsview ), dim( 0 )
  {
  }

  weak_shared_ptr<AObject> aobj;
  float width;
  float height;
  float left;
  float top;
  float min1;
  float max1;
  list<QGraphicsItem *> tmpitems;
  QGraphicsView *graphicsview;
  int dim;
};


MiniPaletteGraphics::MiniPaletteGraphics( QGraphicsView *graphicsview,
                                          AObject *object,
                                          int dim,
                                          float width, float height,
                                          float left, float top )
  : Observer(),
  d( new Private( graphicsview, width, height, left, top ) )
{
  if( object )
    setObject( object, dim );
}


MiniPaletteGraphics::~MiniPaletteGraphics()
{
  clear();
  delete d;
}


AObject *MiniPaletteGraphics::getObject()
{
  if( d->aobj.isNull() )
    return 0;

  return d->aobj.get();
}


void MiniPaletteGraphics::setObject( AObject *obj, int dim )
{
  if( !d->aobj.isNull() )
    d->aobj->deleteObserver( this );
  d->aobj.reset( 0 );

  if( obj )
  {
    d->aobj = weak_shared_ptr<AObject>( obj );
    d->dim = dim;
    GLComponent *glc = obj->glAPI();
    if( glc )
    {
      const GLComponent::TexExtrema & extr = glc->glTexExtrema( 0 );
      AObjectPalette *pal = obj->palette();
      float valmin = extr.minquant[0];
      float valmax = extr.maxquant[0];
      if( pal->zeroCenteredAxis1() )
      {
        valmax = std::max( std::abs( valmin), std::abs( valmax ) );
        valmin = -valmax;
      }
      setRange( valmin, valmax );
    }
    obj->addObserver( this );
  }
}


void MiniPaletteGraphics::setRange( float min1, float max1 )
{
  d->min1 = min1;
  d->max1 = max1;
}


void MiniPaletteGraphics::updateDisplay()
{
  if( !getObject() )
  {
    d->aobj.reset( 0 );
    return;
  }
  _drawPaletteInGraphicsView();
}


void MiniPaletteGraphics::resize( float x, float y, float w, float h )
{
  d->left = x;
  d->top = y;
  d->width = w;
  d->height = h;
  updateDisplay();
}


float MiniPaletteGraphics::width() const
{
  if( d->width < 0 )
    return d->graphicsview->width();
  return d->width;
}


float MiniPaletteGraphics::height() const
{
  if( d->height < 0 )
    return d->graphicsview->height();
  return d->height;
}


float MiniPaletteGraphics::top() const
{
  if( d->top == -10000 )
    return ( d->graphicsview->height() - height() ) / 2;
  if( d->top >= 0 )
    return d->top;
  return d->graphicsview->height() + d->top;
}


float MiniPaletteGraphics::left() const
{
  if( d->left == -10000 )
    return ( d->graphicsview->width() - width() ) / 2;
  if( d->left >= 0 )
    return d->left;
  return d->graphicsview->width() + d->left;
}


float MiniPaletteGraphics::min1() const
{
  return d->min1;
}


float MiniPaletteGraphics::max1() const
{
  return d->max1;
}


void MiniPaletteGraphics::clear()
{
  QGraphicsScene *scene = d->graphicsview->scene();
  if( scene )
  {
    list<QGraphicsItem *>::iterator i, e = d->tmpitems.end();
    for( i=d->tmpitems.begin(); i!=e; ++i )
      scene->removeItem( *i );
    d->tmpitems.clear();
  }
}


void MiniPaletteGraphics::update( const Observable *, void * )
{
  updateDisplay();
}


void MiniPaletteGraphics::_drawPaletteInGraphicsView()
{
  AObject *obj = getObject();
  if( !obj )
    return;
  QGraphicsView *gv = d->graphicsview;
  AObjectPalette *pal = obj->palette();
  float gwidth = width() - 2;
  float gheight = height() - 2;

  float w = gwidth - 12;
  int baseh = int( std::round( (gheight - 10 ) * 0.33 + 5 ) );
  if( baseh > 30 )
    baseh = 30;
  int baseh2 = gheight - baseh + 3;
  QImage *img( pal->toQImage( w, baseh2 - baseh - 1,
                              pal->relValue1( obj, d->min1 ),
                              pal->relValue1( obj, d->max1 ) ) );
  QPixmap pix = QPixmap::fromImage( *img );
  delete img;
  clear();

  QGraphicsScene *scene = gv->scene();
  QPen paintpen( QColor( 150, 150, 150 ) );
  if( !scene )
  {
    scene = new QGraphicsScene( gv );
    gv->setScene( scene );
  }
  scene->setSceneRect( 0, 0, gv->width() - 2, gv->height() - 2 );
  QGraphicsRectItem *item0 = new QGraphicsRectItem( 0, 0, gwidth, gheight );
  item0->setPen( QPen( QColor( 80, 80, 30 ) ) );
  scene->addItem( item0 );
  d->tmpitems.push_back( item0 );

  QGraphicsRectItem *item1 = new QGraphicsRectItem( 5, baseh, gwidth - 10,
                                                    baseh2 - baseh, item0 );
  item1->setPen( paintpen );
  QGraphicsLineItem *item2 = new QGraphicsLineItem( 5, baseh, 5, 5, item0 );
  item2->setPen( paintpen );
  item2 = new QGraphicsLineItem( gwidth - 5, baseh, gwidth - 5, 5, item0 );
  item2->setPen( paintpen );
  QGraphicsPixmapItem *pixitem = new QGraphicsPixmapItem( pix, item0 );
  QTransform tr = pixitem->transform();
  tr.translate( 6, baseh + 1 );
  pixitem->setTransform( tr );
  float palmin = pal->absMin1( obj );
  float palmax = pal->absMax1( obj );
  float valmin = d->min1;
  float valmax = d->max1;

  float xmin = 6 + w * ( palmin - valmin ) / ( valmax - valmin );
  float xmax = 6 + w * ( palmax - valmin ) / ( valmax - valmin );
  float pmin = pal->min1();
  if( xmin >= 0 && xmin < w )
  {
    QGraphicsLineItem *line = new QGraphicsLineItem(
        xmin, baseh2, xmin, gheight-5, item0 );
    line->setPen( paintpen );
  }
  if( xmax >= 0 && xmax < w )
  {
    QGraphicsLineItem *line = new QGraphicsLineItem(
        xmax, baseh2, xmax, gheight-5, item0 );
    line->setPen( paintpen );
  }
  QPen textpen( QColor( 160, 100, 40 ) );
  QGraphicsSimpleTextItem *text
    = _textGraphicsItem( _format( palmin ), xmin, baseh2 + 3,
                         xmax, gwidth - 5, item0 );
  text->setPen( textpen );
  text = _textGraphicsItem( _format( palmax ), xmax, baseh2 + 3,
                            xmin, gwidth - 5, item0 );
  text->setPen( textpen );
  textpen = QPen( QColor(120, 120, 40 ) );
  text = _textGraphicsItem( _format( valmin ), 8, 5,
                            gwidth - 5, gwidth - 5, item0 );
  text->setPen( textpen );
  text = _textGraphicsItem( _format( valmax ), gwidth - 10, 5,
                            gwidth - 5, gwidth - 5, item0 );
  text->setPen( textpen );
  tr = item0->transform();
  tr.translate( left(), top() );
  item0->setTransform( tr );
}


std::string MiniPaletteGraphics::_format( float num ) const
{
  float x = std::abs( num );
  if( x == 0. )
    return "0";
  stringstream s;
  s << setprecision( 4 ) << num;
  return s.str();
}


QGraphicsSimpleTextItem* MiniPaletteGraphics::_textGraphicsItem(
  const std::string & text, float xpos, float ypos, float xmax, float hardmax,
  QGraphicsItem *parentitem ) const
{
  QGraphicsSimpleTextItem* gtext
    = new QGraphicsSimpleTextItem( text.c_str(), parentitem );
  QFont font = gtext->font();
  float fsize = 6;
  if( width() >= 200 and height() >= 80 )
    fsize = 8;
  font.setPointSize( fsize );
  gtext->setFont( font );
  QTransform tr = gtext->transform();
  float x = xpos + 3;
  float w = gtext->boundingRect().right();
  // avoid intersecting xmax
  if( xpos < xmax && x + w >= xmax - 3 )
  {
    x = xmax - 3 - w;
    // avoid intersecting its own line marker
    if( x <= xpos && x + w >= xpos && xpos >= w + 3 )
      x = xpos - w - 3;
  }
  if( x < 4 )
    x = 4;
  // avoid hardmax (right end of the view)
  if( hardmax > 0 && x + w >= hardmax )
      x = hardmax - w - 3;
  tr.translate( x, ypos );
  gtext->setTransform( tr );
  return gtext;
}


int MiniPaletteGraphics::observedDimension() const
{
  return d->dim;
}


// --


struct MiniPaletteWidget::Private
{
  Private( bool allow_edit, bool self_parent, QWidget *edit_parent,
           bool click_to_edit, bool auto_range )
    : editor( 0 ), self_parent( self_parent ), edit_parent( edit_parent ),
      click_to_edit( click_to_edit ), auto_range( auto_range ),
      graphicsview( 0 ), minipg( 0 ), edit_allowed( false )
  {
  }

  ~Private()
  {
    delete graphicsview;
    delete minipg;
  }

  MiniPaletteWidgetTranscient* editor;
  bool self_parent;
  QWidget *edit_parent;
  bool click_to_edit;
  bool auto_range;
  QClickGraphicsView *graphicsview;
  MiniPaletteGraphics *minipg;
  bool edit_allowed;
};


MiniPaletteWidget::MiniPaletteWidget( AObject *object, int dim,
                                      bool allow_edit, bool self_parent,
                                      QWidget *edit_parent, bool click_to_edit,
                                      bool auto_range )
  : QWidget(),
    d( new Private( allow_edit, self_parent, edit_parent, click_to_edit,
                    auto_range ) )
{
  QVBoxLayout *lay = new QVBoxLayout( this );
  lay->setContentsMargins( 0, 0, 0, 0 );
  d->graphicsview = new QClickGraphicsView;
  lay->addWidget( d->graphicsview );
  d->graphicsview->setFocusPolicy( Qt::NoFocus );
  d->minipg = new MiniPaletteGraphics( d->graphicsview, object, dim );
  if( object )
    setObject( object, dim );
  allowEdit( allow_edit, self_parent, edit_parent );
  connect( d->graphicsview, SIGNAL( mouseReleased( QMouseEvent * ) ),
           this, SLOT( gvReleased( QMouseEvent * ) ) );
}


MiniPaletteWidget::~MiniPaletteWidget()
{
  clear();
  delete d;
}


AObject *MiniPaletteWidget::getObject()
{
  return d->minipg->getObject();
}


void MiniPaletteWidget::setObject( AObject *obj, int dim )
{
  d->minipg->setObject( obj, dim );
  updateDisplay();
}


void MiniPaletteWidget::allowEdit( bool allow, bool self_parent,
                                   QWidget *edit_parent )
{
  d->edit_allowed = allow;
  d->self_parent = self_parent;
  d->edit_parent = edit_parent;
  if( allow )
    setFocusPolicy( Qt::StrongFocus );
  else
    setFocusPolicy( Qt::NoFocus );
}


void MiniPaletteWidget::setRange( float min1, float max1 )
{
  d->minipg->setRange( min1, max1 );
}


void MiniPaletteWidget::updateDisplay()
{
  d->minipg->updateDisplay();
}


void MiniPaletteWidget::clear()
{
  d->minipg->clear();
}


void MiniPaletteWidget::showEditor()
{
  AObject *obj = getObject();
  if( !d->edit_allowed || !obj )
    return;

  if( !d->editor )
  {
    QWidget *parent = d->edit_parent;
    if( d->self_parent )
        parent = this;
    d->editor = new MiniPaletteWidgetTranscient(
        obj, observedDimension(), this, parent, d->click_to_edit,
        d->auto_range );
    connect( d->editor, SIGNAL( editorClosed() ),
             this, SLOT( editorClosed() ) );
  }
  else
      d->editor->reposition();
  d->editor->show();
}


void MiniPaletteWidget::hideEditor()
{
  if( d->editor )
    d->editor->hide();
}


void MiniPaletteWidget::resizeEvent( QResizeEvent *event )
{
  QWidget::resizeEvent( event );
  updateDisplay();
}


void MiniPaletteWidget::focusInEvent( QFocusEvent *event )
{
  QWidget::focusInEvent( event );
  if( !d->click_to_edit )
    showEditor();
}


void MiniPaletteWidget::focusOutEvent( QFocusEvent *event )
{
  QWidget::focusOutEvent( event );
}


#if QT_VERSION >= 0x060000
void MiniPaletteWidget::enterEvent( QEnterEvent *event )
{
  QWidget::enterEvent( event );
  if( !d->click_to_edit )
    showEditor();
}

#else
void MiniPaletteWidget::enterEvent( QEvent *event )
{
  QWidget::enterEvent( event );
  if( !d->click_to_edit )
    showEditor();
}
#endif


void MiniPaletteWidget::leaveEvent( QEvent *event )
{
  QWidget::leaveEvent( event );
}


void MiniPaletteWidget::mouseReleaseEvent( QMouseEvent *event )
{
  QWidget::mouseReleaseEvent( event );
  if( d->click_to_edit )
  {
    event->accept();
    showEditor();
  }
}


void MiniPaletteWidget::wheelEvent( QWheelEvent *event )
{
  event->accept();
  float scale = 2.;
  if( event->angleDelta().y() > 0 )
    scale = 0.5;
  const AObject *obj = getObject();
  if( !obj )
    return;
  const AObjectPalette *pal = obj->palette();
  float c = 1.;
  if( pal->zeroCenteredAxis1() )
    c = ( d->minipg->max1() + d->minipg->min1() ) / 2.;
  else
    c = ( pal->absMax1( obj ) + pal->absMin1( obj ) ) / 2;
  float nmin = c - ( d->minipg->max1() - d->minipg->min1() ) / 2 * scale;
  float nmax = c + ( d->minipg->max1() - d->minipg->min1() ) / 2 * scale;

  const GLComponent::TexExtrema & te = obj->glAPI()->glTexExtrema();
  float tmin = te.minquant[0];
  float tmax = te.maxquant[0];
  float absmin1 = pal->absMin1( obj );
  float absmax1 = pal->absMax1( obj );
  float rmax = std::max( std::abs( tmax ), std::abs( tmin ) );
  rmax = std::max( rmax, absmax1 );
  rmax = std::max( rmax, absmin1 );
  float rmin = 0.;
  if( pal->zeroCenteredAxis1() )
  {
    if( nmax < rmax )
        rmax = nmax;
    rmin = -rmax;
  }
  else
  {
    rmin = std::min( absmin1, absmax1 );
    rmin = std::min( rmin, tmin );
    rmin = std::min( rmin, tmax );
    if( rmin < nmin )
      rmin = nmin;
    if( rmax > nmax )
      rmax = nmax;
  }

  setRange( rmin, rmax );
  updateDisplay();
  emit rangeChanged( rmin, rmax );
}


void MiniPaletteWidget::gvReleased( QMouseEvent *event )
{
  if( d->click_to_edit )
  {
    event->accept();
    showEditor();
  }
  else
    emit paletteClicked();
}


void MiniPaletteWidget::editorClosed()
{
  setRange(
    d->editor->editor()->miniPaletteWidget()->miniPaletteGraphics()->min1(),
    d->editor->editor()->miniPaletteWidget()->miniPaletteGraphics()->max1() );
  updateDisplay();
}


MiniPaletteGraphics *MiniPaletteWidget::miniPaletteGraphics()
{
  return d->minipg;
}


QGraphicsView *MiniPaletteWidget::graphicsView()
{
  return d->graphicsview;
}


int MiniPaletteWidget::observedDimension() const
{
  return d->minipg->observedDimension();
}


// ---

struct MiniPaletteWidgetEdit::Private
{
  Private()
    : minslider( 0 ), maxslider( 0 ), minipw( 0 ), auto_range( false ),
      auto_btn( 0 ), auto_btn_timer( 0 ), defmin( 0 ), defmax( 0 )
  {
  }

  ~Private()
  {
  }

  QMagnetSlider *minslider;
  QMagnetSlider *maxslider;
  MiniPaletteWidget *minipw;
  bool auto_range;
  QToolButton *auto_btn;
  QTimer *auto_btn_timer;
  float defmin;
  float defmax;
};


MiniPaletteWidgetEdit::MiniPaletteWidgetEdit( AObject *object,
                                              int dim,
                                              bool auto_range )
  : QWidget(), Observer(), d( new Private() )
{
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout( layout );
  d->minslider = new QMagnetSlider( Qt::Horizontal, this );
  d->minslider->setObjectName( "min_slider" );
  d->minipw = new MiniPaletteWidget( 0, 0, false, true, 0, false );
  d->minipw->setParent( this );
  d->maxslider = new QMagnetSlider( Qt::Horizontal, this );
  d->maxslider->setObjectName( "max_slider" );
  layout->addWidget( d->minslider );
  layout->addWidget( d->minipw );
  layout->addWidget( d->maxslider );
  setObject( object, dim );
  d->minipw->graphicsView()->setMouseTracking( true );
  connect( d->minipw->graphicsView(), SIGNAL( mouseMoved( QMouseEvent * ) ),
           this, SLOT( gvMoved( QMouseEvent * ) ) );
  connect( d->minslider, SIGNAL( absValueChanged( float ) ),
           this, SLOT( minChanged( float ) ) );
  connect( d->maxslider, SIGNAL( absValueChanged( float ) ),
           this, SLOT( maxChanged( float ) ) );
  setAutoRange( auto_range );
  connect( d->minipw, SIGNAL( rangeChanged( float, float ) ),
           this, SLOT( setRange( float, float ) ) );
  connect( d->minipw, SIGNAL( paletteClicked() ),
           this, SLOT( selectPalette() ) );
}


MiniPaletteWidgetEdit::~MiniPaletteWidgetEdit()
{
  delete d;
}


void MiniPaletteWidgetEdit::setObject( AObject *obj, int dim )
{
  d->minipw->setObject( obj, dim );
  if( !obj->glAPI() || obj->glAPI()->glNumTextures() == 0 )
    return;
  obj->addObserver( this );
  adjustRange();
}


void MiniPaletteWidgetEdit::setAutoRange( bool auto_range )
{
  if( auto_range == d->auto_range )
    return;
  d->auto_range = auto_range;
  if( auto_range )
  {
    connect( d->minslider, SIGNAL( sliderReleased() ),
             this, SLOT( adjustRange() ) );
    connect( d->maxslider, SIGNAL( sliderReleased() ),
             this, SLOT( adjustRange() ) );
  }
  else
  {
    disconnect( d->minslider, SIGNAL( sliderReleased() ),
                this, SLOT( adjustRange() ) );
    disconnect( d->maxslider, SIGNAL( sliderReleased() ),
                this, SLOT( adjustRange() ) );
  }
}


void MiniPaletteWidgetEdit::adjustRange()
{
  AObject *obj = d->minipw->getObject();
  if( obj && obj->glAPI() )
  {
    AObjectPalette *pal = obj->palette();
    GLComponent::TexExtrema & te = obj->glAPI()->glTexExtrema();
    float tmin = te.minquant[0];
    float tmax = te.maxquant[0];
    float min1 = pal->min1();
    float max1 = pal->max1();
    float absmin1 = pal->absMin1( obj );
    float absmax1 = pal->absMax1( obj );
    obj->adjustPalette();
    pal = obj->palette();
    d->defmin = pal->absMin1( obj );
    d->defmax = pal->absMax1( obj );
    pal->setMin1( min1 );
    pal->setMax1( max1 );
    float rmax = std::max( std::abs( tmax ), std::abs( tmin ) );
    rmax = std::max( rmax, absmax1 );
    rmax = std::max( rmax, absmin1 );
    float rmin;
    if( pal->zeroCenteredAxis1() )
    {
      if( rmax > std::max( std::abs( absmin1 ), std::abs( absmax1 ) ) * 2 )
        rmax = std::max( std::abs( absmin1 ), std::abs( absmax1 ) ) * 2;
      rmin = -rmax;
    }
    else
    {
      rmin = std::min( absmin1, absmax1 );
      rmin = std::min( rmin, tmin );
      rmin = std::min( rmin, tmax );
      if( std::abs( absmax1 - absmin1 ) < std::abs( rmax - rmin ) / 2 )
      {
        float s = std::abs( absmax1 - absmin1 );
        float c = ( absmin1 + absmax1 ) / 2;
        float rmin2 = c - s;
        float rmax2 = c + s;
        if( rmin < rmin2 )
          rmin = rmin2;
        else
        {
          c = ( rmin + std::max( absmin1, absmax1 ) ) / 2;
          s = std::max( absmin1, absmax1 ) - rmin;
          rmax2 = c + s;
        }
        if( rmax > rmax2 )
          rmax = rmax2;
      }
    }
    d->minslider->setRange( rmin, rmax );
    d->maxslider->setRange( rmin, rmax );
    d->minslider->setValue( absmin1 );
    d->maxslider->setValue( absmax1 );
    d->minipw->setRange( rmin, rmax );
    d->minipw->updateDisplay();
  }
}


void MiniPaletteWidgetEdit::updateDisplay()
{
  d->minipw->updateDisplay();
  AObject *obj = d->minipw->getObject();
  if( obj && obj->glAPI() )
  {
    GLComponent::TexExtrema & te = obj->glAPI()->glTexExtrema();
    AObjectPalette *pal = obj->palette();
    std::set<float> mag;
    mag.insert( te.minquant[0] );
    mag.insert( te.maxquant[0] );
    mag.insert( d->defmin );
    mag.insert( d->defmax );
    if( pal->zeroCenteredAxis1() )
    {
      mag.insert( 0 );
      mag.insert( std::max( std::abs( te.minquant[0] ),
                            std::abs( te.maxquant[0] ) ) );
    }
    d->minslider->setDefault( d->defmin );
    d->minslider->setValue( pal->absMin1(obj) );
    d->maxslider->setMagnets( mag );
    d->maxslider->setDefault( d->defmax );
    d->maxslider->setValue( pal->absMax1( obj ) );
    d->minslider->setMagnets( mag );
  }
}


void MiniPaletteWidgetEdit::update( const Observable *observable, void *arg )
{
  updateDisplay();
}



void MiniPaletteWidgetEdit::minChanged( float value )
{
  AObject *obj = d->minipw->getObject();
  if( obj )
  {
    AObjectPalette *pal = obj->palette();
    if( pal->absMin1( obj ) != value )
    {
      pal->setAbsMin1( obj, value );
      if( obj->glAPI() )
        obj->glAPI()->glSetTexImageChanged();
      obj->notifyObservers();
    }
  }
}


void MiniPaletteWidgetEdit::maxChanged( float value )
{
  AObject *obj = d->minipw->getObject();
  if( obj )
  {
    AObjectPalette *pal = obj->palette();
    if( pal->absMax1( obj ) != value )
    {
      pal->setAbsMax1( obj, value );
      if( obj->glAPI() )
        obj->glAPI()->glSetTexImageChanged();
      obj->notifyObservers();
    }
  }
}


void MiniPaletteWidgetEdit::setRange( float rmin, float rmax )
{
  d->minslider->setRange( rmin, rmax );
  d->maxslider->setRange( rmin, rmax );
  AObject *obj = d->minipw->getObject();
  if( obj )
  {
    AObjectPalette *pal = obj->palette();
    d->minslider->setValue( pal->absMin1( obj ) );
    d->maxslider->setValue( pal->absMax1( obj ) );
  }
}


void MiniPaletteWidgetEdit::selectPalette()
{
  QDialog dial( this, Qt::Popup | Qt::FramelessWindowHint );
  QVBoxLayout *lay = new QVBoxLayout;
  dial.setLayout( lay );
  PaletteSelectWidget* palsel = new PaletteSelectWidget( this );
  lay->addWidget( palsel );
  QHBoxLayout *butl = new QHBoxLayout;
  lay->addLayout( butl );
  QPushButton *but = new QPushButton( "Done" );
  but->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  butl->addWidget( but );
  but->setDefault( true );
  connect( but, SIGNAL( clicked() ), &dial, SLOT( accept() ) );
  connect( palsel, SIGNAL( paletteSelected( const std::string & ) ),
           this, SLOT( setPalette( const std::string & ) ) );
  connect( palsel, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ),
           &dial, SLOT( accept() ) );
  dial.resize(500, 800);
  dial.exec();
}


void MiniPaletteWidgetEdit::setPalette( const std::string & palname )
{
  AObject *obj = d->minipw->getObject();
  if( obj )
  {
    AObjectPalette *pal = obj->palette();
    rc_ptr<APalette> apal = theAnatomist->palettes().find( palname );
    pal->setRefPalette( apal );
    if( obj->glAPI() )
      obj->glAPI()->glSetTexImageChanged();
    obj->notifyObservers();
  }
}


void MiniPaletteWidgetEdit::gvMoved( QMouseEvent *event )
{
#if QT_VERSION >= 0x060000
  QPointF pos = event->position();
#else
  QPointF pos = event->localPos();
#endif

  if( pos.x() >= d->minipw->graphicsView()->width() - 40 && pos.y() <= 80 )
  {
    if( !d->auto_btn )
    {
      d->auto_btn = new QToolButton( d->minipw );
      string icon_path = Paths::findResourceFile( "icons/auto.png",
                                                  "anatomist" );
      QIcon icon( icon_path.c_str() );
      d->auto_btn->setIcon( icon );
      d->auto_btn->setFixedSize( 32, 32 );
      d->auto_btn->setCheckable( true );
      d->auto_btn->setChecked( d->auto_range );
      d->auto_btn->setToolTip( "auto-scale mode" );
      connect( d->auto_btn, SIGNAL( toggled( bool ) ),
               this, SLOT( setAutoRange( bool ) ) );
    }
    d->auto_btn->move( d->minipw->graphicsView()->width() - 40, 30 );
    d->auto_btn->show();
    if( !d->auto_btn_timer )
    {
      d->auto_btn_timer = new QTimer( this );
      d->auto_btn_timer->setSingleShot( true );
      d->auto_btn_timer->setInterval( 2000 );
      connect( d->auto_btn_timer, SIGNAL( timeout() ),
               this, SLOT( clearAutoBtn() ) );
    }
    d->auto_btn_timer->start();
  }
}


void MiniPaletteWidgetEdit::clearAutoBtn()
{
  d->auto_btn->hide();
}


MiniPaletteWidget *MiniPaletteWidgetEdit::miniPaletteWidget()
{
  return d->minipw;
}


QSlider *MiniPaletteWidgetEdit::minSlider()
{
  return d->minslider;
}


QSlider *MiniPaletteWidgetEdit::maxSlider()
{
  return d->maxslider;
}


// ----


struct MiniPaletteWidgetTranscient::Private
{
  Private()
    : pw( 0 ), out_focus( false ), released( true ), opened_by_click( false ),
      minipw( 0 )
  {
  }

  MiniPaletteWidget *pw;
  bool out_focus;
  bool released;
  bool opened_by_click;
  MiniPaletteWidgetEdit *minipw;
};


MiniPaletteWidgetTranscient::MiniPaletteWidgetTranscient(
  AObject *object, int dim, MiniPaletteWidget* pw, QWidget *parent,
  bool opened_by_click, bool auto_range )
  : QWidget( parent, Qt::Popup | Qt::FramelessWindowHint ),
    d( new Private )
{
  setObjectName( "frameless_minipalette" );
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( layout );
  d->pw = pw;
  d->opened_by_click = opened_by_click;
  d->minipw = new MiniPaletteWidgetEdit( object, dim, auto_range );
  layout->addWidget( d->minipw );
  reposition();
  connect( d->minipw->minSlider(), SIGNAL( sliderPressed() ),
           this, SLOT( sliderPressed() ) );
  connect( d->minipw->maxSlider(), SIGNAL( sliderPressed() ),
           this, SLOT( sliderPressed() ) );
  connect( d->minipw->minSlider(), SIGNAL( sliderPressed() ),
           this, SLOT( sliderReleased() ) );
  connect( d->minipw->maxSlider(), SIGNAL( sliderPressed() ),
           this, SLOT( sliderReleased() ) );
}


MiniPaletteWidgetTranscient::~MiniPaletteWidgetTranscient()
{
  delete d;
}


void MiniPaletteWidgetTranscient::reposition()
{
  MiniPaletteWidget *pw = d->pw;
  if( pw )
  {
    pw->ensurePolished();
    QRect rect = pw->geometry();
    QPoint pos = pw->mapToGlobal( QPoint( 0, 0 ) );
    rect.setTop( std::max( pos.y() - 30, 0 ) );
    rect.setLeft( std::max( pos.x() - 9, 0 ) );
    rect.setWidth( pw->width() + 9 );
    rect.setHeight( pw->height() + 30 );
    setGeometry( rect );
  }
}


void MiniPaletteWidgetTranscient::closeIfFinished()
{
  if( d->out_focus && d->released )
    close();
}


void MiniPaletteWidgetTranscient::sliderPressed()
{
  d->released = false;
}


void MiniPaletteWidgetTranscient::sliderReleased()
{
  d->released = true;
  closeIfFinished();
}


void MiniPaletteWidgetTranscient::leaveEvent( QEvent *event )
{
  QWidget::leaveEvent( event );
  /* if opened by a click (an explicit active action),
     don't close when leaving the window: the user will do another
     explicit action (click outside) to close the editor. */
  if( !d->opened_by_click )
  {
    d->out_focus = true;
    closeIfFinished();
  }
}


void MiniPaletteWidgetTranscient::focusOutEvent( QFocusEvent *event )
{
  QWidget::focusOutEvent( event );
  d->out_focus = true;
  closeIfFinished();
}


#if QT_VERSION >= 0x060000
void MiniPaletteWidgetTranscient::enterEvent( QEnterEvent *event )
#else
void MiniPaletteWidgetTranscient::enterEvent( QEvent *event )
#endif
{
  QWidget::enterEvent( event );
  d->out_focus = false;
}


void MiniPaletteWidgetTranscient::focusInEvent( QFocusEvent *event )
{
  QWidget::focusInEvent( event );
  d->out_focus = false;
}


void MiniPaletteWidgetTranscient::keyPressEvent( QKeyEvent *event )
{
  if( event->key() == Qt::Key_Escape )
  {
    event->accept();
    close();
  }
  else
    QWidget::keyPressEvent( event );
}


void MiniPaletteWidgetTranscient::closeEvent( QCloseEvent *event )
{
  QWidget::closeEvent( event );
  if( event->isAccepted() )
    emit editorClosed();
}


MiniPaletteWidgetEdit* MiniPaletteWidgetTranscient::editor()
{
  return d->minipw;
}

