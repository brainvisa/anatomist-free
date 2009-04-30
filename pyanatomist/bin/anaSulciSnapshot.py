#!/usr/bin/env python

import os, sys, qt, sip, numpy, time
from optparse import OptionParser
import sigraph
from soma import aims, aimsalgo
import anatomist.direct.api as anatomist

a = anatomist.Anatomist()
p = a.theProcessor()

def setCamera(win, orientation):
	q = aims.Quaternion()
	q2 = aims.Quaternion()
	if orientation == 'top' : # ok
		q.fromAxis([0, 0, 1], -numpy.pi/2)
	elif orientation == 'bottom' : # ok
		q.fromAxis([0, 0, 1], numpy.pi/2)
		q2.fromAxis([1, 0, 0], numpy.pi)
		q = q.compose(q2)
	elif orientation == 'left' : # ok
		q.fromAxis([0, 1, 0], numpy.pi / 2.)
		q2.fromAxis([1, 0, 0], numpy.pi / 2.)
		q = q.compose(q2)
	elif orientation == 'right' : # ok
		q.fromAxis([0, 1, 0], -numpy.pi / 2.)
		q2.fromAxis([1, 0, 0], numpy.pi / 2.)
		q = q.compose(q2)
	elif orientation == 'back' : # ?
		q.fromAxis([1, 0, 0], -numpy.pi / 2)
		q2.fromAxis([0, 1, 0], numpy.pi)
		q = q.compose(q2)
	elif orientation == 'front' : # ?
		q.fromAxis([1, 0, 0], numpy.pi / 2)
	elif orientation.startswith('auto='):
		v = [float(x) for x in orientation.split('=')[1].split(',')]
		q.setVector(v)
	a.camera(windows=[win], zoom=1,
		observer_position=[10., 10., 10.],
		view_quaternion=q.vector(), force_redraw=True)




def display_graph(transfile, orientation, trm_name, graphname, meshname,
  imagename, selected_sulci, bb, nodisplay, label_state, hiename,
  win=None, wingeom=[0,0] ):

	# load objects
        ag = a.loadObject(graphname)
	g = ag.graph()
	if transfile and transfile != '':
		ft = sigraph.FoldLabelsTranslator(transfile)
		ft.translate(g)
	aobjects = [ag]
	if meshname:
		am = a.loadObject(meshname)
		aobjects.append(am)
	else:	am = None
	ahie = a.loadObject(hiename)

	# referential
	destination = a.centralRef
	origin = a.createReferential()
	if trm_name:
		t = a.loadTransformation(trm_name, origin, destination)
	else:
		motion = aims.GraphManip.talairach(ag.graph())
		vector = motion.toMatrix()[:3,3].tolist() + \
			motion.toMatrix()[:3,:3].T.ravel().tolist()
		t = a.execute("LoadTransformation",
			**{'matrix' : vector,
			'origin' : origin.getInternalRep(),
			'destination' : destination.getInternalRep()})
		t = a.Transformation(a, t.trans())
	ag.setReferential(origin.internalRep)
	if am: am.setReferential(origin.internalRep)

	# Windows
        if win is None:
            win = a.createWindow(wintype='3D', no_decoration=True,
                    options= {'wflags' : qt.Qt.WStyle_Customize | \
                            qt.Qt.WX11BypassWM | qt.Qt.WStyle_StaysOnTop})
        else:
            win.removeObjects( win.getObjects() )
	if selected_sulci is None:
                add_graph_nodes = True
        else:
                add_graph_nodes = False
	a.addObjects(aobjects, win, add_graph_nodes=add_graph_nodes )
	# remove cursor (referential)
	win.setHasCursor(0)
	win.Refresh()
	# hide toolbar/menu
	win.showToolbox(0)

	# display sulci
	a.execute("GraphParams", label_attribute=label_state)
        if selected_sulci is not None:
		names = ' '.join(selected_sulci)
		a.execute('SelectByNomenclature', nomenclature=ahie,
			names=names)
		a.execute('SelectByNomenclature', nomenclature=ahie,
			names=names, modifiers='toggle')

	# see sulci colors
	ag.setColorMode(ag.Normal)
	ag.updateColors()
	ag.notifyObservers()
	ag.setChanged()

	# zoom
	if not bb:
		setCamera(win, orientation)
		win.internalRep.focusView()
		info = a.execute('ObjectInfo', objects=[win])
		res = info.result()
		info = [x for x in res][0]
		bb = info['boundingbox_min'], info['boundingbox_max']
		bb = list(bb[0]), list(bb[1])
	else:
		setCamera(win, orientation)
		win.internalRep.focusView()
		a.camera(windows=[win],
			boundingbox_min=bb[0], boundingbox_max=bb[1])
	if nodisplay: return [win, ag, am, ahie, bb]

	# fullscreen + snapshot
        if wingeom == [0,0]:
            fullscreen = True
            win.setWindowState(win.windowState() | qt.Qt.WindowFullScreen)
        else:
            fullscreen = False
	desktop_geometry = qt.qApp.desktop().geometry()
        if not fullscreen:
            desktop_geometry.setWidth( wingeom[0] )
            desktop_geometry.setHeight( wingeom[1] )
	win.setFocus()
	win.raiseW()
	a.execute("WindowConfig", windows=[win],
		geometry=desktop_geometry.coords(), snapshot=imagename)

	return [win, ag, am, ahie, bb]


def compute_bb(bbfile):
	fd = open(bbfile)
	mi, ma = [], []
	for l in fd.readlines():
		bb = eval(l)
		mi.append(bb[0])
		ma.append(bb[1])
	mi = numpy.vstack(mi)
	ma = numpy.vstack(ma)
	bb = mi.min(axis=0), ma.max(axis=0)
	fd.close()
	return bb

def parseOpts(argv):
	description = 'Snapshots...\n--mesh is optional.'
	parser = OptionParser(description)
	parser.add_option('--orientation', dest='orientation',
		metavar='TYPE', action='store', default='left',
		help='one of left, right, top, bottom (default : %default)')
	parser.add_option('-g', '--graph', dest='graphname',
		metavar='FILE', action='store', default=None,
		help='graph of sulci')
	parser.add_option('-m', '--mesh', dest='meshname',
		metavar='FILE', action='store',	default=None,
		help='mesh : grey/white')
	parser.add_option('-t', '--transformation', dest='trm_name',
		metavar='FILE', action='store',	default=None,
		help='trm file : subject -> Talairach, default: take it from graph')
	parser.add_option('-o', '--outimage', dest='imagename',
		metavar='FILE', action='store',	default=None,
		help='output image name')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'NAME', action='store', default = None,
		help='Select and display only specified sulci (coma-separated ' \
                'list)')
	parser.add_option('-q', '--quaternion', dest='quaternion',
		metavar = 'VALUES', action='store', default = None,
		help='Set orientation of camera by hand from anatomist '
		'quaternion (see history.ana). 4 floatting values splitted '
		'by commas. ex: 1,2,3,4')
	parser.add_option('-b', '--bounding-box', dest='bb',
		metavar = 'VALUES/FILE', action='store', default = None,
		help='Set bounding box of camera (default: autofocus).\n'
		'VALUES=6 floatting values splitted by comas and semi-column. '
		'ex: 1,2,3;4,5,6\nFILE=list of bounding box, one per line. '
		'ex: ([1, 2, 3], [4, 5, 6])')
	parser.add_option('-f', '--bounding-box-file', dest='bbfile',
		metavar = 'FILE', action='store', default = None,
		help='Append boundingbox to this file (one per line)')
	parser.add_option('--nodisplay', dest='nodisplay',
		action='store_true', default = False, help='no display.')
	parser.add_option('--label-mode', dest='label_state',
		metavar = 'FILE', action='store', default = 'name',
		type='choice', choices=('label', 'name'),
		help="use 'label' or 'name' attribute to display sulci " + \
		"(default : %default)")

	shared_path = aims.carto.Paths.shfjShared()
	transfile = os.path.join(aims.carto.Paths.shfjShared(), 'nomenclature',
		'translation', 'sulci_model_noroots.trl')
	hiename = os.path.join(shared_path, 'nomenclature',
			'hierarchy', 'sulcal_root_colors.hie')
	parser.add_option('--translation', dest='transfile',
		metavar = 'FILE', action='store', default = transfile,
		help='translation file (default : %default)')
	parser.add_option('--hie', dest='hiename',
		metavar='FILE', action='store', default=hiename,
		help='hiearchy (default : %default)')
        parser.add_option( '--size', dest='size', metavar = 'VALUES',
                action='store', default = None,
		help='Size (width,height) of the snapshot. 2 int values '
                'separated by a coma. ex: 1024,768, 0,0: fullscreen (default)')

	return parser, parser.parse_args(argv)


def main( argv ):
	'''
    orientation : left, right, bottom, top, auto=q1,q2,q3,q4
	'''
	# options
	parser, (options, args) = parseOpts(argv)
	if None in [options.graphname, options.imagename]:
		parser.print_help()
		sys.exit(1)
	if options.quaternion is not None:
		options.orientation = 'auto=%s' % options.quaternion
	if options.sulci:
		selected_sulci = options.sulci.split(',')
	else:	selected_sulci = None
	if options.bb:
		if os.path.exists(options.bb):
			bb = compute_bb(options.bb)
		else:
			bb = [[float(x) for x in b.split(',')] \
					for b in options.bb.split(';')]
	else:	bb = None
        if options.size is None:
            geom = [ 0, 0 ]
        else:
            geom = [ int(x) for x in options.size.split(',')[:2] ]
	
	# display
	obj = display_graph(options.transfile, options.orientation,
		options.trm_name, options.graphname, options.meshname,
		options.imagename, selected_sulci, bb, options.nodisplay,
		options.label_state, options.hiename, wingeom=geom)
	bb = obj[4]

	if options.bbfile: # write bounding box
		fd = open(options.bbfile, 'a')
		fd.write(str(bb) + '\n')
		fd.close()

        #print 'done.'
        return obj

if __name__ == '__main__':
  main( sys.argv )
  sys.exit(0)
  #qt.qApp.exec_loop()
