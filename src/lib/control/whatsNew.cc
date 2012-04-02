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


#include <anatomist/control/whatsNew.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/application/settings.h>
#include <anatomist/application/globalConfig.h>
#include <cartobase/stream/fileutil.h>

using namespace anatomist;
using namespace carto;
using namespace std;


WhatsNew::WhatsNew( QWidget* parent, const char * name, bool modal, 
                    Qt::WFlags f )
  : QDialog( parent, f )
{
  setCaption( tr( "Anatomist News" ) );
  setObjectName(name);
  setModal(modal);

  QVBoxLayout	*mainlay = new QVBoxLayout( this, 10, 10 );
  QTextBrowser	*txt = new QTextBrowser( this );
  QPushButton	*ok = new QPushButton( tr( "OK" ), this );
  ok->setFixedSize( ok->sizeHint() );

  /*txt->setText( tr( "Anatomist version is now " ) 
    + theAnatomist->versionString().c_str() );*/

  string fname = "po/";
  string lang = "en";
  theAnatomist->config()->getProperty( "language", lang );
  fname += lang + "/new.html";
  fname = Settings::findResourceFile( fname );
  if( FileUtil::fileStat( fname ).find( '+' ) == string::npos
      && lang != "en" )
    fname = Settings::findResourceFile( "po/en/new.html" );
#if QT_VERSION >= 0x040000
  txt->setSource( QUrl( fname.c_str() ) );
#else
  txt->setSource( fname.c_str() );
#endif
  ok->setDefault( true );

  mainlay->addWidget( txt );
  mainlay->addWidget( ok, 0, Qt::AlignRight );

  connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
  resize( 700, 500 );
}


WhatsNew::~WhatsNew()
{
}
