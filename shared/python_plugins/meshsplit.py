import anatomist.cpp as anatomist
from soma import aims
from soma.aims import meshSplit
import sip

class MeshSplitModule(anatomist.Module):
  def name(self):
    return 'Mesh Split Module'

  def description(self):
    return 'Split a mesh into a patches graph according to a label texture'

class MeshSplitFusionMethod(anatomist.FusionMethod):
  def __init__(self):
    anatomist.FusionMethod.__init__(self)

  def canFusion(self, objects):
    if len(objects) != 2:
      return False
    mesh = None
    tex = None
    for o in objects:
      if isinstance(o, anatomist.ASurface_3 ):
        mesh = o
      elif o.type() == anatomist.AObject.TEXTURE:
        tex = o
      else:
        return False
    if mesh and tex:
      return True
    return False

  def fusion(self, objects):
    mesh = None
    tex = None
    for o in objects:
      if isinstance(o, anatomist.ASurface_3 ):
        mesh = o
      elif o.type() == anatomist.AObject.TEXTURE:
        tex = o
    gr = aims.Graph( 'RoiArg' )
    # attribs
    # print "type(tex) : ",type(tex)
    gr = meshSplit.meshSplit(anatomist.AObjectConverter.aims(mesh),anatomist.AObjectConverter.aims(tex),gr)
    del mesh, tex, o
    agr = anatomist.AObjectConverter.anatomist( gr )
    return agr

  def ID(self):
    return "MeshSplitFusionMethod"


f = anatomist.FusionFactory.factory()
m = MeshSplitFusionMethod()
f.registerMethod(m)

pm = MeshSplitModule()
