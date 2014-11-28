
Anatomist programming elements
==============================

.. _dev_static_global:

Global and static variables
---------------------------

This is just a subset of some useful variables

.. |anato| replace:: :anadox:`Anatomist <classanatomist_1_1Anatomist.html>`
.. |processor| replace:: :anadox:`Processor <classanatomist_1_1Processor.html>`
.. |registry| replace:: :anadox:`Registry <classanatomist_1_1Registry.html>`
.. |fusionfactory| replace:: :anadox:`FusionFactory <classanatomist_1_1FusionFactory.html>`
.. |objectreader| replace:: :anadox:`ObjectReader <classanatomist_1_1ObjectReader.html>`

+---------------------------+-------------------------------------------------+
| |anato|                   | singleton class for global state variables      |
+---------------------------+-------------------------------------------------+
| theAnatomist              | access to the singleton                         |
+---------------------------+-------------------------------------------------+
| |processor|               | singleton class for commands processor          |
+---------------------------+-------------------------------------------------+
| theProcessor              | access to the singleton                         |
+---------------------------+-------------------------------------------------+
| |registry|                | singleton class: list of commands readers and   |
|                           | syntax; commands factory                        |
+---------------------------+-------------------------------------------------+
| Registry::instance()      | access to the singleton                         |
+---------------------------+-------------------------------------------------+
| |fusionfactory|           | singleton clas for objects fusion methods;      |
|                           | makes a new object from several input ones;     |
|                           | tells if it is possible to make a fusion        |
+---------------------------+-------------------------------------------------+
| FusionFactory::instance() | access to the singleton                         |
+---------------------------+-------------------------------------------------+
| |objectreader|            | singleton class for objects reading;            |
|                           | objects factory                                 |
+---------------------------+-------------------------------------------------+
| ObjectReader::read()      | access to the singleton                         |
+---------------------------+-------------------------------------------------+


.. _config_options:

Configuration options
---------------------

Anatomist can be customized in a personal options file. This file is located in the ``.anatomist/config`` directory of your account and is named ``settings.cfg`` (``${HOME}/.anatomist/config/settings.cfg`` on unix).

The configuration file holds the settings selected from the *Preferences* menu of Anatomist, and may contain additional, "hidden" options which are not (or not yet) accessible via the user interface. It is basicly a list of options with values and the file can have two distinct formats at the moment.

* The *"Tree"* format variant (older) looks like the following:

::

  *BEGIN TREE anatomist_settings
  anatomist_version   1.30alpha
  enableUnstable      1
  html_browser        konqueror %1 &
  *END

* The *"Python"* format variant (newer) looks like this:

::

  attributes = {
      '__syntax__' : 'anatomist_settings',
      'anatomist_version' : '1.30alpha',
      'enableUnstable' : 1,
      'html_browser' : 'konqueror %1 &',
    }

* In the future this file may move to XML format, but this is only the future.

Python format is more flexible than the older "tree" format because it allows unknown tags. This is important because it keeps compatibility with older anatomist versions: at the moment, new options saved in the settings file will prevent older anatomist versions to read the whole file. Inversely, the python format is only supported (and readable) since Anatomist 1.29.

Options value can take simple types: string, int, float, vector of int...

Here is a complete (I hope) list of the possible options:

.. raw:: html

  <table class="docutils">
    <theader align="center">
      <th><b>Option</b></th>
      <th><b>Type</b></th>
      <th><b>Default value</b></th>
      <th><b>Possible values</b></th>
      <th><b>Description</b></th>
      <th><b>Exists since anatomist version</b></th>
    </theader>
    <tbody valign="top">
    <tr>
      <td><tt>anatomist_version</tt></td>
          <td><tt>string</tt></td>
          <td></td>
          <td></td>
          <td>Higher version used</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>axialConvention</tt></td>
          <td><tt>string</tt></td>
          <td><tt>radio</tt></td>
          <td><tt>radio, neuro</tt></td>
          <td>Display convention for axial and coronal slices</td>
          <td>1.30</td>
    </tr>
    <tr>
      <td><tt>boxSelectionColorMode</tt></td>
          <td><tt>string</tt></td>
          <td><tt>gray</tt></td>
          <td><tt>gray, as_selection, custom</tt></td>
          <td>When box selection highlighting is enabled (see <tt>boxSelectionHighlight</tt>), this parameter determines how boxes colors are set. In <tt>custom</tt> mode, the custom color is given by the <tt>boxSelectionCustomColor</tt> parameter.</td>
          <td>4.2</td>
    </tr>
    <tr>
      <td><tt>boxSelectionCustomColor</tt></td>
          <td><tt>float_vector</tt></td>
          <td><tt>[ 0.7, 0.7, 0.7, 1. ]</tt></td>
          <td>RGB (3 float between 0 and 1), or RGBA</td>
          <td>When box selection highlighting is enabled (see <tt>boxSelectionHighlight</tt>), and in <tt>custom</tt> color mode (see <tt>boxSelectionColorMode</tt>), this parameter specified the boxes colors.</td>
          <td>4.2</td>
    </tr>
    <tr>
      <td><tt>boxSelectionIndividual</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt></td>
          <td>0, 1</td>
          <td>When box selection highlighting is enabled (see <tt>boxSelectionHighlight</tt>), this parameter specified whether there is one global box (<tt>0</tt>) or one box for each selected object (<tt>1</tt>).</td>
          <td>4.2</td>
    </tr>
    <tr>
      <td><tt>boxSelectionHighlight</tt></td>
          <td><tt>int</tt></td>
          <td><tt>1</tt></td>
          <td>0, 1</td>
          <td>Enables (<tt>1</tt>) or disables (<tt>0</tt>) box selection highlighting. Box selection draws a bow around selected objects in seletion control mode. Several parameters control the way bowes are rendered: see <tt>boxSelectionIndividual</tt>, <tt>boxSelectionColorMode</tt>, and <tt>boxSelectionCustomColor</tt>.</td>
          <td>4.2</td>
    </tr>
    <tr>
      <td><tt>clipBrowserValues</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt></td>
          <td><tt>positive int</tt></td>
          <td>Maximum number of characters displayed in graph, nomenclatures and other tree attributes in browsers. 0 means unlimited (default). It may be useful to set a limit here because on some Linux X servers and some Qt implementations, displaying too large text may result in anatomist crashing.
          </td>
          <td>4.0.2</td>
    </tr>
    <tr>
      <td><tt>commonScannerBasedReferential</tt></td>
          <td><tt>int</tt></td>
          <td>0</td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>Assumes all "scanner-based" referentials in image headers are the same. By default they are considered all different.</td>
          <td>4.2</td>
    </tr>
    <tr>
      <td><tt>confirmBeforeQuit</tt></td>
          <td><tt>int</tt></td>
          <td>1</td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>Display or don't display the confirmation box when quittng Anatomist.</td>
          <td>4.3</td>
    </tr>
    <tr>
      <td><tt>controlWindowLogo</tt></td>
          <td><tt>int</tt></td>
          <td>1</td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>Allows to hide the logo image and (take less space)</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>cursorColor</tt></td>
          <td><tt>int_vector</tt></td>
          <td>None</td>
          <td>RGB (3 ints)</td>
          <td>Color of the 3D windows cursor. Only valid if <tt>cursorColorAuto</tt> is 0</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>cursorColorAuto</tt></td>
          <td><tt>int</tt></td>
          <td><tt>1</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>In auto mode, a default color is taken and <tt>cursorColor</tt> is not taken
          into account</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>cursorShape</tt></td>
          <td><tt>string</tt></td>
          <td><tt>cross</tt></td>
          <td><tt>cross</tt>, <tt>circle</tt></td>
          <td>Shape of the 3D cursor. In Anatomist 1.30, only <tt>cross</tt> was implemented.
            In Anatomist 3.0, cursors can be any anatomist object. A number of cursors are
            available, and users can add their own ones in their <tt>.anatomist/cursors</tt>
            directory.
          </td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>cursorSize</tt></td>
      <td><tt>int</tt></td>
      <td><tt>20</tt></td>
      <td>positive int</td>
      <td>Size of the 3D cursor (in mm)</td>
      <td>all</td>
    </tr>
    <tr>
      <td><tt>displayCursorPosition</tt></td>
          <td><tt>int</tt></td>
          <td><tt>1</tt></td>
          <td>0, 1</td>
          <td>Display or not the statusbar in 3D windows with the cursor position and image values</td>
          <td>4.1</td>
    </tr>
    <tr>
      <td><tt>disableOpenGLSelection</tt></td>
      <td><tt>int</tt></td>
      <td><tt>0</tt></td>
      <td><tt>0</tt>, <tt>1</tt></td>
      <td>Avoid using OpenGL-based selection (in selection control, and 3D windows tooltips). It may be needed with some buggy OpenGL implementations which may cause Anatomist to crash. The "Surface Paint" tool also makes use of it in an unconditional way, so this module might still crash with such an OpenGL implementation.
      </td>
      <td>4.1</td>
    </tr>
    <tr>
      <td><tt>displayCursorPosition</tt></td>
      <td><tt>int</tt></td>
      <td><tt>1</tt></td>
      <td><tt>0</tt>, <tt>1</tt></td>
      <td>Display or not the statusbar in 3D windows with the cursor position and image values</td>
      <td>4.1</td>
    </tr>
    <tr>
      <td><tt>enableUnstable</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>Enable or disable unstable and buggy (dangerous) features</td>
          <td>1.30</td>
    </tr>
    <tr>
      <td><tt>graphDisplayMode</tt></td>
          <td><tt>string</tt></td>
          <td><tt>meshes</tt></td>
          <td><tt>meshes</tt>, <tt>voxels</tt>, <tt>all</tt>, <tt>first</tt></td>
          <td>Display mode for 3D objects in graph nodes and relations</td>
          <td>all (used since 3.0)</td>
    </tr>
    <tr>
      <td><tt>graphHierarchyAttribute</tt></td>
          <td><tt>string</tt></td>
          <td><tt>name</tt></td>
          <td><tt>name</tt>, <tt>label</tt></td>
          <td>Graph nodes attribute used to link and color with nomenclature names</td>
          <td>all (used since 3.0)</td>
    </tr>
    <tr>
      <td><tt>graphUseHierarchy</tt></td>
          <td><tt>int</tt></td>
          <td>1</td>
          <td>0, 1</td>
          <td>enable or disable the coloring of graph 3D elements according to a nomenclature
            hierarchy
          </td>
          <td>all (used since 3.0)</td>
    </tr>
    <tr>
      <td><tt>graphUseToolTips</tt></td>
          <td><tt>int</tt></td>
          <td>1</td>
          <td>0, 1</td>
          <td>enable or disable tooltips on 3D views, to display graph nodes labels</td>
          <td>all (used since 3.0)</td>
    </tr>
    <tr>
      <td><tt>html_browser</tt></td>
          <td><tt>string</tt></td>
          <td>system dependent</td>
          <td></td>
          <td>command used to run a HTML borwser for the documentation</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>language</tt></td>
          <td><tt>string</tt></td>
          <td>system default</td>
          <td><tt>en</tt>, <tt>fr</tt>, ...</td>
          <td>Translation language in the GUI</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>leftRightDisplayed</tt></td>
          <td><tt>int</tt></td>
          <td></td>
          <td></td>
          <td>used only since Anatomist 4.3</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>linkedCursor</tt></td>
          <td><tt>int</tt></td>
          <td><tt>1</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>Display / hide the 3D cursor</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>listview_background</tt></td>
      <td><tt>string</tt></td>
      <td>None</td>
      <td></td>
      <td>Image file to be displayed in "listview" widgets of the GUI: the control
      window objects and windows lists, browsers etc. This is only a gadget to customize
      the look of Anatomist. Filenames without an absolute path are taken in the
      <tt>${HOME}/.anatomist/icons/</tt> directory</td>
      <td>all</td>
    </tr>
    <tr>
      <td><tt>maxTextureUnitsUsed</tt></td>
      <td><tt>int</tt></td>
      <td>-1</td>
      <td>&gt;= -1</td>
      <td>Limit the number of OpenGL texture units used. The default is -1, unlimited. Try this option if you encounter OpenGL rendering problems. Such problems have been seen on Windows machines, where rendering was not performed at all if more than 3 texture units were enabled (even on non-tetured objects).
      </td>
      <td>4.1</td>
    </tr>
    <tr>
      <td><tt>path_list</tt></td>
      <td><tt>string</tt></td>
      <td></td>
      <td></td>
      <td>Paths list to be inserted in the pre-seelcted directories of file dialogs. It
      can be useful to reach quickly your favorite data directories</td>
      <td>all</td>
    </tr>
    <tr>
      <td><tt>selectionColor</tt></td>
          <td><tt>int_vector</tt></td>
          <td></td>
          <td>RGB (3 ints)</td>
          <td>Selection color for graph nodes</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>selectionColorInverse</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>If this option is enabled, the selection color is the negative of the original
          color of the selected object. In this case, <tt>selectionColor</tt> is not taken
          into account</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>selectionRenderingMode</tt></td>
      <td><tt>string</tt></td>
      <td><tt>ColoredSelection</tt></td>
      <td><tt>ColoredSelection</tt>, <tt>OutlinedSelection</tt></td>
      <td>Sets the way selected objects visually appear in 3D renderings. The default, <tt>ColoredSelection</tt> draws them in a different color (see <tt>selectionColor</tt> and <tt>selectionColorInverse</tt>). In <tt>OutlinedSelection</tt> mode, selected objects don't change color but have a thick outline with the selection color.
      </td>
      <td>3.1.5</td>
    </tr>
    <tr>
      <td><tt>useSpmOrigin</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>If enabled, SPM volumes with an origin are automatically assigned a referential
          and transformation at load time to take it into account. This used to be the default
          and only possible behaviour in anatomist 1.29 and previous versions, but as it is
          often annoying, it has been disabled. When disabled, the same referential can be
          created via an object-specific option of volumes</td>
          <td>1.30</td>
    </tr>
    <tr>
      <td><tt>volumeInterpolation</tt></td>
          <td><tt>int</tt></td>
          <td><tt>1</tt></td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>By default, resampled volumes values are interpolated from neighbouring voxels.
          This gives a nice smooth aspect but is slower and may not let you see the exact
          limits of the voxels.</td>
          <td>all</td>
    </tr>
    <tr>
      <td><tt>windowSizeFactor</tt></td>
          <td><tt>float</tt></td>
          <td><tt>1.5</tt></td>
          <td><tt>&gt; 0</tt></td>
          <td>Default windows size factor. 1.0 corresponds to 1 pixel for 1mm in data space,
            larger values means larger windows. Depending on the size and resolution of
            your screen, you may want to change this default value.
          </td>
          <td>3.0 (default changed from 1. to 1.5 in Anatomist 4.1)</td>
    </tr>
    <tr>
      <td><tt>windowsUseGraphicsView</tt></td>
          <td><tt>int</tt></td>
          <td><tt>0</tt> on MacOS, <tt>1</tt> on other systems</td>
          <td><tt>0</tt>, <tt>1</tt></td>
          <td>When enabled, use a Qt Graphics View as foreground layer in 3D windows: it allows to draw various things on top of the OpenGL renderings, and will be used for visual feedback during interactions, or to display information. It is used for instance for palette contol feedback.
          </td>
          <td>4.4</td>
    </tr>
    </tbody>
  </table>





