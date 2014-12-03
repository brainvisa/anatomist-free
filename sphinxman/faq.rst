
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


