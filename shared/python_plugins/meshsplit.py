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
      return 0
    mesh = None
    tex = None
    for o in objects:
      if isinstance(o, anatomist.ASurface_3 ):
        mesh = o
      elif o.type() == anatomist.AObject.TEXTURE:
        tex = o
      else:
        return 0
    if mesh and tex:
      return 70
    return 0

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
