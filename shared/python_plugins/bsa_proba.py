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

'''
BSA atlas probabilities
'''

try:
    # python3
    from urllib.request import urlopen, Request
except ImportError:
    # python2
    from urllib2 import urlopen, Request
import ctypes
from soma import aims
import numpy
import anatomist.api as ana
from soma.qt_gui.qt_backend import QtCore
from soma.qt_gui.qt_backend import QtGui
Qt = QtCore.Qt

bsa_url = 'http://static.brainvisa.info/bsa/base2008_global/bsa_2008_global_atlas.nii'
labelsfile = 'http://static.brainvisa.info/bsa/base2008_global/bsa_2008_global_atlas.csv'
use_multirange = True

labels = None


def bsaClickHandler(eventName, params):
  pos=params['position']
  win=params['window']
  a = ana.Anatomist()
  wref = win.getReferential()
  mniref = a.mniTemplateRef
  tr = a.getTransformation( wref, mniref )
  bsaw = BSAWindow._instance
  if bsaw is None:
    return
  lw = bsaw.centralWidget()
  if tr is None and wref != mniref:
    text = '<html>Window has no transformation to the MNI space referential.</html>'
    lw.setText( text )
    return

  if tr is not None:
    pos = tr.transform( pos[:3] )

  hdrsz = 352
  imgdim = [ 91, 109, 91, 145 ]
  vs = aims.AffineTransformation3d( [ 2., 0, 0, 0, 0, 2., 0, 0, 0, 0, 2., 0, 0, 0, 0, 1 ] )
  s2m = aims.AffineTransformation3d( [ 1, 0, 0, 0, 0, -1, 0, 108, 0, 0, -1, 90, 0, 0, 0, 1 ] )
  m2s = s2m.inverse()
  m2mni = aims.AffineTransformation3d( [ -1, 0, 0, 90, 0, -1, 0, 90, 0, 0, -1, 108, 0, 0, 0, 1 ] )
  mni2s = ( m2mni * vs * s2m ).inverse()
  posvox = mni2s.transform( pos[:3] )
  posvox = [ int(round(x)) for x in posvox ]
  if posvox[0] < 0 or posvox[0] >= imgdim[0] \
    or posvox[1] < 0 or posvox[1] >= imgdim[1] \
    or posvox[2] < 0 or posvox[2] >= imgdim[2]:
      lw.setText( '<html>Out of atlas space.</html>' )
      return
  offset0 = ( posvox[0] + posvox[1] * imgdim[0] + posvox[2] * imgdim[0]*imgdim[1] ) *4 + hdrsz
  offsets = [ t * imgdim[0]*imgdim[1]*imgdim[2]*4 + offset0 for t in xrange(imgdim[3]) ]
  req = Request( bsa_url )

  if use_multirange:
    req.headers['Range'] = 'bytes=' + ','.join( [ '%s-%s' % (offset, offset+3) \
      for offset in offsets ] )
    f = urlopen(req)
    values = f.read()
    del f
    poff = 0
    probs = []
    for i in range( imgdim[3] ):
      p = values.find( 'Content-range: bytes', poff )
      if p < 0:
        break
      p = values.find( '\n', p )
      if p < 0:
        break
      p = values.find( '\n', p+1 )
      if p < 0:
        break
      val = values[p+1:p+5]
      poff = p+6
      # WARNING TODO: take byte order into account !
      value = ctypes.cast( val, ctypes.POINTER( ctypes.c_float ) )[0]
      probs.append( value )

  else: # slower
    probs = []
    for offset in offsets:
      req.headers['Range'] = 'bytes=%s-%s' % (offset, offset+3)
      f = urlopen(req)
      values = f.read()
      del f
      # WARNING TODO: take byte order into account !
      value = ctypes.cast( values, ctypes.POINTER( ctypes.c_float ) )[0]
      probs.append( value )

  global labels
  if labels is None:
    labels = urlopen( labelsfile ).readlines()
    labels = [ l.strip().split(',') for l in labels[1:] ]

  text = '<html><p>Position: <b>%f, %f, %f</b></p><p><table><tr><td><b>index:</b></td><td><b>proba:</b></td><td><b>label:</b></td><td><b>common name:</b></td></tr>' % tuple( pos[:3] )
  sp = numpy.argsort( probs )
  for i in range(len(probs)-1,-1,-1):
    if probs[sp[i]] >= 1e-6:
      text += '<tr><td>%d&nbsp;</td><td>%f&nbsp;</td><td>%s&nbsp;</td><td>%s</td></tr>' \
        % ( sp[i], probs[sp[i]], labels[sp[i]][7], labels[sp[i]][8] )
  text += '</p></html>'
  lw.setText( text )



class BSAWindow( ana.cpp.QAWindow ):
  _instance = None
  _classType = ana.cpp.AWindow.Type( ana.cpp.AWindowFactory.types().size() )

  def __init__( self, parent=None, name=None, options=aims.Object(), f=None ):
    '''The releaseref() method should be called after the constructor - see
    the doc of this method.
    It is not called from the constructor for technical anatomist IDs problems
    (which may be solved).
    '''
    if f is None:
      f = Qt.WindowFlags( Qt.Window )
    ana.cpp.QAWindow.__init__( self, parent, name, options, f )
    self.setAttribute( Qt.WA_DeleteOnClose )
    wid = QtGui.QLabel( None )
    self.setCentralWidget( wid )
    # keep a reference to the python object to prevent destruction of the
    # python part
    BSAWindow._instance = self
    self.connect( self, QtCore.SIGNAL( 'destroyed()' ), self.destroyNotified )
    a = ana.Anatomist()
    # register the function on the cursor notifier of anatomist. It will be called when the user click on a window
    a.onCursorNotifier.add( bsaClickHandler )

  def releaseref( self ):
    '''WARNING:
    the instance in _instance shouldn't count on C++ side
    PROBLEM: all python refs are one unique ref for C++,
    all being of the same type, so later references will not be strong refs.
    the less annoying workaround at the moment is that python refs are
    'weak shared references': count as references to keep the object alive,
    but don't actually prevent its destruction whenever the close method
    or anatomist destroy command are called. In such case the python object
    will hold a deleted C++ object.
    This way, only C++ may destroy the object.
    When the C++ instance is destroyed, the QObject destroyed callback is
    used to cleanup the additional python reference in BSAWindow._instance
    so that the python instance can also be destroyed when python doesn't
    use it any longer.
    That's the best I can do for now...
    This releaseref method should be called after the constructor: it is
    called from the createHistogramWindow factory class.
    this means you should _not_ create an instance of AHistogram directly.'''
    a = ana.Anatomist()
    a.execute( 'ExternalReference', elements=[self],
      action_type='TakeWeakSharedRef' )
    a.execute( 'ExternalReference', elements=[self],
      action_type='ReleaseStrongRef' )

  def __del__( self ):
    ana.cpp.QAWindow.__del__( self )
    a = ana.Anatomist()
    a.onCursorNotifier.remove( bsaClickHandler )

  def destroyNotified( self ):
    # release internal reference which kept the python side of the object
    # alive - now the python object may be destroyed since the C++ side
    # will be also destroyed anyway.
    if self == BSAWindow._instance:
      BSAWindow._instance = None

  def type( self ):
    return self._classType;

  def registerObject( self, obj, temporaryObject=False, position=-1 ):
    pass

  def unregisterObject( self, obj ):
    pass

  def baseTitle( self ):
    return 'BSA atlas probabilities'


class BSAModule( ana.cpp.Module ):
  def name(self):
    return 'BSA probabilities module'

  def description(self):
    return __doc__

class createBSAWindow( ana.cpp.AWindowCreator ):
  def __call__( self, dock, options ):
    if BSAWindow._instance is not None:
      return None
    h = BSAWindow()
    h.releaseref()
    h.show()
    return h

createbsa = createBSAWindow()

def init():
  ana.cpp.AWindowFactory.registerType( 'Brain Sulci Atlas probabilities window',
    createbsa )


hm = BSAModule()
init()


