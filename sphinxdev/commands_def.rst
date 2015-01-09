=============================
Anatomist commands definition
=============================

* ActivateAction_
* AddObject_
* ApplyBuiltinReferential_
* Camera_
* CreateWindow_

* LoadReferentialFromHeader_


.. _ActivateAction:

.. raw:: html

  <h3>ActivateAction</h3>
  <b>New in Anatomist 4.5</b><br>
  Triggers window action activation.

  <table width="100%" class="docutils">
  <tbody>
    <tr>
      <td><b>Attribute:</b></td>
      <td><b>Type:</b></td>
      <td><b>Description:</b></td>
    </tr>
    <tr>
      <td><tt>window</tt></td>
      <td><tt>int</tt></td>
      <td id="txt">
        ID of the window to trigger action in. The action must be in the currently active control of the window.
      </td>
    </tr>
    <tr>
      <td><tt>action_type</tt></td>
      <td><tt>string</tt></td>
      <td id="txt">
        type of action: <tt>"key_press"</tt>, <tt>"key_release"</tt>, <tt>"mouse_press"</tt>, <tt>"mouse_release"</tt>, <tt>"mouse_double_click"</tt>, <tt>"mouse_move"</tt>. Additional parameters depend on the action type.
      </td>
    </tr>
    <tr>
      <td><tt>method</tt></td>
      <td><tt>string</tt></td>
      <td>
        Action method name, as registered in the active control. Deteremines what will actually be done.
      </td>
    </tr>
    <tr>
      <td><tt>x</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>x mouse coord, for mouse actions only.
      </td>
    </tr>
    <tr>
      <td><tt>y</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>y mouse coord, for mouse actions only.
      </td>
    </tr>
  </tbody>
  </table>


.. _AddObject:

.. raw:: html

  <h3>AddObject</h3>
  Adds objects in windows

  <table width="100%" class="docutils">
  <tbody>
    <tr>
      <td><b>Attribute:</b></td>
      <td><b>Type:</b></td>
      <td><b>Description:</b></td>
    </tr>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of objects to be added in specified windows</td>
    </tr>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of windows</td>
    </tr>
    <tr>
      <td><tt>add_children</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 3.1.5</b><br>
        Also add the given objects children (useful for graphs for instance)
      </td>
    </tr>
    <tr>
      <td><tt>add_graph_nodes</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 3.1.5</b><br>
        Also add the given objects children nodes if they are graphs. This is a bit more specific than the <tt>add_children</tt> option which adds all children.
        <br/>
        <b>Changed in Anatomist 3.2:</b><br/>
        The default value is now 1.
      </td>
    </tr>
    <tr>
      <td><tt>add_graph_relations</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 3.1.5</b><br>
        Also add the given objects children relations if they are graphs. This is a bit more specific than the <tt>add_children</tt> option which adds all children.
      </td>
    </tr>
  </tbody>
  </table>


.. _ApplyBuiltinReferential:

see LoadReferentialFromHeader_


.. _Camera:

.. raw:: html

  <h3>Camera</h3>
  Sets camera point of view, zoom, etc. in 3D windows.

  <table width="100%" class="docutils">
  <tbody>
    <tr>
      <td><b>Attribute:</b></td>
      <td><b>Type:</b></td>
      <td><b>Description:</b></td>
    </tr>
    <tr>
    <td><tt>windows</tt>
    </td>
    <td><tt>int_vector</tt>
    </td>
    <td id="txt">windows to act on
    </td>
    </tr>
    <tr>
    <tr>
      <td><tt>boundingbox_min</tt>
      </td>
      <td><tt>float_vector</tt> (optional)
      </td>
      <td id="txt">
        <b>New in Anatomist 3.1</b><br>
        set the viewport bounding box (min part) in the window coordinates system.
      </td>
    </tr>
    <tr>
      <td><tt>boundingbox_max</tt>
      </td>
      <td><tt>float_vector</tt> (optional)
      </td>
      <td id="txt">
        <b>New in Anatomist 3.1</b><br>
        set the viewport bounding box (max part) in the window coordinates system.
      </td>
    </tr>
    <tr>
      <td><tt>cursor_position</tt>
      </td>
      <td><tt>float_vector</tt> (optional)
      </td>
      <td id="txt">set cursor position (and also slice plane position) (cf <tt>LinkedCursor</tt>)
      </td>
    </tr>
    <tr>
      <td><tt>observer_position</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">camera position (3&nbsp;coords)</td>
    </tr>
    <tr>
      <td><tt>slice_quaternion</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">rotation of the oblique cut plane (4&nbsp;normed components)</td>
    </tr>
    <tr>
      <td><tt>slice_orientation</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">
        <b>New in Anatomist 4.4</b><br/>
        oblique cut plane orientation, as a normal vector (3&nbsp;components)</td>
    </tr>
    <tr>
      <td><tt>view_quaternion</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">rotation (4&nbsp;normed components)</td>
    </tr>
    <tr>
      <td><tt>force_redraw</tt>
      </td>
      <td><tt>int</tt> (optional)
      </td>
      <td id="txt">force redraw the view: by default views are only redrawn after a slight delay so as to only redraw once if several modifications are done. In "movie" mode, one wants to be sure that the image is correctly updated and saved.
      </td>
    </tr>
    <td><tt>zoom</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">zoom factor (1=normal)</td>
    </tr>

  </tbody>
  </table>


.. _CreateWindow:

.. raw:: html

  <h3>CreateWindow</h3>
  Opens a new Anatomist window.

  <table width="100%" class="docutils">
  <tbody>
    <tr>
      <td><b>Attribute:</b></td>
      <td><b>Type:</b></td>
      <td><b>Description:</b></td>
    </tr>
    <tr>
      <td><tt>type</tt></td>
      <td><tt>string</tt></td>
      <td>"Axial", "Sagittal", "Coronal", "3D", "Browser", "Profile"</td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID of the new window</td>
    </tr>
    <tr>
      <td><tt>geometry</tt></td>
      <td><tt>int_vector</tt> (optional)</td>
      <td>position and size of the window: x, y, w, h</td>
    </tr>
    <tr>
      <td><tt>block</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td><b>New in Anatomist 3.0</b>. Allows to insert the new window in
        a "block" which may contain several views. By default (<tt>block=0</tt>),
        the window will not be in any block. If <tt>block</tt> doesn't exist
        anymore, a new block will be created, otherwise the window will be
        added to an existing block.<br>
        <b>Note:</b> The block number is an ID just like those of objects,
        windows etc.: a number already allocated must not be reused.
      </td>
    </tr>
    <tr>
      <td><tt>block_columns</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 3.1</b>.
        If <tt>block</tt> specifies a new block to be created, then it will have this number of columns. Default: 2, but see below.<br/>
        <b>New in Anatomist 4.2</b>: The default is 2, but if specified, it will force an existing block to resize at 2 columns, whereas if unspecified, the block will be left unchanged.
      </td>
    </tr>
    <tr>
      <td><tt>block_rows</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 4.2</b>.
        If <tt>block</tt> specifies a new block to be created, then it will have this number of rows. As for <tt>block_columns</tt>, the default is 2, but if specified, it will force an existing block to resize at 2 rows, whereas if unspecified, the block will be left unchanged.<br>
        This option is incompatible with <tt>block_columns</tt>. If both are used, <tt>block_columns</tt> will override <tt>block_rows</tt>.
      </td>
    </tr>
    <tr>
      <td><tt>options</tt></td>
      <td><tt><a href="../dicttype.html">dictionary</a></tt> (optional)</td>
      <td><b>New in Anatomist 3.0</b>. Additional options passed to the new window
        upon creation. Some windows types may interpret some specific options.<br>
        For instance:<br>
        <code>options { '__syntax__' : 'tree', 'no_decoration': 1 }</code><br>
        At the moment, known options are:
        <table border="1">
          <tr>
            <td><tt>no_decoration</tt></td>
            <td><tt>bool</tt></td>
            <td>Don't draw "decorations" around the main area of the view: no menus,
              no buttons. This way the view may be inserted in a specialized application
              and be completely controlled by this application.
            </td>
          </tr>
          <tr>
            <td><tt>hidden</tt></td>
            <td><tt>bool</tt></td>
            <td><b>New in Anatomist 4.0.2</b>. Don't display the created window, until a specific show action is used (see <a href="windowconfig.html">WindowConfig</a> / raise, or in an embedded widget)</td>
          </tr>
        </table>
      </td>
    </tr>
  </tbody>
  </table>

.. _LoadReferentialFromHeader:



