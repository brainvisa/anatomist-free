
=====================
Anatomist user manual
=====================


Introduction
============

Anatomist software enables objects visualization and handling for medical imaging. These objects are various representations of anatomical and functional information coming from images (T1 MRI, activation image issued from a parametrical statistical map, PET, hemispheres mesh, fibers bundles obtained from BrainVISA tracking process, sulci graph representation extracted from a T1 MRI image...).

Features and originality of Anatomist are based on:

* Management of several types of objects: structured (sulci, ROI graphs...) or not (3D, 4D volumes, mesh ...)
* Management of coordinate systems and transformations, for instance, you can load transformation matrix to change the object coordinate systems. For example, it is possible to merge a non normalized anatomical volume and a normalized activation map (functional data), and to have a coherent display if we load the transformation from the normalized image to the non normalized anatomical image or the reverse.
* Possibility of building complex 3D scenes with several objects of any type (merging, superimposing...).
* A lot of tools: color palettes, region of interest module, mathematic morphology, manual registration (transformation control) ...

Anatomist control window (main window) enables to have a general view of loaded objects and opened windows, to choose actions for combining objects (merging...), or controls to use on windows according to contained objects. Here's Anatomist control window:

.. figure:: _images/control_window0.png

  Anatomist control window

The general purpose is to load objects (volume, ROI graph, mesh...), change their referential (coordinate system) if needed, modify their characteristics, combining them (merging, superimposing...), and put them in windows in order to visualize them.

So, here are some base principles for using Anatomist:

* Load **objects** that represent volumes, meshes ...
* These **objects** are put in **windows**  for visualization. At a more advanced level of use, it is possible to put several objects in the same windows, for example to superimpose a mesh and a volume.
* It is also possible to handle **objects** characteristics as **referential** (coordinate system) or **colors**, for example by modifying a volume's palette.
* A window offers different **control* modes according to the type of its contained objects. For example, if a window contains a ROI graph, the *selection control* will be available and will enable to select regions in the graph.

Here are some examples about typical actions in Anatomist, for example to look at a volume:

.. raw:: html

  <div class="figure" align="center">
    <div class="mediaobject">
      <object type="application/x-shockwave-flash" data="../ana_man/en/html/images/intro.swf" width="563" height="541">
        <param name="movie" value="../ana_man/en/html/images/intro.swf"><param name="loop" value="true">
      </object>
    </div>
    Visualize and handle a volume with Anatomist
  </div>


Install and start Anatomist software
====================================

* Download: see the `BrainVISA web site download page <http://brainvisa.info/download.html>`_
* See also the :ref:`anatomist_tutorial`
* And the :axonman:`BrainVISA installation section <ch02.html>`


Configuration
=============


Configuration panel
-------------------

When starting to use Anatomist, you may need to configure some options. The most important is to choose (or to check if several users share the same Anatomist configuration) how to display axial and coronal views: either in radiological convention or in neurological convention.

To go to preferences pannel, click on *Settings* and on *Preferences*. Let's see the different tabs:

*Application* tab
+++++++++++++++++

.. figure:: ../ana_man/en/html/images/settings0.png

  *Application* tab

* *Language*: *default* (system language) / *en* (english) / *fr* (french).
* *HTML browser command line*: the browser that will be used to see HTML documentation (firefox / konqueror ...).
* *User level*: basic, advanced or expert. Some features are available only in advanced or expert mode, for example flight simulator control and storage to memory transformation automatic loading.
* *Default referentials*: this advanced option enables to choose a default referential for loaded object and for new windows independently. By clicking on the grey button, you can choose them with:

.. figure:: ../ana_man/en/html/images/settings1.png

  Default referentials configuration

*Linked cursor* tab
+++++++++++++++++++

.. figure:: ../ana_man/en/html/images/settings2.png

* *Display linked cursor*: if enabled, a symbol is displayed to represent the linked cursor position when you click on a window.
* *Cursor shape*: several shapes are available (arrow, cross, multicross ...). It is also possible to load a cursor (regular Anatomist object).
* *Size*: set cursor size.
* *Cursor color*: default color is red. You can choose another color.

*Windows* tab
+++++++++++++

.. figure:: ../ana_man/en/html/images/settings3.png

  *Windows* tab

* *Axial/coronal slices orientation*: selection of images display convention.
* *Default windows size*: windows zoom factor, by default the value is 1 for a volume whose voxels size is (1x1x1). So on screen, a pixel size is 1mm.

*Control window* tab
++++++++++++++++++++

.. figure:: ../ana_man/en/html/images/settings4.png

  *Control window* tab

* *Display nice logo*: enable displaying of Anatomist logo on top of the main window.

*Volumes* tab
+++++++++++++

.. figure:: ../ana_man/en/html/images/settings5.png

  *Volumes* tab

* *Interpolation on volumes when changing referential*: on loading a referential for an image (applying a transformation) or during a fusion, the volume is resampled by a trilinear interpolation or by the closest sibling value.
* *Use referential / transformations information found in objects headers (SPM, NIFTI...)*: if a loaded image has *spm_origin, transformations, or referentials* attributes in its header, it is possible to automatically load the corresponding referentials and transformations in Anatomist. See :ref:`load_referential_info` to know more about this feature.
* *Assume all "scanner-based" referentials are the same*: by default they are considered all different.

*OpenGL* menu
+++++++++++++

.. figure:: ../ana_man/en/html/images/settings5_opengl.png

  *OpenGL* tab

Used in advanced level user to set the number of texture in case of graphic card problems.


Preferences validation
----------------------

To keep these preferences for further sessions, you must save them:

.. figure:: ../ana_man/en/html/images/settings6.png

  *Preferences* validation

If the configuration file is shared between several users, make sure that you all use the same preferences and regularly check that your parameters haven't been changed. Indeed, if a user modifies a parameter like the  display convention (neurological or radiological), images display will change. Configuration is shared if you are identified as the same user.


Customized configuration
------------------------

You can start Anatomist with a customized configuration even if you share the same user with other people. To use a particular profile, start Anatomist with a profile name (even if it doesn't exists yet). For example:

::

  anatomist -u toto

and then save preferences to keep them for a further session.

Every profile has its own configuration directory, which is localized according to the system (*user* is the login used to connect to the computer, it can be for example your name):

* Under Linux/Mac:
  ::

    /home/user/.anatomist-toto

* Under Windows:
  ::

    C:\Documents and Settings\user\.anatomist-toto\

To start Anatomist with this profile:
::

  anatomist -u toto

