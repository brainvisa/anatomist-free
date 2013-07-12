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

#include <anatomist/processor/server.h>
#include <anatomist/processor/pipeReader.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <iostream>

#ifdef QT3_SUPPORT
#include <aims/qtcompat/qserversocket.h>
#endif

using namespace anatomist;
using namespace std;


namespace anatomist
{

  struct CommandServer::Private
  {
    Private() : port( 0 ), qsock( 0 ) {}
    Private( int p ) : port( p ), qsock( 0 ) {}
    ~Private();

    int		port;
#ifdef QT3_SUPPORT
    QServerSocket       *qsock;
#else
    QTcpServer       *qsock;
#endif
  };

  CommandServer::Private::~Private()
  {
    delete qsock;
  }

#ifdef QT3_SUPPORT
  namespace internal
  {

    class CommandServerSocket : public QServerSocket
    {
    public:
      CommandServerSocket( CommandServer* cs, Q_UINT16 port, int backlog = 1, 
                           QObject *parent = 0, const char *name = 0 );
      virtual ~CommandServerSocket();
      virtual void newConnection ( int socket );

    private:
      CommandServer     *server;
    };

    CommandServerSocket::CommandServerSocket( CommandServer* cs, 
                                              Q_UINT16 port, int backlog, 
                                              QObject *parent, 
                                              const char *name )
      : QServerSocket( port, backlog, parent, name ), server( cs )
    {
    }

    CommandServerSocket::~CommandServerSocket()
    {
    }

    void CommandServerSocket::newConnection ( int sock )
    {
      server->newConnection( sock );
    }

  }
#endif

}


static CommandServer* & CommandServer_singleton()
{
  static CommandServer	*serv = 0;
  return serv;
}


CommandServer::CommandServer( int port ) : d( new Private( port ) )
{
  CommandServer_singleton() = this;
  run();
}


CommandServer::~CommandServer()
{
  CommandServer_singleton() = 0;
  delete d;
}


CommandServer* CommandServer::server()
{
  return CommandServer_singleton();
}


void CommandServer::run()
{
  delete d->qsock;

#ifdef QT3_SUPPORT
  d->qsock = new internal::CommandServerSocket( this, (short) d->port, 32 );

#else
//   QNetworkProxy::setApplicationProxy( QNetworkProxy::NoProxy );
  d->qsock = new QTcpServer;
  d->qsock->setMaxPendingConnections( 32 );
  if( !d->qsock->listen( QHostAddress::Any, (short) d->port ) )
  {
    cerr << "Command server cannot listen to connections.\n";
    delete d->qsock;
    d->qsock = 0;
    return;
  }
  d->qsock->connect( d->qsock, SIGNAL( newConnection() ), 
                     this, SLOT( newConnection() ) );
#endif
}


int CommandServer::port() const
{
  return d->port;
}


bool CommandServer::ok() const
{
  if( !d->qsock )
    return false;
#ifdef QT3_SUPPORT
  return d->qsock->ok();
#else
  return d->qsock->isListening();
#endif
}


void CommandServer::setPort( int p )
{
  d->port = p;
  run();
}


#ifdef QT3_SUPPORT
void CommandServer::newConnection( int sock )
{
  cout << "new connection\n" << flush;
  new APipeReader( sock, true );
}

#else
void CommandServer::newConnection()
{
  cout << "new connection\n" << flush;

  while( d->qsock->hasPendingConnections() )
  {
    cout << "getting pending connection\n";
    QTcpSocket *tcpsock = d->qsock->nextPendingConnection();
    tcpsock->setProxy( QNetworkProxy::NoProxy );
    cout << "proxy: " << tcpsock->proxy().type() << endl;
    cout << "state: " << tcpsock->state() << endl;
    new APipeReader( tcpsock->socketDescriptor(), true );
    cout << "pending connection done\n";
  }
}
#endif

