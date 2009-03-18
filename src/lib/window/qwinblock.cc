/* Copyright (c) 1995-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 * This software is governed by the CeCILL license version 2 under 
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the 
 * terms of the CeCILL license version 2 as circulated by CEA, CNRS
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
 * knowledge of the CeCILL license version 2 and that you accept its terms.
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
  int indexC;
  int indexR;
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
  d->indexC=0;
  d->indexR=0;
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
  // if we don't remove the item's parent, the window will not be in the block but next to it.
  item->setParent(0);
  if (d->indexC >= d->cols){ // next row
    d->indexC =0;
    d->indexR++;
  }
  d->layout->addWidget(item, d->indexR, d->indexC);
  d->indexC++;
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
              qw->reparent( this, QPoint( 0, 0 ), true );
              qw->enableDetachMenu( true );
            }
        }
    }
}


