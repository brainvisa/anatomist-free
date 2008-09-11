#! /bin/env python

import sys, os, weakref, gc, operator
import qt
import anatomist.direct.api as anatomist
from soma import aims

import matplotlib, numpy
matplotlib.use('Agg')
import pylab

from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


class MeasuresWindow( qt.QSplitter ):
  def __init__( self, fileName, roiIterator=None, parent=None, name=None,
                anatomistInstance=None ):
    qt.QSplitter.__init__( self, qt.Qt.Horizontal, parent, name )
    if anatomistInstance is None:
      # initialize Anatomist
      self.anatomist = anatomist.Anatomist()
    else:
      self.anatomist = anatomistInstance

    # open an axial window
    self.aWindow = self.anatomist.createWindow( 'Axial', no_decoration=True )
    self.aWindow.reparent( self, 0, qt.QPoint(0,0), True )

    if roiIterator is not None:
      self.roiList = qt.QListBox( self )
      self.maskIterators = []
      # Iterate on each region
      while roiIterator.isValid():
        self.roiList.insertItem( roiIterator.regionName() )
        maskIterator = roiIterator.maskIterator().get()
        maskIterator.bucket = None
        self.maskIterators.append( maskIterator )
        roiIterator.next()
      self.selectedBucket = None
      self.connect( self.roiList, qt.SIGNAL( 'selected( int )' ), self.regionSelected )
    else:
      self.roiList = None
    
    self.infoSplitter = qt.QSplitter( qt.Qt.Vertical, self )
    self.info = qt.QTextEdit( self.infoSplitter )
    self.info.setReadOnly( True )
    self.info.setSizePolicy( qt.QSizePolicy.Preferred,
      qt.QSizePolicy.Preferred )


    self.matplotFigure = Figure()
    self.matplotAxes = self.matplotFigure.add_subplot(111)
    # We want the axes cleared every time plot() is called
    self.matplotAxes.hold(False)
    
    self.matplotCanvas = FigureCanvas( self.matplotFigure )
    self.matplotCanvas.reparent( self.infoSplitter, qt.QPoint( 0, 0 ) )
    self.matplotCanvas.setSizePolicy( qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding )
    self.matplotCanvas.updateGeometry()
      
    self.anatomist.onCursorNotifier.add( self.clicked2 )
    
    self.resize( 800, 600 )

    # Read the image
    dir, base = os.path.split( fileName )
    if dir:
      self.setCaption( base + ' (' + dir + ')' )
    else:
      self.setCaption( base )
    # load any volume as a aims.Volume_* object
    r = aims.Reader( {'Volume' : 'AimsData'} )
    self.volume = r.read( fileName )
    self.interpolator = aims.aims.getLinearInterpolator( self.volume ).get()

    # convert the AimsData volume to Anatomist API
    avol = self.anatomist.toAObject( self.volume )
    # put volume in window
    self.anatomist.addObjects( [ avol ], [ self.aWindow ] )

    self._ignoreClicked = False
    voxelSize = self.volume.header()[ 'voxel_size' ]
    volumeSize = [int(i) \
      for i in self.volume.header()[ 'volume_dimension' ]]
    volumeCenter = [v*s/2 for v,s in zip( volumeSize, voxelSize )]
    self.clicked( volumeCenter )

    infoHeight = self.info.sizeHint().height()
    self.infoSplitter.setSizes( [ infoHeight, self.height() - infoHeight ] )


  def clicked2( self, eventName, eventParameters ):
    self.clicked( eventParameters[ 'position' ], eventParameters[ 'window' ] )
  
  
  def clicked( self, posMM, aWindow=None ):
    posMM = [float(i) for i in posMM]
    if self._ignoreClicked: return
    text = '<html><body>\n'
    text += '<b>Coordinate millimeters:</b> %.2f, %.2f, %.2f, %.2f' % tuple( posMM ) + '<br/>\n'
    voxelSize = self.volume.header()[ 'voxel_size' ]
    posVoxel = [int(round(i/j)) for i,j in zip(posMM,voxelSize)]
    text += '<b>Coordinate voxels:</b> %d, %d, %d, %d' % tuple( posVoxel ) + '<br/>\n'
    volumeSize = [int(i) for i in self.volume.header()[ 'volume_dimension' ]]
    if not [None for i in posVoxel if i < 0] and  \
       not [None for i,j in zip(posVoxel, volumeSize) if i >= j]:
      text += '<b>Voxel value</b>: ' + str( self.volume.value( *posVoxel ) ) + '<br/>\n' 
      if volumeSize[ 3 ] > 1:
        indices = numpy.arange( volumeSize[ 3 ] )
        # Extract values as numarray structure
        values = self.interpolator.values( posVoxel[0] * voxelSize[0], 
                                           posVoxel[1] * voxelSize[1], 
                                           posVoxel[2] * voxelSize[2] )
        self.matplotAxes.plot( indices, numpy.array( values ) )
        self.matplotCanvas.draw()
    text += '</body></html>'
    self.info.setText( text )


  def regionSelected( self ):
    index = self.roiList.currentItem()
    if index >= 0:
      text = '<html><body>\n'
      text += '<h2>' + unicode( self.roiList.text( index ) ) + '</h2>\n'

      maskIterator = self.maskIterators[ index ]
      if maskIterator.bucket is None:
        roiCenter = aims.Point3df(0, 0, 0)
        bucket = aims.BucketMap_VOID()
        bucket.setSizeXYZT( *maskIterator.voxelSize().items() + (1,) )
        maskIterator.restart()
        valid = 0
        invalid = 0
        sum = None
        # Iterate on each point of a region
        while maskIterator.isValid():
          bucket[ 0 ][ maskIterator.value() ] = 1
          p = maskIterator.valueMillimeters()
          roiCenter += p
          # Check if the point is in the image limit
          if self.interpolator.isValid( p ):
            values = self.interpolator.values( p )
            if sum is None:
              sum = values
            else:
              sum = [s+v for s,v in zip(sum,values)]
            valid += 1
          else:
            invalid += 1
          maskIterator.next()
        text += '<b>valid points:</b> ' + str( valid ) + '<br/>\n'
        text += '<b>invalid points:</b> ' + str( invalid ) + '<br/>\n'
        if valid:
          means = [s / float( valid ) for s in sum]
          mean = reduce( operator.add, means ) / len( means )
        else:
          means = []
          mean = 'N/A'
        text += '<b>mean:</b> ' + str( mean ) + '<br/>\n'
        text += '</body></html>'
        maskIterator.text = text
        # convert the BucketMap to Anatomist API
        maskIterator.bucket = self.anatomist.toAObject( bucket )
        maskIterator.bucket.setName( str( self.roiList.text( index ) ) )
        maskIterator.bucket.setChanged()
        count = valid + invalid
        if count:
          maskIterator.roiCenter = [c/count for c in roiCenter.items()]
        else:
          maskIterator.roiCenter = None
        maskIterator.means = means

      # put bucket in window
      self.info.setText( maskIterator.text )
      self.aWindow.addObjects( [ maskIterator.bucket ] )
      # Set selected color to bucket
      maskIterator.bucket.setMaterial( self.anatomist.Material(
                              diffuse = [ 1, 0, 0, 0.5 ],
                              lighting = 0,
                              face_culling = 1,
                              ) )
      # Set unselected color to previously selected bucket
      if self.selectedBucket is not None:
        self.selectedBucket.setMaterial( self.anatomist.Material( diffuse=[0,0.8,0.8,0.8] ) )
      self.selectedBucket = maskIterator.bucket
      if maskIterator.roiCenter is not None:
        self._ignoreClicked = True
        self.aWindow.moveLinkedCursor( maskIterator.roiCenter )
        self._ignoreClicked = False
      if len( maskIterator.means ) > 1:
        indices = numpy.arange( len( maskIterator.means ) )
        self.matplotAxes.plot( indices, numpy.array( maskIterator.means ) )
        self.matplotCanvas.draw()

if __name__ == '__main__':
  qApp = qt.QApplication( sys.argv )

  if len( sys.argv ) == 3:
    roiIterator = aims.aims.getRoiIterator( sys.argv[ 2 ] ).get()
    w = MeasuresWindow( sys.argv[ 1 ], roiIterator=roiIterator )
  else:
    w = MeasuresWindow( sys.argv[ 1 ] )
  w.show()

  qApp.setMainWidget( w )
  anatomist.Anatomist().getControlWindow().hide()
  qApp.exec_loop()
