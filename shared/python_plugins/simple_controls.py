from __future__ import absolute_import, division, print_function

import anatomist.direct.api as ana
import selection
from soma.qt_gui.qt_backend import Qt
import os
import time


class SelectAndRotateAction(ana.cpp.ContinuousTrackball):
    def name(self):
        return 'SelectAndRotateAction'

    def beginTrackball(self, x, y, gx, gy):
        super(SelectAndRotateAction, self).beginTrackball(x, y, gx, gy)
        self.startx = x
        self.starty = y
        self.start_time = time.time()

    def endTrackball(self, x, y, gx, gy):
        super(SelectAndRotateAction, self).endTrackball(x, y, gx, gy)
        if time.time() - self.start_time < 0.5:
            action = self.view().controlSwitch().getAction('SelectAction')
            if action is not None:
                action.execSelect(x, y, gx, gy)


class LinkAndRotateAction(ana.cpp.ContinuousTrackball):
    def name(self):
        return 'LinkAndRotateAction'

    def beginTrackball(self, x, y, gx, gy):
        super(LinkAndRotateAction, self).beginTrackball(x, y, gx, gy)
        self.startx = x
        self.starty = y
        self.start_time = time.time()

    def endTrackball(self, x, y, gx, gy):
        super(LinkAndRotateAction, self).endTrackball(x, y, gx, gy)
        if time.time() - self.start_time < 0.5:
            action = self.view().controlSwitch().getAction('LinkAction')
            if action is not None:
                action.execLink(x, y, gx, gy)


# define another control where rotation is with the left mouse button
# (useful for touch devices)
class LeftSelect3DControl(selection.SelectionControl):

    def __init__(self,
                 name=Qt.QT_TRANSLATE_NOOP('ControlledWindow',
                                           'LeftSelect3DControl')):
        super(LeftSelect3DControl, self).__init__(name)

    def eventAutoSubscription(self, pool):
        key = Qt.Qt
        NoModifier = key.KeyboardModifier.NoModifier
        ShiftModifier = key.ShiftModifier
        ControlModifier = key.ControlModifier
        super(LeftSelect3DControl, self).eventAutoSubscription(pool)
        self.mousePressButtonEventUnsubscribe(key.LeftButton, NoModifier)
        #self.mousePressButtonEventUnsubscribe(key.RightButton, NoModifier)
        self.mouseDoubleClickEventUnsubscribe(key.LeftButton, NoModifier)
        #self.mouseLongEventUnsubscribe(key.MiddleButton, NoModifier)
        self.mouseLongEventSubscribe(
            key.LeftButton, NoModifier,
            pool.action('SelectAndRotateAction').beginTrackball,
            pool.action('SelectAndRotateAction').moveTrackball,
            pool.action('SelectAndRotateAction').endTrackball, True )
        self.keyPressEventSubscribe(key.Key_Space, ControlModifier,
            pool.action("SelectAndRotateAction").startOrStop)
        #self.mousePressButtonEventSubscribe(key.MiddleButton, NoModifier,
            #pool.action('LinkAction').execLink)
        #self.mousePressButtonEventSubscribe(key.MiddleButton, NoModifier,
            #pool.action('SelectAction').execSelect)
        self.mouseDoubleClickEventSubscribe(
            key.LeftButton, NoModifier,
            pool.action('SelectAction').execSelect)
        #self.mousePressButtonEventSubscribe(
            #key.RightButton, NoModifier,
            #pool.action('SelectAction').execSelectToggling)
        self.keyPressEventSubscribe(key.Key_Return, NoModifier,
            pool.action('LabelEditAction').edit)


# define another control where rotation is with the left mouse button
# (useful for touch devices)
class Left3DControl(selection.SelectionControl):

    def __init__(self,
                 name=Qt.QT_TRANSLATE_NOOP('ControlledWindow',
                                           'Left3DControl')):
        super(Left3DControl, self).__init__(name)

    def eventAutoSubscription(self, pool):
        key = Qt.Qt
        NoModifier = key.KeyboardModifier.NoModifier
        ShiftModifier = key.ShiftModifier
        ControlModifier = key.ControlModifier
        super(Left3DControl, self).eventAutoSubscription(pool)
        self.mousePressButtonEventUnsubscribe(key.LeftButton, NoModifier)
        #self.mousePressButtonEventUnsubscribe(key.RightButton, NoModifier)
        self.mouseDoubleClickEventUnsubscribe(key.LeftButton, NoModifier)
        #self.mouseLongEventUnsubscribe(key.MiddleButton, NoModifier)
        self.mouseLongEventSubscribe(
            key.LeftButton, NoModifier,
            pool.action('LinkAndRotateAction').beginTrackball,
            pool.action('LinkAndRotateAction').moveTrackball,
            pool.action('LinkAndRotateAction').endTrackball, True )
        self.keyPressEventSubscribe(key.Key_Space, ControlModifier,
            pool.action("LinkAndRotateAction").startOrStop)
        #self.mousePressButtonEventSubscribe(key.MiddleButton, NoModifier,
            #pool.action('LinkAction').execLink)
        #self.mousePressButtonEventSubscribe(key.MiddleButton, NoModifier,
            #pool.action('SelectAction').execSelect)
        self.mouseDoubleClickEventSubscribe(
            key.LeftButton, NoModifier,
            pool.action('SelectAction').execSelect)
        #self.mousePressButtonEventSubscribe(
            #key.RightButton, NoModifier,
            #pool.action('SelectAction').execSelectToggling)
        self.keyPressEventSubscribe(key.Key_Return, NoModifier,
            pool.action('LabelEditAction').edit)


a = ana.Anatomist('-b')
iconpath = os.path.join(str(a.anatomistSharedPath()), 'icons')
pix = Qt.QPixmap(os.path.join(iconpath, 'trackball_select.xpm'))
ana.cpp.IconDictionary.instance().addIcon('LeftSelect3DControl', pix)
ana.cpp.IconDictionary.instance().addIcon('Left3DControl', pix)
ad = ana.cpp.ActionDictionary.instance()
ad.addAction('SelectAndRotateAction', SelectAndRotateAction)
ad.addAction('LinkAndRotateAction', LinkAndRotateAction)
cd = ana.cpp.ControlDictionary.instance()
cd.addControl('LeftSelect3DControl', LeftSelect3DControl, 250)
cd.addControl('Left3DControl', Left3DControl, 80)
cm = ana.cpp.ControlManager.instance()
#cm.addControl('QAGLWidget3D', '', 'LeftSelect3DControl' )
cm.addControl('QAGLWidget3D', '', 'Left3DControl' )

# del cm, ad, cd, pix, iconpath, a
del pix, a

