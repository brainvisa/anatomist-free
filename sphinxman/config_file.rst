.. _config_file:

Configuration file and options
==============================

Anatomist can be customized in a personal options file. This file is located in the ``.anatomist/config`` directory of your account and is named ``settings.cfg``. On Unix for instance:

::

  ${HOME}/.anatomist/config/settings.cfg

The configuration file holds the settings selected from the Preferences menu of Anatomist, and may contain additional, "hidden" options which are not (or not yet) accessible via the user interface. It is basicly a list of options with values and the file can have two distinct formats at the moment.

* the "Tree" format looks like the following:

  ::

    *BEGIN TREE anatomist_settings
    anatomist_version   1.30alpha
    enableUnstable      1
    html_browser        konqueror %1 &
    *END

* The "Python"/Minf format looks like this:

  ::

    attributes = {
        '__syntax__': 'anatomist_settings',
        'anatomist_version': '1.30alpha',
        'enableUnstable': 1,
        'html_browser': 'konqueror %1 &',
      }

XML format is also supported, but is less human-readable.

The Python/Minf format is more flexible than the older "tree" format because it allows unknown tags. This is important because it keeps compatibility with older anatomist versions: at the moment, new options saved in the settings file will prevent older anatomist versions to read the whole file. Inversely, the python format is only supported (and readable) since Anatomist 1.29.

Options value can take simple types: string, int, float, vector of int...

Here is a complete (I hope) list of the possible options:

.. raw:: html

  <table class="docutils" style="width: 100%;">
    <colgroup>
       <col span="1" style="width: 15%;">
       <col span="1" style="width: 10%;">
       <col span="1" style="width: 10%;">
       <col span="1" style="width: 10%;">
       <col span="1" style="width: 50%;">
       <col span="1" style="width: 5%;">
    </colgroup>
    <thead>
      <tr class="row-odd">
        <th>Option</th>
        <th>Type</th>
        <th>Default value</th>
        <th>Possible values</th>
        <th>Description</th>
        <th>Exists since anatomist version</th>
      </tr>
    </thread>
    <tbody>
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
        <td><tt>maxPolygonsPerObject</tt></td>
        <td><tt>int</tt></td>
        <td>0 (unlimited)</td>
        <td>&gt;= 0</td>
        <td>Limit the number of displayed polygons in a single object (mesh).
          By default (0) there is no limit, but depending on the 3D hardware, displaying very large meshes (like fibers tracts sets) may flood the graphics display and even crash or hang the complete system. Limiting the number of displayed polygons will avoid such problems, but will display only a portion of the complete object. For fibers typically this is not a real problem, provided fibers are randomized before display.
        </td>
        <td>4.5</td>
      </tr>
      <tr>
        <td><tt>maxTextureUnitsUsed</tt></td>
        <td><tt>int</tt></td>
        <td>-1</td>
        <td>&gt;= -1</td>
        <td>Limit the number of OpenGL texture units used. The default is -1, unlimited. Try this option if you encounter OpenGL rendering problems. Such problems have been seen on Windows machines, where rendering was not performed at all if more than 3 texture units were enabled (even on non-textured objects).
        </td>
        <td>4.1</td>
      </tr>
      <tr>

        <td><tt>object_names_list_max_size</tt></td>
        <td><tt>int</tt></td>
        <td>300</td>
        <td>positive int</td>
        <td>Limit to the number of characters dispayed in objects lists for objects and window names. 0 means no limit.</td>
        <td>4.5.1</td>
      </tr>

      <tr>
        <td><tt>path_list</tt></td>
        <td><tt>string</tt></td>
        <td></td>
        <td></td>
        <td>Paths list to be inserted in the pre-selected directories of file dialogs. It
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
        <td><tt>setAutomaticReferential</tt></td>
        <td><tt>int</tt></td>
        <td><tt>0</tt></td>
        <td><tt>0</tt>, <tt>1</tt></td>
        <td>Automatically use any file header information specifying referentials and transformations information. Several referentials may be created, with transformations between them. The object will be assigned one of these referentials.
        </td>
        <td>all</td>
      </tr>
      <tr>
        <td><tt>unselect_on_background</tt></td>
        <td><tt>int</tt></td>
        <td><tt>0</tt></td>
        <td><tt>0</tt>, <tt>1</tt></td>
        <td>In selection mode (selection control), clicking outside any selectable object used to keep the current selection unchanged. The user thus has to use the rigt button popup menu to unselect. Thie option allows to un select when clicking on the background.
        </td>
        <td>4.6.2</td>
      </tr>
      <tr>
        <td><tt>userLevel</tt></td>
        <td><tt>int</tt></td>
        <td><tt>0</tt></td>
        <td>positive int</td>
        <td>Visibility and access to advanced or experimental features may be disabled for lower user levels, in order to keep things simple and robust for beginners.</td>
        <td>all</td>
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
        <td><tt>windowBackground</tt></td>
        <td><tt>float_vector</tt></td>
        <td><tt>1, 1, 1, 1</tt></td>
        <td>RGB (3 float between 0 and 1), or RGBA</td>
        <td>Default 3D windows background color. The default in Anatomist is white (1, 1, 1). The opacity parameter (4th color component) has no effect.
        </td>
        <td>4.5</td>
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
      <tr>
        <td><tt>async_load</tt></td>
        <td><tt>int</tt></td>
        <td><tt>0</tt></td>
        <td><tt>0</tt>, <tt>1</tt></td>
        <td>Use async loading when loading objects via the GUI. Async loading is done in background (in a separate thread) thus doesn't block the user interface. Objects are added in the list as soon as they have finished loading.<br/>
        As this is using threading, some data formats might not be robust to parallelism, which could result in crashes. If you experience such, please disable this rather experimental option.
        </td>
        <td>5.2</td>
      </tr>
      <tr>
        <td><tt>parallel_load</tt></td>
        <td><tt>int</tt></td>
        <td><tt>0</tt></td>
        <td><tt>0</tt>, <tt>1</tt></td>
        <td>Use parallel loading when loading multiple objects via the GUI. Parallel loading is done using all the available CPU cores.<br/>
        This option is compatible with the <tt>async_load</tt> option, and may be used in addition to it.<br/>
        As this is using threading, some data formats might not be robust to parallelism, which could result in crashes. If you experience such, please disable this rather experimental option.
        </td>
        <td>5.2</td>
      </tr>
    </tbody>
  </table>

