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

#include <anatomist/processor/server.h>
#include <anatomist/processor/pipeReader.h>
#include <aims/qtcompat/qserversocket.h>
#include <iostream>

using namespace anatomist;
using namespace std;


namespace anatomist
{

  struct CommandServer::Private
  {
    Private() : port( 0 ), qsock( 0 ) {}
    Private( int p ) : port( p ), qsock( 0 ) {}
    ~Private();

    int			port;
    QServerSocket	*qsock;
  };

  CommandServer::Private::~Private()
  {
    delete qsock;
  }

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
      CommandServer	*server;
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
  d->qsock = new internal::CommandServerSocket( this, (short) d->port, 32 );
}


int CommandServer::port() const
{
  return d->port;
}


bool CommandServer::ok() const
{
  if( !d->qsock )
    return false;
  return d->qsock->ok();
}


void CommandServer::setPort( int p )
{
  d->port = p;
  run();
}


void CommandServer::newConnection( int sock )
{
  cout << "new connection\n" << flush;
  new APipeReader( sock, true );
}
