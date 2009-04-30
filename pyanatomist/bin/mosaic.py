#!/usr/bin/env python
import sys, os
import numpy
from optparse import OptionParser
from PIL import Image

def create_cmd(images, (nx, ny), sizes, (bbx, bby)):
	args = ''
	px = 0
	ind = 0
	for i in range(nx):
		py = 0
		for j in range(ny):
			sx, sy = sizes[ind]
			ox = (bbx - sx) / 2.
			oy = (bby - sy) / 2.
			args += "-page %dx%d+%d+%d %s " % \
				(sx, sy, px + ox, py + oy, images[ind])
			py += bby
			ind += 1
			if ind >= len(images): break
		px += bbx

	return args

def compute_bb(images):
	sizes = []
	bbx, bby = 0, 0
	for filename in images:
		img = Image.open(filename)
		sx, sy = img.size
		sizes.append((sx, sy))
		if sx > bbx: bbx = sx
		if sy > bby: bby = sy
		del img
	return sizes, (bbx, bby)

def shape_square(n):
	nx = int(numpy.sqrt(n))
	ny = int(n / nx)
	if nx * ny < n: ny += 1
	return nx, ny

def shape_ratio(n, bb, ratio):
	bbx, bby = bb
	wratio = ratio * float(bby) / float(bbx)
	h = int(numpy.ceil(numpy.sqrt(float(n) / wratio) - 0.5))
	emin, hmin, wmin = numpy.inf, h, int(n / h)
	while 1:
		w = int(n / h)
		if w * h < n: w += 1
		e = (wratio - w / h) ** 2
		if e < emin: emin, hmin, wmin = e, h, w
		else: break
		h += 1
	w, h = wmin, hmin
	print "size = ", bbx * w, bby * h
	print "ratio = ", float(bbx * w) / float(bby * h)
	return w, h


def parseOpts(argv):
	description = 'Mosaic...\n' + \
	'Usage: mosaic.py -o output -n n     [-r] img1.png img2.png...'+\
	'       mosaic.py -o output -s n1xn2 [-r] img1.png img2.png...'
	parser = OptionParser(description)
	parser.add_option('-o', '--output', dest='output',
		metavar='FILE', action='store', default='left',
		help='output filename. If several files are created there will '
		'be named file_N.ext with the given file.ext parameter and N a '
		' variable number for each output.')
	parser.add_option('-n', '--number', dest='number',
		metavar='INT', action='store', default=1,
		help='number of pages')
	parser.add_option('-s', '--shape', dest='shape',
		metavar='INTxINT', action='store', default=None,
		help='number of rows x columns example: 3x2 (default: disable)')
	parser.add_option('-r', '--ratio', dest='ratio',
		metavar='FLOAT', action='store', default=str(4./3),
		help='page ratio (default : 4./3 landscape)')

	return parser, parser.parse_args(argv)



def main():
	'''
    mosaic output images
	'''
	parser, (options, args) = parseOpts(sys.argv)
	images = args[1:]
	if None in [options.output]:
		parser.print_help()
		sys.exit(1)

	global_n = len(images)
	ratio = float(options.ratio)
	if options.shape is None:
		number = int(options.number)
		# shape
		page_n = (global_n / number)
		if page_n * number < global_n: page_n += 1
	else:
		shape = [int(x) for x in options.shape.split('x')]
		page_n = (shape[0] * shape[1])
		number = int(numpy.ceil(float(global_n) / page_n))

	offset = 0
	for i, page in enumerate(range(number)):
		print "**** page ****"
		offset_high = min(global_n + 1, offset + page_n)
		page_images = images[offset:offset_high]
		print page_images, offset, offset_high, page_n
		sizes, bb = compute_bb(page_images)
		if options.shape:
			nx, ny = shape
			if bb[0] < bb[1]:
				bbx = bb[1] * (ratio * ny) / nx
				bby = bb[1]
			else:
				bbx = bb[0]
				bby = bb[0] * nx / (ratio * ny)
			bb = bbx, bby
		else:	nx, ny = shape_ratio(offset_high - offset, bb, ratio)
		print "bb = ", bb
		print "mosaic shape : ", (nx, ny)

		args = create_cmd(page_images, (nx, ny), sizes, bb)
		# plop.ext -> plop_N.ext
		if number != 1:
			ind = options.output.rfind('.')
			output = options.output[:ind] + ('_%d' % i) + \
				options.output[ind:]
		else:	output = options.output
		cmd = "convert " + args + " -mosaic %s" % output
		print "cmd = ", cmd
		os.system(cmd)
		offset += page_n

main()
