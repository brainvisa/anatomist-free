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
import uuid


class GLTFCreateWindowNotifier(object):

    actions = {}

    def notify(self, name, params):
        win = params['window']
        if hasattr(win, 'getInternalRep'):
            win = win.getInternalRep()
        if isinstance(win, ana.cpp.AWindow3D):
            menu = win.menuBar()
            if not menu.findChildren(Qt.QMenu):
                return
            scene_menu = menu.addMenu('Export')
            num = len(GLTFCreateWindowNotifier.actions)
            ac2 = scene_menu.addAction('Impot GLTF scene...',
                                       partial(self.import_gltf, num))
            ac = scene_menu.addAction('Export scene as GLTF file...',
                                      partial(self.export_gltf, num))
            GLTFCreateWindowNotifier.actions[num] = sip.unwrapinstance(win)


    @staticmethod
    def aobject_to_gltf(obj, win, gltf=None, tex_format='webp',
                        images_as_buffers=True, single_buffer=True):
        if gltf is None:
            gltf = {}

        vs = win.viewState().get()

        cppobj = getattr(obj, 'internalRep', obj)
        if obj.glAPI() is None:
            if isinstance(cppobj, ana.cpp.MObject):
                for i, sobj in enumerate(cppobj.renderedSubObjects(vs)):
                    GLTFCreateWindowNotifier.aobject_to_gltf(
                        sobj, win, gltf=gltf, tex_format=tex_format,
                        images_as_buffers=images_as_buffers,
                        single_buffer=single_buffer)
            return gltf

        glapi = cppobj.glAPI()
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
            teximages = [glapi.glBuildTexImage(vs, tex, -1, -1, False)
                         for tex in range(ntex)]
            for tex in range(ntex):
                tcoord = glapi.glTexCoordArray(vs, tex)
                #te = glapi.glTexExtrema(tex)
                #if te.scaled:
                    #tcoord = (tcoord - te.min[0]) / (te.max[0] - te.min[0]) \
                        #* (te.maxquant[0] - te.minquant[0]) + te.minquant[0]
                textures.append(tcoord)

        name = cppobj.name()
        if hasattr(name, '__call__'):
            name = name()
        gltf_io.add_object_to_gltf_dict(
            vert, norm, poly, material=mat, matrix=matrix, textures=textures,
            teximages=teximages, name=name, gltf=gltf, tex_format=tex_format,
            images_as_buffers=images_as_buffers, single_buffer=single_buffer)

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
            None, 'Export GLTF scene', '', '*.gltf *.glb')
        if not filename:
            return
        filename = filename[0]
        tex_format = 'png'
        images_as_buffers = True
        if not filename:
            return
        try:
            gltf_d = GLTFCreateWindowNotifier.win_gltf(
                win, tex_format=tex_format,
                images_as_buffers=images_as_buffers)
        except Exception as e:
            print(e)
            import traceback
            traceback.print_exc()
            raise

        gltf_io.save_gltf(gltf_d, filename, use_draco=True)

    def win_gltf(win, tex_format='webp', images_as_buffers=True):
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
            GLTFCreateWindowNotifier.aobject_to_gltf(
                obj, win, gltf, tex_format=tex_format,
                images_as_buffers=images_as_buffers)

        gltf_io.gltf_encode_buffers(gltf)

        return gltf

    @staticmethod
    def import_gltf(num):
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

        filename = Qt.QFileDialog.getOpenFileName(
            None, 'Import GLTF scene', '', '*.gltf *.glb')
        if not filename:
            return
        filename = filename[0]
        if not filename:
            return

        reader = AnaGLTFReader()
        objects = reader.load(filename, True, {})

        win.addObjects(objects)


class AnaGLTFParser(gltf_io.AimsGLTFParser):

    def parse_object(self, mesh, name):
        a = ana.Anatomist()
        obj = super().parse_object(mesh, name)
        amesh = a.toAObject(obj['mesh'])
        anaobj = amesh
        textures = obj.get('textures')
        if textures:
            atex = [a.toAObject(tex) for tex in textures]
            for tex, a_tex in zip(textures, atex):
                texinfo = tex.header().get('gltf_texture')
                if texinfo:
                    del tex.header()['gltf_texture']
                    teximage = texinfo.get('teximage')
                    if teximage is not None:
                        tname = '%s-%s' % (a_tex.name, str(uuid.uuid4()))
                        pal = ana.cpp.APalette(tname, teximage.shape[0],
                                               teximage.shape[1])
                        pal[:] = teximage.np
                        a.palettes().push_back(pal)
                        a_tex.setPalette(pal)
            if len(atex) > 1:
                atexture = a.fusionObjects(atex,
                                          method='FusionMultiTextureMethod')
            else:
                atexture = atex[0]
            anaobj = a.fusionObjects([amesh, atexture],
                                     method='FusionTexSurfMethod')
            a.takeObjectRef(anaobj)
        if name is not None:
            anaobj.setName(name)
        return anaobj

    def get_ana_referential(self, aimsref):
        anaref = ana.cpp.Referential.referentialOfUUID(aimsref)
        if anaref is None:
            a = ana.Anatomist()
            anaref = a.createReferential()
            anaref.header()['uuid'] = aimsref
        return anaref

    def set_object_referential(self, obj, ref):
        aimsref = self.get_aims_referential(ref)
        anaref = self.get_ana_referential(aimsref)
        for sobj in obj:
            if hasattr(sobj, 'surface'):
                sobj.setReferential(anaref)
            else:
                mesh = sobj.begin().next()
                mesh.setReferential(anaref)
                sobj.setReferentialInheritance(mesh)

        return obj

    def polish_result(self, mesh_dict):
        a = ana.Anatomist()
        mesh_dict = super().polish_result(mesh_dict)
        trans = mesh_dict.get('transformation_graph', {})
        refs = {}
        for sr, tdef in trans.items():
            s = self.get_ana_referential(sr)
            for dr, tr in tdef.items():
                d = self.get_ana_referential(dr)
                vec = list(tr.affine()[:3, 3].ravel()) \
                    + list(tr.affine()[:3, :3].ravel())
                atr = a.createTransformation(vec, s, d)

        return mesh_dict


class AnaGLTFReader(ana.cpp.ObjectReader.LoadFunctionClass):

    def load(self, filename, subobjects, options):
        a = ana.Anatomist()
        idle = a.theProcessor().execWhileIdle()
        a.theProcessor().allowExecWhileIdle(True)
        try:
            meshes = gltf_io.load_gltf(filename, object_parser=AnaGLTFParser())

            todo = meshes['objects']
            objects = []
            to_unreg = []
            while todo:
                obj = todo.pop(0)
                if isinstance(obj, list):
                    todo += obj
                    continue
                objects.append(obj)
                # remove objects from main control window list
                a.unregisterObject(obj)
                obj.takeAppRef()
                if isinstance(obj.getInternalRep(), ana.cpp.MObject):
                    it = obj.begin()
                    en = obj.end()
                    while it != en:
                      to_unreg.append(it.next())

            while to_unreg:
                obj = to_unreg.pop(0)
                a.unregisterObject(obj)
                a.takeObjectRef(obj)
                if isinstance(obj, ana.cpp.MObject):
                    it = obj.begin()
                    en = obj.end()
                    while it != en:
                      to_unreg.append(it.next())

            gobj = a.groupObjects(objects)

            return [gobj.getInternalRep()]
        finally:
            a.theProcessor().allowExecWhileIdle(idle)


class GLTFIOModule(ana.cpp.Module):

    def name(self):
        return 'GLTF IO'

    def description(self):
        return 'Export an Anatomist window scene as GLTF file'

    def install_menus(self):
        a = ana.Anatomist()
        a.enableListening('CreateWindow', GLTFCreateWindowNotifier())
        self.reader = AnaGLTFReader()
        ana.cpp.ObjectReader.registerLoader('gltf', self.reader)
        ana.cpp.ObjectReader.registerLoader('glb', self.reader)


gltf_mod = GLTFIOModule()
gltf_mod.install_menus()

