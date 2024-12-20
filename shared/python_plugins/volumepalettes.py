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


'''Adds in volumes a menu color/Set distinct palettes, allowing to
set automatically palettes on a set of volumes, in a non-ambiguous (colors
non-overlapping), according to the guessed type of volume (anatomical,
functional, diffusion, labels)
'''

import anatomist.direct.api as ana
from soma.aims import colormaphints


class SetAutoPalettes(ana.cpp.ObjectMenuCallback):

    def __init__(self):
        ana.cpp.ObjectMenuCallback.__init__(self)

    def doit(self, objects):
        oc = ana.cpp.AObjectConverter.aims
        aimsobj = [oc(x) for x in objects]
        hints = []
        for x in aimsobj:
            if 'colormaphints' in x.header():
                hints.append(x.header()['colormaphints'])
            else:
                hint = colormaphints.checkVolume(x)
                hints.append(hint)
                x.header()['colormaphints'] = hint
        cmaps = colormaphints.chooseColormaps(hints)
        a = ana.Anatomist()
        for x, y in zip(objects, cmaps):
            a.execute('SetObjectPalette', objects=[x], palette=y)

callbacks_list = []

# Add pop Menu to an object


def addMenuEntryToOptionMenu(menu):
    autopal = SetAutoPalettes()
    callbacks_list.append(autopal)
    menu.insertItem(['Color'], 'Set distinct palettes', autopal)


class VolumePalettesModule(ana.cpp.Module):

    def name(self):
        return 'Volume palettes module'

    def description(self):
        return __doc__


def init():
    menumap = ana.cpp.AObject.getObjectMenuMap()
    menus = {}
    # Add palette menu to all menus but only once
    for k, v in menumap.items():
        if k.startswith('VOLUME'):
            menus[v] = k
    for m in menus.keys():
        addMenuEntryToOptionMenu(m)


def cleanup():
    menumap = ana.cpp.AObject.getObjectMenuMap()
    # Add palette menu to all menus but only once
    for k, v in menumap.items():
        if k.startswith('VOLUME'):
            v.removeItem(['Color'], 'Set distinct palettes')
    global callbacks_list
    callbacks_list = []


volpal = VolumePalettesModule()
init()
import atexit
atexit.register(cleanup)
