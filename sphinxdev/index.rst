.. image:: ../sphinxman/images/anatomist.png
  :align: center


Anatomist developers documentation
==================================

Anatomist is written in C++ language, and has Python bindings.

.. |anadox| image:: ../user_doc/_images/program.png
  :height: 64pt
  :target: ../doxygen/index.html
.. |pyana| image:: ../user_doc/_images/program.png
  :height: 64pt
  :target: ../../pyanatomist/sphinx/index.html


Links to API docs
-----------------

|anadox| `C++ API <../doxygen/index.html>`_

|pyana| :pyanatomist:`PyAnatomist: Anatomist in Python <index.html>`


Other programming tips (C++)
----------------------------

Mainly for C++, but some things are language-independent or also appy to the low-level Python layer.


General structure
+++++++++++++++++

* Objects : :anadox:`AObject <classanatomist_1_1AObject.html>` base class
* Windows: :anadox:`AWindow <classanatomist_1_1AWindow.html>` base class
* Application and global registry : :anadox:`Anatomist <classanatomist_1_1Anatomist.html>` class
* Main control window: :anadox:`ControlWindow <classControlWindow.html>` class
* There are also a set of static :ref:`global variables, registry systems and customizable factories <dev_static_global>`


Configuration options
+++++++++++++++++++++

:ref:`Configuration options <config_options>`


Commands system
+++++++++++++++

:doc:`Commands system and commands list <commands>` (mainly in french...)

Objects file format (.aobj)
+++++++++++++++++++++++++++

:doc:`Anatomist objects file format <aobj_format>` (since Anatomist 4.6)


View/controler model
++++++++++++++++++++

:doc:`controler`


More or less obsolete docs
++++++++++++++++++++++++++

and in french...

* `Programming a new object type <_static/old_prog_fr/new_aobject.html>`_
* `Programming a new window type <_static/old_prog_fr/new_awindow.html>`_
* `Adding a new property on an object <_static/old_prog_fr/new_optionTree.html>`_ (right-click and *Object specific* menus)
* `Making a new module <_static/old_prog_fr/new_module.html>`_

* `Vew / controler model in Anatomist <_static/old_prog_fr/controls.html>`_


Contents
++++++++

.. toctree::
  :maxdepth: 1

  commands
  developer_details
  controler
  aobj_format
