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

#include <anatomist/window/qwinblock.h>
#include <anatomist/window/qwindow.h>
#include <anatomist/application/settings.h>
#include <anatomist/control/windowdrag.h>
#include <anatomist/controler/icondictionary.h>
#include <anatomist/window/qWinFactory.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qdialog.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <QLabel>
#include <QTimer>
#include <QDrag>


using namespace anatomist;
using namespace std;


struct QAWindowBlock::Private
{
  QGridLayout	*layout;
  bool inrows;
  int colsrows;
  float rectratio;
  set<AWindow *> droppedWins;
  int droprow;
  int dropcol;
};


QAWindowBlock::QAWindowBlock( QWidget *parent, const char* name,
                              Qt::WindowFlags f,
                              int colsrows, bool inrows, bool withmenu )
  : QMainWindow( parent, f ), d( new QAWindowBlock::Private )
{
  setObjectName(name);
  setAttribute(Qt::WA_DeleteOnClose);
  if( windowFlags() & Qt::Window )
    {
      QPixmap	anaicon( Settings::findResourceFile(
                         "icons/icon.xpm" ).c_str() );
      if( !anaicon.isNull() )
        setWindowIcon( anaicon );
    }

  setAcceptDrops( true );

  QWidget *mainw = new QWidget( this );
  setCentralWidget( mainw );
  d->layout = new QGridLayout( mainw );
  d->layout->setSpacing(0); 
  d->layout->setContentsMargins( 5, 5, 5, 5 );
  mainw->setLayout(d->layout);
  d->inrows = inrows;
  d->colsrows = colsrows;
  if( d->colsrows == 0 )
    d->colsrows = 2;
  d->rectratio = 1.;

  if( withmenu )
  {
    QMenuBar *mb = menuBar();
    QMenu *menu = mb->addMenu( tr( "Windows block layout" ) );
    QAction *colac = new QAction( tr( "Lay in columns" ), this );
    menu->addAction( colac );
    QAction *rowac = new QAction( tr( "Lay in rows" ), this );
    menu->addAction( rowac );
    QAction *recac = new QAction( tr( "Lay in rectangle" ), this );
    menu->addAction( recac );
    menu->addSeparator();
    QAction *ncolac = new QAction( tr( "Set number of columns" ), this );
    menu->addAction( ncolac );
    QAction *nrowac = new QAction( tr( "Set number of rows" ), this );
    menu->addAction( nrowac );
    QAction *ratac = new QAction(
      tr( "Set columns / rows ratio for rectangular layout" ), this );
    menu->addAction( ratac );

    connect( colac, SIGNAL( triggered() ), this, SLOT( layInColumns() ) );
    connect( rowac, SIGNAL( triggered() ), this, SLOT( layInRows() ) );
    connect( recac, SIGNAL( triggered() ), this, SLOT( layInRectangle() ) );
    connect( ncolac, SIGNAL( triggered() ), this, SLOT( setColumnsNumber() ) );
    connect( nrowac, SIGNAL( triggered() ), this, SLOT( setRowsNumber() ) );
    connect( ratac, SIGNAL( triggered() ), this,
             SLOT( setRectangularRatio() ) );
  }
}


QAWindowBlock::~QAWindowBlock()
{
   delete d;
}


bool QAWindowBlock::inRows() const
{
  return d->inrows;
}


int QAWindowBlock::columnCount() const
{
  if( d->inrows )
    return d->colsrows;
  else
    return static_cast<QGridLayout *>( layout() )->columnCount();
}


int QAWindowBlock::rowCount() const
{
  if( d->inrows )
    return static_cast<QGridLayout *>( layout() )->rowCount();
  else
    return d->colsrows;
}


float QAWindowBlock::widthHeightRatio() const
{
  return d->rectratio;
}


void QAWindowBlock::addWindowToBlock( QWidget *item, bool withborders )
{
  // if we don't remove the item's parent, the window will not be in the block
  // but next to it.
  if( item->parentWidget()
      && dynamic_cast<DraggableWrapper *>( item->parentWidget() ) )
    item = item->parentWidget();
  item->setParent(0);
  int row = 0, col = 0;
  QLayoutItem *litem;

  if( d->inrows )
  {
    int nr = d->layout->rowCount();
    for( row=0; row<nr; ++row )
    {
      for( col=0; col<d->colsrows; ++col )
      {
        litem = d->layout->itemAtPosition( row, col );
        if( !litem || !litem->widget() )
          break;
      }
      if( col < d->colsrows )
        break;
      col = 0;
    }
  }
  else
  {
    int nc = d->layout->columnCount();
    for( col=0; col<nc; ++col )
    {
      for( row=0; row<d->colsrows; ++row )
      {
        litem = d->layout->itemAtPosition( row, col );
        if( !litem || !litem->widget() )
          break;
      }
      if( row < d->colsrows )
        break;
      row = 0;
    }
  }
  if( withborders && !dynamic_cast<DraggableWrapper *>( item ) )
  {
    int default_stretch = 300;

    if( d->layout->columnStretch( col ) == 0 )
      d->layout->setColumnStretch( col, default_stretch );
    if( d->layout->rowStretch( row ) == 0 )
      d->layout->setRowStretch( row, default_stretch );
    DraggableWrapper *dwrap = new DraggableWrapper( item, d->layout );
    item->setParent( dwrap );
    d->layout->addWidget( dwrap, row, col );
  }
  else
    d->layout->addWidget( item, row, col );
//   item->setParent( centralWidget() ); // seems needed with Qt5

  Observable *obs = dynamic_cast<Observable *>( item );
  if( obs )
    obs->addObserver( this );
}


void QAWindowBlock::dragEnterEvent( QDragEnterEvent* event )
{
  //cout << "QAWindow::dragEnterEvent\n";
  bool ok = QAWindowDrag::canDecode( event->mimeData() );
  event->setAccepted( ok );
  if( ok )
    setCursor( Qt::DragMoveCursor );
  else
    setCursor( Qt::ForbiddenCursor );
}


void QAWindowBlock::dragMoveEvent( QDragMoveEvent* event )
{
  bool ok = QAWindowDrag::canDecode( event->mimeData() );
  event->setAccepted( ok );
  if( ok )
    setCursor( Qt::DragMoveCursor );
  else
    setCursor( Qt::ForbiddenCursor );
}


void QAWindowBlock::dragLeaveEvent( QDragLeaveEvent* event )
{
  setCursor( Qt::ArrowCursor );
}


void QAWindowBlock::dropEvent( QDropEvent* event )
{
  //cout << "QAWindow::dropEvent\n";
  setCursor( Qt::ArrowCursor );

  set<AWindow *>	w;

  if( QAWindowDrag::decode( event->mimeData(), w ) )
  {
    d->droppedWins = w;
    dropRowCol( event->pos().x(), event->pos().y(), d->droprow, d->dropcol );
    QTimer::singleShot( 0, this, SLOT( windowsDropped() ) );
  }
}


void QAWindowBlock::dropRowCol( int x, int y, int & row, int & col ) const
{
  int r, c, nr = d->layout->rowCount(), nc = d->layout->columnCount();
  row = -1;
  col = -1;
  for( r=0; r<nr; ++r )
  {
    QRect rect = d->layout->cellRect( r, 0 );
    if( y >= rect.top() && y < rect.bottom() )
    {
      row = r;
      break;
    }
  }
  if( row < 0 )
    row = nr;
  for( c=0; c<nc; ++c )
  {
    QRect rect = d->layout->cellRect( 0, c );
    if( x >= rect.left() && x < rect.right() )
    {
      col = c;
      break;
    }
  }
  if( col < 0 )
    col = nc;
  QRect rect = d->layout->cellRect( row, col );
  int left = x - rect.left();
  int right = rect.right() - x;
  int top = y - rect.top();
  int bottom = rect.bottom() - y;

  if( right < left && right < top && right < bottom )
    ++col;
  else if( bottom < top && bottom < left && bottom < right )
    ++row;
}



void QAWindowBlock::windowsDropped()
{
  set<AWindow *>::iterator	iw, ew = d->droppedWins.end();
  list<QWidget *> reordered;
  int row, col, nr = d->layout->rowCount(), nc = d->layout->columnCount();
  bool inserted = false;

  // cout << "drop: " << d->droprow << ", " << d->dropcol << endl;

  vector<int> rc( 2, 0 );
  vector<int> nrc( 2 );
  vector<int> drc( 2 );
  nrc[0] = nr;
  nrc[1] = nc;
  drc[0] = d->droprow;
  drc[1] = d->dropcol;
  int index0 = d->inrows ? 1 : 0, index1 = 1 - index0;

  for( rc[index1]=0; rc[index1]<nrc[index1]; ++rc[index1] )
  {
    for( rc[index0]=0; rc[index0]<nrc[index0]; ++rc[index0] )
    {
      if( rc[index0] == drc[index0] && rc[index1] == drc[index1] )
      {
        inserted = true;
        QAWindow			*qw;
        for( iw=d->droppedWins.begin(); iw!=ew; ++iw )
        {
          qw = dynamic_cast<QAWindow *>( *iw );
          if( qw )
          {
            reordered.push_back( qw );
            qw->enableDetachMenu( true );
          }
        }
      }
      QLayoutItem *item = d->layout->itemAtPosition( rc[0], rc[1] );
      if( item && item->widget() )
      {
        QWidget *wid = item->widget();
        DraggableWrapper *dw = dynamic_cast<DraggableWrapper *>( wid );
        QAWindow * qw = 0;
        if( dw )
        {
          QLayoutItem *li = static_cast<QGridLayout *>(
            dw->layout() )->itemAtPosition( 1, 1 );
          if( li )
            qw = dynamic_cast<QAWindow *>( li->widget() );
        }
        if( !qw )
          qw = dynamic_cast<QAWindow *>( wid );
        if( !qw || d->droppedWins.find( qw ) == ew )
          reordered.push_back( wid );
      }
    }
    if( !inserted && rc[index1] == drc[index1] )
    {
      inserted = true;
      QAWindow			*qw;
      for( iw=d->droppedWins.begin(); iw!=ew; ++iw )
      {
        qw = dynamic_cast<QAWindow *>( *iw );
        if( qw )
        {
          reordered.push_back( qw );
          qw->enableDetachMenu( true );
        }
      }
    }
  }

  if( !inserted )
  {
    QAWindow			*qw;
    for( iw=d->droppedWins.begin(); iw!=ew; ++iw )
    {
      qw = dynamic_cast<QAWindow *>( *iw );
      if( qw )
      {
        reordered.push_back( qw );
        qw->enableDetachMenu( true );
      }
    }
  }
  reorderViews( reordered );
}


void QAWindowBlock::closeEvent( QCloseEvent * event )
{
  // if one of its widgets cannot be closed, the block is not closed either but only hidden.
  int row, col;
  int nr = d->layout->rowCount(), nc = d->layout->columnCount();
  QWidget *widget;
  QLayoutItem *item;
  bool closeOk=1;
  for( row=0; row<nr; ++row )
    {
      for( col=0; col<nc; ++col )
      {
        item = d->layout->itemAtPosition( row, col );
        if( item )
        {
          widget = item->widget();
          if( widget )
          {
            closeOk = closeOk & widget->close();
          }
        }
      }
    }
    if (closeOk){
      event->accept();
    }
    else{
      cout << "can't delete windows block - just hiding it." << endl;
      event->ignore();
      hide();
    }
}

void QAWindowBlock::setColsOrRows( bool inrows, int colsrows )
{
  if( d->inrows == inrows && d->colsrows == colsrows
    && ( ( d->inrows && d->layout->columnCount() == colsrows )
      || ( !d->inrows && d->layout->rowCount() == colsrows ) ) )
    return; // nothing to do

  int row = 0, col = 0, irow = 0, icol = 0;
  d->inrows = inrows;
  d->colsrows = colsrows;
  QWidget *widget;
  QLayoutItem *item;
  list<pair<QWidget *, pair<int, int> > > moved;

  if( inrows )
  {
    int nr = d->layout->rowCount(), nc = d->layout->columnCount();
    for( row=0; row<nr; ++row )
    {
      for( col=0; col<nc; ++col )
      {
        item = d->layout->itemAtPosition( row, col );
        if( item )
        {
          widget = item->widget();
          if( widget )
          {
            d->layout->removeWidget( widget );
            item = d->layout->itemAtPosition( row, col );
            if( item )
            {
              d->layout->removeItem( item );
              delete item;
            }
            moved.push_back( make_pair( widget, make_pair( irow, icol ) ) );
//             d->layout->addWidget( widget, irow, icol );
            ++icol;
            if( icol >= colsrows )
            {
              icol = 0;
              ++irow;
            }
          }
          else
          {
            d->layout->removeItem( item );
            delete item;
          }
        }
      }
    }
  }
  else
  {
    int nr = d->layout->rowCount(), nc = d->layout->columnCount();
    for( col=0; col<nc; ++col )
    {
      for( row=0; row<nr; ++row )
      {
        item = d->layout->itemAtPosition( row, col );
        if( item )
        {
          widget = item->widget();
          if( widget )
          {
            d->layout->removeWidget( widget );
            item = d->layout->itemAtPosition( row, col );
            if( item )
            {
              d->layout->removeItem( item );
              delete item;
            }
            moved.push_back( make_pair( widget, make_pair( irow, icol ) ) );
//             d->layout->addWidget( widget, irow, icol );
            ++irow;
            if( irow >= colsrows )
            {
              irow = 0;
              ++icol;
            }
          }
          else
          {
            d->layout->removeItem( item );
            delete item;
          }
        }
      }
    }
  }

  list<pair<QWidget *, pair<int, int> > >::iterator im, em = moved.end();
  for( im=moved.begin(); im!=em; ++im )
    d->layout->addWidget( im->first, im->second.first, im->second.second );

  cleanupLayout();
}


void QAWindowBlock::arrangeInRect( float widthHeightRatio )
{
  int row, col, nr = d->layout->rowCount(), sz = 0;
  QLayoutItem *litem;

  for( row=0; row<nr; ++row )
    for( col=0; col<d->colsrows; ++col )
    {
      litem = d->layout->itemAtPosition( row, col );
      if( litem && litem->widget() )
        ++sz;
    }

  if( sz == 0 )
    return;
  int h = (int) rint( sqrt( sz / widthHeightRatio ) );
  if( h == 0 )
    h = 1;
  int w = (int) ceil( float(sz) / h );
  int h2 = h;
  if( h > 1 && h - sqrt( sz / widthHeightRatio ) >= 0 )
    h2 -= 1;
  else
    h2 += 1;
  int w2 = (int) ceil( float(sz) / h2 );
  // cout << "w: " << w << ", h: " << h << ", w2: " << w2 << ", h2 : " << h2 << endl;
  if( abs( w2 - w ) <= 1 &&  abs( w2 * h2 - sz ) < abs( w * h - sz ) )
  {
    h = h2;
    w = w2;
  }
  // cout << "w: " << w << ", h: " << h << endl;
  d->rectratio = widthHeightRatio;
  if( d->inrows )
    setColsOrRows( true, w );
  else
    setColsOrRows( false, h );
}


void QAWindowBlock::layInColumns()
{
  setColsOrRows( true, d->colsrows );
}


void QAWindowBlock::layInRows()
{
  setColsOrRows( false, d->colsrows );
}


void QAWindowBlock::layInRectangle()
{
  arrangeInRect( d->rectratio );
}


void QAWindowBlock::reorderViews( const list<QWidget *> & wins )
{
  // cout << "reorderViews: " << wins.size() << endl;
  list<QWidget *>::const_iterator iw, ew = wins.end();
  list<QWidget *> reordered;
  list<Observable *> new_wins;
  list<Observable *>::const_iterator iaw, eaw = new_wins.end();

  for( iw=wins.begin(); iw!=ew; ++iw )
  {
    QWidget *wid = *iw;
    DraggableWrapper *dw = dynamic_cast<DraggableWrapper *>( wid );
    int index = d->layout->indexOf( wid );
    while( index < 0 && wid->parentWidget() && !dw )
    {
      wid = wid->parentWidget();
      dw = dynamic_cast<DraggableWrapper *>( wid );
      index = d->layout->indexOf( wid );
    }
    if( index >= 0 )
    {
      reordered.push_back( wid );
      d->layout->removeWidget( wid );
      d->layout->removeItem( d->layout->itemAt( index ) );
    }
    else
    {
      reordered.push_back( *iw );
      Observable *aw = dynamic_cast<Observable *>( *iw );
      if( aw )
        new_wins.push_back( aw );
    }
  }
  int row, col, nr = d->layout->rowCount(), nc = d->layout->columnCount();
  int default_stretch = 300;
  bool withborders = true;

  vector<int> rc( 2, 0 ), nrc( 2 );
  nrc[0] = nr;
  nrc[1] = nc;
  int rcindex0 = d->inrows ? 1 : 0, rcindex1 = 1 - rcindex0;

  for( rc[rcindex1]=0; rc[rcindex1]<nrc[rcindex1]; ++rc[rcindex1] )
    for( rc[rcindex0]=0; rc[rcindex0]<nrc[rcindex0]; ++rc[rcindex0] )
    {
      QLayoutItem *item = d->layout->itemAtPosition( rc[0], rc[1] );
      if( item )
      {
        if( item->widget() )
        {
          reordered.push_back( item->widget() );
          d->layout->removeWidget( item->widget() );
        }
        d->layout->removeItem( item );
      }
    }
  rc[rcindex0] = 0;
  rc[rcindex1] = 0;
  for( iw=reordered.begin(), ew=reordered.end(); iw!=ew; ++iw )
  {
    QWidget *wid = *iw;
    if( !dynamic_cast<DraggableWrapper *>( wid )
        && wid->parentWidget()
        && dynamic_cast<DraggableWrapper *>( wid->parentWidget() ) )
      wid = wid->parentWidget();

    if( withborders && !dynamic_cast<DraggableWrapper *>( wid ) )
    {
      DraggableWrapper *dwrap = new DraggableWrapper( wid, d->layout );
      (*iw)->setParent( dwrap );
      d->layout->addWidget( dwrap, rc[0], rc[1] );
    }
    else
      d->layout->addWidget( wid, rc[0], rc[1] );
    ++rc[rcindex0];
    if( rc[rcindex0] >= d->colsrows )
    {
      rc[rcindex0] = 0;
      ++rc[rcindex1];
    }
  }

  if( withborders )
  {
    nr = d->layout->rowCount();
    nc = d->layout->columnCount();
    for( row=0; row<nr; ++row )
      if( d->layout->rowStretch( row ) == 0 )
        d->layout->setRowStretch( row, default_stretch );
    for( col=0; col<nc; ++col )
      if( d->layout->columnStretch( col ) == 0 )
        d->layout->setColumnStretch( col, default_stretch );
  }

  for( iaw=new_wins.begin(); iaw!=eaw; ++iaw )
    (*iaw)->addObserver( this );
}


void QAWindowBlock::setColumnsNumber()
{
  QDialog dial( this );
  dial.setModal( true );
  dial.setWindowTitle( tr( "Set number of columns" ) );
  QVBoxLayout *lay = new QVBoxLayout( &dial );
  QSpinBox *sb = new QSpinBox( &dial );
  lay->addWidget( sb );
  QHBoxLayout *lay2 = new QHBoxLayout( &dial );
  lay->addLayout( lay2 );
  QPushButton *ok = new QPushButton( tr( "OK" ), &dial );
  lay2->addWidget( ok );
  ok->setDefault( true );
  connect( ok, SIGNAL( pressed() ), &dial, SLOT( accept() ) );
  QPushButton *cancel = new QPushButton( tr( "Cancel" ), &dial );
  lay2->addWidget( cancel );
  connect( cancel, SIGNAL( pressed() ), &dial, SLOT( reject() ) );
  sb->setValue( d->colsrows );
  int res = dial.exec();
  if( res && sb->value() > 0 )
  {
    setColsOrRows( true, sb->value() );
  }
}


void QAWindowBlock::setRowsNumber()
{
  QDialog dial( this );
  dial.setModal( true );
  dial.setWindowTitle( tr( "Set number of rows" ) );
  QVBoxLayout *lay = new QVBoxLayout( &dial );
  QSpinBox *sb = new QSpinBox( &dial );
  lay->addWidget( sb );
  QHBoxLayout *lay2 = new QHBoxLayout( &dial );
  lay->addLayout( lay2 );
  QPushButton *ok = new QPushButton( tr( "OK" ), &dial );
  lay2->addWidget( ok );
  ok->setDefault( true );
  connect( ok, SIGNAL( pressed() ), &dial, SLOT( accept() ) );
  QPushButton *cancel = new QPushButton( tr( "Cancel" ), &dial );
  lay2->addWidget( cancel );
  connect( cancel, SIGNAL( pressed() ), &dial, SLOT( reject() ) );
  sb->setValue( d->colsrows );
  int res = dial.exec();
  if( res && sb->value() > 0 )
  {
    setColsOrRows( false, sb->value() );
  }
}


void QAWindowBlock::setRectangularRatio()
{
  QDialog dial( this );
  dial.setModal( true );
  dial.setWindowTitle( tr( "Set columns / rows ratio" ) );
  QVBoxLayout *lay = new QVBoxLayout( &dial );
  QLineEdit *sb = new QLineEdit( &dial );
  lay->addWidget( sb );
  QHBoxLayout *lay2 = new QHBoxLayout( &dial );
  lay->addLayout( lay2 );
  QPushButton *ok = new QPushButton( tr( "OK" ), &dial );
  lay2->addWidget( ok );
  ok->setDefault( true );
  connect( ok, SIGNAL( pressed() ), &dial, SLOT( accept() ) );
  QPushButton *cancel = new QPushButton( tr( "Cancel" ), &dial );
  lay2->addWidget( cancel );
  connect( cancel, SIGNAL( pressed() ), &dial, SLOT( reject() ) );
  sb->setText( QString::number( d->rectratio ) );
  int res = dial.exec();
  if( res )
  {
    float val = sb->text().toFloat();
    if( val > 0 )
      arrangeInRect( val );
  }
}


void QAWindowBlock::update( const Observable* obs, void* )
{
  if( obs && obs->obsHasChanged( "detachFromParent" ) )
  {
    const QAWindow *caw = dynamic_cast<const QAWindow *>( obs );
    if( caw )
    {
      if( caw->parentWidget() )
      {
        QAWindow *aw = const_cast<QAWindow *>( caw );
        QWidget *pw = aw->parentWidget();
        if( pw )
        {
          int index = d->layout->indexOf( pw );
          if( index >= 0 )
          {
            aw->setParent( 0 );
            delete pw;
            cleanupLayout();
          }
        }
      }
    }

    const_cast<Observable *>( obs )->deleteObserver( this );
  }
}


void QAWindowBlock::unregisterObservable( Observable* obs )
{
  QAWindow *aw = dynamic_cast<QAWindow *>( obs );
  if( aw )
  {
    QWidget *pw = aw->parentWidget();
    if( pw )
    {
      int index = d->layout->indexOf( pw );
      if( index >= 0 )
      {
        aw->setParent( 0 );
        delete pw;
        cleanupLayout();
      }
    }
  }
  Observer::unregisterObservable( obs );
}


void QAWindowBlock::cleanupLayout()
{
  int i, nr = d->layout->rowCount();
  int j, nc = d->layout->columnCount();
  set<int> usedrows, usedcols;
  for( i=0; i<nr; ++i )
    for( j=0; j<nc; ++j )
    {
      QLayoutItem *item = d->layout->itemAtPosition( i, j );
      if( item && item->widget() )
      {
        usedrows.insert( i );
        usedcols.insert( j );
      }
    }
  for( i=0; i<nr; ++i )
    if( usedrows.find( i ) == usedrows.end() )
      d->layout->setRowStretch( i, 0 );
  for( j=0; j<nc; ++j )
    if( usedcols.find( j ) == usedcols.end() )
      d->layout->setColumnStretch( j, 0 );
}


// ----

BlockBorderWidget::BlockBorderWidget( int sides, QGridLayout *gridLayout )
  : QWidget(), _sides( sides ), _gridLayout( gridLayout ), _pressed( false ),
  _last_x( 0 ), _last_y( 0 )
{
  QPalette pal = palette();
  pal.setColor( QPalette::ColorRole::Window, QColor( "white" ) );
  setAutoFillBackground( true );
  setPalette( pal );
  _pressed = false;
  Qt::CursorShape cshape = Qt::SplitVCursor;
  if( ( sides & 3 ) && ! ( sides & 12 ) )
    cshape = Qt::SplitHCursor;
  else if( sides == 5 || sides == 10 )
    cshape = Qt::SizeFDiagCursor;
  else if( sides == 6 || sides == 9 )
    cshape = Qt::SizeBDiagCursor;
  setCursor( cshape );
  setMouseTracking( true );
}


BlockBorderWidget::~BlockBorderWidget()
{
}


void BlockBorderWidget::mousePressEvent( QMouseEvent *event )
{
  QWidget::mousePressEvent( event );
  _pressed = true;
#if QT_VERSION >= 0x060000
  _last_x = int( event->globalPosition().x() );
  _last_y = int( event->globalPosition().y() );
#else
   _last_x = event->globalPos().x();
   _last_y = event->globalPos().y();
#endif
}


void BlockBorderWidget::mouseReleaseEvent( QMouseEvent *event )
{
  _pressed = false;
  QWidget::mouseReleaseEvent( event );
}


void BlockBorderWidget::mouseMoveEvent( QMouseEvent *event )
{
  if( _pressed && _gridLayout )
  {
#if QT_VERSION >= 0x060000
    int dx = int( event->globalPosition().x() ) - _last_x;
    _last_x = int( event->globalPosition().x() );
    int dy = int( event->globalPosition().y() ) - _last_y;
    _last_y = int( event->globalPosition().y() );
#else
    int dx = event->globalPos().x() - _last_x;
    _last_x = event->globalPos().x();
    int dy = event->globalPos().y() - _last_y;
    _last_y = event->globalPos().y();
#endif

    int row, col, dirx, diry;
    getRowCol( row, col, dirx, diry );
    dx *= dirx;
    dy *= diry;

    if( dx != 0 && col >= 0 )
    {
      if( col < _gridLayout->columnCount() )
      {
        int new_stretch = std::max(
          1, _gridLayout->columnStretch( col ) - dx );
        _gridLayout->setColumnStretch( col, new_stretch );
      }
    }

    if( dy != 0 && row >= 0 )
    {
      if( row < _gridLayout->rowCount() )
      {
        int new_stretch = std::max(
          1, _gridLayout->rowStretch( row ) - dy );
        _gridLayout->setRowStretch( row, new_stretch );
      }
    }
  }
  else
    QWidget::mouseMoveEvent( event );
}


void BlockBorderWidget::mouseDoubleClickEvent( QMouseEvent *event )
{
  int default_stretch = 300;
  int row, col, dirx, diry;
  getRowCol( row, col, dirx, diry );

  if( _sides & 3 )
    _gridLayout->setColumnStretch( col, default_stretch );
  if( _sides & 12 )
    _gridLayout->setRowStretch( row, default_stretch );
}


void BlockBorderWidget::getRowCol( int & row, int & col,
                                   int & dirx, int & diry ) const
{
  int index;
  int rspan, cspan;

  index = _gridLayout->indexOf( parentWidget() );
  dirx = 1;
  diry = 1;
  if( index < 0 )
  {
    row = -1;
    col = -1;
    return;
  }
  _gridLayout->getItemPosition( index, &row, &col, &rspan, &cspan );

  if( _sides & 2 )
      dirx *= -1;
  else if( ! ( _sides & 1 ) )
      dirx = 0;

  if( _sides & 8 )
      diry *= -1;
  else if( ! ( _sides & 4 ) )
      diry = 0;

  // left or top border, if not the first, will be about the previous row/col
  if( row > 0 && ( _sides & 4 ) )
  {
    --row;
    diry *= -1;
  }
  if( col > 0 && ( _sides & 1 ) )
  {
    --col;
    dirx *= -1;
  }
}


// ---

class DragWinLabel : public QLabel
{
public:
  DragWinLabel( QWidget* parent );
  virtual ~DragWinLabel();

protected:
  virtual void mouseMoveEvent( QMouseEvent* event );
};


DragWinLabel::DragWinLabel( QWidget* parent )
  : QLabel( parent )
{
  setAutoFillBackground( true );

  const QPixmap *pix = IconDictionary::instance()->getIconInstance( "drag" );
  if( !pix )
  {
    QPixmap mpix( Settings::findResourceFile( "icons/drag.png" ).c_str() );
    if( !mpix.isNull() )
    {
      IconDictionary::instance()->addIcon( "drag", mpix );
      pix = IconDictionary::instance()->getIconInstance( "drag" );
    }
  }

  if( pix )
    setPixmap( *pix );
  setFrameStyle( QFrame::Panel | QFrame::Raised );
//     _dragWidget->setStyleSheet( ".QFrame {backgrund-color: gray; border: 1px solid gray; border-radius: 10px; }" );
  setFixedSize( 32, 32 );

}


DragWinLabel::~DragWinLabel()
{
}


void DragWinLabel::mouseMoveEvent( QMouseEvent *event )
{
  if( event->buttons() == Qt::LeftButton )
  {
    QWidget *bparent = dynamic_cast<DraggableWrapper *>( parentWidget() );
    if( !bparent )
      return;

    QGridLayout *lay = dynamic_cast<QGridLayout *>( bparent->layout() );
    if( !lay )
      return;

    QLayoutItem *item = lay->itemAtPosition( 1, 1 );
    if( !item )
      return;

    QAWindow *aw = dynamic_cast<QAWindow *>( item->widget() );
    if( !aw )
      return;

    set<AWindow *>	so;
    so.insert( aw );

    QAWindowDrag *d = new QAWindowDrag( so );
    QDrag *drag = new QDrag( this );
    drag->setMimeData( d );

    int type = aw->subtype();
    if( type == 0 )
      type = aw->type();
    const QAWindowFactory::PixList	* pixl
      = &QAWindowFactory::pixmaps( type );

    if( pixl->psmall.isNull() )
      pixl = &QAWindowFactory::pixmaps( AWindow::WINDOW_3D );

    if( !pixl->psmall.isNull() )
      drag->setPixmap( pixl->psmall );

    drag->exec( Qt::MoveAction );
  }
}


//

DraggableWrapper::DraggableWrapper( QWidget *widget, QGridLayout *main_layout,
                                    bool withDragGrip )
  : QWidget(), _enableDragGrip( withDragGrip ), _dragWidget( 0 )
{
  QGridLayout *layout = new QGridLayout;
  setLayout( layout );
  layout->setSpacing( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  BlockBorderWidget *border_top = new BlockBorderWidget( 4, main_layout );
  BlockBorderWidget *border_bottom = new BlockBorderWidget( 8, main_layout );
  BlockBorderWidget *border_left = new BlockBorderWidget( 1, main_layout );
  BlockBorderWidget *border_right = new BlockBorderWidget( 2, main_layout );

  BlockBorderWidget *border_top_left_horizontal
    = new BlockBorderWidget( 5, main_layout );
  BlockBorderWidget *border_top_right_horizontal
    = new BlockBorderWidget( 6, main_layout );
  BlockBorderWidget *border_bottom_left_horizontal
    = new BlockBorderWidget( 9, main_layout );
  BlockBorderWidget *border_bottom_right_horizontal
    = new BlockBorderWidget( 10, main_layout );

  layout->addWidget( widget, 1, 1 );
  layout->addWidget( border_top, 0, 1 );
  layout->addWidget( border_bottom, 2, 1 );
  layout->addWidget( border_left, 1, 0 );
  layout->addWidget( border_right, 1, 2 );
  layout->addWidget( border_top_left_horizontal, 0, 0 );
  layout->addWidget( border_top_right_horizontal, 0, 2 );
  layout->addWidget( border_bottom_left_horizontal, 2, 0 );
  layout->addWidget( border_bottom_right_horizontal, 2, 2 );

  int border_width = 5;
  border_top->setFixedHeight( border_width );
  border_bottom->setFixedHeight( border_width );
  border_left->setFixedWidth( border_width );
  border_right->setFixedWidth( border_width );

  // apply size restrictions to corner border widgets to form L-shaped corners
  border_top_left_horizontal->setFixedSize( border_width, border_width );
  border_top_right_horizontal->setFixedSize( border_width, border_width );
  border_bottom_left_horizontal->setFixedSize( border_width, border_width );
  border_bottom_right_horizontal->setFixedSize( border_width, border_width );

  // Apply size policies
  setSizePolicy( QSizePolicy::Policy::Expanding,
                 QSizePolicy::Policy::Expanding );
  widget->setSizePolicy( QSizePolicy::Policy::Expanding,
                         QSizePolicy::Policy::Expanding );

  if( withDragGrip )
  {
    _dragWidget = new DragWinLabel( this );
    _dragWidget->move( 0, 0 );
    _dragWidget->hide();
    _timer = new QTimer( this );
    _timer->setSingleShot( true );
    connect( _timer, SIGNAL( timeout() ), _dragWidget, SLOT( hide() ) );
    setMouseTracking( true );
  }
}


DraggableWrapper::~DraggableWrapper()
{
}


void DraggableWrapper::mouseMoveEvent( QMouseEvent *event )
{
  // never gets called. Why ??
  if( _dragWidget && event->pos().x() < 100 && event->pos().y() < 100 )
  {
    _dragWidget->show();
    _timer->start( 2000 );
  }
}

