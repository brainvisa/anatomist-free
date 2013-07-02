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


#include <anatomist/control/objectDrag.h>
#include <anatomist/object/Object.h>
#include <anatomist/application/Anatomist.h>
#include <string>
#include <qurl.h>
#include <qevent.h>

using namespace anatomist;
using namespace std;


struct QAObjectDrag::Private
{
  int nformats;
  QByteArray  aobjectdata;
};


QAObjectDrag::QAObjectDrag( const set<AObject *> & o, QWidget * dragSource, 
			    const char * name )
  : QUriDrag( dragSource, name ), d( new Private )
{
  // cout << "QAObjectDrag::QAObjectDrag\n";

  QStringList fnames;

  QByteArray	&ba = d->aobjectdata;
  ba.resize( sizeof( AObject * ) * o.size()
      + sizeof( set<AObject*>::size_type ) );

  QDataStream	ds( &ba, QIODevice::ReadWrite );
  // Qt4 doesn't have QDataStream << size_t, what a shame...
  if( sizeof( long ) == 4 )
    ds << (uint32_t) o.size();
  else
    ds << (quint64) o.size();

  set<AObject *>::const_iterator	io, eo=o.end();
  for( io=o.begin(); io!=eo; ++io )
  {
    // cout << "obj " << *io << endl;
    ds.writeRawData( (const char *) &*io, sizeof( AObject * ) );
    if( !(*io)->fileName().empty() )
      fnames.push_back( QString( (*io)->fileName().c_str() ) );
  }

  if( !fnames.empty() )
    setFileNames( fnames );

  for( d->nformats=0; QUriDrag::format( d->nformats ); ++d->nformats ) {}
}


QAObjectDrag::~QAObjectDrag()
{
  //cout << "QAObjectDrag::~QAObjectDrag\n";
  delete d;
}


bool QAObjectDrag::canDecode( const QMimeSource * e )
{
  //cout << "QAObjectDrag::canDecode\n";
  const QMimeData *md = static_cast<const QDropEvent *>( e )->mimeData();
  if( md->hasFormat( "AObject" ) )
    return true;
  return false;
}


bool QAObjectDrag::decode( const QMimeSource * e, set<AObject*> & o )
{
  //cout << "QAObjectDrag::decode\n";
  const QMimeData *md = static_cast<const QDropEvent *>( e )->mimeData();
  if( !md->hasFormat( "AObject" ) )
    return false;
  QByteArray	ba = md->data( "AObject" );
  QDataStream	s( &ba, QIODevice::ReadOnly );
  set<AObject*>::size_type i, n;
  AObject	*ao;
  if( sizeof( set<AObject*>::size_type ) == 4 )
  {
    uint32_t  n2;
    s >> n2;
    n = n2;
  }
  else
  {
    quint64 n2;
    s >> n2;
    n = n2;
  }
  for( i=0; i<n; ++i )
  {
    s.readRawData( (char *) &ao, sizeof( AObject * ) );
    //cout << ao << endl;
    if( theAnatomist->hasObject( ao ) )
      o.insert( ao );
    else
      return false;
  }
  return true;
}


bool QAObjectDrag::canDecodeURI( const QMimeSource * e )
{
  const QMimeData *md = static_cast<const QDropEvent *>( e )->mimeData();
  bool ok = false;
  if( md->hasUrls() )
  {
    QList<QUrl> uris = md->urls();
    QList<QUrl>::iterator iu, eu = uris.end();
    for( iu=uris.begin(); iu!=eu; ++iu )
    {
      // cout << "  - " << iu->toLocalFile().utf8().data() << endl;
      if( !iu->toLocalFile().isEmpty() )
      {
        ok = true;
        break;
      }
    }
    if( ok )
      return true;
  }
  else if( md->hasText() )
  {
    QString txt = md->text();
    QStringList uris = txt.split( '\n', QString::SkipEmptyParts );
    QList<QString>::iterator iu, eu = uris.end();
    bool ok = false;
    for( iu=uris.begin(); iu!=eu; ++iu )
    {
      QUrl  url( *iu );
      if( url.isValid() && !url.toLocalFile().isEmpty() )
      {
        // cout << "  - " << url.toLocalFile().utf8().data() << endl;
        ok = true;
        break;
      }
    }
    if( ok )
      return true;
  }
  return false;
}


bool QAObjectDrag::decodeURI( const QMimeSource * e,
                              std::list<QString> & objects,
                              std::list<QString> & scenars )
{
  const QMimeData *md = static_cast<const QDropEvent *>( e )->mimeData();
  if( md->hasUrls() )
  {
    QList<QUrl> uris = md->urls();
    QList<QUrl>::iterator iu, eu = uris.end();
    for( iu=uris.begin(); iu!=eu; ++iu )
    {
      QString url = iu->toLocalFile();
      if( !url.isEmpty() )
      {
        if( url.endsWith( ".ana" ) )
          scenars.push_back( url );     // script file
        else
          objects.push_back( url );
      }
    }
  }
  else if( md->hasText() )
  {
    QString txt = md->text();
    QStringList uris = txt.split( '\n', QString::SkipEmptyParts );
    QList<QString>::iterator iu, eu = uris.end();
    for( iu=uris.begin(); iu!=eu; ++iu )
    {
      QUrl  url( *iu );
      if( url.isValid() && !url.toLocalFile().isEmpty() )
      {
        QString surl = url.toLocalFile().trimmed();
        if( surl.endsWith( ".ana" ) )
          scenars.push_back( surl );    // script file
        else
          objects.push_back( surl );
      }
    }
  }
  else return false;
  return !objects.empty() || !scenars.empty();
}


const char* QAObjectDrag::format( int n ) const
{
  if( n < d->nformats )
    return QUriDrag::format( n );
  else if( n > d->nformats )
    return 0;
  static char aobjformat[] = "AObject";
  return aobjformat;
}


QByteArray QAObjectDrag::encodedData( const char* format ) const
{
  if( string( format ) == "AObject" )
    return d->aobjectdata;
  return QUriDrag::encodedData( format );
}


bool QAObjectDrag::provides( const char *mimeType ) const
{
  return string( mimeType ) == "AObject" || QUriDrag::provides( mimeType );
}


