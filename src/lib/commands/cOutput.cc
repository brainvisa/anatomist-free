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

#include <anatomist/commands/cOutput.h>
#include <anatomist/processor/Registry.h>
#include <cartobase/object/syntax.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/fdstream.h> // might be replaced by boost
#include <fstream>
#ifdef _WIN32
#include <winsock.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <errno.h>
#include <string.h>

using namespace anatomist;
using namespace carto;
using namespace std;


OutputCommand::OutputCommand( const string & filename, 
			      CommandContext * context ) 
  : RegularCommand(), SerializingCommand( context ), _filename( filename ), 
    _port( 0 )
{
}


OutputCommand::OutputCommand( const string & ip, int port, 
			      CommandContext * context ) 
  : RegularCommand(), SerializingCommand( context ), _ip( ip ), _port( port )
{
}


OutputCommand::~OutputCommand()
{
}


bool OutputCommand::initSyntax()
{
  SyntaxSet	ss;
  Syntax	& s = ss[ "Output" ];

  s[ "filename"        ].type = "string";
  s[ "ip"              ].type = "string";
  s[ "port"            ].type = "int";
  s[ "default_context" ].type = "int";

  Registry::instance()->add( "Output", &read, ss );
  return( true );
}


void OutputCommand::doit()
{
  if( !_filename.empty() )
    context()->attach( new ofstream( _filename.c_str() ), true );
  else if( !_ip.empty() )
    {
      struct hostent	*hent = gethostbyname( _ip.c_str() );
      if( !hent )
	{
	  cerr << "Cannot lookup network address " << _ip << endl;
	  return;
	}
      cout << "name: " << hent->h_name << endl;
      cout << "host: addr_type : " << hent->h_addrtype 
	   << "; len : " << hent->h_length << endl;
      cout << (unsigned) (unsigned char) hent->h_addr[0] << "." 
	   << (unsigned) (unsigned char) hent->h_addr[1] << "." 
	   << (unsigned) (unsigned char) hent->h_addr[2] << "."
	   << (unsigned) (unsigned char) hent->h_addr[3] << endl;
      cout << "port: " << _port << endl;

#ifdef _WIN32
      unsigned long	ipaddr = *hent->h_addr;
      // the following errors have different names in winsock.h
#define ENOTSOCK WSAENOTSOCK
#define EISCONN WSAEISCONN
#define ECONNREFUSED WSAECONNREFUSED
#define ETIMEDOUT WSAETIMEDOUT
#define ENETUNREACH WSAENETUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#define EADDRINUSE WSAEADDRINUSE
#define EINPROGRESS WSAEINPROGRESS
#define EALREADY WSAEALREADY

#else // _WIN32

#if __GLIBC_PREREQ( 2, 2 )
      unsigned long	ipaddr = *(in_addr_t *) hent->h_addr;
#else
      unsigned long	ipaddr = *hent->h_addr;
#endif

#endif
      int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );	// 6: TCP
      if( sock == -1 )
	{
	  cerr << "socket() failed\n";
	  cerr << "errno: " << errno << endl;
	  return;
	}

      struct sockaddr_in	serv_addr;
      int			res;
      // fill in serv_addr
      memset( &serv_addr, 0, sizeof( serv_addr ) );
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons( _port );
      serv_addr.sin_addr.s_addr = ipaddr;

      res = connect( sock, (struct sockaddr *) &serv_addr, 
		     sizeof( struct sockaddr_in ) );
      if( res != 0 )
	{
	  cerr << "Network connection failed\n";
	  switch( errno )
	    {
	    case EBADF:
	      cerr << "EBADF: bad socket descriptor\n";
	      break;
	    case EFAULT:
	      cerr << "EFAULT: bad sockaddr\n";
	      break;
	    case ENOTSOCK:
	      cerr << "ENOTSOCK: descriptor is not a socket\n";
	      break;
	    case EISCONN:
	      cerr << "EISCONN: socket is already connected\n";
	      break;
	    case ECONNREFUSED:
	      cerr << "ECONNREFUSED: connection refused by server\n";
	      break;
	    case ETIMEDOUT:
	      cerr << "ETIMEDOUT: timeout\n";
	      break;
	    case ENETUNREACH:
	      cerr << "ENETUNREACH: network unreachable\n";
	      break;
	    case EHOSTUNREACH:
	      cerr << "EHOSTUNREACH: server unreachable\n";
	      break;
	    case EADDRINUSE:
	      cerr << "EADDRINUSE: address already in use (try another port "
		   << "?)\n";
	      break;
	    case EINPROGRESS:
	      cerr << "EINPROGRESS: connection in progress, giving up\n";
	      break;
	    case EALREADY:
	      cerr << "EALREADY: previous connection trial has not finished\n";
	      break;
	    default:
	      cerr << "unexpected error, errno: " << errno << endl;
	      break;
	    }
	  return;
	}

      cout << "connected.\n";
      cout << ::send( sock, "Anatomist\n", 10, 0 ) << endl;
      context()->attach( new boost::fdostream( sock ), true );
    }
}


Command* OutputCommand::read( const Tree & com, CommandContext* context )
{
  int			port, defc = 0;
  string		ip;
  string		filename;

  com.getProperty( "default_context", defc );
  if( defc )
    context = &CommandContext::defaultContext();
  if( com.getProperty( "filename", filename ) )
    return new OutputCommand( filename , context );
  if( com.getProperty( "ip", ip ) && com.getProperty( "port", port ) )
    return new OutputCommand( ip, port , context );
  return new OutputCommand( filename , context );
}


void OutputCommand::write( Tree & com, Serializer* ) const
{
  Tree		*t = new Tree( true, name() );

  if( !_filename.empty() )
    t->setProperty( "filename", _filename );
  else if( !_ip.empty() )
    {
      t->setProperty( "ip", _ip );
      t->setProperty( "port", _port );
    }
  if( context() == &CommandContext::defaultContext() )
    t->setProperty( "default_context", int(1) );

  com.insert( t );
}


