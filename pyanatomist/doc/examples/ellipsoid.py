#!/usr/bin/env python
from soma import aims
import anatomist.direct.api as anatomist
import time, numpy


a = anatomist.Anatomist()

def rotation_3d(a1, a2, a3):
	'''
    Return matrix 4x4 of rotation along 3 canonic axis based on 3 angles.

    - a1, a2, a3 : rotation angles.
	'''
	c1, s1 = numpy.cos(a1), numpy.sin(a1)
	c2, s2 = numpy.cos(a2), numpy.sin(a2)
	c3, s3 = numpy.cos(a3), numpy.sin(a3)
	m1 = numpy.matrix([
		[c1, -s1, 0, 0],
		[s1, c1,  0, 0],
		[0,   0,  1, 0],
		[0,   0,  0, 1]])
	m2 = numpy.matrix([
		[1,  0,   0, 0],
		[0, c2, -s2, 0],
		[0, s2,  c2, 0],
		[0,  0,   0, 1]])
	m3 = numpy.matrix([
		[c3, 0, -s3, 0],
		[0,  1,   0, 0],
		[s3, 0,  c3, 0],
		[0,  0,   0, 1]])
	return m1 * m2 * m3

def apply_transform(mesh, scaling, rot, translate):
	'''
    Transform mesh with scaling, translation and rotation matrix.
	'''
	mesh2 = aims.AimsTimeSurface_3(mesh)
	# create transformation
	scale = numpy.asmatrix(numpy.diag(scaling))
	rot[:, 3] = numpy.asmatrix(translate).T
	cov = rot * scale * rot.I
	# apply transformation
	motion = aims.Motion(numpy.asarray(cov).flatten())
	aims.SurfaceManip.meshTransform(mesh2, motion)
	# update mesh in anatomist object
	asphere.setSurface(mesh2)

def transform(mesh, angles, scaling, translate):
	'''
    Create and apply transformation to mesh.
	'''
	angles[0] += 0.02
	angles[1] += 0.03
	angles[2] += 0.05
	a1, a2, a3 = angles
	scaling = numpy.array([1.1 + numpy.cos(a1 * 3), 1.1 + numpy.cos(a2 * 2),
	 				1.1 + numpy.cos(a3), 0]) * 0.5
	scaling[3] = 1.
	rot = rotation_3d(a1, a2, a3)
	apply_transform(mesh, scaling, rot, translate)

mesh = aims.SurfaceGenerator.sphere(aims.Point3df(0,0,0), 1, 500)
asphere = a.toAObject(mesh)
aw = a.createWindow('3D')
aw.setHasCursor(0)
a.addObjects([asphere], [aw])


if __name__ == '__main__' :
	import qt
	angles = numpy.array([0., 0., 0.])
	scaling = numpy.array([1., 1., 1., 0.])
	translate = numpy.array([0, 0, 0, 1])
	while 1:
		transform(mesh, angles, scaling, translate)
		asphere.setChanged()
		asphere.notifyObservers()
		qt.qApp.processEvents()
		time.sleep(0.01)
