# -*- coding: utf-8 -*-
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

import anatomist.direct.api as ana
from soma.aims import gltf_io
from soma.qt_gui.qt_backend import QtCore, Qt
from functools import partial
import sip
import json


class GLTFCreateWindowNotifier(object):

    actions = {}

    def notify(self, name, params):
        win = params['window']
        if hasattr(win, 'getInternalRep'):
            win = win.getInternalRep()
        if isinstance(win, ana.cpp.AWindow3D):
            menu = win.menuBar()
            scene_menu = menu.addMenu('Export')
            num = len(GLTFCreateWindowNotifier.actions)
            ac = scene_menu.addAction('Export scene as GLTF file...',
                                      partial(self.export_gltf, num))
            GLTFCreateWindowNotifier.actions[num] = sip.unwrapinstance(win)


    @staticmethod
    def aobject_to_gltf(obj, win, gltf={}):
        if obj.glAPI() is None:
            return gltf

        glapi = obj.glAPI()
        vs = win.viewState().get()
        vert = glapi.glVertexArray(vs)
        norm = glapi.glNormalArray(vs)
        poly = glapi.glPolygonArray(vs)

        a = ana.Anatomist()
        tr = a.getTransformation(win.getReferential(),
                                 obj.getReferential())
        matrix = None
        if tr:
            matrix = list(tr.affine().np.transpose().ravel())

        mat = glapi.glMaterial()
        if mat is not None:
            mat = mat.genericDescription()

        cmap = None
        textures = []
        teximages = []
        if glapi.glNumTextures(vs) != 0:
            ntex = glapi.glNumTextures(vs)
            teximages = [glapi.glBuildTexImage(vs, tex) for tex in range(ntex)]
            #cmap = glapi.glPalette().colors()
            textures = [glapi.glTexCoordArray(vs, tex) for tex in range(ntex)]

        gltf_io.add_object_to_gltf_dict(
            vert, norm, poly, material=mat, matrix=matrix, textures=textures,
            teximages=teximages, name=obj.name, gltf=gltf)

        return gltf

    @staticmethod
    def export_gltf(num):
        a = ana.Anatomist()
        w = [w for n, w in GLTFCreateWindowNotifier.actions.items()
             if n == num]
        win = None
        if len(w)!= 0:
            w = w[0]
            wins = [win for win in a.getWindows()
                    if sip.unwrapinstance(win.internalRep) == w]
            if wins:
                win = wins[0]
        if win is None:
            return

        filename = Qt.QFileDialog.getSaveFileName(
            None, 'Export GLTF scene', '', '*.gltf')
        if not filename:
            return
        filename = filename[0]
        if not filename:
            return
        try:
            gltf_d = GLTFCreateWindowNotifier.win_gltf(win)
        except Exception as e:
            print(e)
            import traceback
            traceback.print_exc()
            raise
        with open(filename, 'w') as f:
            json.dump(gltf_d, f, indent=4)

    def win_gltf(win):
        matrix = None
        if not win.getReferential().isDirect():
            matrix = [
                -1,  0,  0,  0,
                0, -1,  0,  0,
                0,  0, -1,  0,
                0,  0,  0,  1
            ]
        gltf = gltf_io.default_gltf_scene(matrix)

        for obj in win.objects:
            GLTFCreateWindowNotifier.aobject_to_gltf(obj, win, gltf)

        return gltf


class GLTFIOModule(ana.cpp.Module):

    def name(self):
        return 'GLTF IO'

    def description(self):
        return 'Export an Anatomist window scene as GLTF file'

    def install_menus(self):
        a = ana.Anatomist()
        a.enableListening('CreateWindow', GLTFCreateWindowNotifier())


gltf_mod = GLTFIOModule()
gltf_mod.install_menus()

