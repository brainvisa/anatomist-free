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

"""Models Fusion Method
"""

import sys, os
if sys.modules.has_key( 'PyQt4' ):
  import PyQt4.QtCore as qt
  import PyQt4.QtGui as qtui
else:
  import qt, qtui
import anatomist.cpp as anatomist
from soma import aims

an = anatomist.Anatomist()
processor = an.theProcessor()

class Configurator(object):
	def __init__(self, widget, objects):
		self._widget = widget
		self._objects = objects
		oneobj = self._objects[0]
		self._choosen_data = oneobj._choosen_data
		self._choosen_type = oneobj._choosen_type
		self._abs_enable = oneobj._abs_enable
		self._connect()	

	def _connect(self):
		clicked = qt.SIGNAL("clicked()")
		activated = qt.SIGNAL("activated(int)")
		toggled = qt.SIGNAL("toggled(bool)")
		widgets_slots = {'applyButton' : (clicked, self.applySlot),
			'dataComboBox' : (activated, self.dataSlot),
			'typeComboBox' : (activated, self.typeSlot),
			'absRadioButton' : (toggled, self.absSlot)}
		widgets = dict([(name, self._widget.child(name)) \
				for name in widgets_slots.keys()])	
		for name, (signal, slot) in widgets_slots.items():
			qt.QObject.connect(widgets[name], signal, slot)
		dataToId = {'raw' : 0, 'mean' : 1, 'good' : 2, 'bad' : 3}
		typeToId = {'vertex' : 0, 'all' : 1}
		widgets['absRadioButton'].setChecked(self._abs_enable)
		widgets['dataComboBox'].setCurrentItem(\
			dataToId[self._choosen_data])
		widgets['typeComboBox'].setCurrentItem(\
			typeToId[self._choosen_type])

	def applySlot(self):
		for o in self._objects:
			o.compute(self._abs_enable)
			o.display(self._choosen_data, self._choosen_type)
					
	def okSlot(self):
		self.applySlot()
		self._widget.close()
		
	def dataSlot(self, n):
		dataFromId = ['raw', 'mean', 'bad', 'good']
		self._choosen_data = dataFromId[n]

	def typeSlot(self, n):
		typeFromId = ['vertex', 'all']
		self._choosen_type = typeFromId[n]

	def absSlot(self, on):
		self._abs_enable = on


class ConfigureCallback(anatomist.ObjectMenuCallback):
	def __init__(self):
		anatomist.ObjectMenuCallback.__init__(self)
		self._conf = None

	def doit(self, objects):
		ui = __file__[:__file__.rfind('.') + 1] + 'ui'
		parent = an.getControlWindow()
		widget = qtui.QWidgetFactory().create(ui, None, parent)
		if widget:
			self._conf = Configurator(widget, objects)
			widget.show()
		else:	print "can't load ui file '%s'" % ui


class AModelsGraphFusionObject(anatomist.ObjectList):
	_type = anatomist.AObject.registerObjectType(\
				'AModelsGraphFusionObject')
	_menus = None
	shared_path = str( an.anatomistSharedPath() )
	icon = os.path.join( shared_path,
				'icons', 'list_afgraph.xpm')
	ot = anatomist.QObjectTree
	ot.setObjectTypeName(_type, 'GraphInteraction')
	ot.setObjectTypeIcon(_type, icon)
	_id_ = 0

	def __init__(self, amodels, agraph):
		anatomist.ObjectList.__init__(self)
		self.setType(AModelsGraphFusionObject._type)
		self._amodels = amodels
		self._agraph = agraph
		AModelsGraphFusionObject._id_ += 1
		self._id = AModelsGraphFusionObject._id_
		self._mypalette = 'AModelsGraphFusionObject_pal_%d' % self._id
		self.setReferential(self._agraph.getReferential())
		self.insert(self._agraph)
		for m in self._amodels: self.insert(m)
		self._init_labels()
		self._newpalette()
		self.compute(False)
		self.display('mean', 'vertex')
		self._init_color_mode()

	def _init_labels(self):
		model = self._amodels[0].graph()

		# translate low level names to high level labels(at model level)
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		import sigraph
		sigraph.si().setLabelsTranslPath(transfile)
		mf = model.modelFinder()
		mf.initCliques(self._agraph.graph(), False, True)

	def _init_color_mode(self):
		ag = self._agraph
		ag.setColorMode(ag.PropertyMap)
		ag.setColorProperty('diff')

	def _newpalette(self):
		import paletteViewer as PV
		an_shared_path = str( an.anatomistSharedPath() )
		imgfilename = os.path.join(an_shared_path,'rgb', 'Blue-Red.ima')
		r = aims.Reader()
		img = r.read(imgfilename)
		colors = PV.convertVolumeToArray(img).ravel().tolist()
		colors[-4:] = (255, 255, 255, 64)
		processor.execute('NewPalette', {'name' : self._mypalette})
		processor.execute('ChangePalette', {'name' : self._mypalette,
				'colors' : colors, 'color_mode' : 'RGBA'})

	def compute(self, abs_enable):
		import sigraph.models as SM

		models = [m.graph() for m in self._amodels]
		self._abs_enable = abs_enable
		self._md = SM.ModelDifferentiator(models, abs_enable)
		self._md.diff()

	def display(self, choosen_data, choosen_type):
		self._choosen_data = choosen_data
		self._choosen_type = choosen_type

	        # palette
		infomin, infomax = self._md.getMinMax(choosen_data)
		vol = self.getOrCreatePalette().refPalette().volume().get()
		size = vol.getSizeX()
		coeff = (size + 1) / float(size)
		processor.execute('SetObjectPalette',
				{'objects' : [self._agraph],
				'palette' : self._mypalette, 'min' : 0,
				'max' : (infomax - infomin) * coeff})

		# labels
		if choosen_type == 'vertex':
			self._display_vertices_info(choosen_data)
		elif choosen_type == 'all':
			self._display_all_info(choosen_data)
		#self.notifyObservers()
		self._agraph.updateColors()
		#ag = self._agraph
		#ag.setColorMode(ag.Normal)
		#ag.setColorMode(ag.PropertyMap)

	def _display_vertices_info(self, c):
		diff = {}
		for v in self._amodels[0].graph().vertices():
			name, d = self._md.getVertexValue(v, c)
			diff[name] = d
		for v in self._agraph.graph().vertices():
			name = v['label']
			try:
				v['diff'] = diff[name]
			except KeyError:
				v['diff'] = 1. #bad value

	def _display_all_info(self, c):
		vertices = {}
		for v in self._amodels[0].graph().vertices():
			name = v['label']
			vertices[name] = v
		for v in self._agraph.graph().vertices():
			name, v['diff'] = self._md.getVertexValue(v, c)
			diff = v['diff']
			try:
				edges = vertices[name].edges()
			except KeyError:
				edges = None
			if edges:
				for e in edges:
					name, d = self._md.getEdgeValue(e, c)
					diff += d
				v['diff'] = diff / (1 + len(edges))
			else:	v['diff'] = 1. #bad value
		
	def glAPI(self):
		return self._agraph.glAPI()

	def Is2DObject(self):
		return self._agraph.Is2DObject()

	def Is3DObject(self):
		return self._agraph.Is3DObject()

	def boundingBox(self, bmin, bmax):
		return self._agraph.boundingBox(bmin, bmax)

	def optionTree(self):
		_t = qt.QT_TRANSLATE_NOOP
		if AModelsGraphFusionObject._menus is None:
			m = anatomist.ObjectMenu()
			conf = ConfigureCallback()
			import paletteViewer
			showHidePalette =paletteViewer.ShowHidePaletteCallback()
			m.insertItem([], 'configure', conf)
			m.insertItem([], 'show/hide palette', showHidePalette)
			AModelsGraphFusionObject._menus = m.releaseTree()
			AModelsGraphFusionObject._nodelete = \
				[conf, showHidePalette]
		return AModelsGraphFusionObject._menus

	def update(self, observable, arg):
		self.setChanged()
		self.notifyObservers()

	def palette(self):
		return self._agraph.palette()
	
	def getOrCreatePalette(self):
		return self._agraph.getOrCreatePalette()

class ModelsFusionMethod(anatomist.FusionMethod):
	def __init__(self):
		anatomist.FusionMethod.__init__(self)

	def canFusion(self, objects):
		import sigraph
		if len(objects) not in [2, 3]: return False
		types = []
		for o in objects:
			if not isinstance(o, anatomist.AGraph):
				return False
			types.append(o.graph())
		nCgraph = len([t for t in types \
				if isinstance(t, sigraph.CGraph)])
		nFRGraph = len([t for t in types \
				if isinstance(t, sigraph.FRGraph)])
		if nCgraph != 1 or (nFRGraph != len(objects) - 1):
			return False
		return True

	def fusion(self, objects):
		import sigraph
		amodels = []
		for o in objects.list():
			g = o.graph()
			if isinstance(g, sigraph.CGraph):
				agraph = o
			elif isinstance(g, sigraph.FRGraph):
				amodels.append(o) 
		return AModelsGraphFusionObject(amodels, agraph)

	def ID(self):
		return "modelsFusionMethod"
	
	def orederingMatters():
		return True


class ModelsFusionModule(anatomist.Module):
	def name(self):
		return 'Models Fusion Method Module'

	def description(self):
		return __doc__

f = anatomist.FusionFactory.factory()
m = ModelsFusionMethod()
f.registerMethod(m)

pm = ModelsFusionModule()
