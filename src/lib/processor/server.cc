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

using namespace anatomist;
using namespace std;

namespace anatomist
{

  class AnaTcpServer : public QTcpServer
  {
  public:
    AnaTcpServer();
    virtual ~AnaTcpServer();

  protected:
#if QT_VERSION >= 0x050000
    virtual void incomingConnection( qintptr fd );
#else
    virtual void incomingConnection( int fd );
#endif
  };

  AnaTcpServer::AnaTcpServer() : QTcpServer()
  {
  }

  AnaTcpServer::~AnaTcpServer()
  {
  }

#if QT_VERSION >= 0x050000
  void AnaTcpServer::incomingConnection( qintptr fd )
#else
  void AnaTcpServer::incomingConnection( int fd )
#endif
  {
    // The default incomingConnection implementation is not thread-safe.
    // see Qt QTcpServer doc.
    cout << "incoming connection: " << fd << endl;
    new APipeReader( fd, true );
  }


  struct CommandServer::Private
  {
    Private() : port( 0 ), qsock( 0 ) {}
    Private( int p ) : port( p ), qsock( 0 ) {}
    ~Private();

    int		port;
    AnaTcpServer       *qsock;
  };

  CommandServer::Private::~Private()
  {
    delete qsock;
  }

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

  d->qsock = new AnaTcpServer;
  d->qsock->setMaxPendingConnections( 32 );
  if( !d->qsock->listen( QHostAddress::Any, (short) d->port ) )
  {
    cerr << "Command server cannot listen to connections.\n";
    delete d->qsock;
    d->qsock = 0;
    return;
  }
}


int CommandServer::port() const
{
  return d->port;
}


bool CommandServer::ok() const
{
  if( !d->qsock )
    return false;
  return d->qsock->isListening();
}


void CommandServer::setPort( int p )
{
  d->port = p;
  run();
}

