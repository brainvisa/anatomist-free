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
import Anatomist
import os
import quaternion
import operator

a = Anatomist.anatomist()
window = a.getInfo( windows=1 )[ 'windows' ]
if window:
  # Only the first window is taken into account
  window = window[ 0 ]
  windowInfo = a.objectInfo( objects=window )[ window ]

  meshFileNames = []
  for object in  windowInfo[ 'objects' ]:
    objectInfo = a.objectInfo( objects=object )[ object ]
    if objectInfo[ 'objectType' ] == 'SURFACE':
      meshFileNames.append( ( objectInfo[ 'filename' ], None, None ) )
    elif objectInfo[ 'objectType' ] == 'FUSION3D':
      children = objectInfo[ 'children' ]
      if len( children ) == 2:
        found = 0
        for c in children:
          oi = a.objectInfo( objects=c )[ c ]
          type = oi[ 'objectType' ]
          if type == 'SURFACE':
            meshFileName = oi[ 'filename' ]
            found += 1
          elif type == 'VOLUME':
            found += 1
            palette = oi[ 'palette' ][ 'palette' ]
        if found == 2:
          print '!!!'
          texture = '/tmp/anatomist2blender.tex'
          a.execute( 'ExportTexture', object=object, filename=texture )
          meshFileNames.append( ( meshFileName, texture, palette ) )
          
  cameraLocation = windowInfo[ 'observer_position' ]
  
  cameraLocation[ 2 ]  = -cameraLocation[ 2 ]
  m = quaternion.Quaternion( windowInfo[ 'view_quaternion' ] ).rotationMatrix()
  cameraMatrix = [ m[0:4], m[4:8], m[8:12], m[12:16] ]
#  for i in xrange( 3 ):
#    cameraMatrix[ i ][ 2 ] = -cameraMatrix[ i ][ 2 ]
#  print cameraMatrix
  
  blenderScript = '''
import os, struct, re
import Blender
from Blender import Scene, Object, Camera, NMesh, Material
from Blender.Mathutils import Matrix

class PaletteReader:
  def __init__( self, fileName ):
    dim, ima = findFileNames( fileName, [ '.dim', '.ima' ] )
    #�Read header
    f = open( dim )
    volume_dimension = ([int(x) for x in f.readline().split()] + [ 1, 1, 1 ])[:4]
    if volume_dimension[ 1: ] != [ 1, 1, 1 ]:
      raise RuntimeError( 'Only one-dimensional palettes are supported' )
    self.size = volume_dimension[ 0 ]

    self._byteOrder = '>'
    ascii = False
    for line in f:
      l = line.split()
      if l:
        if l[ 0 ] == '-type':
          type = l[ 1 ]
          if type not in ( 'RGB', 'RGBA' ):
            raise RuntimeError( 'Unsupported image type: ' + l[1] )
        elif l[ 0 ] == '-om':
          ascii = l[ 1 ] == 'ascii'
        elif l[ 0 ] == '-bo':
          if l[1] == 'DCBA':
            self._byteOrder = '<'
    if not type:
      raise RuntimeError( 'Unknown image type' )
    f.close()
    
    readers = {
      ( 'RGB', False ): self._readBinaryRGB,
      ( 'RGBA', False ): self._readBinaryRGBA,
      ( 'RGB', True ): self._readAsciiRGB,
      ( 'RGBA', True ): self._readAsciiRGBA,
    }
    self.read = readers[ ( type, ascii ) ]
    if ascii:
      self._file = open( ima, 'r' )
    else:
      self._file = open( ima, 'rb' )
    
  def _readBinaryRGB( self ):
    for i in xrange( self.size ):
      yield struct.unpack( self._byteOrder + 'BBB',
                           self._file.read( 3 ) ) + (255,)

  def _readBinaryRGBA( self ):
    for i in xrange( self.size ):
      yield struct.unpack( self._byteOrder + 'BBBB',
                           self._file.read( 4 ) )
                        
  def _readAsciiRGB( self ):
    for c in re.finditer( r'\([^)]*\)', self._file.read() ):
      yield eval( c.group(0) ) + (255,)
   
  def _readAsciiRGBA( self ):
    for c in re.finditer( r'\([^)]*\)', self._file.read() ):
      yield eval( c.group(0) )
   

def anatomistPalette( name ):
  fileName = os.path.join( os.environ.get( 'BRAINVISA_SHARE', '/home/a-sac-ns-research/share' ), 'anatomist', 'rgb', name )
  return PaletteReader( fileName ).read()




def readAndUnpack( format, file ):
  return struct.unpack( format, file.read( struct.calcsize( format ) ) )

class BinaryItemReader:
  def __init__( self, bigEndian=True ):
    if bigEndian:
      self._endianess = '>'
    else:
      self._endianess = '<'
    
  def read( self, format, file ):
    result = ()
    format = format.split( 's' )
    if format:
      if format[0]:
        result = readAndUnpack( self._endianess + format[0], file )
      else:
        result = ()
      for f in format[ 1: ]:
        size = readAndUnpack( self._endianess + 'L', file )[0]
        result = result + ( file.read( size ), )
        result = result + readAndUnpack( self._endianess + f, file )
    return result
        
class MeshReader:
  def __init__( self, fileName ):
    self._file = None
    self._file = open( fileName, 'rb' )
    if self._file.read( 5 ) == 'binar':
      self._itemReader = BinaryItemReader( self._file.read( 4 ) == 'ABCD' )
    else:
      raise RuntimeError( 'Ascii mesh format not implemented' )
    void, self._polygonDimension, timeStepCount = self._itemReader.read( 'sLL', self._file )
    if timeStepCount == 0:
      raise RuntimeError( 'No mesh in this file' )
    instant = self._itemReader.read( 'L', self._file )[0]

    self.verticesCount = self._itemReader.read( 'L', self._file )[0]
    self._verticesRead = False
    self.normalsCount = None
    self._normalsRead = False
    self.facesCount = None
    self._facesRead = False
  
  def vertices( self ):
    if self._verticesRead:
      raise RuntimeError( 'Vertices can be read only once' )
    for i in xrange( self.verticesCount ):
      yield self._itemReader.read( 'fff', self._file )
    self._verticesRead = True
    self.normalsCount = self._itemReader.read( 'L', self._file )[0]
    
  def normals( self ):
    if not self._verticesRead:
      if self.verticesCount == 0:
        self._verticesRead = True
      else:
        raise RuntimeError( 'Vertices must be read before normals' )
    if self._normalsRead:
      raise RuntimeError( 'Normals can be read only once' )
    for i in xrange( self.normalsCount ):
      yield self._itemReader.read( 'fff', self._file )
    self._normalsRead = True
    textureCount = self._itemReader.read( 'L', self._file )[0]
    if textureCount != 0:
      raise RuntimeError( 'Texture in mesh file not supported' )
    self.facesCount = self._itemReader.read( 'L', self._file )[0]

  def faces( self ):
    if not self._verticesRead:
      if self.verticesCount == 0:
        self._verticesRead = True
      else:
        raise RuntimeError( 'Vertices must be read before faces' )
    if not self._normalsRead:
      if self.normalsCount == 0:
        self._normalsRead = True
        textureCount = self._itemReader.read( 'L', self._file )[0]
        if textureCount != 0:
          raise RuntimeError( 'Texture in mesh file not supported' )
        self.facesCount = self._itemReader.read( 'L', self._file )[0]
      else:
        raise RuntimeError( 'Normals must be read before faces' )
    if self._facesRead:
      raise RuntimeError( 'Faces can be read only once' )
    format = 'L' * self._polygonDimension
    for i in xrange( self.facesCount ):
      yield self._itemReader.read( format, self._file )
    self._facesRead = True
    self._file.close()
    self._file = None

  def __del__( self ):
    if self._file is not Nonblender -w -P /tmp/anatomist2blender.pye:
      self._file.close()

def readMesh( filename, colors=None, scene=None ):
  reader = MeshReader( filename )

  object = Blender.Object.New( 'Mesh' )  
  object.name = Blender.sys.splitext(Blender.sys.basename(filename))[0]

  mesh = NMesh.New()
  if colors:
    mesh.hasVertexColours( 1 )
    material = Material.New( object.name )
    material.rgbCol = [0.8, 0.8, 0.8]
    material.mode |= Material.Modes.VCOL_PAINT
    mesh.materials.append( material )
  for x, y, z in reader.vertices():
    mesh.verts.append( NMesh.Vert( x, y, -z ) )
  for n in reader.normals():
    pass
  for f in reader.faces():
    face = NMesh.Face( [mesh.verts[i] for i in f] )
    if colors:
      face.col = [NMesh.Col(*colors[i]) for i in f]
    face.smooth = True
    mesh.faces.append( face )
  
  if scene is None:
    scene = Blender.Scene.getCurrent()
  object.link( mesh )
  mesh.update( True )
  scene.link( object )
  object.setLocation( 0, 0, 0 )

def findFileNames( userFileName, extensions ):
  f = userFileName
  for e in extensions:
    if userFileName.endswith( e ):
      f = userFileName[ :-len(e) ]
      break
  return [f+e for e in extensions]


class TextureReader:
  def __init__( self, fileName ):
    self._file = None
    self._file = open( fileName, 'rb' )
    if self._file.read( 5 ) == 'binar':
      self._itemReader = BinaryItemReader( self._file.read( 4 ) == 'ABCD' )
    else:
      raise RuntimeError( 'Ascii mesh format not implemented' )
    type, timeStepCount, instant, self._size = self._itemReader.read( 'sLLL', self._file )

  def read( self ):
    for i in xrange( self._size ):
      t = self._itemReader.read( 'f', self._file )[ 0 ]
      yield int( t )

def findFileNames( userFileName, extensions ):
  f = userFileName
  for e in extensions:
    if userFileName.endswith( e ):
      f = userFileName[ :-len(e) ]
      break
  return [f+e for e in extensions]



cameraLocation = ''' + repr(cameraLocation) + '''

camdata = Camera.New('ortho')           # create new camera data
camdata.setLens( 130.0 )

scene = Scene.New('NewScene')           # create a new scene
camobj = Object.New('Camera')           # create a new camera object
camobj.setLocation( *[-x for x in cameraLocation] )

camobj.link(camdata)                    # (*) link data to object first
scene.link(camobj)                      # and then link object to scene
scene.makeCurrent()                     # make this the current scene
scene.setCurrentCamera( camobj )
camparent = Object.New( 'Empty' )

camparent.setLocation( *cameraLocation )
scene.link( camparent )
#camparent.makeParent( [ camobj ] )
#camobj.setEuler( 1.5707963267948966, 0, 3.1415926535897931 ) # Makes the camera look at his parent

# camparent.setMatrix( Matrix( *''' + repr(cameraMatrix) + ''') )

for meshFile, texture, palette in ''' + repr( meshFileNames ) + ''':
  if texture is not None:
    palette = tuple( anatomistPalette( palette ) )
    readMesh( meshFile, colors=[palette[i] for i in TextureReader( texture ).read()] )
  else:
    readMesh( meshFile )
'''

  f = open( '/tmp/anatomist2blender.py', 'w' )
  f.write( blenderScript )
  f.close()
  os.system( 'blender -w -P /tmp/anatomist2blender.py &' )
  
