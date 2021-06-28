
from __future__ import print_function
from __future__ import absolute_import

import anatomist.direct.api as ana
from soma.qt_gui.qt_backend import Qt
from soma import aims, aimsalgo
import re
import numpy as np
import anacontrolmenu  # needs this one to be initialized first
from six.moves import range
from six.moves import zip


def get_resampled_volume(source, target, order, background, new_dim=None):
    '''
    Get a source volume resampled into the space of a target one
    '''
    asource = source
    atarget = target
    source = asource.toAimsObject().volume()
    target = atarget.toAimsObject().volume()
    # get a resampler for source voxel type
    t = np.asarray(source).dtype
    tc = aims.typeCode(t)
    resampler = getattr(aims,
                        'ResamplerFactory_%s' % tc)().getResampler(order)
    ref = asource.getReferential()
    tr = a.getTransformation(ref, atarget.referential)
    if tr:
        atr = tr.motion()
    else:
        atr = aims.AffineTransformation3d()
    if new_dim:
        resampler.setRef(source)
        dest = resampler.doit(
            atr, new_dim[0][0], new_dim[0][1], new_dim[0][2],
            new_dim[1][:3])._get()
    else:
        # create an output volume in dest space and dimensions
        dest = aims.Volume(target.getSize(), dtype=tc)
        resampler.resample(source, atr, background, dest)
    target_atts = ('referentials', 'transformations')
    for att in target_atts:
        if att in target.header():
            dest.header()[att] = target.header()[att]
    if 'referential' not in target.header():
        dest.header()['referential'] = atarget.referential.uuid()
    return dest

def combine_vols():
    # build GUI for formula
    w = Qt.QDialog()
    lay = Qt.QVBoxLayout()
    w.setLayout(lay)
    fw = Qt.QLabel(w.tr('enter formula:'))
    lay.addWidget(fw)
    doc = (
        '<style>.code {color: #800000; background-color: #ffffff; '
        'font-family: monospace; font-weight: bold; '
        'padding: 5px 20px 5px 20px;}</style>'
        'Formula involving volumes. Ex:'
        '<blockquote class="code">I0 * 2.3 + I1 * I0 / 2 + 4.5</blockquote>'
        'If volumes do not have the same size / resolution, resampling occurs '
        'to the smallest resolution and largest field of view, which may in '
        'some cases require much computer memory (and some time).<br/>'
        'The formula is inerpreted in Python language, using the PyAims API '
        'of Volume objects, thus it is possible to use richer expressions, '
        'such as:'
        '<blockquote class="code">I0.astype("FLOAT") * I1</blockquote>'
        'It is also possible to manipuate and/or return numpy arrays built '
        'from volumes:'
        '<blockquote class="code">np.asarray(I0).astype(np.float32)**3'
        '</blockquote>'
        'Images names are given as I0, I1 etc. or image0, image1 etc. '
        'The correspondance with available objects is given in the combobox '
        'below.')
    fw.setToolTip(w.tr(doc))
    le = Qt.QLineEdit()
    lay.addWidget(le)
    lay.addWidget(Qt.QLabel(w.tr('objects with ids:')))
    obox = Qt.QComboBox()
    lay.addWidget(obox)
    hl1 = Qt.QHBoxLayout()
    lay.addLayout(hl1)
    hl1.addWidget(Qt.QLabel(w.tr('Resampling order:')))
    rsp_combo = Qt.QComboBox()
    hl1.addWidget(rsp_combo)
    for i in range(8):
        rsp_combo.addItem(str(i))
    # resampling order could be set to 0 if all volumes are label volumes
    # (but it would need checking ang guessing for it)
    rsp_combo.setCurrentIndex(1)

    hlay = Qt.QHBoxLayout()
    lay.addLayout(hlay)
    ok = Qt.QPushButton('OK')
    hlay.addWidget(ok)
    cancel = Qt.QPushButton('Cancel')
    hlay.addWidget(cancel)
    ok.clicked.connect(w.accept)
    cancel.clicked.connect(w.reject)

    a = ana.Anatomist()
    # only volumes allowed, other sliceable are only defined as RGB
    allowed_types = ('VOLUME', )
    vols = [x for x in a.getObjects() if x.objectType in allowed_types]
    vols.sort(key=lambda x: x.name)
    for i, x in enumerate(vols):
        obox.addItem('I%d : %s' % (i, x.name))

    def obj_selected(num):
        new_text = le.text()
        if len(new_text) != 0 and not new_text.endswith(' '):
            new_text += ' '
        new_text += 'I%d' % num
        le.setText(new_text)
    obox.activated.connect(obj_selected)
    res = w.exec_()

    if res == Qt.QDialog.Accepted:
        resample_order = int(rsp_combo.currentText())
        formula = le.text()
        formula = re.sub('I([0-9]+)', 'image[\\1]', formula)
        # print('formula:', formula)
        used_im = re.findall('image\[[0-9]+\]', formula)
        if len(used_im) != 0:
            Qt.qApp.setOverrideCursor(Qt.QCursor(Qt.Qt.WaitCursor))
            try:
                used_im = set(used_im)
                # print('used:', used_im)
                image = [None] * len(vols)
                dims_vs = []
                mindim = None
                resample = False
                for im in used_im:
                    num = int(im[6:-1])
                    # print(num)
                    image[num] = a.toAimsObject(vols[num]).volume()._get()
                    vs = list(image[num].getVoxelSize())
                    dim = list(image[num].getSize())
                    tr = a.getTransformation(vols[num].referential,
                                             vols[0].referential)
                    if tr:
                        resample = True
                        vdim = [d * v for d, v in zip(dim[:3], vs[:3])]
                        atr = tr.motion()
                        tdim = atr.transform(vdim)
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([0, vdim[1],
                                                               vdim[2]]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([vdim[0], 0,
                                                               vdim[2]]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([vdim[0],
                                                               vdim[1], 0]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([0, 0,
                                                               vdim[2]]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([vdim[0], 0,
                                                               0]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([0, vdim[1],
                                                               0]))]
                        tdim = [max(x, y)
                                for x, y in zip(tdim,
                                                atr.transform([0, 0, 0]))]
                        dim = [int(np.ceil(x/v)) for x, v in zip(tdim, vs)]
                    dims_vs.append((dim, vs))
                    if mindim is None:
                        mindim = [dim, vs]
                    if dim != dims_vs[0][0] or vs != dims_vs[0][1]:
                        viewsize = [x * y for x, y in zip(*mindim)]
                        nvsize = [x * y for x, y in zip(dim, vs)]
                        new_vsize = [max(x, y) for x, y in zip(viewsize,
                                                               nvsize)]
                        mindim[1] = [min(x, y) for x, y in zip(vs, mindim[1])]
                        mindim[0] = [np.ceil(x / y)
                                     for x, y in zip(new_vsize, mindim[1])]
                        resample = True
                if resample:
                    print('resampling needed:', mindim)
                    new_dim =  mindim[0][:3], mindim[1][:3]
                    for i, im in enumerate(used_im):
                        num = int(im[6:-1])
                        if dims_vs[i] != mindim:
                            vol = image[num]
                            image[num] = get_resampled_volume(
                                vols[num], vols[0], resample_order, 0, new_dim)
                try:
                    new_image = eval(formula)
                    if isinstance(new_image, np.ndarray):
                        if mindim is not None:
                            np_image = new_image  # save ref to np array
                            del new_image
                            new_image = aims.Volume(np_image)
                            # copy volume to avoid pointing to deleted np array
                            new_image = aims.Volume(new_image)
                            new_image.header()['voxel_size'] = mindim[1]
                        else:
                            Qt.QMessageBox.critical(
                                None, w.tr('Formula error'),
                                w.tr('The formula result is a numpy array, '
                                     'but no voxel size could be found in '
                                     'image arguments'))
                            return
                    elif not hasattr(new_image, '__class__') \
                            or (not new_image.__class__.__name__.startswith(
                                'Volume_')
                                and not
                                new_image.__class__.__name__.startswith(
                                    'AimsData_')):
                        Qt.QMessageBox.critical(
                            None, w.tr('Formula error'),
                            w.tr('The formula result is not a Volume or a '
                                 'numpy array.'))
                        return
                    new_vol = a.toAObject(new_image)
                    if not resample:
                        new_vol.assignReferential(vols[0].referential)
                    a.registerObject(new_vol)
                except Exception as e:
                    Qt.QMessageBox.critical(None, str(e.__class__.__name__),
                                            str(e))
            finally:
                Qt.qApp.restoreOverrideCursor()

def init_image_math():
    a = ana.Anatomist()
    cw = a.getControlWindow()
    if cw is not None:
        # add menu in menubar
        # if there is a Python menu, add in it instead of the main menubar
        menu = cw.menuBar()
        menus = [x for x in menu.children()
                if isinstance(x, Qt.QMenu) and x.menuAction().text() == 'Python']
        if len(menus) != 0:
            menu = menus[0]
        p = menu.addMenu('Volume')
        p.addAction(cw.menuBar().tr('Combine volumes'), combine_vols)

a = ana.Anatomist()
cw = a.getControlWindow()
if not cw:
    timer = Qt.QTimer.singleShot(10, init_image_math)
else:
    init_image_math()

del cw, a
