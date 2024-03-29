# -*- coding: utf-8 -*-
#  This software and supporting documentation are distributed by
#      Institut Federatif de Recherche 49
#      CEA/NeuroSpin, Batiment 145,
#      91191 Gif-sur-Yvette cedex
#      France
#
# This software is governed by the CeCILL-B license under
# French law and abiding by the rules of distribution of free software.
# You can  use, modify and/or redistribute the software under the
# terms of the CeCILL-B license as circulated by CEA, CNRS
# and INRIA at the following URL "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or
# data to be ensured and,  more generally, to use and operate it in the
# same conditions as regards security.
#
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL-B license and that you accept its terms.

# a test example for python plugin in Anatomist (Pyanatomist module)

"""Python menu in Anatomist control window
This module adds a menu "Python" in Anatomist control window, with a few
options dealing with python modules:
* list currnently imported python modules
* open a python shell in a threaded window
* run a python program file in Anatomist
"""

import sys
import os
import anatomist.cpp as anatomist
from soma.qt_gui import qt_backend
from soma.qt_gui.qt_backend.QtCore import *
from soma.qt_gui.qt_backend.QtGui import *
from soma.qt_gui.qt_backend import QtGui
import threading

import six
from six.moves import range

Slot = pyqtSlot


consoleShellRunning = False
_ipsubprocs_lock = threading.RLock()
_ipsubprocs = []
pythonscriptloader = None


class PyAnatomistModule(anatomist.Module):

    def name(self):
        return 'Python menu'

    def description(self):
        return __doc__


a = anatomist.Anatomist()


def pyCuteShell():
    # mostly copied from PyCute executable from PyQwt
    try:
        if qVersion()[0] == '3':
            from qwt.PyCute3 import PyCute
        elif qVersion()[0] == '2':
            from qwt.PyCute2 import PyCute
        else:  # Qt >= 4 (or <= 1)
            return None
    except:
        return None

    class AnaPyCuteShell(PyCute):

        def keyPressEvent(self, e):
            if e.state() & Qt.ControlButton and e.key() == self.eofKey:
                # don't exit the app but close the shell window
                self.close()
                del sys.modules['__main__'].__w__
            else:
                PyCute.keyPressEvent(self, e)

    # locals = .. -- make all names in __main__ visible to the PyCute shell
    # log    = .. -- save session in 'log'
    locals = sys.modules['__main__']
    locals.___w___ = AnaPyCuteShell(locals.__dict__)  # , log='log')
    locals.___w___.show()
    return 1


def fixMatplotlib():
    # fix matplotlib if if is already loaded
    if 'matplotlib' in sys.modules:
        # print 'fixing matplotlib'
        try:
            from soma.qt_gui.qt_backend import init_matplotlib_backend
            init_matplotlib_backend()
            import matplotlib.backends
            if hasattr(matplotlib.backends, 'pylab_setup'):
                backend_conf = matplotlib.backends.pylab_setup()
                if len(backend_conf) <= 3:
                    # seems not to exist any longer in mpl 1.3
                    matplotlib.backends.new_figure_manager = backend_conf[0]
                    matplotlib.backends.draw_if_interactive = backend_conf[1]
                    matplotlib.backends.show = backend_conf[2]
        except Exception as e:
            print('exception:', e)
            pass


class _ProcDeleter(threading.Thread):

    def __init__(self, o):
        threading.Thread.__init__(self)
        self.o = o

    def __del__(self):
        try:
            self.o.kill()
        except Exception:
            pass
        if getattr(self, 'console', False):
            global consoleShellRunning
            consoleShellRunning = False

    def run(self):
        try:
            self.o.communicate()
        except Exception as e:
            print('exception in ipython process:', e)
        global _ipsubprocs
        try:
            with _ipsubprocs_lock:
                _ipsubprocs.remove(self)
        except Exception:
            pass


try:
    from zmq.eventloop import ioloop
    from zmq.eventloop.ioloop import IOLoop
    have_zmq = True
except ImportError:
    have_zmq = False
import signal
import threading
# import thread
import logging
import heapq


def _my_ioloop_start(self):
        # this is a hacked version of tornado.ioloop.PollIOLoop.start()
        # with smaller timeout (2 instead of 3600 s)
        # and breaks when idle with this max timeout
        # (used only with ipython 1.x)
    if not logging.getLogger().handlers:
        # The IOLoop catches and logs exceptions, so it's
        # important that log output be visible.  However, python's
        # default behavior for non-root loggers (prior to python
        # 3.2) is to print an unhelpful "no handlers could be
        # found" message rather than the actual log entry, so we
        # must explicitly configure logging if we've made it this
        # far without anything.
        logging.basicConfig()
    if self._stopped:
        self._stopped = False
        return
    old_current = getattr(IOLoop._current, "instance", None)
    IOLoop._current.instance = self
    # self._thread_ident = thread.get_ident()
    self._running = True

    # signal.set_wakeup_fd closes a race condition in event loops:
    # a signal may arrive at the beginning of select/poll/etc
    # before it goes into its interruptible sleep, so the signal
    # will be consumed without waking the select.  The solution is
    # for the (C, synchronous) signal handler to write to a pipe,
    # which will then be seen by select.
    #
    # In python's signal handling semantics, this only matters on the
    # main thread (fortunately, set_wakeup_fd only works on the main
    # thread and will raise a ValueError otherwise).
    #
    # If someone has already set a wakeup fd, we don't want to
    # disturb it.  This is an issue for twisted, which does its
    # SIGCHILD processing in response to its own wakeup fd being
    # written to.  As long as the wakeup fd is registered on the IOLoop,
    # the loop will still wake up and everything should work.
    old_wakeup_fd = None
    if os.name == 'posix':
        # Requires Unix.  set_wakeup_fd exists but crashes
        # the python process on windows.
        try:
            old_wakeup_fd = signal.set_wakeup_fd(self._waker.write_fileno())
            if old_wakeup_fd != -1:
                # Already set, restore previous value.  This is a little racy,
                # but there's no clean get_wakeup_fd and in real use the
                # IOLoop is just started once at the beginning.
                signal.set_wakeup_fd(old_wakeup_fd)
                old_wakeup_fd = None
        except ValueError:  # non-main thread
            pass

    iter = 0
    while True:
        do_break = True
        poll_timeout = 2.0

        # Prevent IO event starvation by delaying new callbacks
        # to the next iteration of the event loop.
        with self._callback_lock:
            callbacks = self._callbacks
            self._callbacks = []
        for callback in callbacks:
            do_berak = False
            self._run_callback(callback)

        if self._timeouts:
            now = self.time()
            while self._timeouts:
                if self._timeouts[0].callback is None:
                    # the timeout was cancelled
                    do_break = False
                    heapq.heappop(self._timeouts)
                    self._cancellations -= 1
                elif self._timeouts[0].deadline <= now:
                    do_break = False
                    timeout = heapq.heappop(self._timeouts)
                    self._run_callback(timeout.callback)
                else:
                    seconds = self._timeouts[0].deadline - now
                    poll_timeout = min(seconds, poll_timeout)
                    if poll_timeout < 2.:
                        do_break = False
                    break
            if (self._cancellations > 512
                    and self._cancellations > (len(self._timeouts) >> 1)):
                # Clean up the timeout queue when it gets large and it's
                # more than half cancellations.
                self._cancellations = 0
                self._timeouts = [x for x in self._timeouts
                                  if x.callback is not None]
                heapq.heapify(self._timeouts)

        if self._callbacks:
            do_break = False
            # If any callbacks or timeouts called add_callback,
            # we don't want to wait in poll() before we run them.
            poll_timeout = 0.0

        if not self._running:
            break

        if self._blocking_signal_threshold is not None:
            # clear alarm so it doesn't fire while poll is waiting for
            # events.
            signal.setitimer(signal.ITIMER_REAL, 0, 0)

        if do_break:
            break  # END LOOP NOW.

        try:
            event_pairs = self._impl.poll(poll_timeout)
        except Exception as e:
            # Depending on python version and IOLoop implementation,
            # different exception types may be thrown and there are
            # two ways EINTR might be signaled:
            # * e.errno == errno.EINTR
            # * e.args is like (errno.EINTR, 'Interrupted system call')
            if (getattr(e, 'errno', None) == errno.EINTR or
                (isinstance(getattr(e, 'args', None), tuple) and
                 len(e.args) == 2 and e.args[0] == errno.EINTR)):
                continue
            else:
                raise

        if self._blocking_signal_threshold is not None:
            signal.setitimer(signal.ITIMER_REAL,
                             self._blocking_signal_threshold, 0)

        # Pop one fd at a time from the set of pending fds and run
        # its handler. Since that handler may perform actions on
        # other file descriptors, there may be reentrant calls to
        # this IOLoop that update self._events
        self._events.update(event_pairs)
        while self._events:
            fd, events = self._events.popitem()
            try:
                #open('/tmp/analog.txt', 'w').write(
                    #'handlers[fd]: %s, fd: %d\n'
                    #% (repr(self._handlers[fd]), fd))
                handler = self._handlers[fd]
                if isinstance(handler, tuple):
                    # tornado >= 4
                    handler[1](fd, events)
                else:
                    # older tornado <= 3
                    handler(fd, events)
            except (OSError, IOError) as e:
                if e.args[0] == errno.EPIPE:
                    # Happens when the client closes the connection
                    pass
                else:
                    raise
                    # app_log.error("Exception in I/O handler for fd %s",
                                  # fd, exc_info=True)
            # except Exception:
                # app_log.error("Exception in I/O handler for fd %s",
                              # fd, exc_info=True)

    # reset the stopped flag so another start/stop pair can be issued
    self._stopped = False
    if self._blocking_signal_threshold is not None:
        signal.setitimer(signal.ITIMER_REAL, 0, 0)
    IOLoop._current.instance = old_current
    if old_wakeup_fd is not None:
        signal.set_wakeup_fd(old_wakeup_fd)


def runIPConsoleKernel(mode='qtconsole'):
    import IPython
    from IPython.lib import guisupport
    qtapp = QtGui.QApplication.instance()
    qtapp._in_event_loop = True
    guisupport.in_event_loop = True
    ipversion = [int(x) for x in IPython.__version__.split('.')]

    if False:  # ipversion >= [3, 0]:
    # embedded ipython engine + qt loop in the same process.
    # works for ipython >= 3 but forbids connection from outside
    # so it is not so interesting after all.
        qt_api = qt_backend.get_qt_backend().lower()
        if qt_api == 'pyqt4':
            qt_api = 'pyqt'
        os.environ['QT_API'] = qt_api
        from IPython.qt.inprocess import QtInProcessKernelManager
        kernel_manager = QtInProcessKernelManager()
        kernel_manager.start_kernel()
        kernel = kernel_manager.kernel
        if qt_api == 'pyqt4':
            kernel.gui = 'qt4'
        elif qt_api == 'pyqt5':
            kernel.gui = 'qt5'
        elif qt_api == 'pyqt6':
            kernel.gui = 'qt6'
        # kernel.shell.push({'foo': 43, 'print_process_id': print_process_id})

        kernel_client = kernel_manager.client()
        kernel_client.start_channels()

        def stop():
            kernel_client.stop_channels()
            kernel_manager.shutdown_kernel()

        from IPython.qt.console.rich_ipython_widget import RichIPythonWidget
        control = RichIPythonWidget()
        control.kernel_manager = kernel_manager
        control.kernel_client = kernel_client
        control.exit_requested.connect(stop)
        control.show()
        return None

    elif ipversion >= [1, 0]:
        if not have_zmq:
            return None  # no zmq: fail
        if ipversion >= [4, 0]:
            from ipykernel.kernelapp import IPKernelApp
        else:
            from IPython.kernel.zmq.kernelapp import IPKernelApp
        app = IPKernelApp.instance()
        if not app.initialized() or not app.kernel:
            print('runing IP console kernel')
            app.hb_port = 50042  # don't know why this is not set automatically
            app.initialize([mode, '--gui=qt', # '--pylab=qt',
                            "--KernelApp.parent_appname='ipython-%s'" % mode])
            # in ipython >= 1.2, app.start() blocks until a ctrl-c is issued in
            # the terminal. Seems to block in tornado.ioloop.PollIOLoop.start()
            #
            # So, don't call app.start because it would begin a zmq/tornado loop
            # instead we must just initialize its callback.
            # if app.poller is not None:
                # app.poller.start()
            app.kernel.start()

            from zmq.eventloop import ioloop
            if ipversion >= [2, 0]:
                # IP 2 allows just calling the current callbacks.
                # For IP 1 it is not sufficient.
                import tornado
                if tornado.version_info >= (4, 5):
                    # tornado 5 is using a decque for _callbacks, not a
                    # list + explicit locking
                    def my_start_ioloop_callbacks(self):
                        if hasattr(self, '_callbacks'):
                            ncallbacks = len(self._callbacks)
                            for i in range(ncallbacks):
                                self._run_callback(self._callbacks.popleft())
                else:
                    def my_start_ioloop_callbacks(self):
                        with self._callback_lock:
                            callbacks = self._callbacks
                            self._callbacks = []
                        for callback in callbacks:
                            self._run_callback(callback)

                my_start_ioloop_callbacks(ioloop.IOLoop.instance())
            else:
                # For IP 1, use the hacked copy of the start() method
                try:
                    _my_ioloop_start(ioloop.IOLoop.instance())
                except KeyboardInterrupt:
                    pass

        if hasattr(sys, '__ip_stdout'):
            sys.stdout = sys.__ip_stdout
            sys.stderr = sys.__ip_stderr
            del sys.__ip_stdout
            del sys.__ip_stderr
        return app

    else:
        # ipython 0.x API
        from IPython.zmq.ipkernel import IPKernelApp
        app = IPKernelApp.instance()
        if not app.initialized() or not app.kernel:
            print('runing IP console kernel')
            app.hb_port = 50042  # don't know why this is not set automatically
            app.initialize(['qtconsole', '--pylab=qt',
                            "--KernelApp.parent_appname='ipython-qtconsole'"])
            app.start()
        return app


def ipythonShell(mode='qtconsole'):
    try:
        import jupyter_core.application
        ipfunc = 'from jupyter_core import application; ' \
            'app = application.JupyterApp(); app.initialize(); app.start()'
    except ImportError:
        try:
            import IPython
            ipversion = [int(x) for x in IPython.__version__.split('.')]
            if ipversion >= [0, 11]:
                # ipython >= 0.11, use client/server mode
                print('ipversion:', ipversion)
                if ipversion >= [1, 0]:
                    ipmodule = 'IPython.terminal.ipapp'
                else:
                    ipmodule = 'IPython.frontend.terminal.ipapp'
                ipfunc = 'from %s import launch_new_instance; ' \
                    'launch_new_instance()' % ipmodule
        except Exception:
            print('failed to run jupyter console')
            return 0

    fixMatplotlib()

    global _ipsubprocs
    if ipfunc:
        import soma.subprocess

        ipConsole = runIPConsoleKernel(mode)
        exe = sys.executable
        if sys.platform == 'darwin':
            exe = 'python'
        if ipConsole:
            qt_api = qt_backend.get_qt_backend()
            qt_apis = {'PyQt4': 'pyqt', 'PyQt5': 'pyqt5', 'PySide': 'pyside',
                       'PyQt6': 'pyqt6'}
            qt_api_code = qt_apis.get(qt_api, qt_api.lower())
            cmd = [exe, '-c',
                   'import os; os.environ["QT_API"] = "%s"; %s'
                   % (qt_api_code, ipfunc),
                   mode, '--existing',
                   '--shell=%d' % ipConsole.shell_port,
                   '--iopub=%d' % ipConsole.iopub_port,
                   '--stdin=%d' % ipConsole.stdin_port,
                   '--hb=%d' % ipConsole.hb_port]
            sp = soma.subprocess.Popen(cmd)
            pd = _ProcDeleter(sp)
            with _ipsubprocs_lock:
                _ipsubprocs.append(pd)
            pd.start()
        return 1
    return 0


def clean_ipsubprocs():
    global _ipsubprocs
    with _ipsubprocs_lock:
        _ipsubprocs = []
    import gc
    gc.collect()

    # unregister readers
    anatomist.ObjectReader.cleanup()
    global pythonscriptloader
    pythonscriptloader = None


#def child_exited(sig, frame):
    #print('** child exited:', sig, '**')
    #global _ipsubprocs
    #todel = []
    #with _ipsubprocs_lock:
        #for proc in _ipsubprocs:
            #proc.o.poll()
            #if proc.o.returncode is not None:
                #todel.append(proc)
                #proc.o.wait()
        #for proc in todel:
            #_ipsubprocs.remove(proc)
    #global old_sigchld
    #print('old_sigchld:', old_sigchld)
    #if old_sigchld:
        #print('calling old sigchld:')
        ##old_sigchld(sig, frame)
        #old_sigchld(sig)


def ipythonQtConsoleShell():
    return ipythonShell('qtconsole')


def ipythonNotebook():
    # does not work anyway...
    return ipythonShell('notebook')
    #try:
        #import IPython
        #fixMatplotlib()
    #except:
        #return 0
    #ipversion = [int(x) for x in IPython.__version__.split('.')]
    #if ipversion < [0, 11]:
        #return 0  # Qt console does not exist in ipython <= 0.10
    #ipConsole = runIPConsoleKernel('notebook')


def ipythonConsoleShell():
    global consoleShellRunning
    if consoleShellRunning:
        print('console shell is already running.')
        return 1
    res = ipythonShell('console')
    if res:
        consoleShellRunning = True
        global _ipsubprocs
        with _ipsubprocs_lock:
            _ipsubprocs[-1].console = True

    return res

def pythonShell():
    global consoleShellRunning
    if consoleShellRunning:
        print('console shell is already running.')
        return 1
    try:
        import code
        # try Qwt hook into Qt loop
        # (not needed with Qt 4)
        iqt = None
        try:
            # then readline
            import readline
            import rlcompleter
            readline.parse_and_bind('tab: complete')
        except:
            pass
        # run interpreter
        print('running interactive interpreter.')
        consoleShellRunning = True
        code.interact()
        consoleShellRunning = False
        print('shell terminated')
        return 1
    except:
        return 0


def pyShell():
    # import threading
    try:
        import six.moves.tkinter as Tkinter
    except:
        return 0  # no Tkinter: fails
    try:
        from idlelib.PyShell import main
    except:
        try:
            from idle.PyShell import main
        except:
            return 0
    print('PyShell imported')
    old_argv = sys.argv
    sys.argv = ['anatomist']
    # shellThread = threading.Thread( target=main )
    # shellThread.start()
    main()
    sys.argv = old_argv
    return 1


def openshell():
    if ipythonQtConsoleShell():
        return
    if pyCuteShell():
        return  # OK
    global consoleShellRunning
    if consoleShellRunning:
        print('console shell is already running.')
        return
    if ipythonConsoleShell():
        return
    if pyShell():
        return
    print('No shell available. Sorry.')


def listmods():
    print('python modules:')
    print()
    sz = 0
    for x in anatomist.loaded_modules:
        try:
            exec('import ' + x)
            sz = max(sz, len(x))
        except ImportError:
            pass
    for x in anatomist.loaded_modules:
        try:
            s = ' ' * (sz - len(x))
            print(' ', x, s + ':', eval(x + '.__file__'))
            descr = eval(x + '.__doc__')
            if descr is not None:
                print(descr)
            print()
        except Exception:
            pass


def loadpython():
    print('load python file')
    file = QFileDialog.getOpenFileName(
        None, '*.py', options=QFileDialog.DontUseNativeDialog)
    if file is not None:
        import sip
        if sip.getapi('QString') == 1:
            file = file.toLocal8Bit().data()
        with open(file, 'rb') as f:
            code = compile(f.read(), file, 'exec')
        six.exec_(code)


class PythonScriptRun(anatomist.ObjectReader.LoadFunctionClass):

    @staticmethod
    def run(filename, options):
        print('run:', filename, 'with options:', options)
        try:
            a.theProcessor().allowExecWhileIdle(True)
            with open(filename, 'rb') as f:
                code = compile(f.read(), f.name, 'exec')
            six.exec_(code)
        except Exception as e:
            import traceback
            import sys
            sys.stdout.flush()
            sys.stderr.flush()
            print(e, file=sys.stderr)
            traceback.print_stack()
            sys.stderr.flush()
        a.theProcessor().allowExecWhileIdle(False)
        return []

    def load(self, filename, subobjects, options):
        import threading
        if not isinstance(threading.current_thread(), threading._MainThread):
            print('warning, running python script in an arbitrary thread')
        res = self.run(filename, options)
        return res


# class ExecutePythonCommand( anatomist.WaitCommand ):
    # def __init__( self, filename ):
        # self._filename = filename
    # def name( self ):
        # return 'ExecutePython'
    # def write( self, context ):
        # from soma import aims
        # obj = aims.Object( { '__syntax__' : 'ExecutePythonCommand' } )
        # obj[ 'filename' ] = self._filename
        # return obj
    # def doit( self ):
        # try:
            # execfile( self._filename )
        # except Exception as e:
            # import traceback, sys
            # sys.stdout.flush()
            # sys.stderr.flush()
            # print >> sys.stderr, e
            # traceback.print_stack()
            # sys.stderr.flush()

def add_gui_menus():
    cw = a.getControlWindow()
    if cw is not None:
        menu = cw.menuBar()
        p = menu.addMenu('Python')
        p.addAction('Open python shell', openshell)
        pop = p.addMenu('Specific python shells')
        ipcshell = pop.addAction('Graphical IPython shell',
                                 ipythonQtConsoleShell)
        ipshell = pop.addAction('Console IPython shell', ipythonConsoleShell)
        pcshell = pop.addAction('Pycute shell', pyCuteShell)
        pshell = pop.addAction('Console standard python shell', pythonShell)
        pyshell = pop.addAction('PyShell', pyShell)
        # ipnotebook = pop.addAction( 'IPython Notebook server', ipythonNotebook )
        p.addSeparator()
        p.addAction('list loaded python modules', listmods)
        p.addAction('run python script file...', loadpython)
        try:
            import Pycute
        except:
            pcshell.setEnabled(False)
        try:
            import IPython
            fixMatplotlib()
            if [int(x) for x in IPython.__version__.split('.')] < [0, 11]:
                ipcshell.setEnabled(False)
                # ipnotebook.setEnabled(False)
        except:
            ipcshell.setEnabled(False)
            ipshell.setEnabled(False)
            # ipnotebook.setEnabled(False)
        try:
            import six.moves.tkinter as Tkinter
            try:
                import idlelib.PyShell
            except:
                try:
                    import idle.PyShell
                except:
                    pyshell.setEnabled(False)
        except:
            pyshell.setEnabled(False)

#menu_slot = Slot(add_gui_menus)

pm = PyAnatomistModule()
QTimer.singleShot(10, add_gui_menus)
pythonscriptloader = PythonScriptRun()
anatomist.ObjectReader.registerLoader('py', pythonscriptloader)
import atexit
atexit.register(clean_ipsubprocs)  # doesn't get called anyway...
# can't make this work...
#import signal
#import ctypes
#print('get SIGCHILD')
#dll = ctypes.cdll.LoadLibrary('')
#print('call signal')
#old_sigchld = dll.signal(signal.SIGCHLD, None)
#print('old_sigchld:', old_sigchld)
#if old_sigchld != 0:
    #old_sigchld = ctypes.CFUNCTYPE(None, ctypes.c_int)(old_sigchld)
#else:
    #old_sigchld = None
#signal.signal(signal.SIGCHLD, child_exited)

