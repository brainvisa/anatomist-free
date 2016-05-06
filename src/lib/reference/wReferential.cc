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
#include <anatomist/misc/error.h>
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
    if( r )
    {
      string  name;
      PythonHeader  & ph = r->header();
      if( !ph.getProperty( "name", name ) )
        name = "&lt;unnamed&gt;";

      QPixmap pix( 16, 16 );
      QPainter      ptr( &pix );
      AimsRGB       col = r->Color();
      ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
      ptr.fillRect( 0, 0, 16, 16, QColor( 255, 255, 255 ) );
      ptr.drawEllipse( 0, 0, 16, 16 );
      ptr.end();
      pix.setMask( pix.createHeuristicMask() );
      int fd;
      string pixfname = FileUtil::temporaryFile( "anarefpixmap.png", fd );
      pix.save( QString( pixfname.c_str() ), "PNG" );

      /* QTextDocument document;
      document.addResource( QTextDocument::ImageResource, QUrl("refimage.png"),
                            pix ); */

      QString text( "<h4>Referential:  <img src=\"" );
      text += pixfname.c_str();
      text += "\"></img></h4><em><b>  ";
      text += name.c_str();
      text += "</b></em><br/><b>UUID</b>        :  ";
      text += r->uuid().toString().c_str();
      text += "<br/>";
      set<string> exclude;
      exclude.insert( "name" );
      exclude.insert( "uuid" );
      text += headerPrint( ph, exclude );
      QToolTip::showText( _refwin->mapToGlobal( p ), text );
      ::close( fd );
      unlink( pixfname.c_str() );
    }
    else
    {
      anatomist::Transformation  *t = _refwin->transfAt( p );
      if( t )
      {
        QPixmap     pix( 64, 16 );
        QPainter    ptr( &pix );
        AimsRGB     col = t->source()->Color();
        ptr.setBackgroundMode( Qt::OpaqueMode );
        ptr.fillRect( 0, 0, 64, 16, QColor( 255, 255, 255 ) );
        ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
        ptr.drawEllipse( 0, 0, 16, 16 );
        col = t->destination()->Color();
        ptr.setBrush( QBrush( QColor( col.red(), col.green(), col.blue() ) ) );
        ptr.drawEllipse( 48, 0, 16, 16 );
        ptr.drawLine( 16, 8, 48, 8 );
        ptr.drawLine( 40, 4, 48, 8 );
        ptr.drawLine( 40, 12, 48, 8 );
        ptr.end();
        pix.setMask( pix.createHeuristicMask() );
        int fd;
        string pixfname = FileUtil::temporaryFile( "anarefpixmap.png", fd );
        pix.save( QString( pixfname.c_str() ), "PNG" );

        QString text( "<h4>Transformation:  <img src=\"" );
        text += pixfname.c_str();
        text += "\"/></h4>";
        AimsData<float> r = t->motion().rotation();
        text += "<table border=1 cellspacing=0><tr>"
            "<td colspan=3><b>R:</b></td><td><b>T:</b></td></tr>"
            "<tr><td>"
            + QString::number( r( 0,0 ) ) + "</td><td>"
            + QString::number( r( 0,1 ) ) + "</td><td>"
            + QString::number( r( 0,2 ) ) + "</td><td>"
            + QString::number( t->Translation( 0 ) ) + "</td></tr><tr><td>"
            + QString::number( r( 1,0 ) ) + "</td><td>"
            + QString::number( r( 1,1 ) ) + "</td><td>"
            + QString::number( r( 1,2 ) ) + "</td><td>"
            + QString::number( t->Translation( 1 ) ) + "</td></tr><tr><td>"
            + QString::number( r( 2,0 ) ) + "</td><td>"
            + QString::number( r( 2,1 ) ) + "</td><td>"
            + QString::number( r( 2,2 ) ) + "</td><td>"
            + QString::number( t->Translation( 2 ) ) + "</td></tr></table>";
        PythonHeader  *ph = t->motion().header();
        if( ph )
          text += headerPrint( *ph );
        QToolTip::showText( _refwin->mapToGlobal( p ), text );
        ::close( fd );
        unlink( pixfname.c_str() );
      }
    }
  }

}

// --------

namespace anatomist
{

  struct ReferentialWindow_PrivateData
  {
    ReferentialWindow_PrivateData() 
      : srcref( 0 ), dstref( 0 ), trans( 0 ), tracking( false ), refmenu( 0 ),
      bgmenu( 0 )
    {}

    Referential			*srcref;
    Referential			*dstref;
    Transformation		*trans;
    QPoint			pos;
    bool			tracking;
    map<Referential *, QPoint>	refpos;
    QMenu			*refmenu;
    QMenu                  *bgmenu;
    RefToolTip                  *tooltip;
  };

}


ReferentialWindow::ReferentialWindow( QWidget* parent, const char* name, 
				      Qt::WindowFlags f )
  : QLabel( parent, f ), 
  pdat( new ReferentialWindow_PrivateData )
{
  setWindowTitle( tr( "Referentials" ) );
  setObjectName(name);
  resize( 256, 256 );
  setPixmap( QPixmap( width(), height() ) );
  pdat->tooltip = new RefToolTip( this );
  setAttribute( Qt::WA_PaintOutsidePaintEvent );
}

ReferentialWindow::~ReferentialWindow()
{
  if (theAnatomist->getControlWindow() != 0)
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
  delete pdat;
}


void ReferentialWindow::closeEvent( QCloseEvent * ev )
{
  QLabel::closeEvent( ev );
  if( theAnatomist->getControlWindow() )
    theAnatomist->getControlWindow()->enableRefWinMenu( true );
}


void ReferentialWindow::resizeEvent( QResizeEvent* ev )
{
  QLabel::resizeEvent( ev );
  refresh();
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


void ReferentialWindow::refresh()
{
  pdat->refpos.clear();

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
  int			w = width(), h = height(), R = w, Rmin = 50;
  QPixmap		pix( w, h );

  QPainter		p( &pix );

  p.setPen( QPen( Qt::black ) );
  p.eraseRect( 0, 0, w, h );
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
  setPixmap( pix );
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
	    popupRefMenu( ev->globalPos() );
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
      p.drawPixmap( 0, 0, *pixmap() );

      QPoint		dummy;
      pdat->dstref = refAt( ev->pos(), dummy );

      if( pdat->dstref && pdat->srcref != pdat->dstref 
          && !ATransformSet::instance()->transformation( pdat->srcref, 
                                                         pdat->dstref ) )
      {
        if( ev->modifiers() & Qt::ControlModifier )
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
            = new LoadTransformationCommand( matrix, pdat->srcref, 
                                              pdat->dstref );
          theProcessor->execute( com );
          refresh();
        }
        else
          openSelectBox();
      }
  }
}


void ReferentialWindow::mouseMoveEvent( QMouseEvent* ev )
{
  if( pdat->tracking )
    {
      QPainter	p( this );
      p.drawPixmap( 0, 0, *pixmap() );
      p.drawLine( pdat->pos, ev->pos() );
    }
}


Referential* ReferentialWindow::refAt( const QPoint & pos, QPoint & newpos )
{
  map<Referential*, QPoint>::const_iterator	ir, fr = pdat->refpos.end();
  Referential		*ref;
  int			sz = 10;
  QPoint		rpos;

  for( ir=pdat->refpos.begin(); ir!=fr; ++ir )
    {
      ref = (*ir).first;
      rpos = (*ir).second;
      if( pos.x() >= rpos.x()-sz && pos.x() < rpos.x()+sz 
	  && pos.y() >= rpos.y()-sz && pos.y() < rpos.y()+sz )
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

  // 1st pass on loaded transformations
  for( it=trans.begin(); it!=ft; ++it )
    {
      t = *it;
      if( !t->isGenerated() )
        {
          const QPoint & rpos1 = pdat->refpos[ t->source() ];
          rvec = pdat->refpos[ t->destination() ] - rpos1;
          relp = pos - rpos1;
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
          relp = pos - rpos1;
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

  pop->popup( pos );
}


void ReferentialWindow::popupTransfMenu( const QPoint & pos )
{
  vector<anatomist::Transformation *>  trans = transformsAt( pos );
  if( trans.empty() )
    return;

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
  refresh();

  set<AWindow *>		win = theAnatomist->getWindows();
  set<AWindow*>::iterator	iw, fw = win.end();

  for( iw=win.begin(); iw!=fw; ++iw )
    (*iw)->SetRefreshFlag();
  theAnatomist->Refresh();
}


void ReferentialWindow::clearUnusedReferentials()
{
  set<Referential *> refs = theAnatomist->getReferentials();
  set<Referential *>::iterator i, e = refs.end();
  Referential *ref;
  set<Referential *> usedrefs;
  for( i=refs.begin(); i!=e; ++i )
  {
    ref = *i;
    if( ref == Referential::acPcReferential()
        || ref == Referential::mniTemplateReferential()
        || !ref->AnaWin().empty() || !ref->AnaObj().empty() )
      usedrefs.insert( ref );
  }
  // check other referentials
  set<Referential *>::iterator j, k, unused = usedrefs.end();
  ATransformSet *ts = ATransformSet::instance();
  for( i=refs.begin(); i!=e; )
  {
    ref = *i;
    if( usedrefs.find( ref ) == unused )
    {
      // get the connected component this ref is in
      set<Referential *>
          cc = ts->connectedComponent( ref );
      // check whether there are any useful ref in this CC
      for( j=cc.begin(), k=cc.end(); j!=k; ++j )
        if( usedrefs.find( *j ) != unused )
          break;
      if( j == k ) // no useful ref: we can delete the entire CC
      {
        for( j=cc.begin(), k=cc.end(); j!=k; ++j )
          if( *j != ref ) // don't delete ref yet
          {
            refs.erase( *j );
            delete *j;
          }
        ++i; // increment iterator
        delete ref; // then we can delete ref safely
      }
      else
      {
        // ref is linked to a "useful connected component"
        // we must check whether it would break the CC if we remove it
        set<anatomist::Transformation *> trs = ts->transformationsWith( ref );
        set<anatomist::Transformation *>::iterator it, jt, et = trs.end();
        // filter out generated transformations
        for( it=trs.begin(), et=trs.end(); it!=et; )
        {
          if( (*it)->isGenerated() )
          {
            jt = it;
            ++it;
            trs.erase( jt );
          }
          else
            ++it;
        }
        if( trs.size() <= 1 ) // then no link goes through ref
        {
          ++i;
          delete ref;
        }
        else // now trs *must* contain exactly 2 transfos
        {
          //debug
          if( trs.size() != 2 )
            cerr << "BUG in ReferentialWindow::clearUnusedReferentials: more "
                "than 2 connections to a ref inside a connected component"
                << endl;
          int usedcc = 0;
          for( it=trs.begin(), et=trs.end(); it!=et; ++it )
          {
            anatomist::Transformation *tr = *it;
            // get other end
            Referential *ref2 = tr->source();
            if( ref2 == ref )
              ref2 = tr->destination();
            // temporarily disable the transformation
            tr->unregisterTrans();
            // get CC of other end
            set<Referential *> cc2 = ts->connectedComponent( ref2 );
            // if cc2 has useful refs, then ref is useful
            if( cc2.size() != cc.size() )
            {
              for( j=cc2.begin(), k=cc2.end(); j!=k; ++j )
                if( usedrefs.find( *j ) != unused )
                  break;
              if( j == k ) // no useful ref: we can delete the entire CC2
              {
                for( j=cc2.begin(), k=cc2.end(); j!=k; ++j )
                  if( *j != ref ) // don't delete ref yet
                  {
                    refs.erase( *j );
                    delete *j;
                  }
                tr = 0;
              }
              else
              {
                tr->registerTrans();
                ++usedcc;
              }
            }
            else
              ++usedcc;
            if( tr )
              tr->registerTrans();
          }
          if( usedcc <= 1 ) // ref is not useful
          {
            ++i;
            delete ref;
          }
          else
            ++i;
        }
      }
    }
    else
      ++i;
  }
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
  theAnatomist->Refresh();
  refresh();
}


void ReferentialWindow::seeObjectsInReferential()
{
  set<AObject *> objs = theAnatomist->getObjects();
  set<AObject *>::iterator io, eo = objs.end();
  CreateWindowCommand *wc = new CreateWindowCommand( "Browser" );
  theProcessor->execute( wc );
  AWindow *win = wc->createdWindow();
  set<AObject *> selobj;
  for( io=objs.begin(); io!=eo; ++io )
    if( (*io)->getReferential() == pdat->srcref )
      selobj.insert( *io );
  if( !selobj.empty() )
  {
    set<AWindow *> winset;
    winset.insert( win );
    AddObjectCommand *add = new AddObjectCommand( selobj, winset, false,
                                                  false, false );
    theProcessor->execute( add );
  }
}


#if QT_VERSION >= 0x040000
bool ReferentialWindow::event( QEvent* event )
{
  if (event->type() == QEvent::ToolTip)
  {
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    pdat->tooltip->maybeTip( helpEvent->pos() );
  }
  return QWidget::event(event);
}
#endif


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
