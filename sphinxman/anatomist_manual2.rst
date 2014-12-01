
=========================
Anatomist user manual (2)
=========================

Combining objects
=================

Objects superimposing
---------------------

General definition
++++++++++++++++++

Objects superimposing consists in placing several objects in the same window. For example, you can superimpose hemispheres meshes or regions of interest with the matching anatomy (cf. examples below). So there is no specific menu to do that but you can change objects color to have a better display. For example, you can modify transparency, or lights on a mesh.

Be careful, superimposed objects must be in coherent referentials in order to get informative display. This means that if superimposed objects do not come from the same object (like hemispheres meshes that comes from a T1 MRI) or if they are non equivalent volumes (different subject, modality, point of view, voxels resolution...), you'll probably have to load trnasformations between referentials in order to put all objects in the same coordinates system.


Application: Superimposing an anatomy and regions of interest (grey central nuclei)
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In the following example, 3 objects are loaded in Anatomist:

* Object1 (O1): anatomy
* Object2 (O2): regions of interest graph drawn from the anatomy. So these 2 objects are in the same referential.
* Object3 (O3): nomenclature to associate colors to regions of interest according to their name. This   object does not have to be put in a window. Link between names in the nomenclature and in regions of interest is done automatically by Anatomist.

These 2 objects are in the same coordinates system, so they will be placed in the same referential, that is to say they will have the same color circle (red by default).

.. figure:: ../ana_man/en/html/images/superpo1.png

  Superimposing an anatomy and regions of interest


Application: Superimposing hemispheres meshes and head mesh
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In the following example, 3 objects are loaded in Anatomist:

* Object1 (O1): head mesh, object obtained from a T1 MRI with BrainVISA anatomical pipeline. Opacity is decreased in the example below to enable visualization of the other objects.
* Object2 (O2): right hemisphere mesh, object obtained from the T1 MRI with BrainVISA anatomical pipeline.
* Object3 (O3): left hemisphere mesh, maillage de l'hemisphère gauche, object obtained from the T1 MRI with BrainVISA anatomical pipeline.

These three objects are in the same coordinates system, so they will be placed in the same referential, that is to say they will have the same color circle (red by default).

.. figure:: ../ana_man/en/html/images/superpo2.png

  Superimposing hemispheres meshes and head mesh


.. _object_fusion:

Objects fusion
--------------

General definition
++++++++++++++++++

.. |fusion| image:: ../ana_man/en/html/images/fusion.png

Objects fusion enables to create a new object from 2 or more other objects. Indeed, if you only put two volumes in the same window, you will see only one. To see the two volumes, you need to mix voxels from the two volumes in order to obtain a new volume. Note that fusionning more than 2 objects is only possible since 1.30 version. Besides, several new features have been added for fusion management. Several fusion combinations between objects are available,but for the moment, let's see a fusion between two 3D volumes for example:

* **STEP 1:** Load the images to merge. Here, we will fusion an anatomy and the brain mask obtained from BrainVISA anatomical pipeline.
* **STEP 2:** Select the two volumes in objects list with **Ctrl + left button**.
* **STEP 3:** Then click on fusion button  |fusion|.
* **STEP 4:** A new window pop up to select objects order and fusion type (fusion types offered differs according to selected objects, this will be detailled later, in advanced part of the manual).

.. figure:: ../ana_man/en/html/images/fusion1.png

  Fusion type

* **STEP 5:** Click on **Ok** to create the new **Fusion2D** object.
* **STEP 6:** Put the **Fusion2D** object in a window.
* **STEP 7:** If the **Fusion2D** object is all in black, you must change fusion mode. So right click on **Fusion2D** object to get its menu. Choose **Fusion => Control 2D fusion**. This window opens:

.. figure:: ../ana_man/en/html/images/fusion2.png

  Fusion control

* **STEP 8:** You can change the mapping mode. The default is **Linear**: it does a linear combination of the two volumes. The **Geometric** mode does RGB channels multiplication. For linear fusions, you can set objects transparency with the cursor **Mixing rate**.
* **STEP 9:** By default, the 2 volumes will have the same palette. To change at least one, do **Right clik menu on a volume => Color => Palette**.

.. figure:: ../ana_man/en/html/images/fusion_volumes.png

  Example of a linear fusion between two 3D volumes

.. note::

  It is possible to fusion more than 2 objects. For volumes, here is the method: with for example 3 volumes (V1, V2 and V3), Anatomist     actually fusion the last volume and the volume above in the list (V2 and V3 gives V23). Then, from this fusion object, it creates a new fusion with the volume above (V23 and V1 gives V23_1). To set parameters for each fusion, you have to select the fusion's second volume. For example, to set parameters for fusion V23_1, you must select V2.

.. note::

  In this example, we did not have to matter about objects referential management because the brain mask (brain_lesson1.ima) have been generated from the anatomy, so objects are in the same referential. But if we had done a fusion between an anatomical volume and an activation map (which is in another referential since this map comes from a functional volume), we would have to handle referentials to put the objects in a coherent coordinates system.


Types of fusions
++++++++++++++++

The following table shows the available types of fusion according to the type of objects. This list is extensible so all fusion types may not be in this list.

**Fusion descriptions:**

.. raw:: html

  <table class="docutils">
    <thead>
      <tr class="row-odd">
        <th>Objects</th>
        <th>Fusion name</th>
        <th>Description</th>
      </row>
    </thead>
    <tbody>
      <tr class="row-even">
        <td>
          Only one volume or 2D fusion <br/>
          <img src="../ana_man/en/html/images/fusion_slice_method_little.png" />
        </td>
        <td>FusionSliceMethod</td>
        <td>
          Fusion allowing to cut a volume across itself: to view/intersect 2 different slices of the same volume in the same window.
        </td>
      </tr>
      <tr class="row-odd">
        <td>
          2 or more volumes <br/>
          <img src="../ana_man/en/html/images/fusion_map_anat_little.png" />
        </td>
        <td>Fusion2DMethod</td>
        <td>
          The volumes are merged in one volume. A voxel of the resulting volume is a combination of the same voxel in each original volume.
        </td>
      </tr>
      <tr class="row-even">
        <td>
          Volume + Mesh <br/>
          <img src="../ana_man/en/html/images/fusion_map_iwhitemesh_little.png" />
        </td>
        <td>Fusion3DMethod</td>
        <td>
          Maps on the mesh a texture corrsponding to the volume values.
        </td>
      </tr>
      <tr class="row-odd">
        <td>Mesh + (Volume or 2D fusion)</td>
        <td>FusionCutMeshMethod</td>
        <td>
          Mesh cut by a plane: the cutting plane will have the texture of the volume slice. When you put this object in a 3D window, the "cut mesh" control is available. It enables to control the orientation of the slice (<emphasis>shift</emphasis>) and its position (<emphasis>ctrl</emphasis>) against the mesh.
        </td>
      </tr>
      <tr class="row-even">
        <td>2 textures</td>
        <td>FusionTextureMethod</td>
        <td>Creates a 2D texture from two 1D textures.</td>
      </tr>
      <tr>
        <td>Several textures</td>
        <td>FusionMulitTextureMethod</td>
        <td>Multi-texture: allows to map several textures on a mesh.</td>
      </tr>
      <tr class="row-odd">
        <td>Mesh + Mesh</td>
        <td>SurfaceMatcher</td>
        <td>
          Matching surfaces. This object gives access to a surface deformation algorithm. It tries to transform one surface into the other.
        </td>
      </tr>
      <tr class="row-even">
        <td>Mesh + Texture</td>
        <td>FusionTexSurfMethod</td>
        <td>Textured surface.</td>
      </tr>
      <tr class="row-odd">
        <td>Any object(s)</td>
        <td>FusionClipMethod</td>
        <td>
          Clipping: clips objects with a clipping plane. One side of the plane is cut and not displayed.
        </td>
      </tr>
      <tr class="row-even">
        <td>Volume or 2D fusion</td>
        <td>FusionRGBAVolumeMethod</td>
        <td>
          Converts an intensity-based volume (normally using a colormap) into a RGBA volume representation.
        </td>
      </tr>
      <tr class="row-odd">
        <td>Volume or 2D fusion</td>
        <td>VolumeRenderingFusionMethod</td>
        <td>
          Displays a volumic object in 3D using intensities transparency on the whole volume.
        </td>
      </tr>
      <tr class="row-even">
        <td>Textured mesh + another mesh</td>
        <td>Interpoler</td>
        <td>
          Allows to map and interpolate textures from a mesh to another mesh with a different geometry. To obtain reasonable results, it is recommended to use a "SurfaceMatcher" first, it will warp a surface towards the other one
        </td>
      </tr>
    </tbody>
  </table>


Fusion3D parameters
+++++++++++++++++++

The 3D Fusion is a fusion between a volume and a mesh. This fusion can be parameterized through the right click menu on the fusion object.

You can change:

* Fusion mode: Geometrical, linear, rate.
* Methods of interpolation: the method to estimate the value for the intersection between the mesh and the volum at each point.

  **Interpolation methods:**

  .. raw:: html

    <table class="docutils">
      <thead>
        <tr class="row-odd">
          <th>Section</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        <tr class="row-even">
          <td>Point to point</td>
          <td>
            the simplest: only the information coming from the voxel directly under the mesh vertex is used, directly. Do not use the depth and the step prameters.
          </td>
        </tr>
        <tr class="row-odd">
          <td>Point to point with depth offset (inside/outside) </td>
          <td>
            Only one voxel is taken into account, but its position is shifted along the normal to the mesh (either inside the mesh or outside), for each mesh vertex (&lt;Step&gt; is not used here).
          </td>
        </tr>
        <tr class="row-even">
          <td>Line to point </td>
          <td>
            Information is taken along the normal line, both inside and outside, with a sampling (depth and step) specified by appropriate parameters.
          </td>
        </tr>
        <tr class="row-odd">
          <td>Inside line to point</td>
          <td>
            The value corresponds to &lt;the_choosen_submethod&gt; value for the interpolation for a inside line localized at &lt;Depth&gt; and for a sampling &lt;Step&gt;
          </td>
        </tr>
        <tr class="row-even">
          <td>Outside line to point </td>
          <td>
            The value corresponds to &lt;the_choosen_submethod&gt; value for the interpolation for a ouside line localized at &lt;Depth&gt; and for a sampling &lt;Step&gt;
          </td>
        </tr>
        <tr class="row-odd">
          <td>Sphere to point</td>
          <td>
            A sampling into a sphere (depth and step parameters apply) is used to get locations in the 3D volume
          </td>
        </tr>
      </tbody>
    </table>

* Submethods: This only applies to interpolation methods that are not single-voxel (such as point to point methods)

  **Interpolation sub-methods:**

  .. raw:: html

    <table class="docutils">
      <thead>
        <tr class="row-odd">
          <th>Section</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        <tr class="row-even">
          <td>Max</td>
          <td>
            The maximum value of all voxels of the volume at the sampled locations is mapped on the mesh
          </td>
        </tr>
        <tr class="row-odd">
          <td>Min</td>
          <td>
            The minimun value of all voxels of the volume at the sampled locations is mapped on the mesh
          </td>
        </tr>
        <tr class="row-even">
          <td>Mean</td>
          <td>
            Standard mean (sum of values divided by the number of locations)
          </td>
        </tr>
        <tr class="row-odd">
          <td>Corrected mean</td>
          <td>
            Only non-nul values are taken into account in the mean computation: this is more suitable for thresholded activation maps for instance to avoid blurring the mapped values.
          </td>
        </tr>
        <tr class="row-even">
          <td>Enhanced mean</td>
          <td>
            In the enhanced mean variant, a weighting of the final value is applied depending on the proportion of null values in the set of mixed values.
          </td>
        </tr>
      </tbody>
    </table>

* Parameters: definition of localization of another point to do an interpolation

  **Interpolation parameters:**

  .. raw:: html

    <table class="docutils">
      <thead>
        <tr class="row-odd">
          <th>Section</th>
          <th>Description</th>
        </tr>
      </thead>
      <tbody>
        <tr class="row-even">
          <td>Depth</td>
          <td>Position of the other point</td>
        </tr>
        <tr class="row-odd">
          <td>Step</td>
          <td>Sampling step. Always inferior to Depth.</td>
        </tr>
      </tbody>
    </table>


.. note::

  Be aware that all this is only a visualization toy and is not very robust: no real interpolation of the volume values is performed to get a continuous intersection along the mesh: especially the methods taking points along normals can produce inaccurate results on high curvature regions (produce discontinuities, map the same voxel value on several vertices etc). The sphere mode is more robust but involves an averaging (blurring) effet, and can take values outside the brain or grey matter...


About referentials
==================

.. _load_and_display_objects:

Load and display objects
------------------------

Loading and displaying are two different actions in Anatomist. Loading is reading data stored in memory. While displaying is visualizing the object in Anatomist windows, with maybe modifications. For example, you can load data written in radiological convention and display it in neurological convention. The display options does not change data on disk.


Axis orientation
++++++++++++++++

Axis in Anatomist  are oriented like this:

* X axis: right => left
* Y axis: anterior => posterior
* Z axis: top => bottom
* T axis: 4th dimension to visualize a volume with an adding cursor to move from volume to volume; This axis can stand for the time in functional volumes vizualisation or directions for a diffusion sequence.


Neurological and radiological convention managing
+++++++++++++++++++++++++++++++++++++++++++++++++

Reading volumes on disk
#######################

When loading a volume, data organisation is supposed to match axis organisation describe before. That is to say data is supposed to be in radiological convention. In this case, data is not modified for displaying in radiological convention.

For volumes in ANALYZE format, reading and displaying data depends on the following properties (attributes in ``.minf`` file and ``.aimsrc`` configuration file):

* Attributes *spm_normalized* and *spm_radio_convention* in .minf file of the volume (GIS format).
* ``.aimsrc`` configuration file of the user.
* ``.aimsrc`` configuration file of the site.
* ``.aimsrc`` configuration file of the package.
* By default, the SPM2 mode is used.

.. note::

  If *spm_radio_convention* attribute value is 1, data in ANALYZE format is in radiological convention on disk. Else, if *spm_radio_convention* value is 0, data is in neurological convention.

  See `.aimsrc file configuration <a_aimsrc_>`_


.. _mSPM99:

What is SPM99 mode ?
####################

**The following explanations are valid only if your site / computer is configured as ours according to the flip parameter of SPM99.** So for us, non normalized volumes are in radiological convention and volumes nomralized by SPM99 are in neurological convention. That's why volumes identified as normalized volumes (according to their size in mm) are automatically flipped, to have a coherent display with data in radiological convention.

To go on working in SPM99 mode, your ``.aimsrc`` file must be configured like this:

::

  attributes = {
      '__syntax__': 'aims_settings',
      'spm_input_radio_convention': 1,
      'spm_input_spm2_normalization': 0,
      'spm_output_radio_convention': 1,
      'spm_output_spm2_normalization': 0,
  }

Volumes in analyze format are read in radiological convention (``'spm_input_radio_convention': 1``) and normalized volumes in neurological convention (``'spm_input_spm2_normalization': 0``).

**Reading normalized volumes in SPM99 mode**

As it is said before, in SPM99 mode, Aims tests the volume dimensions to see if it is normlized or not. A volume is considered as a normalized volume if its dimensions in mm are 152 < x < 165, 185 < y < 195, 130 < z < 145 or 178 < x < 185, 215 < y < 220, 178 < z < 185. In this case, the volume is supposed to be in neurological convention and it is automatically flipped at loading to display it in radiological convention.


.. _mSPM2:

What is SPM2 mode ?
###################

It is different for volumes normalized with SPM2. Indeed, there is a parameter *defaults.analyze.flip* which indicates if input data must be flipped. (for more details, see SPM2 documentation). So data normalized with SPM2 can be either in radiological convention or in neurological convention. The aim is to keep the same convention for input and output data (before and after normalization).

To work in SPM2 mode (input convention = output convention), your ``.aimsrc`` file must be configured like this if your data is in radiological convention:

::

  attributes = {
      '__syntax__': 'aims_settings',
      'spm_input_radio_convention': 1,
      'spm_input_spm2_normalization': 1,
      'spm_output_radio_convention': 1,
      'spm_output_spm2_normalization': 1,
  }

.. note::

  We will not deal with the case where input data is in radio convention and output data (normalized data) in neuro convention.

Anatomist needs make this kind of guess because there is no reliable information to indicate the convention in the Analyze format. Some header attributes have this information but it can be incorrect or out of date.

For this reason **it is strongly recommended not to use the Analyze format**, but to prefer more "modern" volume formats such as NIFTI.


Origin of volumes
+++++++++++++++++

Reading origin
##############

The origin of volumes is the voxel whose coordinates are (0, 0, 0). This voxel is located forward, on top and on the right of the volume. So, in an axial Anatomist window, this point will be at the top left corner if you keep the radiological display mode. If the origin is in mm, the origin of the volume is the centre of the voxel located at the origin.


Coordinates system
------------------

Real world sampling: coordinates in mm and in voxels
++++++++++++++++++++++++++++++++++++++++++++++++++++

Definition
##########

When loading any object (volume, mesh, ROI graph...), the real world is sampled. That is to say, the view is sampled according to the image matrix and the voxels resolution. So coordinates can be expressed in mm (real world) and in voxels (after sampling).

Mecanism
########

When you click on an object in a window, the position of the cursor appears in the console. This position is given in mm and in voxel if the object is a volume. After the position, you find the value of the voxel.

If the window contains several objects, the coordinates of each objects are displayed.

.. figure:: ../ana_man/en/html/images/coord_leg.png

  Coordinates in mm / coordinates in voxel


Coordinates systems
###################

There are several coordinates systems (referentials) managed by Anatomist more or less automatically. That is to say some transformations can be loaded automatically either via BrainVISA, or by Anatomist. For example, if a volume is identified as a normalized volume (SPM), then the transformation toward SPM referential is loaded.

* **Object referential**: this is the real world sampled like explained before.
* **Talairach-AC/PC-Anatomist referential**: In BrainVISA Morphologist pipeline (T1 anatomical images segmentation), an affine transformation  is computed: it is based on AC and PC points indicated by the user on the anatomy.
* **Talairach-MNI template-SPM referential**: Transformation applicated if the volume is normalized.

.. note::

  Anatomist always loads a transformation from Talairach-AC/PC-Anatomist referential to Talairach-MNI template-SPM referential.

Linked cursor position
++++++++++++++++++++++

The linked cursor position is defined from the origin of the object. You can move the cursor to an exact position by fixing x, y, and z via the window menu: *Scene => Manually specify linked cursor position*. You can also use the shortcut *Ctrl + P*. You specify the coordinates in mm like this: x y z.


.. _referential:

Referentials and transformations in Anatomist
---------------------------------------------

General definition
++++++++++++++++++

A referential stands for a coordinates system and can be allocated to an object. So an object can moved from its referential to another if it exists a transformation between the two referentials. This transformation enables to change the corrdinates.

A referential can be allocated to an object or to a window. A transformation between two objects enables to align one object with the other. Whereas a transformation between an object and a window  changes the point of view for visualizing the object.


Transformation format
+++++++++++++++++++++

Anatomist manages affine transformations: translation, rotation, zoom. It uses its own ``.trm`` format. It is an ASCII file, so it can be opened and modified with any text editor. It contains a 3x3 matrix and a translation vector.

.. figure:: ../ana_man/en/html/images/fichier_trm.png

  Example of a transformation ``.trm`` file

This means:

+-------+--------+-------+
| *Tx*  | *Ty*   | *Tz*  |
+-------+--------+-------+
| *Rxx* | *Rxy*  | *Rxz* |
+-------+--------+-------+
| *Ryx* | *Ryy*  | *Ryz* |
+-------+--------+-------+
| *Rzx* | *Rzy*  | *Rzz* |
+-------+--------+-------+

This format defines a translation *T* and a rotation matrix *R*. Let *R1* the refenential of a first volume and *R2* the referential of a second volume. The file ``R1_TO_R2.trm`` specifies a transformation that moves from *R1* to *R2*. So we have the following relation between coordinates of the two referentials: ``X2 = R *  X1 + T``. This can also be written using a classical 4x4 matrix, where *T* is the 4th column.

The name of transformation file is generally ``*TO*.trm`` where each ``*`` is a referential. For example: ``refimage_TO_Talairach.trm``. These transformations apply between two referentials  and modifies only display of corresponding objects. Data on disk is not modified.


Referential of an object
++++++++++++++++++++++++

Definition
##########

Each object has an associated referential. Referentials are useful to compare objects. Without referentials each objects would be in its own coordinates system (sampling of the real world according to volume dimensions and voxels size) and objects coming from different modalities or acquisition would not be comparable. With identified referentials and transformations to move from one to another, it is possible to put objects in the same referentials in order to superimpose or merge them in a consistent way.


Why changing the referential of an object, and how ?
####################################################

**Why:**

* Put an object in a new referential and load a transformation to another object in order to compare the two objects. For example, you have registered an fMRI to a T1 MRI and obtained a transformation file. You can use this transformation to visualize the two original images in a consistent way: put each object in a referential and load the transformation from fMRI referential to T1 MRI referential.

* Put a new object (o1) in the referential of another object (o2) which has already a transformation to the referential of a third object. In this case o1 and o2 are supposed to be in the same coordinate system. For example, o1 can be a mesh obtained from the anatomy o2.

**How:** Click on the object and select *Referential => Load* And then choose an existing referential or create a new one if needed.


Referential of a window
+++++++++++++++++++++++

Definition
##########

Each window is associated to a referential. This referential is used to visualize objects. If the referential of the window is different from the referential of the containing objects, it can change the point of view if there is a transformation between the two referentials.


Why changing the referential of a window, and how ?
###################################################

**Why**: For example to see several anatomical MRI in the same referential, Talairach-AC/PC-Anatomist based on AC and PC points defined in the process **Prepare subject** of BrainVISA.

**How:**

* Click on the colored bar indicating the referential. See the figure below:

  .. figure:: ../ana_man/en/html/images/ref_fen1.png

    Modifying the referential of a window (1)

* A new window opens and you can create a new referential or choose an existing one:

  .. figure:: ../ana_man/en/html/images/ref_fen2.png

    Modifying the referential of a window (2)


Loading a transformation between two referentials
+++++++++++++++++++++++++++++++++++++++++++++++++

To load a transformation between two referentials:

* Select the menu *Settings => Referential window*.
* A new window opens showing existing referentials (colored points) and transformations (arrows) between them. To load a new transformation, you need at least 2 referentials.
* To load the transformation, draw a line with the mouse from one referential to the other (take care of the direction). Then a file dialog opens and you select the transformation file.
* Windows and objects associated to these referentials are updated.


Actions on transformations: delete, save...
+++++++++++++++++++++++++++++++++++++++++++

The transformation menu is available by right click on the arrow representing the transformation (in the window *Settings => Referential window*). Here is this menu:

* **Delete transformation**: deletes the transformation between the two referentials.
* **Invert transformation**: inverts the direction of the transformation.
* **Reload transformation**: enables to change the transformation information by choosing a .trm file.
* **Save transformation**: saves the transformation in a file. Used in transformation control (manual registration).


Application: loading a transformation (coming from registration) between an anatomical volume and a functional volume.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

For example, we want to visualize an antomical image (``anat.nii``) and a functional image (``func.nii``). We need to align the volumes using a transformation matrix (previously computed ``anatTOfunc.trm``). Each volume has its own coordinates system.

* **STEP 1:** Load volumes, set a new referential for ``func.nii`` and create a linear fusion between ``anat.nii`` and ``func.nii``.
* **STEP 2:** Select the menu **Settings => Referential window**.
* **STEP 3:** Then you see the referentials window.
* **STEP 4:** Draw a line with the mouse from one referential to the other according to the direction of the transformation. A file dialog will open.
* **STEP 5:** We can see that there is a now transformation between the two referentials and that the fusion display is updated.

.. raw:: html

  <div class="figure" align="center">
    <div class="mediaobject">
      <object type="application/x-shockwave-flash" data="../ana_man/en/html/images/registration.swf" width="900" height="700">
        <param name="movie" value="../ana_man/en/html/images/registration.swf"><param name="loop" value="true">
      </object>
    </div>
    Loading a transformation between an anatomical volume and a functional volume.
  </div>


How to get a transformation file ?
++++++++++++++++++++++++++++++++++

There are several ways to get a ``.trm`` file:

* Using Aims commands to register 2 volumes (AimsMIRegister and AimsManualRegistration).
* Using Anatomist transformation control in order to do a manual registration.
* Convert SPM ``.mat`` files into ``.trm`` files using BrainVISA processes: **fMRI => converters => SPM to AIMS transformation converter** and **fMRI => converters => SPM sn3d to AIMS transformation converter**. Only the affine part of the transformation is converted.
* You can write your own transformation file.


.. _manual_registration:

Manual registration using Anatomist transformation control
##########################################################

Example: *manual registration* between a functional volume (*Vf*)  and an anatomical volume (*Va*)

#. Load the volumes *Va* and *Vf*.
#. Fusion the 2 volumes to see their relative position.
#. Put the volume that have to be moved, *Vf*, in a window.
#. Select *Vf* in the window:

  * Right click on the window
  * Choose *View / select objects*
  * select Vf in the browser window.</para>

5. In the window containing *Vf*, click on the transformation control.
#. You can move *Vf* in its window using the keyboard shortcuts.

  **NB:** When you move *Vf*, a new referential is assigned to it. Indeed, *Vf* and *Va* are in two different referentials, and the transformation between these referentials is being computed.

  **NB:** While you are moving *Vf* and *Va*, do not forget to look at all views: axial, coronal and sagittal.

7. Save the transformation:

  * *Settings -> Referential window*
  * The referential window opens. Each referential is represented by a colored round.
  * Find the arrow betwenn Va and Vf referentials
  * Right click on it and choose **Save transformation**
  * Generally, the name is something like ``*TO*.trm``, for example ``VfTOVa.trm``

8. Edit the ``.trm`` file with a simple text editor to see the parameters of the transformation.


.. _load_referential_info_man:

Using transformation information contained in SPM/NIFTI headers
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

SPM and NIFTI formats can store information about referentials and transformations in the header file. This information can be found in attributes *spm_origin, referentials, transformations*. It deals with referentials used in SPM. It can be useful to load these referentials and transformations when you have several images which are aligned in one of these referentials.

The Anatomist feature *Load information from file header* (in *object menu => referential*) loads the referentials and transformations mentionned in the header. If there is no transformation information, the feature does nothing. You can see the new referentials and transformations in the referentials window (in settings menu). The referential assigned to the object also changes. But by default, the display does not changes. Indeed, the default windows referential is *Talairach AC/PC Anatomist* and there is generally no link between this referential and the referential of the object. If there is a transformation between object's referential and another referential and you change the window's referential to this destination referential, the display will change.

If you set the user level to *Expert*, you can see another transformation when using *Load information from file header*: the transformation between the referential of the data on disk and the referential of the loaded object. This information is in the *storage_to_memory* attribute.

See also :ref:`the tutorial <load_referential_info>`


.. _a_add_palette:

.. _roi_toolbox:

.. _surfpaint_man:

:ref:`Surface paint module tutorial <surfpaint>`

.. _a_aimsrc:


