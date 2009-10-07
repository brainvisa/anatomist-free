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


#ifndef ANATOMIST_THREADS_ASYNCHANDLER_H
#define ANATOMIST_THREADS_ASYNCHANDLER_H


//#==== Forward References
class QSocketNotifier;


#include <qwidget.h>

#ifdef USE_QXT
#define String XtString
#include <X11/Intrinsic.h>
#undef String
#endif

#ifdef QT_THREAD_SUPPORT
class QCustomEvent;
#endif

/**	Communication entre threads et X/Qt.
	Brian P. Theodore" <theodore@jpsd2.std.saic.com>
	voir http://www.troll.no/qt-interest/aix7.html#aiy1
*/
class AsyncSync : public QObject
{
  Q_OBJECT

public:

  AsyncSync();
  ~AsyncSync();

protected:
#ifdef QT_THREAD_SUPPORT
  virtual void customEvent( QCustomEvent* );

#endif

protected slots:
  /** Slot called synchronously by the X server in response to the
      asynchronous file descriptor it is watching having data ready. */
  void syncHandler();

public slots:

  /** Slot to be called by asynchronous client.  This slot does not more
      than write to pipe, which will trigger the X server to respond
      to the file descriptor, and synchronously call the SyncHandler */
  void asyncHandler();

signals:

  /// Signal emitted when the sync handler gets called
  void activated();

private:
#ifdef USE_QXT
  /// Xt pipe input callback
  static void xtPipeCallback( void* caller, int *, XtInputId * );
  /// Xt input handler Id
  XtInputId	xtInpId;
#endif

#ifndef QT_THREAD_SUPPORT
  /// IPC pipe for async/sync communication with X server
  int fds[2];
  /// Socket notifier to call slot when pipe has message to read
  QSocketNotifier *sn;
#endif
};


//------ eof ------

#endif
