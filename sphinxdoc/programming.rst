
Anatomist programming doc
=========================

Anatomist is written in C++ language, and has Python bindings.

.. |anadox| image:: ../html/images/program.png
  :height: 64pt
  :target: ../doxygen/index.html
.. |pyana| image:: ../html/images/program.png
  :height: 64pt
  :target: ../../pyanatomist/sphinx/index.html


Links to API docs
-----------------

|anadox| `C++ API <../doxygen/index.html>`_

|anadox| :pyanatomist:`PyAnatomist: Anatomist in Python <index.html>`


Other programming tips (C++)
----------------------------

Mainly for C++, but some things are language-independent or also appy to the low-level Python layer.

General structure
+++++++++++++++++

* Objects : AObject base class
* Windows: AWindow base class
* Application and global registry : Anatomist class
* Main control window: ControlWindow class
* There are also a set of static `global variables, registry systems and customizable factories <../html/fr/programmation/globals.html>`_

Configuration options
+++++++++++++++++++++

`Configuration options <../html/en/programming/config.html>`_

Commands system
+++++++++++++++

`Commands system and commands list <../html/fr/programmation/commands.html>`_ (mainly in french...)


More or less obsolete docs
++++++++++++++++++++++++++

and in french...

* `Programming a new object type <../html/fr/programmation/new_aobject.html>`_
* `Programming a new window type <../html/fr/programmation/new_awindow.html>`_
* `Adding a new property on an object <../html/fr/programmation/new_optionTree.html>`_ (right-click and *Object specific* menus)
* `Making a new module <../html/fr/programmation/new_module.html>`_

* `Vew / controler model in Anatomist <html/fr/programmation/controls.html>`_

