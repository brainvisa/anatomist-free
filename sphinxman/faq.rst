
==========================================
Anatomist Frequently Asked Questions (FAQ)
==========================================

Nothing is displayed in Anatomist views
---------------------------------------

You may experience a 3D driver problem. Please check your graphics card driver installation, and possibly OpenGL libraries installation.


Meshes do display as expected, but not volumes or any textured object
---------------------------------------------------------------------

You may experience a 3D driver problem. Please check your graphics card driver installation, and possibly OpenGL libraries installation.


Why are objects and windows not always in the same referential ?
----------------------------------------------------------------

Each object and window has its own referentials, they are independant and can be different. If an object and window have different referentials and a transformation between the two referentials exists, the transformation is applied to the object in order to visualize it in the window's referential.  If the transformation doesn't exist, no transformation is applied to the object and it is exactly as if the object and the window had the same referential.


How to create a transformation ?
--------------------------------

There are several ways:

* Using *Aims* commands to register 2 volumes (``AimsMIRegister`` and ``AimsManualRegistration``).
* Using *Anatomist* :ref:`transformation control <d_ctr_transformation>` in order to do a manual registration.
* Convert *SPM* ``.mat`` files into ``.trm`` files with *BrainVISA processes*:
*Tools => converters => SPM to AIMS transformation converter* and
*Tools => converters => SPM sn3d to AIMS transformation converter*. Only the affine part of the transformation is converted.
* You can write your own transformation file.


How to reorient an image if the sagittal view is inverted ?
-----------------------------------------------------------

You can use the commands ``AimsFlip`` or ``VipFlip`` in order to invert axis X, Y or Z.


What is the difference between superimposing and merging images ?
-----------------------------------------------------------------

A fusion creates a new object. There are several types of fusion: 2 volumes, a texture and a mesh... Superimposing object also enables to visualize several objects at the same but it doesn't create a new object. Whereas a fusion is a new object which has its own properties.


Is it possible to normalize with Anatomist ?
--------------------------------------------

Anatomist can load transformations, but does not include registration or normalization algorithms.


What is the difference between loaded and displayed images in Anatomist ?
-------------------------------------------------------------------------

Loading an image doesn't modify it whereas it can be modified for display. For example, your loaded image is in radiological mode but you configured 'Display in neurological mode', so the image is modified only for display, but not on disk.


Why talking about objects rather than images ?
----------------------------------------------

Because Anatomist enables to visualize several types of objects, not only images. For example, Anatomist can display meshes, and as another example, a graph is not really an image but a structured object with nodes and attributes.


Why do I see a shift when I merge 2 normalized volumes ?
--------------------------------------------------------

Indeed, *Anatomist* and *SPM* do not manage referentials in the same way. Particularly, it is not the same voxel that is considered as the origin of the volume. But information about referentials, transformations is generally written in the file header and *Anatomist* can read this information and load the corresponding referentials and transformations. To do that, use the menu :ref:`Referential -> Load information from file header <load_referential_info>`.


Why is the origin of my volume (0,0,0) located in the top right corner of the brain, but on the left in the display ?
---------------------------------------------------------------------------------------------------------------------

Indeed, in *Anatomist*, the voxel (0,0,0), the origin of the volume is located at the front top right corner of the volume box. However as the default display convention is the radiological mode, it appears on the left. Remember: in radiological convention, the brain is seen from the bottom in axial views.

How to save a graph node (ROI, sulcus ...) into a volume ?
----------------------------------------------------------

Load the graph, select the node and right-click on it. Then choose *File -> Export mask as volume*.


How to zoom, translate and rotate a volume ?
--------------------------------------------

Use *Anatomist* :ref:`default control <d_ctr_def>`. To see which are the available keyboard shortcuts, lay the mouse on the control's icon.


How do I know the software versions (Aims, BrainVISA and Anatomist) ?
---------------------------------------------------------------------

You can run an *Aims* command with the ``--version`` option, for example:

::

  AimsFileInfo --version

The version number is also displayed at the top of Brainvisa and Anatomist interface..


How to know the volume of a region ?
------------------------------------

There are 2 ways:

#. in *Anatomist ROI toolbox*, select *Region => Morpho Stats*. The volume is displayed in the console.
#. *Aims* commands ``AimsRoiFeatures`` or ``AimsVoiStat``.


When I put a volume in a window, I don't see the volume.
--------------------------------------------------------

* Check that the window is in an orientation which does not display slices orthogonal to the view: use axial, coronal or sagittal, not 3D. Indeed, if the window is a 3D window, you have to rotate the volume. By default it is not visible. To rotate, use the middle mouse button and move the mouse.
* Maybe the view is focused on a point of view where the volume is not visible. Use *Scene => Focus view on objects* (or *Home* key).
* Maybe the slice cursor is outside the volume: move the slider of go to a known position (*Ctrl P* then enter for instance 0 0 0).


How to draw a ROI on the 3 views at the same time ?
---------------------------------------------------

Check that the option *LinkedCursor* is enabled in the *Paint* tab of the ROI toolbox and open all views (axial, coronal, sagittal et 3D) via *Painting views* in the *ROI management* tab.


Can I draw on a fusion ?
------------------------

Yes, but be careful that if fusioned volumes do not have the same voxel size, the ROI voxel size will be a mix (smallest) between them.


Is it possible to draw a ROI on a mesh with Anatomist ?
-------------------------------------------------------

Yes, it is possible since *version 4.1*. Please, see the :ref:`Surface paint module <surfpaint>`.


Two volumes are registered in SPM but do not appear matched in Anatomist
------------------------------------------------------------------------

They probably contain an internal transformation that is not used by default by Anatomist.

* load both volumes in Anatomist
* select both in the main control window
* in the right-click popup menu, select "referential / load info from file header"

Each volume should be assigned a new referential, and linked to another one by a transformation (there may be several).

In some cases, this will be enough to make it right, but not in all cases.

When performing a registration or normalization, the information telling that the destination referential for both images transformations is the same.

* open the referentials window (main window menu "settings / referentials and transformations)
* identify the referentials attached to both volumes, and the ones they are linked to through outgoing transformations. These latter may be named "Scanner-based anatomical coordinates for ...". They should be the same, but are not recognized as such.
* Draw an identity transformation linking these scanner-based referentials: ctrl+mouse draw between referentials

To fully understand all this, you may make use of the :anatomist:`Slides on the referentials system <anatomist_referentials.pdf>`, and the :ref:`chapter on coordinates systems handling in Anatomist <about-referentials>`.

* make sure you are viewing all images in windows in compatible coordinates systems: the windows referentials must have a links path to the image ones in the transformations graph.

Now this is OK for my volumes, but I have the same problem with meshes, or ROI graphs
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

It depends somewhat whether the meshes/ROIs contain the same kind of information or not (this depends on the tools which have generated them).

If the transformations information is present, the same solution as for volumes may apply.

If not, you will have to load the corresponding volumes.

In neuroimaging, each mesh or ROI set is built from (on on top of) a volume. The mesh/ROI should thus be in the same referential as the volume. So the procedure is:

* load the volumes and apply the above procedure
* select each mesh/ROI and assign it the same referential as the volume it corresponds

How to script that ?
++++++++++++++++++++

See :pyanatomist:`this topic in PyAnatomist hints <pyanatomist_howto.html#apply_transformations>`.

