
Anatomist objects file format
=============================

For decades Anatomist was able to load various data files formats, containing images, meshes, textures, voxels lists, graphs or ROI sets, etc. But there has been no way to save the various fusion objects created within Anatomist, and to load them back. The only way was scripting to re-create the objects.

Anatomist 4.6 finally introduces the means to save all kinds of objects together with their properties. Users can now select any objects (one or several), and save them in a single ``.aobj`` file. Loading then can bring back several objects.

The ``.aobj`` format is not meant to replace standard and widely used file formats from the neuroimaging community: it will not save images, or meshes, or any object which already used to have an associated file. Rather it describes the structure of objects (their combination in a fusion, for instance) and their properties. It is a simple JSON format text file (very very similar to ``.minf`` used in AIMS). It stores lists of objects, each described by character strings or dictionaries.


The syntax of a ``.aobj`` file is the following:
------------------------------------------------

* Lists are between square brackets ``[`` ... ``]``
* Dictionaries are between curly braces ``{`` ... ``}``
* Strings are between double quotes ``"`` ... ``"``
* Numbers are directly as numbers
* Elements are separated by comas ``,``
* Line breaks are optional and may occur between elements


Description of objects inside a ``.aobj`` file:
-----------------------------------------------

.. code-block:: bash

    object_descr: <object> or [<object> ...]

``<object>`` describes an object:

.. code-block:: bash

    object: <identifier> | <filename> | {<obj_dict>}

*identifier*: (string)
    internal name of the object, allowing to reuse the same sub-object without duplicating/reloading it. If given alone, it is a reference to an existing object which has already been assigned this identifier

*filename*: (string)
    file name for an object to be loaded. If the name is a relative file name, it is relative to the directory of the ``.aobj`` file.
    Filenames are also used as identifiers, unless given a different value through a dictionary description, so their syntax is the same: the second time a filename is used, the same object is shared.

``obj_dict``: dict describing an object. It may contain several keys:

    ``identifier`` (str, optional):
        assign an ID to the new object
    ``object_type`` (str, optional):
        set the Anatomist object type (``"FUSION2D"``, ``"TEXTURED SURF."``...). If not set or set to ``"list"``, it assumes the object is not an *AObject* but a list of objects.
    ``fusion`` (str, optional):
        instead of the object type, specifies a fusion method which should be used to build the object from its children. It may be more precise than ``object_type`` in ambiguous cases.
    ``name`` (str, optional):
        name set to the object in Anatomist
    ``objects`` (mandatory):
        .. code-block:: bash

            <object> | [<object> ...]

        describes the object(s) inside the current one. If it is a list, then objects are its children.
    ``properties`` (dict, optional):
        properties attached to the object. They are added to the object header, to all properties supported by anatomist header parsing are supported (``material``, ``palette``, ``texture_properties``...).
        Additionally, some fusion properties are supported:

        ``texturing_params``:
            with parameters corresponding to the :ref:`TexturingParams` command
        ``fusion3dparams``:
            with parameters corresponding to the :ref:`Fusion3DParams` command
        ``slice_plane``: (float_vector)
            for objects containing a slice plane (slice objects, cutmesh, clipped objects), this specifies the slice plane equation parameters (4 coefficients).

Example
-------

::

    [
        {
            "object_type": "FUSION2D",
            "name": "Fusion_IRM_SPECT",
            "identifier": "irm_spect",
            "objects": [
                "irm.ima",
                {
                    "identifier": "spect",
                    "objects": "spect.ima",
                    "properties": {
                        "palette": {"palette": "Blue-Red"}},
                    "name": "SPECT",
                }
            ],
            "properties": {
                "texturing_params": {
                    "mode": "linear",
                    "rate": 0.8,
                    "texture_index": 1,
                },
            }
        },
        {
            "identifier": "tex_mesh",
            "fusion": "FusionTexSurfMethod",
            "objects": [
                "ra_head.mesh",
                {
                    "objects": "ra_head_tex.gii",
                    "properties": {
                        "palette": {"palette": "Blue-Red-fusion_invert"},
                        "texture_properties": [{"interpolation": "rgb"}],
                    }
                },
            ],
        }
    ]

