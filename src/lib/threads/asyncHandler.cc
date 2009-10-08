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


#ifndef QT_THREAD_SUPPORT
//#==== System includes
#include <unistd.h> // for pipe
#include <stdio.h>

//#==== QT includes
#include <qsocketnotifier.h>

// version avec QXt, il faut employer une autre méthode
//#define USE_QXT
#endif

//#==== JBIIS includes
#include <anatomist/threads/asyncHandler.h>

#ifndef QT_THREAD_SUPPORT
#include <iostream>
#ifdef USE_QXT
// (pour avoir l'AppContext)
#include <mpp/kernel.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#endif

#else // thread

#include <qevent.h>
#include <qapplication.h>

#endif


// Default Constructor
AsyncSync::AsyncSync( void )
{
#ifndef QT_THREAD_SUPPORT
  // Create IPC pipe for async/sync communication with X server
#ifdef _WIN32
  if( _pipe( fds, 32, O_BINARY ) != 0 )
#else
  if ( ::pipe( fds ) == -1 )
#endif
    {
      perror( "Creating pipe" );
      fds[0] = fds[1] = -1;
    }

#ifdef USE_QXT
  // dans ce cas c'est Xt qui se charge de déclencher un événement
  xtInpId = XtAppAddInput( theApplication->AppContext(), fds[0], 
			   (XtPointer) XtInputReadMask, &xtPipeCallback, 
			   this );
#else
  // Create new socket notifier to monitory when data is ready to be
  // read from pipe
  sn = new QSocketNotifier( fds[0], QSocketNotifier::Read );

  // Connect up the socket notifier's activated routine to the dequeue
  // any new clients added to Database
  connect( sn, SIGNAL( activated( int ) ), SLOT( syncHandler() ) );
#endif
#endif
}


#ifdef USE_QXT
void AsyncSync::xtPipeCallback( void* caller, int*, XtInputId * )
{
  ((AsyncSync *) caller)->syncHandler();
}
#endif


// Destructor  
AsyncSync::~AsyncSync( void )
{
#ifndef QT_THREAD_SUPPORT
#ifdef USE_QXT
  XtRemoveInput( xtInpId );
#else
  // Delete socket notifier
  delete sn;
#endif

  // Close pipe file descriptors
  if ( ::close( fds[0] ) == -1 )
    {
      perror( "Closing read file descriptor" );
    }

  if ( ::close( fds[1] ) == -1 )
    {
      perror( "Closing writing file descriptor" );
    }
#endif
}

// Slot called synchronously by the X server in response to the
// asynchronous file descriptor it is watching having data ready.
void AsyncSync::syncHandler( void )
{
  //cout << "AsyncSync::syncHandler\n";
#ifndef QT_THREAD_SUPPORT
  // First remove message from pipe ( the writer only wrote 1 byte )
  static char buf;
  if ( ::read( fds[0], &buf, 1 ) == -1 )
    {
      ::perror( "Reading from pipe" );
    }
#endif

  // Now emit activated signal, and let user decide what to do
  emit activated();
}


// Slot to be called by asynchronous client.  This slot does not more
// than write to pipe, which will trigger the X server to respond
// to the file descriptor, and synchronously call the SyncHandler
void AsyncSync::asyncHandler( void )
{
  //cout << "AsyncSync::asyncHandler\n";
#ifdef QT_THREAD_SUPPORT
  // in threaded Qt, signals can pass through threads
  QApplication::postEvent( this, new QCustomEvent( QEvent::User + 103 ) );

#else
  // Just send a single byte of data;
  static const char *buf = "";
  if ( ::write( fds[1], buf, 1 ) == -1 )
    {
      ::perror( "Writing to pipe" );
    }
#endif
}

#ifdef QT_THREAD_SUPPORT
void AsyncSync::customEvent( QCustomEvent* )
{
  emit activated();
}
#endif


#undef USE_QXT


//------ eof ------
