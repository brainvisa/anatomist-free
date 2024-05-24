
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

:anausr:`See this document <config_file.html>`






