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

"""Palette controls: add palette contrast actions in standard anatomist
controls:
ctrl+right mouse button: left-right: change min, up-down: change max
"""

from __future__ import absolute_import
import anatomist.cpp as anatomist
from anatomist.cpp import palettecontrastaction
import types

import six


def neweventAutoSubscription(self, pool):
    key = palettecontrastaction.QtCore.Qt
    NoModifier = key.NoModifier
    ShiftModifier = key.ShiftModifier
    ControlModifier = key.ControlModifier
    AltModifier = key.AltModifier
    if hasattr(self, '_initial_eventAutoSubscription'):
        self._initial_eventAutoSubscription(pool)
    else:
        self.__class__.__base__.eventAutoSubscription(self, pool)
    self.mouseLongEventSubscribe(key.RightButton, ControlModifier,
                                 pool.action(
                                     'PaletteContrastAction').startContrast,
                                 pool.action(
                                     'PaletteContrastAction').moveContrast,
                                 pool.action('PaletteContrastAction').stopContrast, True)
    self.keyPressEventSubscribe(key.Key_C, NoModifier,
                                pool.action("PaletteContrastAction").resetPalette)


def makePalettedSubclass(c):
    if type(c.eventAutoSubscription) is types.BuiltinMethodType:
        clname = 'Paletted_' + c.__class__.__name__
        cmd = 'class ' + clname + '( anatomist.' + c.__class__.__name__ \
            + ' ): pass'

        six.exec_(cmd)
        cl = eval(clname)
        setattr(cl, 'eventAutoSubscription', neweventAutoSubscription)
    else:
        cl = c.__class__
        cl._initial_eventAutoSubscription = cl.eventAutoSubscription
        setattr(cl, 'eventAutoSubscription', neweventAutoSubscription)
    cd = anatomist.ControlDictionary.instance()
    cd.removeControl(c.name())
    cd.addControl(c.name(), cl, c.priority(), True)


cd = anatomist.ControlDictionary.instance()
c = cd.getControlInstance('Default 3D control')
makePalettedSubclass(c)
c = cd.getControlInstance('ObliqueControl')
makePalettedSubclass(c)
try:
    import selection
    c = cd.getControlInstance('SelectionControl')
    makePalettedSubclass(c)
except:
    c = cd.getControlInstance('Selection 3D')
    makePalettedSubclass(c)
c = cd.getControlInstance('TransformControl')
makePalettedSubclass(c)
c = cd.getControlInstance('CutControl')
makePalettedSubclass(c)
c = cd.getControlInstance('Flight control')
makePalettedSubclass(c)

del c, cd
