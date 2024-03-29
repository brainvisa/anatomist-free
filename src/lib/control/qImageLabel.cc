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
  {}
  ~QImageLabel_privateData()
  {
    movie = 0;
  }

  QMovie	*movie;
};


QImageLabel::QImageLabel( QWidget* parent, const char* name,
                          Qt::WindowFlags f )
  : QLabel( parent, f ), _privdata( new QImageLabel_privateData )
{
  setObjectName(name);
  string	align;
  theAnatomist->config()->getProperty( "controlWindowLogoAlignment", align );
  if( align == "left" )
    setAlignment( Qt::AlignLeft );
  else if( align == "right" )
    setAlignment( Qt::AlignRight );
  else
    setAlignment( Qt::AlignCenter );
  installImage();

  setToolTip( tr( "  This big stupid logo annoys you ?  \n"
                  "  Remove it !\n"
                  "  (in Settings->Prefs)..." ) );
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
    anapix.load( Settings::findResourceFile( "icons/anatomist.png" ).c_str() );
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
  QLabel::mousePressEvent( e );
}


void QImageLabel::removeTip()
{
  setToolTip( QString() );
}


bool QImageLabel::event( QEvent* e )
{
  if( e->type() == QEvent::ToolTip )
    QTimer::singleShot( 3000, this, SLOT( removeTip() ) );
  return QLabel::event( e );
}



