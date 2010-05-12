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
#if QT_VERSION >= 0x040000
  int cols;
#endif

};


QAWindowBlock::QAWindowBlock( QWidget *parent, const char* name, Qt::WFlags f,
  int cols)
  : QWidget( parent, name, f ), d( new QAWindowBlock::Private )
{
  if( parent == 0 )
    {
      QPixmap	anaicon( ( Settings::globalPath() 
			   + "/icons/icon.xpm" ).c_str() );
      if( !anaicon.isNull() )
        setIcon( anaicon );
    }

  setAcceptDrops(TRUE);
  
  

#if QT_VERSION >= 0x040000
  d->layout = new QGridLayout;
  d->layout->setSpacing(0); 
  d->layout->setMargin(5);
  setLayout(d->layout);
  d->cols =cols;
#else
  d->layout = new QGridLayout( this, 1, cols, 0, 5 );
  d->layout->setAutoAdd( true );
#endif
}


QAWindowBlock::~QAWindowBlock()
{
  delete d;
}

void QAWindowBlock::addWindowToBlock(QWidget *item)
{
#if QT_VERSION >= 0x040000
  // if we don't remove the item's parent, the window will not be in the block
  // but next to it.
  item->setParent(0);
#if QT_VERSION >= 0x040400
  int row = 0, col = 0, nr = d->layout->rowCount();
  for( row=0; row<nr; ++row )
  {
    for( col=0; col<d->cols; ++col )
    {
      if( !d->layout->itemAtPosition( row, col ) )
        break;
    }
    if( col < d->cols )
      break;
    col = 0;
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
#else
  // Qt3
  item->reparent( this, QPoint( 0, 0 ), true );
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


