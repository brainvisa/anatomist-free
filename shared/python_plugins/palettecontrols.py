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

import anatomist.cpp as anatomist
from anatomist.cpp import palettecontrastaction  # noqa: F401
from soma import aims
from soma.qt_gui.qt_backend import Qt
from soma.qt_gui.qt_backend import sip
import types
import gc

import six


def neweventAutoSubscription(self, pool):
    NoModifier = Qt.Qt.KeyboardModifier.NoModifier
    ControlModifier = Qt.Qt.KeyboardModifier.ControlModifier
    if hasattr(self, '_initial_eventAutoSubscription'):
        self._initial_eventAutoSubscription(pool)
    else:
        self.__class__.__base__.eventAutoSubscription(self, pool)
    self.mouseLongEventSubscribe(Qt.Qt.MouseButton.RightButton,
                                 ControlModifier,
                                 pool.action(
                                     'PaletteContrastAction').startContrast,
                                 pool.action(
                                     'PaletteContrastAction').moveContrast,
                                 pool.action(
                                     'PaletteContrastAction').stopContrast,
                                 True)
    self.keyPressEventSubscribe(Qt.Qt.Key.Key_C, NoModifier,
                                pool.action(
                                    "PaletteContrastAction").resetPalette)


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


class MiniPaletteExtensionAction(anatomist.APaletteExtensionAction):

    widgets = []

    def __init__(self, icon, text, parent):
        super().__init__(icon, text, parent)

    def extensionTriggered(self, objects):
        import anatomist.cpp as mpw

        if len(objects) != 0:
            w = mpw.MiniPaletteWidget(next(iter(objects)),
                                      0,
                                      True,
                                      True,
                                      None,
                                      True,
                                      False)
            w.setAttribute(Qt.Qt.WA_DeleteOnClose, True)
            w.resize(260, 60)
            w.show()
            sip.transferto(w, None)
            MiniPaletteExtensionAction.widgets.append(w)
            w.destroyed.connect(MiniPaletteExtensionAction.clear_widget)

    @staticmethod
    def clear_widget(obj):
        # print('clear_widget')
        MiniPaletteExtensionAction.widgets = [
            w for w in MiniPaletteExtensionAction.widgets
            if not sip.isdeleted(w)]
        gc.collect()
        # print('widgets:', len(MiniPaletteExtensionAction.widgets))


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

# icon = Qt.QIcon(aims.carto.Paths.findResourceFile(
#     'icons/palette.jpg', 'anatomist'))
# ac = MiniPaletteExtensionAction(icon, 'minpalette', None)
# anatomist.QAPaletteWin.addExtensionAction(ac)

del c, cd
