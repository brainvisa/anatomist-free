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


#include <anatomist/control/qImageLabel.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <qtooltip.h>
#if QT_VERSION >= 0x040000
#include <QMouseEvent>
#include <QHelpEvent>
#include <QTimer>
#endif
#include <iostream>

using namespace anatomist;
using namespace std;

struct QImageLabel_privateData
{
  QImageLabel_privateData()
    : movie( 0 )
#if QT_VERSION < 0x040000
    , ttg( 0 )
#endif
  {}
  ~QImageLabel_privateData()
  {
#if QT_VERSION < 0x040000
    delete movie;
#endif
    movie = 0;
  }

  QMovie	*movie;
#if QT_VERSION < 0x040000
  QToolTipGroup	*ttg;
#endif
};


QImageLabel::QImageLabel( QWidget* parent, const char* name, Qt::WFlags f )
  : QLabel( parent, name, f ), _privdata( new QImageLabel_privateData )
{
  string	align;
  theAnatomist->config()->getProperty( "controlWindowLogoAlignment", align );
  if( align == "left" )
    setAlignment( Qt::AlignLeft );
  else if( align == "right" )
    setAlignment( Qt::AlignRight );
  else
    setAlignment( Qt::AlignCenter );
  installImage();

#if QT_VERSION >= 0x040000
  setToolTip( tr( "  This big stupid logo annoys you ?  \n"
                  "  Remove it !\n"
                  "  (in Settings->Prefs)..." ) );
#else
  _privdata->ttg = new QToolTipGroup( this );
  QToolTip::add( this, tr( "  This big stupid logo annoys you ?  \n"
			   "  Remove it !\n"
			   "  (in Settings->Prefs)..." ), _privdata->ttg, "" );
  connect( _privdata->ttg, SIGNAL( removeTip() ), this, SLOT( removeTip() ) );
#endif
}


QImageLabel::~QImageLabel()
{
  delete _privdata;
}


void QImageLabel::installImage()
{
  string icon( Settings::localPath() + "/icons/anatomist.png" );
  QPixmap anapix( icon.c_str() );
  if( anapix.isNull() )
    anapix.load( ( Settings::globalPath() + "/icons/anatomist.png" ).c_str() );
  if( !anapix.isNull() )
    setPixmap( anapix );
}


#if QT_VERSION >= 0x040000
#warning TODO: Qt4 QImageLabel::movieStatusChanged() not compiled
#if 1 //0 // Pb: Moc of Qt3 doesn't handle #ifdefs
void QImageLabel::movieStatusChanged( int /* QMovie::MovieState */ s )
{
  switch( s )
    {
    case QMovie::NotRunning:
      installImage();
      break;
    default:
      break;
    }
}
#endif

#else

void QImageLabel::movieStatusChanged( int s )
{
  switch( s )
    {
    case QMovie::SourceEmpty:
      cerr << "QMovie::SourceEmpty\n";
      break;
    case QMovie::UnrecognizedFormat:
      cerr << "QMovie::UnrecognizedFormat\n";
      break;
    case QMovie::Paused:
      break;
    case QMovie::EndOfFrame:
      break;
    case QMovie::EndOfLoop:
    case QMovie::EndOfMovie:
      installImage();
      break;
    default:
      cerr << "movieStatusChanged : unknown status\n";
      break;
    }
}
#endif


void QImageLabel::mousePressEvent( QMouseEvent* e )
{
  if( e->button() == Qt::RightButton && !movie() )
    {
#if QT_VERSION >= 0x040000
      if( !_privdata->movie )
	{
	  _privdata->movie = 
	    new QMovie( ( Settings::globalPath()
			  + "/movie/anatomist.gif" ).c_str(), QByteArray(), 
                        this );
          connect( _privdata->movie, 
                   SIGNAL( stateChanged( QMovie::MovieState ) ), 
                   this, SLOT( stateChanged( QMovie::MovieState ) ) );
	}
      else
	{
	  _privdata->movie->start();
	}
      setMovie( _privdata->movie );

#else

      if( !_privdata->movie )
	{
	  _privdata->movie = 
	    new QMovie( ( Settings::globalPath()
			  + "/movie/anatomist.gif" ).c_str() );
	  _privdata->movie->connectStatus( this, 
					   SLOT( movieStatusChanged( int ) ) );
	}
      else
	{
	  _privdata->movie->restart();
	  //QRect	sz = _privdata->movie->getValidRect();
	  //resize( sz.width(), sz.height() );
	}
      setMovie( *_privdata->movie );
#endif
    }
  QLabel::mousePressEvent( e );
}


void QImageLabel::removeTip()
{
#if QT_VERSION >= 0x040000
  setToolTip( QString::null );
#else
  _privdata->ttg->setEnabled( false );
#endif
}


#if QT_VERSION >= 0x040000
bool QImageLabel::event( QEvent* e )
{
  if( e->type() == QEvent::ToolTip )
    QTimer::singleShot( 3000, this, SLOT( removeTip() ) );
  return QLabel::event( e );
}
#endif



