
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
        self.poly_ended = False
        self.redo_pts = []
        self.cyl_radius = 0.2
        self.sphere_radius = 0.5

    def name(self):
        return 'MeasureAction'

    def add_point(self, x, y, globx, globy):
        print('add_point')
        v = self.view()
        link = v.controlSwitch().getAction('LinkAction')
        link.execLink(x, y, globx, globy)

        w = v.aWindow()
        pos = w.getPosition()

        poly = []
        if len(self.points) == 0 or self.poly_ended:
            poly = []
            self.points.append(poly)
        else:
            poly = self.points[-1]
        poly.append(pos)
        self.poly_ended = False
        self.update_display()

    def end_segment(self):
        if len(self.points) == 0:
            self.poly_ended = False
            return

        self.poly_ended = True

        if len(self.points[-1]) == 0:
            self.points = self.points[:-1]
            return
        self.update_display()

    def cycle_path(self):
        if len(self.points) == 0:
            self.poly_ended = False
            return

        self.poly_ended = True

        if len(self.points[-1]) == 0:
            self.points = self.points[:-1]
            return
        if self.points[-1][-1] is None:
            return
        self.points[-1].append(None)  # mark cycling
        self.update_display()

    def cleanup(self):
        print('cleanup')
        self.temp = []
        self.points = []
        self.poly_ended = False
        self.redo_pts = []

    def mesh_path(self):
        print('mesh path')
        self.make_mesh()

    def length(self):
        leng = 0.
        l1 = 0.
        for poly in self.points:
            p0 = None
            for p in poly:
                if p is None:
                    p = poly[0]
                if p is not None and p0 is not None:
                    l1 = (p - p0).norm()
                    leng += l1
                p0 = p
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
        for poly in self.points:
            p0 = None
            for p in poly:
                if p is None:
                    p = poly[0]
                else:
                    sph = aims.SurfaceGenerator.icosphere(
                        p, self.sphere_radius, 20)
                    aims.SurfaceManip.meshMerge(sph_mesh, sph)
                if p is not None and p0 is not None:
                    cyl = aims.SurfaceGenerator.cylinder(
                        p0, p, self.cyl_radius, self.cyl_radius, 6, False,
                        True)
                    aims.SurfaceManip.meshMerge(cyl_mesh, cyl)
                p0 = p
        objs = []
        a = anatomist.Anatomist()
        if len(cyl_mesh.vertex()) != 0:
            o = a.toAObject(cyl_mesh)
            a.unmapObject(o)
            a.releaseObject(o)
            objs.append(o)
        if len(sph_mesh.vertex()) != 0:
            o = a.toAObject(sph_mesh)
            a.unmapObject(o)
            a.releaseObject(o)
            objs.append(o)
        return objs

    def make_mesh(self):
        mesh = aims.AimsTimeSurface(2)
        vertices = []
        polygons = []
        mesh.header()['material'] = {'diffuse': [0.1, 0.3, 0.8]}
        for poly in self.points:
            p0 = None
            for p in poly:
                if p is None:
                    p = poly[0]
                    polygons.append((len(vertices) - 1,
                                     len(vertices) - len(poly) + 1))
                else:
                    vertices.append(p)
                if p is not None and p0 is not None:
                    polygons.append((len(vertices) - 2, len(vertices) - 1))
                p0 = p

        if len(polygons) == 0:
            return

        mesh.vertex().assign(vertices)
        mesh.polygon().assign(polygons)
        a = anatomist.Anatomist()
        amesh = a.toAObject(mesh)
        return amesh

    def display_path(self):
        self.temp = self.make_cylinders()

        v = self.view()
        w = v.aWindow()
        for o in self.temp:
            w.registerObject(o, True)
        w.Refresh()

    def undo(self):
        print('undo')


class MeasureControl(anatomist.cpp.Control3D):

    def __init__(self, priority=100,
                 name=QtCore.QT_TRANSLATE_NOOP(
                    'ControlledWindow', 'MeasureControl')):
        super().__init__(priority, name)

    def eventAutoSubscription(self, pool):
        super().eventAutoSubscription(pool)
        self.mouseLongEventUnsubscribe(
            QtCore.Qt.LeftButton, QtCore.Qt.KeyboardModifier.NoModifier)
        self.mousePressButtonEventSubscribe(
            QtCore.Qt.LeftButton, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').add_point)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Escape, QtCore.Qt.KeyboardModifier.NoModifier,
            pool.action('MeasureAction').end_segment)
        self.keyPressEventSubscribe(
            QtCore.Qt.Key_Escape, QtCore.Qt.KeyboardModifier.ShiftModifier,
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

    def doAlsoOnDeselect(self, actionpool):
        super().doAlsoOnDeselect(actionpool)
        actionpool.action("MeasureAction").cleanup()


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
