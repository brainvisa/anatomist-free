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


#include <anatomist/processor/pipeReader.h>
#include <anatomist/processor/pipeReaderP.h>
#include <anatomist/application/Anatomist.h>
#include <anatomist/processor/Reader.h>
#include <anatomist/processor/unserializer.h>
#include <cartobase/stream/fdstream.h> // might be replaced by boost include
#include <qapplication.h>
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef ANA_THREADED_PIPEREADER
#include <cartobase/thread/thread.h>
#include <cartobase/smart/mutexrcptr.h>
#else
#include <qsocketnotifier.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

using namespace anatomist;
using namespace carto;
using namespace std;

namespace anatomist
{
  struct APipeReader::Private
  {
    Private();
    ~Private();

    MutexRcPtr<CommandContext>	context;
    CommandReader	*creader;
    bool		destroyWhenEmpty;
    string		filename;
#ifdef ANA_THREADED_PIPEREADER
    Thread		*thread;
#else
    QSocketNotifier	*socnot;
    APipeReader_Bridge	*pbridge;
#endif
    bool		reopen;
  };

}


APipeReader::Private::Private()
  : context( new CommandContext ), creader( 0 ), 
#ifdef ANA_THREADED_PIPEREADER
    thread( 0 ), 
#else
    socnot( 0 ), pbridge( 0 ), 
#endif
    reopen( false )
{
  context->unserial = rc_ptr<Unserializer>( new Unserializer );
  context->ostr = &cout;
}


APipeReader::Private::~Private()
{
#ifdef ANA_THREADED_PIPEREADER
  // cout << "APipeReader::Private::~Private()\n";
  if( thread->isCurrent() )
    thread->detach();
  delete thread;
#else
  delete pbridge;
  delete socnot;
#endif
}


#ifdef ANA_THREADED_PIPEREADER

namespace
{

  class APipeReaderThread : public Thread
  {
  public:
    APipeReaderThread( APipeReader* pr );
    virtual ~APipeReaderThread();

  protected:
    virtual void doRun();

    APipeReader	*pread;
  };


  APipeReaderThread::APipeReaderThread( APipeReader* pr )
    : Thread(), pread( pr )
  {
    // initialize CommandReader_Bridge in the main thread
    anatomist::internal::CommandReader_Bridge::_executor();
    setSuicideSafe( true );
  }


  APipeReaderThread::~APipeReaderThread()
  {
  }


  void APipeReaderThread::doRun()
  {
    pread->open();
  }

}

#endif


APipeReader_Bridge::~APipeReader_Bridge()
{
}


void APipeReader_Bridge::readSocket( int )
{
  //cout<< "APipeReader_Bridge::readSocket( int )\n";
  preader->readSocket();
}


// ----

APipeReader::APipeReader( const string & filename, bool destroyWhenEmpty )
  : d( new APipeReader::Private ) 
{
  //cout << "APipeReader::APipeReader\n";
  d->destroyWhenEmpty = destroyWhenEmpty;
  d->filename = filename;

#ifdef ANA_THREADED_PIPEREADER
  d->thread = new APipeReaderThread( this );
  d->thread->launch();

#else
  d->pbridge = new APipeReader_Bridge( this );
  open();
#endif
}


APipeReader::APipeReader( int sock, bool destroyWhenEmpty )
  : d( new APipeReader::Private ) 
{
  // cout << "APipeReader::APipeReader / socket: " << sock << "\n" << flush;
  d->destroyWhenEmpty = destroyWhenEmpty;
  d->context->infd = sock;

#ifdef ANA_THREADED_PIPEREADER
  d->thread = new APipeReaderThread( this );
  d->thread->launch();

#else
  d->pbridge = new APipeReader_Bridge( this );
  open();
#endif
}


APipeReader::~APipeReader()
{
  cout << "Anatomist: closing incoming connection\n";
  close();
  delete d;
}


void APipeReader::open()
{
  if( !_open() )
    delete this;
}


bool APipeReader::_open()
{
  int	fd;

  if( d->filename.empty() )	// socket
    {
      /* cout << "PipeReader opening on socket: " << d->context->infd 
         << endl << flush; */
      d->context.lock();
      fd = d->context->infd;
      d->context->ostr = new boost::fdostream( fd );
      d->context->ownstream = true;
      /* WARNING: we should ensure fd is not closed asynchronously for 
	 reading and writing */
      d->context.unlock();
    }
  else
    {

      int	flags = O_RDONLY;

#ifndef _WIN32

      bool	block = true;
      struct stat	stbuf;
      if( stat( d->filename.c_str(), &stbuf ) )
	{
	  cerr << "inexistant file\n";
	  return false;
	}
      if( !S_ISREG( stbuf.st_mode ) ) // S_ISFIFO() S_ISSOCK()
	{
	  //cout << "non-regular file\n";
	  d->reopen = true;
#ifndef ANA_THREADED_PIPEREADER
          //	non-blocking open (pipes...)
	  flags |= O_NONBLOCK;
	  block = false;
#endif
	}

#endif	// _WIN32

      fd = ::open( d->filename.c_str(), flags );
      // lock before accessing d->context which is a rc_ptr
      d->context.lock();
      d->context->infd = fd;
      if( fd == -1 )
	{
	  cerr << "cannot open " << d->filename << endl;
	  return false;
	}

#ifndef _WIN32

      if( !block )
	{
	  //	get back to blocking mode
	  fcntl( fd, F_GETFL, &flags );
	  flags &= ~O_NONBLOCK;
	  fcntl( fd, F_SETFL, flags );
	}

#endif	// _WIN32

    }

  d->context->istr = new boost::fdistream( fd );
  d->context.unlock();

#ifdef ANA_THREADED_PIPEREADER

  readSocket();
  return true;

#else

  d->socnot = new QSocketNotifier( fd, QSocketNotifier::Read );
  QObject::connect( d->socnot, SIGNAL( activated( int ) ), d->pbridge, 
		    SLOT( readSocket( int ) ) );
  return true;
#endif
}


void APipeReader::close()
{
#ifdef ANA_THREADED_PIPEREADER
  // can be called from the main thread or the reading thread (auto-closing)
  d->context->closein();
  if( d->thread && ! d->thread->isCurrent() )
    d->thread->join();

#else
  delete d->socnot;
  d->socnot = 0;
#endif

  delete d->creader;
  d->creader = 0;

#ifdef ANA_THREADED_PIPEREADER
  if( d->thread && ! d->thread->isCurrent() )
    {
      delete d->thread;
      d->thread = 0;
    }
#else
  d->context->closein();
#endif
}


void APipeReader::readSocket()
{
  // in threaded version we are always in the reading thread

  //cout << "ReadSocket\n" << flush;
  //cout << "loopLevel : " << qApp->loopLevel() << endl;
  if( qApp->loopLevel() > 1 )	// sub-loop, don't read
    return;

  istream	*&istr = d->context->istr;
  do
    {
      readOne();
    }
  while( ( istr && *istr && !istr->eof() 
#ifndef ANA_THREADED_PIPEREADER
	 && istr->rdbuf()->in_avail()
#endif
           ) || d->reopen );

  if( !istr || istr->eof() || !(*istr) )
    {
      //cout << "eof. finished\n";
      if( d->destroyWhenEmpty )
	delete this;
    }
}


void APipeReader::readOne()
{
  // cout << "APipeReader::readOne\n" << flush;
  /* cout << "avail : " 
    << d->context->istr->rdbuf()->in_avail() 
    << "\n" << flush; */
  d->context.lock();
  // cout << "pipeReader locked\n" << flush;

  istream	*istr = d->context->istr;
#ifndef _WIN32
  if( !*istr )	// error on stream
    {
      // cout << "stream vide\n" << flush;
      if( !d->destroyWhenEmpty && d->reopen )
	{
	  CommandReader	*cr = d->creader;
	  rc_ptr<Unserializer>	us = d->context->unserial;

	  d->context->unserial.reset();
	  d->creader = 0;
	  close();
	  //	re-open
	  if( _open() )
	    {
	      d->creader = cr;
	      d->context->unserial = us;
	    }
	}
      else
	cerr << "warning: stream reader in bad state, not reopening\n";

      // cout << "pipeReader unlock (empty)\n" << flush;
      d->context.unlock();
      return;
    }
#endif

  // cout << "pipeReader unlock\n" << flush;
  d->context.unlock();

  char	c;

  // cout << "PipeReader: getting 1st char\n" << flush;
  istr->get( c );
  // cout << "PipeReader: 1sh char: " << (int) c << ": " << c << endl << flush;
  if( c == ' ' || c == '\n' )
    {
      /* cout << "pipe: garbage " << (int) c << ", avail: " 
	   << istr->rdbuf()->in_avail() << ", eof: " << istr->eof() 
	   << "\n" << flush; */
      return;	// drop garbage
    }
  istr->unget();

  d->context.lock();
  // cout << "pipeReader locked 2\n" << flush;

  if( !d->creader )
    {
      //cout << "unlocking 2.1...r\n";
      d->context.unlock();
      // cout << "creating Unserializer\n";
      d->context->unserial = rc_ptr<Unserializer>( new Unserializer );
      d->creader = new CommandReader( d->context );
      d->creader->attach( *istr );
    }
  else
    {
      // cout << "unlocking 2.2...r\n";
      d->context.unlock();
      // cout << "pipeReader unlocked 2\n" << flush;
    }

  try
    {
      // cout << "PipeReader: reading 1 tree\n" << flush;
      d->creader->readOne();
      // cout << "after reading :\n" << flush;
      /* cout << "askedToClose : " << d->creader->askedToClose() << endl;
      cout << "askedToRemove : " << d->creader->askedToRemovePipeFile() 
      << endl << flush; */
    }
  catch( exception & e )
    {
      if( !istr->eof() )
        {
          cerr << "exception while reading" << endl;
          cerr << e.what() << endl << flush;
        }
    }

  if( d->creader->askedToClose() )
    {
      //cout << "Pipe asked closing....\n";
      d->destroyWhenEmpty = true;
      bool	rmv = d->creader->askedToRemovePipeFile();
      d->creader->close();
      close();
      if( rmv )
	{
	  //cout << "asked to remove pipe file\n";
	  remove( d->filename.c_str() );
	}
    }

  /* cout << "avail : " << istr->rdbuf()->in_avail() 
    << "\n\n" << flush; */
}


bool APipeReader::operator ! () const
{
  return( !d->context->istr || !*d->context->istr );
}
