
import anatomist.direct.api as anatomist
from soma.qt_gui.qt_backend import QtCore, QtGui
import numpy as np
from soma import aims
import os


class MeasureAction(anatomist.cpp.Action):

    def __init__(self):
        super().__init__()
        self.temp = []
        self.points = []
        self.redo_pts = []
        self.cyl_radius = 0.2
        self.sphere_radius = 0.5

    def name(self):
        return 'MeasureAction'

    def add_point(self, x, y, globx, globy):
        v = self.view()
        link = v.controlSwitch().getAction('LinkAction')
        link.execLink(x, y, globx, globy)

        w = v.aWindow()
        pos = w.getPosition()

        self.points.append(pos)
        self.update_display()

    def end_segment(self):
        if len(self.points) == 0 or self.points[-1] in (None, 0):
            return

        self.points.append(None)  # mark end
        self.redo_pts = []
        self.update_display()

    def cycle_path(self):
        if len(self.points) == 0 or self.points[-1] in (None, 0):
            return

        self.points.append(0)  # mark cycling
        self.redo_pts = []
        self.update_display()

    def cleanup(self):
        self.temp = []
        self.points = []
        self.redo_pts = []

    def mesh_path(self):
        self.make_mesh()

    def length(self):
        leng = 0.
        l1 = 0.
        p0 = None
        lastp = 0
        for i, p in enumerate(self.points):
            if p == 0:
                p = self.points[lastp]
                lastp = i + 1
            elif p is None:
                lastp = i + 1
            if p is not None and p0 is not None:
                l1 = (p - p0).norm()
                leng += l1
            if lastp <= i:
                p0 = p
            else:
                p0 = None
        return leng, l1

    def update_display(self):
        self.display_len()
        self.display_path()

    def display_len(self):
        leng, last = self.length()
        print('total:', leng)
        print('last:', last)
        # print(self.points)
        v = self.view()
        w = v.aWindow()
        w.statusBar().showMessage(f'path length: {leng} ({last})')

    def make_cylinders(self):
        cyl_mesh = aims.AimsSurfaceTriangle()
        sph_mesh = aims.AimsSurfaceTriangle()
        cyl_mesh.header()['material'] = {'diffuse': [0.6, 0.4, 0.1]}
        sph_mesh.header()['material'] = {'diffuse': [1., 0., 0]}
        p0 = None
        lastp = 0
        for i, p in enumerate(self.points):
            if p == 0:
                p = self.points[lastp]
                lastp = i + 1
            elif p is None:
                lastp = i + 1
            elif p is not None:
                sph = aims.SurfaceGenerator.icosphere(
                    p, self.sphere_radius, 20)
                aims.SurfaceManip.meshMerge(sph_mesh, sph)
            if p is not None and p0 is not None:
                cyl = aims.SurfaceGenerator.cylinder(
                    p0, p, self.cyl_radius, self.cyl_radius, 6, False,
                    True)
                aims.SurfaceManip.meshMerge(cyl_mesh, cyl)
            if lastp <= i:
                p0 = p
            else:
                p0 = None
        objs = []
        a = anatomist.Anatomist()
        v = self.view()
        w = v.aWindow()
        if len(cyl_mesh.vertex()) != 0:
            o = a.toAObject(cyl_mesh)
            a.unmapObject(o)
            a.releaseObject(o)
            objs.append(o)
            o.setReferential(w.getReferential())
        if len(sph_mesh.vertex()) != 0:
            o = a.toAObject(sph_mesh)
            a.unmapObject(o)
            a.releaseObject(o)
            objs.append(o)
            o.setReferential(w.getReferential())
        return objs

    def make_mesh(self):
        mesh = aims.AimsTimeSurface(2)
        vertices = []
        polygons = []
        mesh.header()['material'] = {'diffuse': [0.1, 0.3, 0.8]}
        p0 = None
        lastp = 0
        for i, p in enumerate(self.points):
            if p == 0:
                p = None
                polygons.append((len(vertices) - 1,
                                 len(vertices) - i + lastp))
                lastp = i + 1
            elif p is None:
                lastp = i + 1
            else:
                vertices.append(p)
                if p0 is not None:
                    polygons.append((len(vertices) - 2, len(vertices) - 1))
            p0 = p

        if len(polygons) == 0:
            return

        mesh.vertex().assign(vertices)
        mesh.polygon().assign(polygons)
        a = anatomist.Anatomist()
        amesh = a.toAObject(mesh)
        v = self.view()
        w = v.aWindow()
        amesh.setReferential(w.getReferential())

        return amesh

    def display_path(self):
        self.temp = self.make_cylinders()

        v = self.view()
        w = v.aWindow()
        for o in self.temp:
            w.registerObject(o, True)
        w.Refresh()

    def undo(self):
        # print('undo')
        if len(self.points) == 0:
            return

        last = self.points.pop(-1)
        self.redo_pts.append(last)
        self.update_display()

    def redo(self):
        # print('redo')
        if len(self.redo_pts) == 0:
            return
        last = self.redo_pts.pop(-1)
        self.points.append(last)
        self.update_display()


class MeasureControl(anatomist.cpp.Control3D):

    def __init__(self, priority=100,
                 name=QtCore.QT_TRANSLATE_NOOP(
                    'ControlledWindow', 'MeasureControl')):
        super().__init__(priority, name)

    def eventAutoSubscription(self, pool):
        super().eventAutoSubscription(pool)
        self.mouseLongEventUnsubscribe(
            QtCore.Qt.LeftButton, QtCore.Qt.KeyboardModifier.NoModifier)
        self.keyPressEventUnsubscribe(
            QtCore.Qt.Key_Space, QtCore.Qt.KeyboardModifier.NoModifier)
        self.mousePressButtonEventSubscribe(
            QtCore.Qt.LeftButton, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').add_point)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Space, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').end_segment)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Escape, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').cleanup)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_M, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').mesh_path)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_C, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').cycle_path)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Z, QtCore.Qt.KeyboardModifier.ControlModifier,
            pool.action('MeasureAction').undo)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Z,
            QtCore.Qt.KeyboardModifier.ControlModifier
            | QtCore.Qt.KeyboardModifier.ShiftModifier,
            pool.action('MeasureAction').redo)

    def doAlsoOnDeselect(self, actionpool):
        super().doAlsoOnDeselect(actionpool)
        actionpool.action("MeasureAction").cleanup()

    def description(self):
        return '''<b>Measurement control</b><br/><br/>
Draw paths (continuous or discontinuous), and display the path length.<br/>

<table>
<tr><td>Left btn:</td><td>add path point</td></tr>
<tr><td>Space:</td><td>end path - next point will be a disconnected path</td></tr>
<tr><td>C:</td><td>cycle path: connect to first point of the path, and end path</td></tr>
<tr><td>Esc:</td><td>Cancel/reset: erase current paths</td></tr>
<tr><td>M: Mesh:</td><td>make a segments mesh from the paths</td></tr>
<tr><td>Ctrl-Z:</td><td>undo last point/interruption</td></tr>
<tr><td>Shift-Ctrl-Z:</td><td>redo</td></tr>
</table>
'''


a = anatomist.Anatomist()
pix = QtGui.QPixmap(a.anatomistSharedPath() + os.sep + 'icons'
                    + os.sep + 'measure.xpm')
anatomist.cpp.IconDictionary.instance().addIcon('MeasureControl', pix)
ad = anatomist.cpp.ActionDictionary.instance()
ad.addAction('MeasureAction', MeasureAction)
cd = anatomist.cpp.ControlDictionary.instance()
cd.addControl('MeasureControl', MeasureControl, 199)
cm = anatomist.cpp.ControlManager.instance()
cm.addControl('QAGLWidget3D', '', 'MeasureControl')

# don't del these, it will unregister the new controls (why?)
# del ad , cd, cm, icon
del pix
