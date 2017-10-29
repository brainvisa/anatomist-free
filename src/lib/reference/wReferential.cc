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

#include <anatomist/reference/wReferential.h>
#include <anatomist/reference/wChooseReferential.h>
#include <anatomist/application/fileDialog.h>
#include <anatomist/control/wControl.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/reference/Referential.h>
#include <anatomist/reference/Transformation.h>
#include <anatomist/reference/transfSet.h>
#include <anatomist/reference/refpixmap.h>
#include <anatomist/processor/Processor.h>
#include <anatomist/commands/cLoadTransformation.h>
#include <anatomist/commands/cSaveTransformation.h>
#include <anatomist/commands/cAssignReferential.h>
#include <anatomist/commands/cCreateWindow.h>
#include <anatomist/commands/cAddObject.h>
#include <anatomist/reference/wReferential_3d.h>
#include <anatomist/misc/error.h>
#include <anatomist/mobject/MObject.h>
#include <aims/def/general.h>
#include <QMouseEvent>
#include <cartobase/object/pythonwriter.h>
#include <cartobase/stream/fileutil.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <map>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <qmenu.h>
#include <qaction.h>
#include <qicon.h>
#include <QTimer>


using namespace anatomist;
using namespace aims;
using namespace carto;
using namespace std;


namespace
{

  class RefToolTip
  {
  public:
    RefToolTip( ReferentialWindow* parent );
    virtual ~RefToolTip();

    virtual void maybeTip( const QPoint & p );

  private:
    ReferentialWindow *_refwin;
  };

  RefToolTip::RefToolTip( ReferentialWindow* parent )
  : _refwin( parent )
  {
  }

  RefToolTip::~RefToolTip()
  {
  }


  QString headerPrint( PythonHeader & ph,
                       const set<string> & exclude = set<string>() )
  {
    QString text;
    Object  i;;
    string  key, val;
    int     l = 12, x;
    set<string>::const_iterator printable = exclude.end();
    for( i=ph.objectIterator(); i->isValid(); i->next() )
    {
      key = i->key();
      if( exclude.find( key ) == printable )
        try
        {
          val = i->currentValue()->getString();
          text += "<br/><b>";
          text += key.c_str();
          text += ":</b> ";
          for( x=key.length()+2; x<l; ++x )
            text += " ";
          text += val.c_str();
        }
        catch( ... )
        {
          try
          {
            PythonWriter  pw;
            ostringstream ss;
            pw.attach( ss );
            pw.setSingleLineMode( true );
            pw.write( i->currentValue(), false, false );
            text += "<br/><b>";
            text += key.c_str();
            text += ":</b> ";
            for( x=key.length()+2; x<l; ++x )
              text += " ";
            text += ss.str().c_str();
          }
          catch( ... )
          {
          }
        }
    }
    return text;
  }


  void RefToolTip::maybeTip( const QPoint & p )
  {
    QPoint  pos;
    Referential *r = _refwin->refAt( p, pos );
    if( r && theAnatomist->hasReferential( r ) )
    {
      list<string> temp;
      QString text = ReferentialWindow::referentialToolTipText( r, temp );
      QToolTip::showText( _refwin->mapToGlobal( p ), text );
      ReferentialWindow::unlinkFiles( temp );
    }
    else
    {
      anatomist::Transformation  *t = _refwin->transfAt( p );
      if( t && ATransformSet::instance()->hasTransformation( t ) )
      {
        list<string> temp;
        QString text = ReferentialWindow::transformationToolTipText( t, temp );
        QToolTip::showText( _refwin->mapToGlobal( p ), text );
        ReferentialWindow::unlinkFiles( temp );
      }
    }
  }


  set<AObject *> objectsInReferential( Referential *ref )
  {
    const set<AObject *> & objs = theAnatomist->getObjects();
    set<AObject *>::const_iterator io, eo = objs.end();
    set<AObject *> selobj;
    ControlWindow *ctrl = theAnatomist->getControlWindow();
    for( io=objs.begin(); io!=eo; ++io )
      if( (*io)->getReferential() == ref
        && theAnatomist->hasObject( *io )
        && (!ctrl || ctrl->hasObject( *io ) ) ) // skip hidden objects
      {
        const AObject::ParentList & parents = (*io)->parents();
        AObject::ParentList::const_iterator ip, ep = parents.end();
        for( ip=parents.begin(); ip!=ep; ++ip )
          if( (*ip)->getReferential() == ref )
            // parent with same referential: drop the child, get just the parent
            break;
        if( ip == ep )
          selobj.insert( *io );
      }

    return selobj;
  }

}

// --------

namespace anatomist
{

  struct ReferentialWindow_PrivateData
  {
    ReferentialWindow_PrivateData() 
      : srcref( 0 ), dstref( 0 ), trans( 0 ), tracking( false ), refmenu( 0 ),
      bgmenu( 0 ), view2d( 0 ), view3d( 0 ), has_changed( true ),
      refreshtimer( 0 ), refreshneeded( false )
    {}

    Referential			*srcref;
    Referential			*dstref;
    Transformation		*trans;
    QPoint			pos;
    bool			tracking;
    map<Referential *, QPoint>	refpos;
    QMenu			*refmenu;
    QMenu                       *bgmenu;
    RefToolTip                  *tooltip;
    QLabel                      *view2d;
    /* FIXME: we would need a weak_shared_ptr<RefWindow> here,
       or rc_ptr<RefWindow>, but rc_ptr<RefWindow> does not compile.
       Neither does rc_ptr<AWindow3D>.
       Actually AWindow3D inherits SharedObject and RCObject via 2 bases
       (diamond), via AWindow and Observable, which apparently introduces
       an ambiguity in destroy().
    */
    RefWindow                   *view3d;
    weak_shared_ptr<AWindow>    view3d_ref;
    bool                        has_changed;
    QTimer                      *refreshtimer;
    bool                        refreshneeded;
  };

}


ReferentialWindow::ReferentialWindow( QWidget* parent, const char* name, 
                                      Qt::WindowFlags f )
  : QWidget( parent, f ),
  pdat( new ReferentialWindow_PrivateData )
{
  setWindowTitle( tr( "Referentials" ) );
  setObjectName(name);
  QHBoxLayout *lay = new QHBoxLayout;
  lay->setContentsMargins( 3, 3, 3, 3 );
  setLayout( lay );
  pdat->view2d = new QLabel( this );
  pdat->view2d->setSizePolicy( QSizePolicy::Ignored,
                               QSizePolicy::Ignored );
  lay->addWidget( pdat->view2d );
  resize( 500, 400 );
  pdat->view2d->setPixmap( QPixmap( width(), height() ) );
  pdat->tooltip = new RefToolTip( this );
#if QT_VERSION >= 0x050000
#warning Qt::WA_PaintOutsidePaintEvent not set.
#else
  setAttribute( Qt::WA_PaintOutsidePaintEvent );
#endif
  // switch directly to the 3D view
  set3DView();
}

ReferentialWindow::~ReferentialWindow()
{
  if (theAnatomist->getControlWindow() != 0)
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
  delete pdat;
}


void ReferentialWindow::closeEvent( QCloseEvent * ev )
{
  QWidget::closeEvent( ev );
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
}


void ReferentialWindow::resizeEvent( QResizeEvent* ev )
{
  QWidget::resizeEvent( ev );
  refresh( true );
}


void ReferentialWindow::openSelectBox()
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( tr( "Open transformation" ) );
  fd.setFileMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QStringList selected = fd.selectedFiles();
  if ( !selected.isEmpty() )
  {
    QString filename = selected[0];
    loadTransformation( filename.toStdString() );
  }
}


void ReferentialWindow::saveTransformation( anatomist::Transformation* trans )
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);;";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog	& fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( tr( "Save transformation" ) );
  fd.setFileMode( QFileDialog::AnyFile );
  if( !fd.exec() )
    return;
  QStringList selected = fd.selectedFiles();
  if ( !selected.isEmpty() )
  {
    QString filename = selected[0];
    pdat->trans = trans;
    saveTransformation( filename.toStdString() );
  }
}


void ReferentialWindow::loadTransformation( const string & filename )
{
  LoadTransformationCommand	*com 
    = new LoadTransformationCommand( filename, pdat->srcref, pdat->dstref );
  theProcessor->execute( com );
  refresh();
}


void ReferentialWindow::saveTransformation( const string & filename )
{
  //cout << "saveTransformation " << filename << endl;
  /*cout << "src: " << pdat->srcref << ", dst: " << pdat->dstref << endl;
    cout << "trans: " << pdat->trans << endl;*/

  anatomist::Transformation 
    *t = pdat->trans;

  if( t )
    {
      SaveTransformationCommand	*com 
	= new SaveTransformationCommand( filename, t );
      theProcessor->execute( com );
    }
  else
    cerr << "No transformation to save\n";
}


void ReferentialWindow::refresh( bool partial )
{
  if( !partial )
    pdat->has_changed = true;

  if( !pdat->refreshtimer )
  {
    pdat->refreshtimer = new QTimer( this );
    pdat->refreshtimer->setObjectName( "ReferentialWindow_refreshtimer" );
    connect( pdat->refreshtimer, SIGNAL( timeout() ), this,
            SLOT( refreshNow() ) );
  }
  if( !pdat->refreshneeded )
  {
    pdat->refreshneeded = true;
    pdat->refreshtimer->setSingleShot( true );
    pdat->refreshtimer->start( 30 );
  }
}


void ReferentialWindow::refreshNow()
{
  pdat->refreshneeded = false;
  pdat->refpos.clear();
  if( pdat->view3d && pdat->has_changed )
    pdat->view3d->updateReferentialView();

  pdat->has_changed = false;

  if( !pdat->view2d->isVisible() || pdat->view2d->width() == 0
      || pdat->view2d->height() == 0 )
    return;

  set<Referential *>	refs = theAnatomist->getReferentials();
  set<anatomist::Transformation *>	trans 
    = ATransformSet::instance()->allTransformations();
  unsigned		n = refs.size(), i;
  set<Referential *>::const_iterator	ir, fr=refs.end(), jr;
  set<anatomist::Transformation *>::const_iterator	it, ft=trans.end();
  AimsRGB		col;
  Referential		*ref;
  anatomist::Transformation	*tr;
  unsigned		x, y, sz = 20;
  int			w = pdat->view2d->width(),
                        h = pdat->view2d->height(),
                        R = w, Rmin = 50;
  QPixmap		pix( w, h );

  QPainter		p( &pix );

  p.setPen( QPen( Qt::black ) );
  p.fillRect( 0, 0, w, h, Qt::white );
  if( h < R )
    R = h;
  R = R/2 - 50;
  if( R < 20 )
    R = 20;

  for( ir=refs.begin(), i = 0; ir!=fr; )
  {
    if( (*ir)->index() == 0 )
    {
      --n;
      ++ir;
    }
    else if( (*ir)->hidden() )
    {
      jr = ir;
      ++ir;
      refs.erase( jr );
      --n;
    }
    else
      ++ir;
  }

  for( ir=refs.begin(), i = 0; ir!=fr; ++ir )
    {
      ref = *ir;
      if( ref->index() == 0 )
	{
	  x = (unsigned) ( w * 0.45 );
	  y = (unsigned) ( h * 0.45 );
	}
      else
	{
	  x = (unsigned) ( R * cos( ( (float) i / n )*2.*M_PI ) + w/2 );
	  y = (unsigned) ( R * sin( ( (float) i / n )*2.*M_PI ) + h/2 );
	  ++i;
	}
      col = ref->Color();
      //p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
      //p.drawEllipse( x-sz/2, y-sz/2, sz, sz );
      p.drawPixmap( x-sz/2, y-sz/2,
                    ReferencePixmap::referencePixmap( ref, false, sz ) );
      pdat->refpos[ref] = QPoint( x, y );
    }

  for( it=trans.begin(); it!=ft; ++it )
    {
      tr = *it;
      if( !tr->isGenerated() )
	{
	  if( pdat->refpos.find( tr->source() ) == pdat->refpos.end() 
	      || pdat->refpos.find( tr->destination() ) == pdat->refpos.end() )
          {
            /*
	    cerr << "Transformation from " << tr->source() << " to " 
                 << tr->destination() << " : ref not registered !\n";
            */
          }
	  else
          {
            QPoint p1 = pdat->refpos[ tr->source() ],
              p2 = pdat->refpos[ tr->destination() ],
              p3 = ( p1 - p2 );
            p3 *= 1000. / sqrt( float( p3.x() ) * p3.x() 
                                + float( p3.y() ) * p3.y() );
            p1 -= p3 * double( sz ) / 2000.;
            p2 += p3 * double( sz ) / 2000.;
	    p.drawLine( p1, p2 );
            if( R >= Rmin )
            {
              p.drawLine( p2, p2 + p3 * 0.015 
                  + QPoint( int( -p3.y() * 0.005), int( p3.x() * 0.005 ) ) );
              p.drawLine( p2, p2 + p3 * 0.015 
                  + QPoint( int( p3.y() * 0.005 ), int( -p3.x() * 0.005 ) ) );
            }
          }
	}
    }

  p.end();
  pdat->view2d->setPixmap( pix );
}


void ReferentialWindow::mousePressEvent( QMouseEvent* ev )
{
  pdat->trans = 0;

  switch( ev->button() )
    {
    case Qt::LeftButton:
      pdat->srcref = refAt( ev->pos(), pdat->pos );
      if( pdat->srcref )
	{
	  pdat->tracking = true;
	  pdat->dstref = 0;
	}
      break;
    case Qt::RightButton:
      {
	QPoint	dummy;
	pdat->srcref = refAt( ev->pos(), dummy );
	if( pdat->srcref )
	  {
	    /*cout << "ref : " << ref->Color().r << ", " << ref->Color().g 
	      << ", " << ref->Color().b << endl;*/
	    popupRefMenu( ev->pos() );
	    break;
	  }
	else
	  {
	    anatomist::Transformation	*tr = transfAt( ev->pos() );
	    if( tr )
	      {
		/*Referential	*src = tr->source(), *dst = tr->destination();
		cout << "trans : " << src->Color().r << ", " 
		     << src->Color().g << ", " << src->Color().b 
		     << " -> " << dst->Color().r << ", " << dst->Color().g 
		     << ", " << dst->Color().b << endl;*/
		pdat->trans = tr;
		popupTransfMenu( ev->pos() );
		break;
	      }
            else
              popupBackgroundMenu( ev->pos() );
	  }
      }
      break;
    default:
      break;
    }
}


void ReferentialWindow::mouseReleaseEvent( QMouseEvent* ev )
{
  if( pdat->tracking )
  {
    pdat->tracking = false;
    QPainter	p( this );
    p.drawPixmap( pdat->view2d->mapToParent( QPoint( 0, 0 ) ),
                  *pdat->view2d->pixmap() );

    QPoint		dummy;
    pdat->dstref = refAt( ev->pos(), dummy );

    if( pdat->dstref && pdat->srcref != pdat->dstref )
    {
      anatomist::Transformation *tr
        = ATransformSet::instance()->transformation( pdat->srcref,
                                                     pdat->dstref );
      bool id = false;
      bool merge = false;
      if( ev->modifiers() & Qt::ControlModifier )
        id = true;
      else if( ev->modifiers() & Qt::ShiftModifier )
        merge = true;
      if( !tr || merge )
        addTransformationGui( pdat->srcref, pdat->dstref, id, merge );
    }
  }
}


void ReferentialWindow::addTransformationGui( Referential* source,
                                              Referential* dest,
                                              bool identity, bool merge )
{
  if( identity )
  {
    float	matrix[4][3];
    matrix[0][0] = 0;
    matrix[0][1] = 0;
    matrix[0][2] = 0;
    matrix[1][0] = 1;
    matrix[1][1] = 0;
    matrix[1][2] = 0;
    matrix[2][0] = 0;
    matrix[2][1] = 1;
    matrix[2][2] = 0;
    matrix[3][0] = 0;
    matrix[3][1] = 0;
    matrix[3][2] = 1;

    LoadTransformationCommand	*com
      = new LoadTransformationCommand( matrix, source, dest );
    theProcessor->execute( com );
    refresh();
  }
  else if( merge )
  {
    if( Referential::mergeReferentials( source, dest ) )
      refresh();
  }
  else
  {
    pdat->srcref = source;
    pdat->dstref = dest;
    openSelectBox();
  }
}


void ReferentialWindow::mouseMoveEvent( QMouseEvent* ev )
{
  if( pdat->tracking )
  {
    QPainter	p( this );
    p.drawPixmap( pdat->view2d->mapToParent( QPoint( 0, 0 ) ),
                  *pdat->view2d->pixmap() );
    p.drawLine( pdat->view2d->mapToParent( pdat->pos ),
                pdat->view2d->mapFromParent( ev->pos() ) );
  }
}


Referential* ReferentialWindow::refAt( const QPoint & pos, QPoint & newpos )
{
  map<Referential*, QPoint>::const_iterator	ir, fr = pdat->refpos.end();
  Referential		*ref;
  int			sz = 10;
  QPoint		rpos;
  QPoint                refpos = pdat->view2d->mapFromParent( pos );

  for( ir=pdat->refpos.begin(); ir!=fr; ++ir )
    {
      ref = (*ir).first;
      rpos = (*ir).second;
      if( refpos.x() >= rpos.x()-sz && refpos.x() < rpos.x()+sz
	  && refpos.y() >= rpos.y()-sz && refpos.y() < rpos.y()+sz )
	{
	  newpos = rpos;
	  return( ref );
	}
    }
  return( 0 );
}


anatomist::Transformation* ReferentialWindow::transfAt( const QPoint & pos )
{
  vector<anatomist::Transformation *> trat = transformsAt( pos );
  if( trat.empty() )
    return 0;
  return trat[0];
}


vector<anatomist::Transformation*> ReferentialWindow::transformsAt(
    const QPoint & pos )
{
  set<anatomist::Transformation *>                 trans 
    = ATransformSet::instance()->allTransformations();
  set<anatomist::Transformation *>::const_iterator it, ft=trans.end();
  anatomist::Transformation                        *t;
  QPoint                                rvec, relp;
  float                                 x, y, normv, norm;
  vector<anatomist::Transformation *>              trat;
  QPoint refpos = pdat->view2d->mapFromParent( pos );

  // 1st pass on loaded transformations
  for( it=trans.begin(); it!=ft; ++it )
    {
      t = *it;
      if( !t->isGenerated() )
        {
          const QPoint & rpos1 = pdat->refpos[ t->source() ];
          rvec = pdat->refpos[ t->destination() ] - rpos1;
          relp = refpos - rpos1;
          // project
          x = relp.x() * rvec.x() + relp.y() * rvec.y();
          norm = rvec.x() * rvec.x() + rvec.y() * rvec.y();
          normv = sqrt( norm );
          //cout << "x : " << ((float) x) / norm << endl;
          if( x >= 0 && x <= norm )     // between 2 ref points
            {
              // distance from line
              y = relp.y() * rvec.x() - relp.x() * rvec.y();
              //cout << "y : " << ((float) y) / normv << endl;
              if( y >= -normv * 5 && y <= normv * 5 )
                trat.push_back( t );
            }
        }
    }

  // 2nd pass on generated transformations
  for( it=trans.begin(); it!=ft; ++it )
    {
      t = *it;
      if( t->isGenerated() )
        {
          const QPoint & rpos1 = pdat->refpos[ t->source() ];
          rvec = pdat->refpos[ t->destination() ] - rpos1;
          relp = refpos - rpos1;
          // project
          x = relp.x() * rvec.x() + relp.y() * rvec.y();
          norm = rvec.x() * rvec.x() + rvec.y() * rvec.y();
          normv = sqrt( norm );
          //cout << "x : " << ((float) x) / norm << endl;
          if( x >= 0 && x <= norm )     // between 2 ref points
            {
              // distance from line
              y = relp.y() * rvec.x() - relp.x() * rvec.y();
              //cout << "y : " << ((float) y) / normv << endl;
              if( y >= -normv * 5 && y <= normv * 5 )
                trat.push_back( t );
            }
        }
    }

  return trat;
}


void ReferentialWindow::popupRefMenu( const QPoint & pos,
                                      Referential* ref )
{
  pdat->srcref = ref;
  popupRefMenu( pos );
}


void ReferentialWindow::popupRefMenu( const QPoint & pos )
{
  QMenu	*pop = pdat->refmenu;
  QAction *delete_action, *load_action, *icon_action, *see_objects_action;

  if( !pop )
    {
      pop = new QMenu( this );
      pdat->refmenu = pop;

      pop->addAction( QIcon(QPixmap( 16, 16 )), "" );
      pop->addSeparator();
      see_objects_action = pop->addAction(
        tr( "See objects in this referential" ), this,
        SLOT( seeObjectsInReferential() ) );
      delete_action = pop->addAction( tr( "Delete referential" ), this,
               SLOT( deleteReferential() ) );
      load_action = pop->addAction( tr( "Load referential information" ), this,
                       SLOT( loadReferential() ));
      pop->addAction( tr( "Split referential to disconnect transformations" ),
        this, SLOT( splitReferential() ));
    }
  else{
      delete_action = pop->actions()[1];
      load_action = pop->actions()[2];
  }
  delete_action->setEnabled( pdat->srcref->index() != 0 );
  load_action->setEnabled( pdat->srcref->index() != 0 );

  QPixmap	pix( 16, 16 );
  QPainter	p( &pix );
  AimsRGB	col = pdat->srcref->Color();

  p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
  p.fillRect( 0, 0, 16, 16, pop->palette().color( QPalette::Window ) );
  p.drawEllipse( 0, 0, 16, 16 );
  p.end();
  pix.setMask( pix.createHeuristicMask() );
  icon_action = pop->actions()[0];
  icon_action->setIcon( QIcon(pix) );

  pop->popup( mapToGlobal( pos ) );
}


void ReferentialWindow::popupTransfMenu( const QPoint & pos )
{
  vector<anatomist::Transformation *>  trans = transformsAt( pos );
  if( trans.empty() )
    return;
  popupTransfMenu( pos, trans );
}


void ReferentialWindow::popupTransfMenu(
    const QPoint & pos,
    const vector<anatomist::Transformation *> & trans )
{
  unsigned        i, n = trans.size();
  anatomist::Transformation  *t;
  QMenu      pop( this );
  vector<ReferentialWindow_TransCallback *> cbks;
  ReferentialWindow_TransCallback           *cbk;

  cbks.reserve( n );

  for( i=0; i<n; ++i )
  {
    t = trans[i];

    QPixmap     pix( 64, 16 );

    QPainter    p( &pix );
    AimsRGB     col = t->source()->Color();
    //cout << "source: " << t->source() << ": " << col << endl;

    p.setBackgroundMode( Qt::OpaqueMode );
    p.fillRect( 0, 0, 64, 16, pop.palette().color( QPalette::Window ) );
    p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    p.drawEllipse( 0, 0, 16, 16 );
    col = t->destination()->Color();
    // cout << "dest: " << t->source() << ": " << col << endl;
    p.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    p.drawEllipse( 48, 0, 16, 16 );
    p.drawLine( 16, 8, 48, 8 );
    p.drawLine( 40, 4, 48, 8 );
    p.drawLine( 40, 12, 48, 8 );
    p.end();
    pix.setMask( pix.createHeuristicMask() );
    pop.addAction( QIcon(pix), "" );

    pop.addSeparator();
    cbk = new ReferentialWindow_TransCallback( this, t );
    cbks.push_back( cbk );
    if( !t->isGenerated() )
    {
      pop.addAction( tr( "Delete transformation" ), cbk,
                      SLOT( deleteTransformation() ) );
      pop.addAction( tr( "Invert transformation" ), cbk,
                      SLOT( invertTransformation() ) );
      pop.addAction( tr( "Reload transformation" ), cbk,
                      SLOT( reloadTransformation() ) );
      QAction *ac = pop.addAction( tr( "Merge referentials" ), cbk,
                                   SLOT( mergeReferentials() ) );
      if( !t->motion().isIdentity() )
        ac->setEnabled( false );
    }
    pop.addAction( tr( "Save transformation..." ), cbk,
                    SLOT( saveTransformation() ) );
    if( i < n-1 )
      pop.addSeparator();
  }

  pop.exec( mapToGlobal( pos ) );

  for( i=0; i<n; ++i )
    delete cbks[i];
}


void ReferentialWindow::popupBackgroundMenu( const QPoint & pos )
{
  QMenu    *pop = pdat->bgmenu;

  if( !pop )
  {
    pop = new QMenu( this );
    pdat->bgmenu = pop;

    pop->addAction( tr( "New referential" ), this,
                     SLOT( newReferential() ) );
    pop->addAction( tr( "Load referential" ), this,
                     SLOT( loadReferential() ) );
    pop->addAction( tr( "Load transformation" ), this,
                     SLOT( loadNewTransformation() ) );
    pop->addAction( tr( "Clear unused referentials" ), this,
                     SLOT( clearUnusedReferentials() ) );
    pop->addAction( tr( "Merge identical referentials" ), this,
                    SLOT( mergeIdenticalReferentials() ) );
    pop->addSeparator();
    pop->addAction( tr( "Switch to 3D view" ), this, SLOT( set3DView() ) );
  }

  pop->popup( mapToGlobal( pos ) );
}


void ReferentialWindow::deleteReferential()
{
  delete pdat->srcref;
  theAnatomist->Refresh();
}


void ReferentialWindow::deleteTransformation( anatomist::Transformation* trans )
{
  delete trans;

  set<AWindow *>		win = theAnatomist->getWindows();
  set<AWindow*>::iterator	iw, fw = win.end();

  for( iw=win.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void ReferentialWindow::clearUnusedReferentials()
{
  Referential::clearUnusedReferentials();
  refresh();
}


void ReferentialWindow::invertTransformation( anatomist::Transformation* trans )
{
  anatomist::Transformation	*other
    = ATransformSet::instance()->transformation( trans->destination(),
						 trans->source() );
  if( !other )
    {
      cerr << "BUG: inverse transformation not found\n";
      return;
    }
  *other = *trans;
  trans->invert();

  set<AWindow *>		win = theAnatomist->getWindows();
  set<AWindow*>::iterator	iw, fw = win.end();

  for( iw=win.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void ReferentialWindow::reloadTransformation( anatomist::Transformation* trans )
{
  pdat->srcref = trans->source();
  pdat->dstref = trans->destination();
  openSelectBox();
}


void ReferentialWindow::newReferential()
{
  set<AObject *>  o;
  set<AWindow *>  w;
  AssignReferentialCommand	*com
      = new AssignReferentialCommand( 0, o, w, -1 );
  pdat->has_changed = true;
  theProcessor->execute( com );
}


void ReferentialWindow::loadReferential()
{
  QString filter = tr( "Referential" );
  filter += " (*.referential);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog   & fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( tr( "Load referential information" ) );
  fd.setFileMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QStringList selected = fd.selectedFiles();
  if ( !selected.isEmpty() )
  {
    QString filename = selected[0];
    set<AObject *> o;
    set<AWindow *> w;
    AssignReferentialCommand  *com
        = new AssignReferentialCommand( pdat->srcref, o, w, -1, 0,
                                        filename.toStdString() );
    pdat->has_changed = true;
    theProcessor->execute( com );
  }
}


void ReferentialWindow::loadNewTransformation()
{
  QString filter = tr( "Transformation" );
  filter += " (*.trm *TO*);; ";
  filter += ControlWindow::tr( "All files" );
  filter += " (*)";
  QFileDialog   & fd = fileDialog();
  fd.setNameFilter( filter );
  fd.setWindowTitle( tr( "Open transformation" ) );
  fd.setFileMode( QFileDialog::ExistingFile );
  if( !fd.exec() )
    return;
  QStringList selected = fd.selectedFiles();
  if ( !selected.isEmpty() )
  {
    QString filename = selected[0];
    pdat->srcref = 0;
    pdat->dstref = 0;
    loadTransformation( filename.toStdString() );
    pdat->has_changed = true;
  }
}


void ReferentialWindow::splitReferential()
{
  set<anatomist::Transformation *> trs
    = ATransformSet::instance()->transformationsWith( pdat->srcref );
  set<anatomist::Transformation *>::iterator it, et = trs.end();
  anatomist::Transformation *tr;
  Referential *sr, *dr;
  set<AObject *>  o;
  set<AWindow *>  w;
  bool first = true;
  for( it=trs.begin(); it!=et; ++it )
  {
    tr = *it;
    if( !tr->isGenerated() )
    {
      if( first )
      {
        // let the first transfo unchanged
        first = false;
      }
      else
      {
        AssignReferentialCommand      *com
          = new AssignReferentialCommand( 0, o, w, -1 );
        theProcessor->execute( com );
        Referential *ref = com->ref();
        if( tr->source() == pdat->srcref )
        {
          sr = ref;
          dr = tr->destination();
        }
        else
        {
          sr = tr->source();
          dr = ref;
        }
        vector<float> matrix(12);
        matrix[0] = tr->motion().translation()[0];
        matrix[1] = tr->motion().translation()[1];
        matrix[2] = tr->motion().translation()[2];
        matrix[3] = tr->motion().rotation()(0,0);
        matrix[4] = tr->motion().rotation()(1,0);
        matrix[5] = tr->motion().rotation()(2,0);
        matrix[6] = tr->motion().rotation()(0,1);
        matrix[7] = tr->motion().rotation()(1,1);
        matrix[8] = tr->motion().rotation()(2,1);
        matrix[9] = tr->motion().rotation()(0,2);
        matrix[10] = tr->motion().rotation()(1,2);
        matrix[11] = tr->motion().rotation()(2,2);

        delete tr;
        /* cout << "create " << sr->uuid().toString() << " -> "
          << dr->uuid().toString() << endl; */
        LoadTransformationCommand *com2
          = new LoadTransformationCommand( matrix, sr, dr );
        theProcessor->execute( com2 );
      }
    }
  }
  pdat->has_changed = true;
  theAnatomist->Refresh();
  refresh();
}


void ReferentialWindow::seeObjectsInReferential()
{
  CreateWindowCommand *wc = new CreateWindowCommand( "Browser" );
  theProcessor->execute( wc );
  AWindow *win = wc->createdWindow();
  set<AObject *> selobj = objectsInReferential( pdat->srcref );
  if( !selobj.empty() )
  {
    set<AWindow *> winset;
    winset.insert( win );
    AddObjectCommand *add = new AddObjectCommand( selobj, winset, false,
                                                  false, false );
    theProcessor->execute( add );
  }
}


bool ReferentialWindow::event( QEvent* event )
{
  if( event->type() == QEvent::ToolTip && pdat->view2d->isVisible()
    && pdat->view2d->width() != 0 && pdat->view2d->height() != 0 )
  {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    pdat->tooltip->maybeTip( helpEvent->pos() );
  }
  return QWidget::event(event);
}


void ReferentialWindow::set3DView()
{
  if( pdat->view3d )
  {
    pdat->view3d->updateReferentialView();
    pdat->view3d->show();
    return;
  }

  RefWindow *rwin = new RefWindow;
  pdat->view3d_ref.reset( rwin );
  // make the window not appear in control win, and be a weak reference in
  // the app.
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->unregisterWindow( rwin );
  theAnatomist->releaseWindow( rwin );

  rwin->updateReferentialView();
  layout()->addWidget( rwin );
  pdat->view3d = rwin;
  connect( pdat->view3d, SIGNAL( destroyed() ),
           this, SLOT( view3dDeleted() ) );
  rwin->show();
}


void ReferentialWindow::view3dDeleted()
{
  pdat->view3d_ref.release();
  pdat->view3d = 0;
  refresh();
}


QString ReferentialWindow::referentialToolTipText(
  Referential *ref, list<string> & temp_filenames )
{
  string  name;
  PythonHeader  & ph = ref->header();
  if( !ph.getProperty( "name", name ) )
    name = "&lt;unnamed&gt;";

  QPixmap pix( 16, 16 );
  QPainter      ptr( &pix );
  AimsRGB       col = ref->Color();
  ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
  ptr.fillRect( 0, 0, 16, 16, QColor( 255, 255, 255 ) );
  ptr.drawEllipse( 0, 0, 16, 16 );
  ptr.end();
  pix.setMask( pix.createHeuristicMask() );
  int fd;
  string pixfname = FileUtil::temporaryFile( "anarefpixmap.png", fd );
  ::close( fd );
  pix.save( QString( pixfname.c_str() ), "PNG" );
  temp_filenames.push_back( pixfname );

  /* QTextDocument document;
  document.addResource( QTextDocument::ImageResource, QUrl("refimage.png"),
                        pix ); */

  QString text( "<h4>Referential:  <img src=\"" );
  text += pixfname.c_str();
  text += "\"></img></h4><em><b>  ";
  text += name.c_str();
  text += "</b></em><br/><b>UUID</b>        :  ";
  text += ref->uuid().toString().c_str();
  text += "<br/>";
  set<string> exclude;
  exclude.insert( "name" );
  exclude.insert( "uuid" );
  text += headerPrint( ph, exclude );

  set<AObject *> objs = objectsInReferential( ref );
  set<AObject *>::const_iterator io, eo = objs.end();

  if( !objs.empty() )
  {
    text += "<h4>Objects in this referential:</h4><ul>";
    unsigned i = 0;
    for( io=objs.begin(); io!=eo && i<10; ++io, ++i )
      text += QString( "<li>" ) + (*io)->name().c_str() + "</li>";
    if( objs.size() > 10 )
      text += "<li>...</li>";
    text += "</ul>";
  }

  return text;
}


QString ReferentialWindow::transformationToolTipText(
  anatomist::Transformation *tr, list<string> & temp_filenames )
{
  using anatomist::Transformation;

  QPixmap     pix( 64, 16 );
  QPainter    ptr( &pix );
  AimsRGB     col = tr->source()->Color();
  ptr.setBackgroundMode( Qt::OpaqueMode );
  ptr.fillRect( 0, 0, 64, 16, QColor( 255, 255, 255 ) );
  ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
  ptr.drawEllipse( 0, 0, 16, 16 );
  col = tr->destination()->Color();
  ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
  ptr.drawEllipse( 48, 0, 16, 16 );
  ptr.drawLine( 16, 8, 48, 8 );
  ptr.drawLine( 40, 4, 48, 8 );
  ptr.drawLine( 40, 12, 48, 8 );
  ptr.end();
  pix.setMask( pix.createHeuristicMask() );
  int fd;
  string pixfname = FileUtil::temporaryFile( "anarefpixmap.png", fd );
  ::close( fd );
  pix.save( QString( pixfname.c_str() ), "PNG" );
  temp_filenames.push_back( pixfname );

  QString text( "<h4>Transformation:  <img src=\"" );
  text += pixfname.c_str();
  text += "\"/></h4>";
  AimsData<float> r = tr->motion().rotation();
  text += "<table border=1 cellspacing=0><tr>"
      "<td colspan=3><b>R:</b></td><td><b>T:</b></td></tr>"
      "<tr><td>"
      + QString::number( r( 0,0 ) ) + "</td><td>"
      + QString::number( r( 0,1 ) ) + "</td><td>"
      + QString::number( r( 0,2 ) ) + "</td><td>"
      + QString::number( tr->Translation( 0 ) ) + "</td></tr><tr><td>"
      + QString::number( r( 1,0 ) ) + "</td><td>"
      + QString::number( r( 1,1 ) ) + "</td><td>"
      + QString::number( r( 1,2 ) ) + "</td><td>"
      + QString::number( tr->Translation( 1 ) ) + "</td></tr><tr><td>"
      + QString::number( r( 2,0 ) ) + "</td><td>"
      + QString::number( r( 2,1 ) ) + "</td><td>"
      + QString::number( r( 2,2 ) ) + "</td><td>"
      + QString::number( tr->Translation( 2 ) ) + "</td></tr></table>";
  PythonHeader  *ph = tr->motion().header();
  if( ph )
    text += headerPrint( *ph );

  if( !tr->motion().isIdentity() )
  {
    Transformation *trinv = ATransformSet::instance()->transformation(
      tr->destination(), tr->source() );

    col = tr->destination()->Color();
    ptr.begin( &pix );
    ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    ptr.drawEllipse( 0, 0, 16, 16 );
    col = tr->source()->Color();
    ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
    ptr.drawEllipse( 48, 0, 16, 16 );
    ptr.end();
    string pixfname_inv = FileUtil::temporaryFile( "anarefpixmap.png",
                                                   fd );
    ::close( fd );
    pix.save( QString( pixfname_inv.c_str() ), "PNG" );
    temp_filenames.push_back( pixfname_inv );

    text += "<br/><h4>Inverse: <img src=\"";
    text += pixfname_inv.c_str();
    text += "\"/></h4>";
    r = trinv->motion().rotation();
    text += "<table border=1 cellspacing=0><tr>"
        "<td colspan=3><b>R:</b></td><td><b>T:</b></td></tr>"
        "<tr><td>"
        + QString::number( r( 0,0 ) ) + "</td><td>"
        + QString::number( r( 0,1 ) ) + "</td><td>"
        + QString::number( r( 0,2 ) ) + "</td><td>"
        + QString::number( trinv->Translation( 0 ) ) + "</td></tr><tr><td>"
        + QString::number( r( 1,0 ) ) + "</td><td>"
        + QString::number( r( 1,1 ) ) + "</td><td>"
        + QString::number( r( 1,2 ) ) + "</td><td>"
        + QString::number( trinv->Translation( 1 ) ) + "</td></tr><tr><td>"
        + QString::number( r( 2,0 ) ) + "</td><td>"
        + QString::number( r( 2,1 ) ) + "</td><td>"
        + QString::number( r( 2,2 ) ) + "</td><td>"
        + QString::number( trinv->Translation( 2 ) ) + "</td></tr></table>";
  }

  return text;
}


void ReferentialWindow::mergeReferentials( anatomist::Transformation* tr )
{
  if( !tr->motion().isIdentity() )
  {
    cout << "transformation is not identity: cannot merge referentials\n";
    return;
  }
  Referential::mergeReferentials( tr->source(), tr->destination() );
  refresh();
}


void ReferentialWindow::mergeIdenticalReferentials()
{
  Referential::mergeIdenticalReferentials();
}


void ReferentialWindow::unlinkFiles( const list<string> & temp_filenames )
{
  list<string>::const_iterator il, el = temp_filenames.end();
  for( il=temp_filenames.begin(); il!=el; ++il )
    unlink( il->c_str() );
}

// -----------

ReferentialWindow_TransCallback::ReferentialWindow_TransCallback
  ( ReferentialWindow* rw, anatomist::Transformation* t )
  : QObject(), refwin( rw ), trans( t )
{
}

ReferentialWindow_TransCallback::~ReferentialWindow_TransCallback()
{
}

void ReferentialWindow_TransCallback::deleteTransformation()
{
  refwin->deleteTransformation( trans );
}

void ReferentialWindow_TransCallback::invertTransformation()
{
  refwin->invertTransformation( trans );
}

void ReferentialWindow_TransCallback::reloadTransformation()
{
  refwin->reloadTransformation( trans );
}

void ReferentialWindow_TransCallback::saveTransformation()
{
  refwin->saveTransformation( trans );
}

void ReferentialWindow_TransCallback::mergeReferentials()
{
  refwin->mergeReferentials( trans );
}

