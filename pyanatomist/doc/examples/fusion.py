from soma import aims
import os
import anatomist.direct.api as anatomist

a = anatomist.Anatomist()

class MyAObjectKrakboumCallback( anatomist.cpp.ObjectMenuCallback ):
  def __init__( self ):
    anatomist.cpp.ObjectMenuCallback.__init__( self )

  def doit( self, objects ):
    print 'MyAObjectKrakboumCallback:', objects


class MyAObject(anatomist.cpp.AObject):
	_type = anatomist.cpp.AObject.registerObjectType('MyAObject')
        _menus = None
	icon = os.path.join( a.anatomistSharedPath().latin1(),
                             'icons', 'list_cutmesh.xpm' )
	ot = anatomist.cpp.QObjectTree
	ot.setObjectTypeName(_type, 'Example of a custom AObject')
	ot.setObjectTypeIcon(_type, icon)

	def __init__(self, filename=''):
		anatomist.cpp.AObject.__init__(self, filename)
		self.setType(MyAObject._type)
		self.setReferential(a.centralReferential())

        def optionTree( self ):
          if MyAObject._menus is None:
            m = anatomist.cpp.ObjectMenu()
            krak = MyAObjectKrakboumCallback()
            m.insertItem( [ 'File' ], 'Reload',
                          anatomist.cpp.ObjectActions.fileReloadMenuCallback() )
            m.insertItem( [ 'Color' ], 'Material',
                          anatomist.cpp.ObjectActions.colorMaterialMenuCallback() )
            m.insertItem( [ 'Rototo', 'pouet' ], 'krakboum', krak )
            MyAObject._menus = m.releaseTree()
            # avoid deleting the python part of the callback
            MyAObject._nodelete = [ krak ]
          return MyAObject._menus


class MyFusion(anatomist.cpp.FusionMethod):
	def __init__(self):
		anatomist.cpp.FusionMethod.__init__(self)
		print "init myfusion"

	def canFusion(self, objects):
		print "MyFusion : canFusion"
		return True

	def fusion(self, objects):
		print "MyFusion : fusion"
		return MyAObject()

	def ID(self):
		return 'myFusion'

	def orderingMatters(self):
		return False
		
# Register MyFusion
anatomist.cpp.FusionFactory.registerMethod(MyFusion())

# Load an object
obj=a.loadObject('test.mesh')

#if __name__ == '__main__' :
	#import qt
        #if qt.QApplication.startingUp():
	        #qt.qApp.exec_loop()

