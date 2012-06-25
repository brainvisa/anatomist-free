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


#include <anatomist/application/fileDialog.h>
#include <qcombobox.h>
#include <aims/def/path.h>
#include <anatomist/application/globalConfig.h>
#include <anatomist/application/Anatomist.h>
#include <cartobase/stream/fileutil.h>

using namespace aims;
using namespace carto;
using namespace std;


QFileDialog & anatomist::fileDialog()
{
  static QFileDialog *_fdialog = 0;
  QString path;
  QRect geom;
  bool oldone = false;

  if( _fdialog )
  {
    oldone = true;
    QStringList filenames = _fdialog->selectedFiles();
    if( !filenames.empty() )
    {
      path = FileUtil::dirname( filenames.begin()->toUtf8().data() ).c_str();
      geom = _fdialog->geometry();
    }
    delete _fdialog;
  }

  _fdialog = new QFileDialog( 0, Qt::Dialog );
  _fdialog->setWindowModality( Qt::ApplicationModal );
  if( oldone )
  {
    if( !path.isEmpty() )
      _fdialog->setDirectory( path );
    _fdialog->setGeometry( geom );
  }

  GlobalConfiguration       *cfg = theAnatomist->config();
  string            cpath;

  QStringList hist;
  string            p = Path::singleton().hierarchy() + '/';
  hist += p.c_str();

  if( cfg && cfg->getProperty( "path_list", cpath ) )
  {
    string::size_type     pos;
    while( !cpath.empty() )
    {
      pos = cpath.find( FileUtil::pathSeparator() );
      if( pos == string::npos )
        pos = cpath.length();
      if( pos != 0 )
      {
        p = cpath.substr( 0, pos );
        if( p[p.length() - 1] != '/' )
          p += '/';
        hist += p.c_str();
      }
      cpath.erase( 0, pos+1 );
    }
  }
  _fdialog->setHistory( hist );

  return( *_fdialog );
}
