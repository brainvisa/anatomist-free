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
#include <qlayout.h>
#include <qpixmap.h>

using namespace anatomist;
using namespace std;

struct QAWindowBlock::Private
{
  QGridLayout	*layout;
  bool inrows;
  int colsrows;
};


QAWindowBlock::QAWindowBlock( QWidget *parent, const char* name, Qt::WFlags f,
  int colsrows, bool inrows )
  : QWidget( parent, name, f ), d( new QAWindowBlock::Private )
{
  if( parent == 0 )
    {
      QPixmap	anaicon( Settings::findResourceFile(
                         "icons/icon.xpm" ).c_str() );
      if( !anaicon.isNull() )
        setIcon( anaicon );
    }

  setAcceptDrops( true );

  d->layout = new QGridLayout;
  d->layout->setSpacing(0); 
  d->layout->setMargin(5);
  setLayout(d->layout);
  d->inrows = inrows;
  d->colsrows = colsrows;
  if( d->colsrows == 0 )
    d->colsrows = 2;
}


QAWindowBlock::~QAWindowBlock()
{
  delete d;
}

void QAWindowBlock::addWindowToBlock(QWidget *item)
{
  // if we don't remove the item's parent, the window will not be in the block
  // but next to it.
  item->setParent(0);
#if QT_VERSION >= 0x040400
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
  d->layout->addWidget( item, row, col );
#else // Qt version >= 4.0 and <= 4.3
  vector< vector<bool> > used;
  int i, j, k, n = d->layout->count();
  int row, col, rspan, cspan;

  for( i=0; i<n; ++i )
  {
    d->layout->getItemPosition( i, &row, &col, &rspan, &cspan );
    while( row+rspan >= used.size() )
      used.push_back( vector<bool>( false, d->cols ) );
    for( j=row; j<row+rspan; ++j )
    {
      vector<bool> & urow = used[j];
      while( urow.size() < col + cspan )
        urow.push_back( false );
      for( k=col; k<col+cspan; ++k )
        urow[k] = true;
    }
  }
  int nr = used.size(), nc;
  col = 0;
  row = 0;
  for( row=0; row<nr; ++row )
  {
    vector<bool> & urow = used[row];
    nc = urow.size();
    for( col=0; col<nc; ++col )
    {
      if( !urow[col] )
        break;
    }
    if( col < d->cols )
      break;
    col = 0;
  }
  d->layout->addWidget( item, row, col );
#endif
}

void QAWindowBlock::dragEnterEvent( QDragEnterEvent* event )
{
  //cout << "QAWindow::dragEnterEvent\n";
  event->accept( QAWindowDrag::canDecode( event ) );
}


void QAWindowBlock::dropEvent( QDropEvent* event )
{
  //cout << "QAWindow::dropEvent\n";
  set<AWindow *>	w;

  if( QAWindowDrag::decode( event, w ) )
    {
      //cout << "window decoded, " << w.size() << " windows\n";
      set<AWindow *>::iterator	iw, ew = w.end();
      QAWindow			*qw;
      for( iw=w.begin(); iw!=ew; ++iw )
        {
          qw = dynamic_cast<QAWindow *>( *iw );
          if( qw )
            {
              addWindowToBlock( qw );
              qw->enableDetachMenu( true );
            }
        }
    }
}


void QAWindowBlock::setColsOrRows( bool inrows, int colsrows )
{
  if( d->inrows == inrows && d->colsrows == colsrows )
    return; // nothing to do

#if QT_VERSION >= 0x040400
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

#else // Qt version >= 4.0 and <= 4.3
  // nothing....
#warning QAWindowBlock::setColsOrRows not implemented for Qt < 4.4
#endif
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
  int h = (int) floor( sqrt( sz / widthHeightRatio ) );
  if( h == 0 )
    h = 1;
  int w = (int) ceil( float(sz) / h );
  int h2 = h + 1;
  int w2 = (int) ceil( float(sz) / h2 );
//   cout << "w: " << w << ", h: " << h << ", w2: " << w2 << ", h2 : " << h2 << endl;
  if( w * h > w2 * h2 )
  {
    h = h2;
    w = w2;
  }
//   cout << "w: " << w << ", h: " << h << endl;
  if( d->inrows )
    setColsOrRows( true, w );
  else
    setColsOrRows( false, h );
}


