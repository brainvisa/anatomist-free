
Anatomist surface matching module
=================================

Surface matcher
---------------

This module allows to drive the deformation of a mesh towards another. It was written originally to be used for EEG applications, where the EEG model (generally a sphere) needed to be matched to the subject head to get the coordinates of EEG electrodes.

* Load 2 meshes in Anatomist
* Perform a fusion of both meshes, using the "Surface matcher" mode. This results in a new *SurfMatch* object.
* Select this *SurfMatch* object contol panel via the right-click menu *Object manipulations => Fusion =>Surface matching window*.
* Open the *SurfMatch* obect tree. It contains 3 children objects:

  * the two initial surfaces
  * a new mesh: *matchsurf.mesh*. This one will be the mesh actually deforming.

* Open a 3D window
* Put the target surface and the *matchsurf.mesh* mesh in the window. At the beginning, the deforming surface os not visible (it is uninitialized, and empty).

.. image:: ../html/images/texture/ctrl_matcher.gif
  :align: center
.. figure:: ../html/images/texture/surfmatcher_win.gif
  :align: center

  Surface matcher control window. Note the antique anatomist 1.17 graphical interface... The module has not changed a lot since this age...

* select the transformation direction in the surface matching window using the *Change* button
* Click on *Reset*: the deforming surface appears, and is identical to the source mesh.
* To see a bit more clearly, it may be useful to set different color materials to the 3 meshes, and some transparency on the target mesh (decrease the alpha coef of the :ref:`diffuse material <change_opacity_mesh>`).
* Set the matching parameters in the central column, *Matching parameters*.
* If needed, add some **control points**: pairs of points directly attracted between both surfaces.
* the *Record over time* option allows to keep the whole iterative deformation sequence in the *matchsurf.mesh* mesh, in a time sequence. The sequence can be saved on disk as a regular mesh (it contains a timestep cursor). **Warning**: it may require quite much memory since all deformed mesh steps are kept in memory, especially if the source mesh contains many vertices/polygons.
* When everything is setup, the matching begins by clicking the *Start* button.
* It is possible to interactively change matching parameters, and to add or remove control points while the deformation is still processing: changes will be taken into account at the next processing step.
* When the user is OK with the result, processing can be stopped with the *Stop* button. **Note** that the processing will **not stop on its own**: there is no convergence criterion.

.. image:: ../html/images/texture/surfmatch_3Dinit.jpg
.. image:: ../html/images/texture/surfmatch_3Dfinal.jpg

A deformed mesh has the same topology as the source mesh: same number of vertices and polygons, in the same order. Just the position of vertices have changed (to fit the destination mesh). Thus one can map textures suitable for the source mesh on the deformed one:

.. image:: ../html/images/texture/surfmatch_texture.jpg
.. image:: ../html/images/texture/surfmatch_texture_head.jpg


Intepolation of functional data from one surface to another
-----------------------------------------------------------

This interpolation is meant to map texture data on a mesh which is finer than the mesh it is normally associated with. Typically, map EEG data acquired on a set of electodes locations (generally 64, 128 or 256) on a head mesh with several thousands of vertices.

To work properly, the ineterpolation should use two "registered" meshes: the source mesh should be deformed to match the destination mesh geometry, which is typically done using the `Surface Matcher`_ feature.

If the matching has not been done, the interpolation will probably produce erratic and ugly results.

* Load both meshes
* Load the texture for the source mesh (functional data)
* Build a *Textured Surface* fusion between the source mesh and the functional texture (ie EEG electrodes net mesh, deformed to match the subject head, and EEG data)
* Make a second fusion between the textured surface (1st fusion) and the target mesh (ie head mesh). Select *Interpoler* mode.
* Put this *Interpoler* object in a 3D window, and change its palette

.. image:: ../html/images/texture/ctrl_interpoler.gif
  :align: center
.. image:: ../html/images/texture/interpoler.jpg
  :align: center

The interpolation is a linear one (fast and simple).

Interpolation takes place on the fly, time slider actions may be slower than on a regular texture.

