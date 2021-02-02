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

from __future__ import print_function

from __future__ import absolute_import
import anatomist.direct.api as ana
from soma.qt_gui import qt_backend
from soma.qt_gui.qt_backend import Qt
from soma import aims, aimsalgo
import numpy as np

from six.moves import range, zip


class SaveResampled(ana.cpp.ObjectMenuCallback):

    def doit(self, objects):
        # we have to work on one object
        if len(objects) != 1:
            Qt.QMessageBox.critical(
                None, 'Save resampled error',
                'Only one object should be saved at a time')
            return
        # get 1st object (in a std::set)
        obj = next(iter(objects))
        ref = obj.getReferential()
        a = ana.Anatomist()
        # get volumes which are linked to obj via a transformation
        # so that we are able to resample in their space
        vols = [o for o in a.getObjects()
                if o.getInternalRep() is not obj
                and o.objectType.startswith('VOLUME')]
        tr = [a.getTransformation(ref, v.referential) for v in vols]
        vols = [(v, t) for v, t in zip(vols, tr) if t is not None]

        if len(vols) == 0:
            Qt.QMessageBox.critical(
                None, 'Save resampled error',
                'No compatible target volume')
            return

        # gui to select target volume and resampling order
        ql = Qt.QDialog()
        l = Qt.QVBoxLayout()
        ql.setLayout(l)
        l.addWidget(Qt.QLabel('resample into space of:'))
        lw = Qt.QListWidget()
        l.addWidget(lw)
        for v in vols:
            lw.addItem(v[0].name)
        l.addWidget(Qt.QLabel('resampling order:'))
        sb = Qt.QSpinBox()
        l.addWidget(sb)
        sb.setRange(0, 7)
        sb.setValue(1)
        bl = Qt.QHBoxLayout()
        l.addLayout(bl)
        bl.addWidget(Qt.QLabel('background value:'))
        be = Qt.QLineEdit()
        bl.addWidget(be)
        be.setText('0')
        be.setValidator(Qt.QIntValidator(be))

        bl = Qt.QHBoxLayout()
        l.addLayout(bl)
        ok = Qt.QPushButton('OK')
        c = Qt.QPushButton('Cancel')
        bl.addWidget(ok)
        bl.addWidget(c)
        ok.clicked.connect(ql.accept)
        c.clicked.connect(ql.reject)
        res = ql.exec_()

        if res != Qt.QDialog.Accepted:
            return
        if len(lw.selectedItems()) != 1:
            Qt.QMessageBox.critical(
                None, 'Save resampled error',
                'Select one target volume')
            return

        # get output resampled filename
        out_filename = qt_backend.getSaveFileName(
            None, 'Save resampled volume as', '', a.objectsFileFilter())
        if out_filename in (None, ''):
            return

        Qt.qApp.setOverrideCursor(Qt.QCursor(Qt.Qt.WaitCursor))
        try:
            # get source and dest aims volumes
            sel = [vols[i]
                   for i in range(lw.count()) if lw.item(i).isSelected()]
            sel, tr = sel[0]
            source = a.AObject(a, obj).toAimsObject().volume()
            target = sel.toAimsObject().volume()
            # get a resampler for source voxel type
            t = np.asarray(source).dtype
            tc = aims.typeCode(t)
            order = sb.value()
            background = int(be.text())
            resampler = getattr(aims,
                                'ResamplerFactory_%s' % tc)().getResampler(order)
            # create an output volume in dest space and dimensions
            dest = aims.Volume(target.getSize(), dtype=tc)
            resampler.resample(source, tr.motion(), background, dest)
            target_atts = ('referentials', 'transformations')
            for att in target_atts:
                if att in target.header():
                    dest.header()[att] = target.header()[att]
            if 'referential' not in target.header():
                dest.header()['referential'] = sel.referential.uuid()
            aims.write(dest, out_filename)
        finally:
            Qt.qApp.restoreOverrideCursor()


class SaveResampledModule(ana.cpp.Module):
    callbacks_list = []

    def name(self):
        return 'SaveResampled'

    def description(self):
        return 'Save a volume resampled in the referential / field of view ' \
            'of another'

    @staticmethod
    def addMenuEntryToOptionMenu(menu):
        '''Add menu to optionMenu (new menu system API)'''
        save_resampled = SaveResampled()
        SaveResampledModule.callbacks_list.append(save_resampled)
        menu.insertItem(['File'], 'Save resampled', save_resampled)

    def objectPropertiesDeclaration(self):
        '''Add here entry to optionTree for save new menu'''
        menumap = ana.cpp.AObject.getObjectMenuMap()
        menus = {}
        # Add palette menu to all menus but only once
        for k, v in menumap.items():
            if k.startswith('VOLUME<'):
                menus[v] = k
        for m in menus.keys():
            self.addMenuEntryToOptionMenu(m)


sm = SaveResampledModule()
sm.init()
