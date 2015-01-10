
=========================
Anatomist commands system
=========================

Syntaxe des commandes
=====================

.. raw:: html

  Les commandes peuvent &ecirc;tre
  <li> lues dans un fichier
  de  config.     depuis  le menu "File / Replay scenario" de la fen&ecirc;tre
  de contr&ocirc;le</li>
  <li> lues dans un fichier depuis le d&eacute;marrage d'Anatomist:
  "<tt>anatomist -f fichier.his</tt>"</li>
  <li> lues sur un "pipe nomm&eacute;" ouvert au d&eacute;marrage
    et  sur   lequel on peut &eacute;crire n'importe quand (-&gt; t&eacute;l&eacute;commande):
  "<tt>anatomist -p nom_pipe</tt>"</li>

  <p><br>
  La syntaxe des commandes est celle des arbres de la librairie
  <b>  graph</b>        de Dimitri Papadopoulos: </p>

  <blockquote>
    <pre>*BEGIN TREE syntaxe<br>attribut1&nbsp;&nbsp;&nbsp; valeur1<br>attribut2&nbsp;&nbsp;&nbsp; valeur2<br>...<br>*END</pre>
                </blockquote>
                La syntaxe est connue d'Anatomist (chaque commande d&eacute;clare
      la  syntaxe   qu'elle attend). <br>
                Chaque type syntaxique (ou type d'arbre, attribut donn&eacute;
    apr&egrave;s      "<tt>*BEGIN TREE</tt>") d&eacute;cide des attributs qu'il
    est possible   de  trouver dans cet &eacute;l&eacute;ment (arbre). <br>
                Les attributs ont chacun un type, et peuvent &ecirc;tre obligatoires
      ou  non.  <br>
                Les types de base pour les attributs sont: <br>
                &nbsp; <br>

  <table class="docutils">
  <thead>
    <th>Type</th>
    <th>Example</th>
  </thead>
  <tbody>
    <tr>
      <td><tt>int</tt></td>
      <td><tt>12</tt></td>
    </tr>
    <tr>
      <td><tt>float</tt></td>
      <td><tt>-134.654</tt></td>
    </tr>
    <tr>
      <td><tt>string</tt></td>
      <td><tt>toto et tutu</tt></td>
    </tr>
    <tr>
      <td><tt>int_vector</tt></td>
      <td><tt>1 20 -2 4 87</tt></td>
    </tr>
    <tr>
      <td><tt>float_vector</tt></td>
      <td><tt>1.2 -3.4 0.123 -12.</tt></td>
    </tr>
    <tr>
      <td><tt><a href="#dicttype">dictionary</a></tt></td>
      <td><tt>{ '__syntax__' : 'dictionary', 'an_attribute' : [ 1, 5.3, 'value' ] }</tt></td>
    </tr>
    </tbody>
  </table>

  <p>Les commandes sont encapsul&eacute;es dans des arbres imbriqu&eacute;s
        a priori sur 2 niveaux: le premier pr&eacute;cise si la commande doit
    &ecirc;tre    ex&eacute;cut&eacute;e ou annul&eacute;e: syntaxe <tt>EXECUTE</tt>
      ou   <tt>  UNDO</tt>.   Le second niveau est la commande elle-m&ecirc;me.
    Ex:    </p>

  <blockquote>
    <pre>*BEGIN TREE EXECUTE<br>*BEGIN TREE LoadObject<br>filename&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; toto.ima<br>res_pointer&nbsp;&nbsp;&nbsp; 1<br>*END<br>*END</pre>
                </blockquote>
                Les objets et fen&ecirc;tres sont manipul&eacute;es en interne
    dans   Anatomist   par leurs pointeurs. Evidemment ce format n'est pas exportable
      dans les entr&eacute;es/sorties disque. On passe donc par une conversion
    pointeur - identifiant entier, effectu&eacute;e par les classes <tt>Serializer</tt>
          et <tt>Unserializer</tt>. Le principe est qu'un pointeur en m&eacute;moire
      est identifi&eacute; depuis les commandes par un entier. L'entier est
  attribu&eacute;    au pointeur depuis sa cr&eacute;ation, par la commande
  qui r&eacute;sulte    en la cr&eacute;ation de pointeur (nouvel objet charg&eacute;
  ou cr&eacute;e    par fusion, nouvelle fen&ecirc;tre ouverte, etc.). Ces
  commandes ont g&eacute;n&eacute;ralement    un attribut "<tt>res_pointer</tt>
  " qui donne l'identifiant entier qui sera   ensuite utilis&eacute; pour r&eacute;f&eacute;rer
  &agrave; ce pointeur (objet,   fen&ecirc;tre, r&eacute;f&eacute;rentiel...).
    <br>
                Dans la suite, &agrave; chaque fois qu'on manipule des objets,
    des   fen&ecirc;tre,    des r&eacute;f&eacute;rentiels, on les d&eacute;signe
    par  cet identifiant.    <br>
                Ex: la commande suivante met les objets d'identifiants 1, 2
  et  3  dans   les  fen&ecirc;tres d'identifiants 100 et 101.

  <blockquote>

    <pre>*BEGIN TREE EXECUTE<br>*BEGIN TREE AddObject<br>objects&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 1 2 3<br>windows&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 100 101<br>*END<br>*END</pre>
                </blockquote>
                <b><font color="#cc0000"><font size="+1">Attention:</font></font></b>
          dans  les "scripts" de commande, ne jamais confondre des identifiants
      d'&eacute;l&eacute;ments    de types diff&eacute;rents (par exemple un
  ID   de fen&ecirc;tre dans une  liste d'objets), c'est pas du tout blind&eacute;
      pour le moment et &ccedil;a  doit pouvoir faire planter m&eacute;chamment.

  <h4> Pr&eacute;cisions sur les identifiants:</h4>
                  <li> Il sont locaux &agrave; un "flux d'entr&eacute;e" donn&eacute;
      dans  Anatomist: un fichier sc&eacute;nario, ou un pipe. En clair &ccedil;a
      veut  dire que lorsqu'on lit 2 fichiers de sc&eacute;nario &agrave; la
    suite,   le  second ne peut pas r&eacute;utiliser les ID du premier sans
  les red&eacute;finir.     Par contre ils restent valables sur un pipe pendant
    toute sa dur&eacute;e     de vie: on peut envoyer &agrave; la suite des
  fichiers  sur le pipe ("<tt>cat&nbsp;fichier.his&nbsp;&gt;&gt;&nbsp;pipe</tt>" sous
  Unix) en r&eacute;utilisant les   m&ecirc;mes ID pour d&eacute;signer les
  m&ecirc;mes  &eacute;l&eacute;ments.</li>
                  <li> En principe rien n'emp&ecirc;che de red&eacute;finir
  un  ID  d&eacute;ja     utilis&eacute;. A ce moment-l&agrave; l'ancienne affectation
    de cet entier     dispara&icirc;t et l'objet auquel il correspondait n'est
    plus accessible    par les commandes. L'avantage c'est que &ccedil;a permet
    de faire facilement    des "sc&eacute;narios g&eacute;n&eacute;riques":
  appliquer  une m&ecirc;me    s&eacute;rie de commandes &agrave; des objets
  diff&eacute;rents,  en utilisant    des fichiers de sc&eacute;nario tout
  faits, en initialisant  juste l'ID par   un nouveau chargement d'objet. Par
  ex. sur un pipe nomm&eacute;,  on peut  envoyer d'abord une commande <tt>LoadObject</tt>
  d&eacute;finissant  l'ID 1, puis un fichier de sc&eacute;nario tout fait
  utilisant (sans le d&eacute;finir)     l'objet 1. Ensuite il est possible
  de lancer une autre commande <tt>LoadObject</tt>        red&eacute;finissant
  le m&ecirc;me ID 1 et r&eacute;-envoyer le m&ecirc;me     sc&eacute;nario:
  il sera alors appliqu&eacute; de la m&ecirc;me fa&ccedil;on     au nouvel
  objet. Le syst&egrave;me peut bien sur &ecirc;tre am&eacute;lior&eacute;,
      mais &ccedil;a permet d&eacute;j&agrave; de faire des choses.</li>


.. _dicttype:

Dictionary type
===============

The ``dictionary`` type is a free type that allows to contain almost any type of generic data: numbers, strings, lists, dictionaries.

This dictionary type format is taken from python format (for now: XML will also be handled in the future) because the standard commands format ("tree" format) does not handle this type.

For instance:

::

  { '__syntax__' : 'dictionary', 'no_decoration': 1 }

The ``__syntax__`` attribute is mandatory right now (even if it is not used) because of a limitation of the reading system which can be regarded as a bug and that we will fix one day.


Commands definition
===================

.. hlist::
  :columns: 4

  * :ref:`ActivateAction`
  * :ref:`AddNode`
  * :ref:`AddObject`
  * :ref:`ApplyBuiltinReferential`
  * :ref:`AskTexExtrema`
  * :ref:`AssignReferential`
  * :ref:`Camera`
  * :ref:`ChangePalette`
  * :ref:`ClosePipe`
  * :ref:`CloseWindow`
  * :ref:`ControlsParams`
  * :ref:`CreateControlWindow`
  * :ref:`CreateGraph`
  * :ref:`CreateWindow`
  * :ref:`DeleteAll`
  * :ref:`DeleteElement`
  * :ref:`DeleteObject`
  * :ref:`DuplicateObject`
  * :ref:`EventFilter`
  * :ref:`Exit`
  * :ref:`ExportTexture`
  * :ref:`ExternalReference`
  * :ref:`ExtractTexture`
  * :ref:`Fusion2DParams`
  * :ref:`Fusion3DParams`
  * :ref:`FusionInfo`
  * :ref:`FusionObjects`
  * :ref:`GenerateTexture`
  * :ref:`GetInfo`
  * :ref:`GraphDisplayProperties`
  * :ref:`GraphParams`
  * :ref:`GroupObjects`
  * :ref:`LinkedCursor`
  * :ref:`LinkWindows`
  * :ref:`LoadGraphSubObjects`
  * :ref:`LoadObject`
  * :ref:`LoadReferentialFromHeader`
  * :ref:`LoadTransformation`
  * :ref:`NewId`
  * :ref:`NewPalette`
  * :ref:`ObjectInfo`
  * :ref:`Output`
  * :ref:`PaintParams`
  * :ref:`PopupPalette`
  * :ref:`ReloadObject`
  * :ref:`RemoveObject`
  * :ref:`SaveObject`
  * :ref:`SaveTransformation`
  * :ref:`Select`
  * :ref:`SelectByHierarchy`
  * :ref:`SelectByNomenclature`
  * :ref:`Server`
  * :ref:`SetControl`
  * :ref:`SetMaterial`
  * :ref:`SetObjectPalette`
  * :ref:`ShowObject`
  * :ref:`SliceParams`
  * :ref:`TexturingParams`
  * :ref:`WindowBlock`
  * :ref:`WindowConfig`


.. _ActivateAction:

ActivateAction
--------------

.. raw:: html

  <b>New in Anatomist 4.5</b><br>
  Triggers window action activation.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
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

AddObject
---------

.. raw:: html

  Adds objects in windows

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
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

ApplyBuiltinReferential
-----------------------

see LoadReferentialFromHeader_


.. _AskTexExtrema:

AskTexExtrema
-------------

.. raw:: html

  Provoque l'affichage dans la sortie standard d'Anatomist des valeurs
  extr&ecirc;mes  de la texture de l'objet<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>object</tt></td>
      <td><tt>int</tt></td>
      <td id="txt">objet dont on demande les bornes de la texture</td>
    </tr>
  </tbody>
  </table>


.. _AssignReferential:

AssignReferential
-----------------

.. raw:: html

  Attribue (et crée au besoin) un référential à des objets et des fenêtres.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt> (optionnel)</td>
      <td></td>
    </tr>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt> (optionnel)</td>
      <td></td>
    </tr>
    <tr>
      <td><tt>central_ref</tt></td>
      <td><tt>int</tt></td>
      <td id="txt">
      non nul si on désigne le référentiel central (indestructible) d'Anatomist
      </td>
    </tr>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td id="txt">
        <b>Nouveau dans Anatomist 3.1</b><br>
        fichier contenant des information supplémentaires sur le référentiel: en particulier un nom et un UUID (identifiant unique). Si l'UUID lu depuis ce fichier existe déjà  dans anatomist, le référentiel n'est pas recréé (sinon l'identifiant ne serait plus unique).<br>
        On peut donc utiliser la commande <tt>AssignReferential</tt> juste pour charger des informations complémentaires sur un repère existant. Ce fichier est au format de type <tt>.minf</tt> (bien que l'extension soit généralement <tt>.referential</tt>).
      </td>
    <tr>
      <td><tt>ref_uuid</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td id="txt">
        <b>Nouveau dans Anatomist 3.1</b><br>
        Précise optionnellement qu'on veut utiliser un référentiel précis, d'identifiant unique (et persistant) connu, et déjà présent dans Anatomist. Ce paramètre est généralement incompatible avec le paramètre <tt>filename</tt> parce que l'UUID trouvé dans le fichier est potentiellement différent de celui précisé ici.
      </td>
    </tr>
    </tr>

  </tbody>
  </table>


.. _Camera:

Camera
------

.. raw:: html

  Sets camera point of view, zoom, etc. in 3D windows.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
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


.. _ChangePalette:

ChangePalette
-------------

.. raw:: html

  Modifie une palette de la liste (panneau de gauche de la fen&ecirc;tre
  de palettes)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>colors</tt></td>
      <td><tt>int_vector</tt></td>
      <td>vecteurs de couleurs RGB (composantes &agrave; la suite)</td>
    </tr>
    <tr>
      <td valign="top"><tt>color_mode</tt><br>
      </td>
      <td valign="top"><tt>string</tt> (optionnel)<br>
      </td>
      <td valign="top">"<tt>RGB</tt>" ou "<tt>RGBA</tt>"<br>
      </td>
    </tr>

  </tbody>
  </table>


.. _ClosePipe:

ClosePipe
---------

.. raw:: html

  Ferme le pipe de lecture sur lequel cette commande est envoy&eacute;e
  - fermeture de la t&eacute;l&eacute;commande, en d'autres termes.<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
    <td><tt>remove_file</tt></td>
    <td><tt>int</tt></td>
    <td>flag pr&eacute;sisant si Anatomist doit effacer le fichier
      pipe   nomm&eacute;   apr&egrave;s fermeture</td>
    </tr>

  </tbody>
  </table>


.. _CloseWindow:

CloseWindow
-----------

.. raw:: html

  Ferme des fen&ecirc;tres

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
  </tbody>
  </table>


.. _ControlsParams:

ControlsParams
--------------

.. raw:: html

  Ouvre ou ferme la fenête de paramètres ds contrôles

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>window</tt></td>
      <td><tt>int</tt></td>
      <td>Fenêtre depuis laquelle les paramètres des contrôles doivent
        être ouverts</td>
    </tr>
    <tr>
      <td><tt>show</tt></td>
      <td><tt>int</tt></td>
      <td>0: fermer, 1: ouvrir</td>
    </tr>
  </tbody>
  </table>


.. _CreateControlWindow:

CreateControlWindow
-------------------

.. raw:: html

  Cr&eacute;e la fen&ecirc;tre de contr&ocirc;le.
  Comme   c'est fait automatiquement  dans l'appli, cette commande ne sert
  en fait  &agrave; rien...


.. _CreateWindow:

CreateWindow
------------

.. raw:: html

  Opens a new Anatomist window.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
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
      <td><tt><a href="#dicttype">dictionary</a></tt> (optional)</td>
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
            <td><b>New in Anatomist 4.0.2</b>. Don't display the created window, until a specific show action is used (see <a href="#windowconfig">WindowConfig</a> / raise, or in an embedded widget)</td>
          </tr>
        </table>
      </td>
    </tr>
  </tbody>
  </table>


.. _DeleteAll:

DeleteAll
---------

.. raw:: html

  <b>New in Anatomist 3.1.7</b><br/>
  Deletes all elements loaded in Anatomist (objects, windows, referentials, transformations).<br/>
  No parameters.


.. _DeleteElement:

DeleteElement
-------------

.. raw:: html

  D&eacute;truit n'importe quel type d'&eacute;l&eacute;ment anatomist
  connu  (objets, fen&ecirc;tres, r&eacute;f&eacute;rentiels)<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>elements</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
  </tbody>
  </table>


.. _DeleteObject:

DeleteObject
------------

.. raw:: html

  D&eacute;truit des objets<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
  </tbody>
  </table>


.. _DuplicateObject:

DuplicateObject
---------------

.. raw:: html

  <b>New in Anatomist 3.1</b>.<br>
  Duplicates an existing object, doing a deep or shallow copy of it. Object duplication is especially useful when needing to assign severeal palettes/materials to a single data.<br>
  Objects copying is object-dependent and is not a mandatory feature of all object types: some objects may not be able to be copied. So this command may fail and do nothing. So it is better to check the result unless you exactly know which object you are duplicating. Check can be achieved via the <tt><a href="#objectinfo">ObjectInfo</a></tt> command on the <tt>res_pointer</tt> ID.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>source</tt></td>
      <td><tt>int</tt></td>
      <td>source object ID, to be duplicated</td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID of the copy object</td>
    </tr>
    <tr>
      <td><tt>hidden</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        A hidden object does not appear in Anatomist main control window.
      </td>
    </tr>
    <tr>
      <td><tt>shallow</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>
        A shallow copy will try to share the same underlying low-level data (volume data block, mesh veretices/polygons etc), whereas a deep copy will try to duplicate everything. We say "try" here because the actual copy operation is object-dependent, may not be implemented at all and
        so can fail.<br>
        Default: 1 (actualy shallow)
      </td>
    </tr>
  </tbody>
  </table>


.. _EventFilter:

EventFilter
-----------

.. raw:: html

  Active/d&eacute;sactive et r&egrave;gle le filtre d'&eacute;v&eacute;nement
  sur le canal de sortie associ&eacute; &agrave; ce canal de commandes
  (cf commande Outpout)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>filter</tt><br>
      </td>
      <td valign="top"><tt>string</tt><br>
      </td>
      <td valign="top">liste d'&eacute;v&eacute;nements
        &agrave;   filtrer (c.a.d. &agrave; laiser voir si le filtrage par d&eacute;faut
        est   actif, ou au contraire &agrave; ne pas voir si le filtrage par d&eacute;faut
        est inactif). <tt>filter </tt>est une liste de chaines s&eacute;par&eacute;es
        par des espaces<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>unfilter</tt><br>
      </td>
      <td valign="top"><tt>string</tt><br>
      </td>
      <td valign="top">liste d'&eacute;v&eacute;nements
        &agrave;   ne plus filtrer: ceux-ci sont enlev&eacute;s de la liste <tt>"filter"
        </tt>    pr&eacute;c&eacute;dente du filtre.<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>default_filtering</tt><br>
      </td>
      <td valign="top"><tt>int </tt>(0 ou 1)<br>
      </td>
      <td valign="top">Le filtrage par d&eacute;faut
        d&eacute;finit     si le filtre fonctionne par "addition" ou par "soustraction".
        En mode de   filtrage par d&eacute;faut actif (ce qui est le cas au d&eacute;marrrage),
        tous les &eacute;v&eacute;nements sont filtr&eacute;s par d&eacute;faut
        (c.a.d.  invisibles), la liste <tt>"filter" </tt>permet de voir les &eacute;v&eacute;nements
        choisis (mode soustraction en quelque sorte).<br>
        En mode de filtrage par d&eacute;faut inactif, c'est l'inverse; tous
        les   &eacute;v&eacute;nements sont d&eacute;clench&eacute;s sauf ceux
        donn&eacute;s     par la liste <tt>"filter"</tt> (mode addition).<br>
        Si <tt>default_filtering </tt>est sp&eacute;cifi&eacute;, le filtre
        est   remis &agrave; z&eacute;ro: listes de d'&eacute;v&eacute;nements activ&eacute;s
        auparavant sont effac&eacute;es.<br>
      </td>
    </tr>

  </tbody>
  </table>


.. _Exit:

Exit
----

.. raw:: html

  Sort d'Anatomist




.. _ExportTexture:

ExportTexture
-------------

.. raw:: html

  Extrait la texture d'un objet textur&eacute; et l'&eacute;crit sur disque
  sous forme de texture

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>filename</tt><br>
      </td>
      <td valign="top"><tt>string</tt><br>
      </td>
      <td valign="top">fichier de texture &agrave; &eacute;crire<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>object</tt><br>
      </td>
      <td valign="top"><tt>int</tt><br>
      </td>
      <td valign="top">objet dont on veut exporter la texture<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>time</tt><br>
      </td>
      <td valign="top"><tt>float</tt> (optionnel)<br>
      </td>
      <td valign="top">temps auquel on veut extraire la texture
    (dans  le cas d'objets temporels). Si <tt>time &lt; 0</tt> ou s'il n'est
    pas pr&eacute;cis&eacute;,  tous les temps seront extraits, la texture &eacute;crite
    sera temporelle (seuls le temps de la texture compte: par ex. une fusion
    maillage temporel + texture fixe ne donnera qu'un seul instant)<br>
      </td>
    </tr>

  </tbody>
  </table>


.. _ExternalReference:

ExternalReference
-----------------

.. raw:: html

  <b>New in Anatomist 3.1</b><br>
  Manages reference counting inside Anatomist for objects, windows etc., allowing external applications (like BrainVisa) to use reference counting for their objects.<br>
  <b>Warning:</b> this command is reserved to experts and is highly dangerous since it can easily cause Anatomist to crash.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>elements</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of elements</td>
    </tr>
    <tr>
      <td><tt>action_type</tt></td>
      <td><tt>string</tt></td>
      <td id="txt">
        specifies which of reference counting operation is to be performed:
        <ul>
          <li><tt><b>TakeStrongRef</b></tt>: increments the "strong" (normal) reference counting counter of selected objects
          </li>
          <li><tt><b>TakeWeakSharedRef</b></tt>: increments the "weak shared" counter. weak shared references don't forbit manual deletion of objects, but will maintain a count and automatically delete them when nobody references them; It is like a standard reference counter, but still allows forcing destruction.
          </li>
          <li><tt><b>TakeWeakRef</b></tt>: no effect so far, no action is needed. A weak counter is an observer and is notified when the object is destroyed.
          </li>
          <li><tt><b>ReleaseStrongRef</b></tt>: decrements the strong reference counter on the object (and possibly destroy it if it reaches 0).
          </li>
          <li><tt><b>ReleaseWeakSharedRef</b></tt>: decrements the "weak shared" counter, possibly deleting the object.
          </li>
          <li><tt><b>ReleaseWeakRef</b></tt>: no effect so far.
          </li>
          <li><tt><b>ReleaseApplication</b></tt>: releases the Anatomist application reference on the object. By default all objects are referenced once in the application to keep them alive. If you wish to use reference counting from another application, you should first remove this Anatomist reference.
          </li>
          <li><tt><b>TakeApplication</b></tt>, <b>New in Anatomist 4.4</b>: tell the Anatomist application to take back a reference on the object. It is the contrary of <tt>ReleaseApplication</tt>, and is only useful to revert this latest action.
          </li>
        </ul>
      </td>
    </tr>
  </tbody>
  </table>


.. _ExtractTexture:

ExtractTexture
--------------

.. raw:: html

  <b>New in Anatomist 3.0</b><br>
  Extracts the texture of a textured object and maks a new texture object from it

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
    <td><tt>object</tt>
    </td>
    <td><tt>int</tt>
    </td>
    <td id="txt">object from which the texture has to be exported
    </td>
    </tr>
    <tr>
      <td><tt>time</tt>
      </td>
      <td><tt>float</tt> (optional)
      </td>
      <td id="txt">If provided on a time object, the texture will
        be extracted at the given time position. If <tt>time &lt; 0</tt> or if
        not provided, all timesteps will be extracted and the resulting texture
        will have several timesteps (only texture time is taken into account: for
        instance a fusion time mesh + still texture will lead to only one timestep).
      </td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt>
      </td>
      <td><tt>int</tt> (optional)
      </td>
      <td id="txt">resulting texture
      </td>
    </tr>

  </tbody>
  </table>


.. _Fusion2DParams:

Fusion2DParams
--------------

.. raw:: html

  R&egrave;gle les param&egrave;tres d'une fusion 2D. Cette commande est
  en grande partie obsolète à partir d'Anatomist 3.0. En fait, seul le
  paramètre <tt>reorder_objects</tt> a encore une utilité propre. Les
  autres paramètres sont maintenant dans la commande
  <tt><a href="#texturingparams">TexturingParams</a></tt> (puisqu'ils ne
  sont plus spécifiques aux fusions 2D).<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>object</tt></td>
      <td><tt>int</tt></td>
      <td></td>
    </tr>
    <tr>
      <td><tt>mode</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td><tt>linear</tt>, <tt>geometric</tt> ou <tt>linear_on_defined</tt>. This obsolete parameter is replaced by the one in <tt><a href="#texturingparams">TexturingParams</a></tt>, which supports many more modes.</td>
    </tr>
    <tr>
      <td><tt>rate</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>taux de fusion (entre 0 et 1)</td>
    </tr>
    <tr>
      <td><tt>reorder_objects</tt></td>
      <td><tt>int_vector</tt> (optionnel)</td>
      <td>num&eacute;ros des objets dans le bon ordre</td>
    </tr>
  </tbody>
  </table>


.. _Fusion3DParams:

Fusion3DParams
--------------

.. raw:: html

  <b>New in Anatomist 3.0</b><br>
  Sets texture mapping parameters for Fusion3D objects (mesh+volume)<br>

  <table class="docutils">
  <tbody>
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
    <tr>
      <td>object</td>
      <td><tt>int</tt></td>
      <td>target object to change parameters on</td>
    </tr>
    <tr>
      <td><tt>method</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">Spatial neighborhood calculation method:
        <table>
          <tr>
            <td><tt>point</tt> (default):</td>
            <td>value of the voxel under each mesh vertex location.</td>
          </tr>
          <tr>
            <td><tt>point_offset_internal</tt>:</td>
            <td>value of the voxel shifted towards mesh interior along mesh normals.</td>
          </tr>
          <tr>
            <td><tt>point_offset_external</tt>:</td>
            <td>value of the voxel shifted towards mesh exterior along mesh normals.</td>
          </tr>
          <tr>
            <td><tt>line</tt>:</td>
            <td>integrate along mesh normals, according to the specified submethod: see <tt>submethod</tt>.</td>
          </tr>
          <tr>
            <td><tt>line_internal</tt>:</td>
            <td>as <tt>line</tt> but only towards mesh interior.</td>
          </tr>
          <tr>
            <td><tt>line_external</tt>:</td>
            <td>as <tt>line</tt> but only towards mesh exterior.</td>
          </tr>
          <tr>
            <td><tt>sphere</tt>:</td>
            <td>integrate in a sphere of raduis <tt>depth</tt> around each mesh vertex.</td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td><tt>sumbethod</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">Integration method in the specified neighborhood for each mesh vertex, in order to mix several voxels values in the neighborhood. Thus it is not relevant for <tt>point*</tt> methods.
        <table>
          <tr>
            <td><tt>max</tt> (default):</td>
            <td></td>
          </tr>
          <tr>
            <td><tt>min</tt>:</td>
            <td></td>
          </tr>
          <tr>
            <td><tt>mean</tt>:</td>
            <td></td>
          </tr>
          <tr>
            <td><tt>mean_corrected</tt>:</td>
            <td>taking only non-zero values in the averaging</td>
          </tr>
          <tr>
            <td><tt>mean_enhanced</tt>:</td>
            <td>Nobody remembers what this mode is supposed to do...</td>
          </tr>
          <tr>
            <td><tt>abs_max</tt>:</td>
            <td>max, in absolute value</td>
          </tr>
          <tr>
            <td><tt>median</tt>:</td>
            <td> <b>New in Anatomist 4.4:</b> median value (majority)</td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td><tt>depth</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">Integration neighborhood size, in mm.</td>
    </tr>
    <tr>
      <td><tt>step</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">Integration sampling step, in mm.</td>
    </tr>
  </tbody>
  </table>


.. _FusionInfo:

FusionInfo
----------

.. raw:: html

  <b>New in Anatomist 3.2.1</b><br>
  Lists fusion types, either globally or those allowed for a given set of objects.<br/>
  Information is returned in the same way as for <a href="#getinfo">GetInfo</a> or <a href="#objectinfo">ObjectInfo</a>: a python dictionary.<br/>
  If <tt>filename</tt> is not specified, information is written on the current output (see <a href="#output">Output</a> command).

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
    <td><tt>objects</tt>
    </td>
    <td><tt>int_vector</tt> (optional)
    </td>
    <td id="txt">objects to be fusioned. If not specified, the global list of fusion methods is output
    </td>
    </tr>
    <tr>
      <td><tt>filename</tt>
      </td>
      <td><tt>string</tt> (optional)
      </td>
      <td id="txt">file or "named pipe" in which Anatomist will write output informtaion in
      </td>
    </tr>
    <tr>
      <td><tt>request_id</tt>
      </td>
      <td><tt>int</tt> (optional)
      </td>
      <td id="txt">ID used in Anatomist answer to identify the request. this option is used or instance by BrainVisa to identify answers to its asynchronous requests, which are not necessarily processed in the order they were sent in a multi-threaded context.
      </td>
    </tr>

  </tbody>
  </table>


.. _FusionObjects:

FusionObjects
-------------

.. raw:: html

  Cr&eacute;e un objet fusion &agrave; partir de plusieurs autres<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td><br>
      </td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID de l'objet fusion nouvellement cr&eacute;e</td>
    </tr>
    <tr>
      <td><tt>method</tt></td>
      <td><tt>string</tt></td>
      <td>m&eacute;thode de fusion utilis&eacute;e</td>
    </tr>
  </tbody>
  </table>


.. _GenerateTexture:

GenerateTexture
---------------

.. raw:: html

  <b>Nouveau dans Anatomist 3.0</b><br>
  Génère une texture vierge (valeur 0 partout) correspondant à un objet maillé
  (maillage, bucket...). La texture sera créée en mode "auto-généré" (c'est à dire
  que les valeurs seront générées par OpenGL à l'affichage, voir
  <a href="#texturingparams">TexturingParams</a>)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>object</tt><br>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)<br>
      </td>
      <td valign="top">objet sur lequel on construit la texture. S'il n'est
        pas donné, la texture n'aura qu'un seul point et ne sera utilisable
        qu'en mode "généré par OpenGL".
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>dimension</tt>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)
      </td>
      <td valign="top">dimension de la texture: 1 ou 2.<br>
        Défaut: 1
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>res_pointer</tt> (optionnel)<br>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)<br>
      </td>
      <td valign="top">objet résultat
      </td>
    </tr>

  </tbody>
  </table>


.. _GetInfo:

GetInfo
-------

.. raw:: html

  Demande &agrave; Anatomist des informations sur l'&eacute;tat
  de  l'application      (objets, fen&ecirc;tres, ...).&nbsp; <br>
  Les informations sont donn&eacute;es entre accolades sous forme
  de  dictionnaire    python (directement utilisable par un interpr&eacute;teur
  python).&nbsp;         <br>
  Pour obtenir des infos pr&eacute;cises sur des objets ou fen&ecirc;tres,
  utiliser la commande <tt>ObjectInfo</tt>.<br>
  Si <tt>filename</tt> n'est pas pr&eacute;cis&eacute;, les informations
  sont  &eacute;crites sur la sortie courante (r&eacute;glable avec la
  commande <tt> Output</tt>)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>aims_info</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.1</b><br>
        donne des informations sur la librairie AIMS (texte non structuré)
      </td>
    </tr>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>fichier ou du "pipe nomm&eacute;" dans lequel Anatomist
        &eacute;crit       les informations demand&eacute;es</td>
    </tr>
    <tr>
      <td><tt>linkcursor_lastpos</tt><br>
      </td>
      <td><tt>int</tt> (bool) (optionnel)<br>
      </td>
      <td>donne la derni&egrave;re position cliqu&eacute;e
        pour le curseur li&eacute; (tous groupes confondus), dans le rep&egrave;re
        donn&eacute; par le param&egrave;tre <tt>linkcursor_referential</tt> s'il
        est pr&eacute;cis&eacute;<br>
      </td>
    </tr>
    <tr>
      <td><tt>linkcursor_referential</tt><br>
      </td>
      <td><tt>int</tt> (optionnel)<br>
      </td>
      <td>rep&egrave;re dans lequel la position du curseur
        li&eacute; doit &ecirc;tre donn&eacute;e (utilse seulement avec le param&egrave;tre
        <tt>linkcursor_lastpos</tt>). S'il n'est pas pr&eacute;cis&eacute;,
        le "rep&egrave;re central" d'anatomist est utilis&eacute;<br>
      </td>
    </tr>
    <tr>
      <td><tt>list_commands</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.1</b><br>
        donne la liste des commandes connues par Anatomist, et leurs paramètres
      </td>
    </tr>
    <tr>
      <td><tt>modules_info</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.1</b><br>
        donne la liste des modules et leur description
      </td>
    </tr>
    <tr>
      <td><tt>name_objects</tt><br>
      </td>
      <td><tt>string</tt> (optionnel)<br>
      </td>
      <td>Permet de donner des noms (id)
        aux   objets  qui n'en ont pas dans le contexte courant.<tt><br>
        "top"</tt>: assigne des noms aux objets "toplevel" seulement (ceux
        qui   n'ont  pas de parent).<br>
        <tt>"all"</tt>, <tt>"yes"</tt>, <tt>"1"</tt>:
        assigne des noms &agrave; tous les objets contenus dans Anatomist<br>
      </td>
    </tr>
    <tr>
      <td><tt>name_referentials</tt><br>
      </td>
      <td><tt>int</tt> (bool) (optionnel)<br>
      </td>
      <td><b>Nouveau dans Anatomist 3.0</b><br>
        assigne des noms (id) aux référentiels qui n'en ont pas dans le contexte courant
      </td>
    </tr>
    <tr>
      <td><tt>name_transformations</tt><br>
      </td>
      <td><tt>int</tt> (bool) (optionnel)<br>
      </td>
      <td><b>Nouveau dans Anatomist 3.0</b><br>
        assigne des noms (id) aux transformations qui n'en ont pas dans le contexte courant
      </td>
    </tr>
    <tr>
      <td><tt>name_windows</tt><br>
      </td>
      <td><tt>int</tt> (bool) (optionnel)<br>
      </td>
      <td>
        assigne des noms (id) aux fenêtres qui n'en ont pas dans le contexte courant
      </td>
    </tr>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td>
        demande la liste des ID des objets concernant le canal de communication courant
      </td>
    </tr>
    <tr>
      <td><tt>palettes</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td>demande la liste des palettes (noms)</td>
    </tr>
    <tr>
      <td><tt>referentials</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td>demande la liste des référentiels
      </td>
    </tr>
    <tr>
      <td><tt>request_id</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.0.3</b><br>
        ID utilisé dans la réponse d'Anatomist pour identifier la requête. Cette option est utilisée par exemple par BrainVisa pour identifier les réponses à ses requêtes, qui ne sont pas nécéssairement traitées dans le bon ordre dans un contexte "multi-threadé"
      </td>
    </tr>
    <tr>
      <td><tt>selections</tt><br>
      </td>
      <td><tt>int</tt> (bool) (optionnel)<br>
      </td>
      <td>
        donne les listes d'objets s&eacute;lectionn&eacute;s (par groupe)<br>
      </td>
    </tr>
    <tr>
      <td><tt>transformations</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td>demande la liste des transformations
      </td>
    </tr>
    <tr>
      <td><tt>version</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.1</b><br>
        demande la version d'Anatomist
      </td>
    </tr>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int</tt> (bool) (optionnel)</td>
      <td>demande la liste des ID des fen&ecirc;tres</td>
    </tr>
  </tbody>
  </table>


.. _GraphDisplayProperties:

GraphDisplayProperties
----------------------

.. raw:: html

  <b>New in Anatomist 3.0.2</b><br/>
  Sets per-graph display properties. This allows to display a numeric property value using a colormap, or select name/label identification modes.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td>target graphs</td>
    </tr>
    <tr>
      <td><tt>display_mode</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td>"<tt>Normal</tt>" or "<tt>PropertyMap</tt>"
      </td>
    </tr>
    <tr>
      <td><tt>display_property</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td>name of the property to display in <tt>PropertyMap</tt> mode
      </td>
    </tr>
    <tr>
      <td><tt>property_mask</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td><b>New in Anatomist 3.1.7</b><br/>
        bitwise combination of: 1: nodes, 2: relations, to set if the property should be displayed from nodes/relations values
      </td>
    </tr>
    <tr>
      <td><tt>nomenclature_property</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td><b>New in Anatomist 3.1.7</b><br/>
        "<tt>name</tt>" or "<tt>label</tt>", forces the nomenclature label property for the given graphs. To get back to the default global settings (see <a href="#graphparams">GraphParams</a> for global settings), use the value "<tt>default</tt>".
      </td>
    </tr>
  </tbody>
  </table>


.. _GraphParams:

GraphParams
-----------

.. raw:: html

  Change les options globales relatives aux graphes et aux s&eacute;lections

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>display_mode</tt><br>
      </td>
      <td valign="top"><tt>string</tt> (optionnel)<br>
      </td>
      <td valign="top">Mode d'affichage des sous-objets contenus dans les noeuds de graphes: "<tt>mesh</tt>", "<tt>bucket</tt>", "<tt>all</tt>   ", "<tt>first</tt>"
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>label_attribute</tt><br>
      </td>
      <td valign="top"><tt>string</tt> (optionnel)<br>
      </td>
      <td valign="top">Attribut des noeuds de graphes utilis&eacute; comme filtre de s&eacute;lection, g&eacute;n&eacute;ralement "<tt>label</tt>" ou "<tt>name</tt>"
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>save_only_modified</tt><br>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)<br>
      </td>
      <td valign="top">La sauvegarde d'un graphe sauve soit tous les sous-objets, soit essaie de ne sauver que ceux qui ont &eacute;t&eacute;     modifi&eacute;s (si le graphe est r&eacute;&eacute;crit et pas d&eacute;plac&eacute;)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>saving_mode</tt><br>
      </td>
      <td valign="top"><tt>string</tt> (optionnel)<br>
      </td>
      <td valign="top">Mode de sauvegarde: "<tt>unchanged</tt>" (comme il a &eacute;t&eacute; lu), "<tt>global</tt>" (1 fichier pour tous les sous-objets de la m&ecirc;me cat&eacute;gorie), ou "<tt>local</tt>" (1 fichier par sous-objet)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>selection_color</tt><br>
      </td>
      <td valign="top"><tt>int_vector</tt>&nbsp; (optionnel)<br>
      </td>
      <td valign="top">Couleur de s&eacute;lection, sous la  forme                   <tt>R G B [A [nA]]</tt>, o&ugrave; <tt>A</tt> est  l'opacit&eacute; et <tt>NA</tt> (0 ou 1) est un bool&eacute;en qui pr&eacute;cise   si l'opacit&eacute; s'applique ou si on utilise celle de l'objet s&eacute;lectionn&eacute;.
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>selection_color_inverse</tt><br>
      </td>
      <td valign="top"><tt>int</tt>&nbsp; (optionnel)<br>
      </td>
      <td valign="top">bool&eacute;en, s'il est mis, la s&eacute;lection   inverse les couleurs plut&ocirc;t que d'utiliser une couleur fixe<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>set_base_directory</tt><br>
      </td>
      <td valign="top"><tt>int</tt>&nbsp; (optionnel)<br>
      </td>
      <td valign="top">bool&eacute;en, s'il est mis les sous-objets   d'un graphe sont sauv&eacute;s dans un r&eacute;pertoire qui porte le m&ecirc;me   nom qui lui, avec l'extension "<tt>.data</tt>". Sinon l'ancien nom est gard&eacute;   m&ecirc;me si on change le nom du graphe
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>show_tooltips</tt><br>
      </td>
      <td valign="top"><tt>int</tt>&nbsp; (optionnel)<br>
      </td>
      <td valign="top">Active ou invalide l'affichage des bulles  qui indiquent les noms des noeuds de graphes dans les fen&ecirc;tres 2D/3D
      </td>
    </tr>
    <tr>
      <td valign="top"><tt><em>use_hierarchy</em></tt>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)
      </td>
      <td valign="top"><b>déprécié depuis Anatomist 3.0</b>: utilisez <tt>use_nomenclature</tt> maintenant.
      </td>
    </tr>
    </tr>
    <tr>
      <td valign="top"><tt>use_nomenclature</tt>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)<br>
      </td>
      <td valign="top"><b>Nouveau dans Anatomist 3.0</b>.
      Active ou invalide la colorisation des graphes en fonction de la
      nomenclature
      </td>
    </tr>

  </tbody>
  </table>


.. _GroupObjects:

GroupObjects
------------

.. raw:: html

  Cr&eacute;e une liste d'objets (groupe simple)<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID de l'objet groupe nouvellement cr&eacute;&eacute;</td>
    </tr>
  </tbody>
  </table>


.. _LinkedCursor:

LinkedCursor
------------

.. raw:: html

  D&eacute;place le curseur li&eacute; sur les fen&ecirc;tres du
  m&ecirc;me        groupe<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>window</tt></td>
      <td><tt>int</tt></td>
      <td>fen&ecirc;tre d&eacute;clenchant l'action</td>
    </tr>
    <tr>
      <td><tt>position</tt></td>
      <td><tt>float_vector</tt></td>
      <td>position 3D ou 4D</td>
    </tr>
  </tbody>
  </table>


.. _LinkWindows:

LinkWindows
-----------

.. raw:: html

  Lie les fen&ecirc;tres en un groupe

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt></td>
      <td>fen&ecirc;tres &agrave; lier</td>
    </tr>
    <tr>
      <td><tt>group</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>
        <b>Nouveau dans Anatomist 3.1</b>.
        Numéro du groupe. -1 (défaut) signifie un nouveau groupe. On peut réutiliser un groupe existant.
      </td>
    </tr>
  </tbody>
  </table>


.. _LoadGraphSubObjects:

LoadGraphSubObjects
-------------------

.. raw:: html

  Loads graph elements which may be not still in memory, like visualizable objects in relations.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td>graphs to be completed</td>
    </tr>
    <tr>
      <td><tt>objects_mask</tt></td>
      <td><tt>int</tt></td>
      <td>bitwise combination of codes which indicate which graphe elements should be loaded: 1 (nodes), 2 (relations).
      </td>
    </tr>
  </tbody>
  </table>


.. _LoadObject:

LoadObject
----------

.. raw:: html

  Loads an object from a file on disk

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt></td>
      <td>file to read the object from (volume, mesh etc.)</td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID of the loaded object</td>
    </tr>
    <tr>
      <td><tt>as_cursor</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td><b>New in Anatomist 3.0</b>.
        If this flag is set, the object will not be included in the "regular" objects list,
        but will be useable as a cursor in 3D views.
      </td>
    </tr>
    <tr>
      <td><tt>options</tt></td>
      <td><tt><a href="#dicttype">dictionary</a></tt> (optionnel)</td>
      <td><b>New in Anatomist 3.0</b>.
        Additional options to be passes to reading functions.<br>
        At the moment, only one option is recognized at anatomist level (but in the future, more
        options could be interpreted by reading functions for specific objects or formats):
        <table class="docutils">
          <tr>
            <td><tt>restrict_object_types</tt></td>
            <td><tt>dictionary</tt></td>
            <td>Restricts objects types that can be read. Ex:<br>
              <pre>options { '__syntax__' : 'dictionary', 'Volume' : [ 'S16', 'FLOAT' ] }</pre>
            </td>
          </tr>
          <tr>
            <td><tt>hidden</tt></td>
            <td><tt>int</tt></td>
            <td><b>New in Anatomist 3.1</b>.
              A hidden object does not appear in Anatomist main control window.
            </td>
          </tr>
        </table>
      </td>
    </tr>
  </tbody>
  </table>


.. _LoadReferentialFromHeader:

LoadReferentialFromHeader
-------------------------

.. raw:: html

  <p><b>Renamed in Anatomist 4.2</b><br/>
  This command was formerly named <b><tt>ApplyBuiltinReferential</tt></b>. It has been renamed for clarity. An alias is still available under the older name.</p>

  <p><b>New in Anatomist 4.0</b><br/>
  Extracts referentials / transformations from objects headers when they contain such information, and assign them.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of objects</td>
    </tr>
  </tbody>
  </table>


.. _LoadTransformation:

LoadTransformation
------------------

.. raw:: html

  Initialise une transformation entre deux référentiels, soit à partir d'un fichier disque (matrice de translation et rotation, format ASCII), soit avec une matrice donnée directement.
  <br>
  Il faut préciser soit le nom de fichier, soit la matrice de transformation. Si les deux sont donnés, seul le fichier est pris  en compte.<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>origin</tt></td>
      <td><tt>int</tt></td>
      <td>
        <b>Optionnel depuis Anatomist 3.1</b><br>
        ID du référentiel de départ<br>
        Lorsqu'il n'est pas précisé, il peut être donné par le fichier associé.<br>
        S'il n'y a pas de fichier associé, ou qu'il ne contient pas d'information d'origine, alors un nouveau repère est créé.
      </td>
    </tr>
    <tr>
      <td><tt>destination</tt></td>
      <td><tt>int</tt></td>
      <td>
        <b>Optionnel depuis Anatomist 3.1</b><br>
        ID du référentiel d'arrivée<br>
        Comme pour l'origine, le référentiel peut être donné par le fichier associé, ou créé à la volée.
      </td>
    </tr>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>nom du fichier de transformation</td>
    </tr>
    <tr>
      <td><tt>matrix</tt></td>
      <td><tt>float_vector</tt> (optionnel)</td>
      <td>
        matrice de transformation: elle doit avoir 12 éléments à la suite (représentant 4 lignes de 3 colonnes). La 1ère ligne est la translation, le reste la matrice de rotation (même format que les fichiers)
      </td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID de la transformation résultante</td>
    </tr>

  </tbody>
  </table>


.. _NewId:

NewId
-----

.. raw:: html

  <b>New in Anatomist 3.0.2</b><br>
  Generates new free ID numbers for objects and writes them to the current output stream.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>num_ids</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td id="txt">number of IDs to generate (default: 1)</td>
    </tr>
    <tr>
      <td><tt>request_id</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0.3</b><br>
        ID attribute that is used in Anatomist answer to identify the request. This feature is used for instance by BrainVisa to identify its requests that are not necessarily processed in the right order in a multi-threaded environment
      </td>
    </tr>
  </tbody>
  </table>


.. _NewPalette:

NewPalette
----------

.. raw:: html

  Ajoute une nouvelle palette vierge dans la liste des palettes
  (liste      de  gauche dans les fen&ecirc;tres de palettes). Contrairement
  aux autres      &eacute;l&eacute;ments  d'Anatomist, les palettes ne sont
  pas identifi&eacute;es      par un num&eacute;ro  mais par leur nom.<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>name</tt></td>
      <td><tt>string</tt></td>
      <td>nom donn&eacute; &agrave; la nouvelle palette</td>
    </tr>
  </tbody>
  </table>


.. _ObjectInfo:

ObjectInfo
----------

.. raw:: html

  D&eacute;crit aussi pr&eacute;cis&eacute;ment que possible  les
  &eacute;l&eacute;ments     donn&eacute;s par leur ID. Il peut s'agir d'objets
  anatomist, de fen&ecirc;tres,     de r&eacute;f&eacute;rentiels, de transformations,
  ...&nbsp; <br>
  La description est faite sous forme de dictionnaire python.<br>
  Si <tt>filename</tt> n'est pas pr&eacute;cis&eacute;, les informations
  sont &eacute;crites sur la sortie courante (r&eacute;glable avec la commande
  <tt>Output</tt>)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string </tt>(optionnel)</td>
      <td>fichier ou "pipe nommé" dans lequel Anatomist écrit les informations</td>
    </tr>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td>IDs des éléments à décrire</td>
    </tr>
    <tr>
      <td><tt>name_children</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>si ce flag est non-nul, un ID est assigné à chaque sous-objet qui n'en a pas</td>
    </tr>
    <tr>
      <td valign="top"><tt>name_referentials</tt>
      </td>
      <td valign="top"><tt>int</tt> (optionnel)
      </td>
      <td valign="top">si ce flag est non-nul, un ID est assigné à chaque reférentiel qui n'en a pas et qui est cité par les infos
      </td>
    </tr>
    <tr>
      <td><tt>request_id</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td id="txt"><b>Nouveau dans Anatomist 3.0.3</b><br>
        ID utilisé dans la réponse d'Anatomist pour identifier la requête. Cette option est utilisée par exemple par BrainVisa pour identifier les réponses à ses requêtes, qui ne sont pas nécéssairement traitées dans le bon ordre dans un contexte "multi-threadé"
      </td>
    </tr>

  </tbody>
  </table>


.. _Output:

Output
------

.. raw:: html

  Ouvre un "flux" de sortie, soit par un nom de  fichier, soit par une connexion réseau TCP/IP (adresse internet + port)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>filename</tt>
      </td>
      <td valign="top"><tt>string </tt>(optionnel)
      </td>
      <td valign="top">fichier ou "pipe nommé"
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>ip</tt>
      </td>
      <td valign="top"><tt>string </tt>(optionnel)
      </td>
      <td valign="top">adresse tcp/ip réseau de la  machine à contacter
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>port</tt>
      </td>
      <td valign="top"><tt>int </tt>(optionnel)
      </td>
      <td valign="top">port tcp/ip sur lequel se connecter par réseau
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>default_context</tt>
      </td>
      <td valign="top"><tt>int </tt>(optionnel)
      </td>
      <td valign="top">indique s'il faut changer la sortie du  contexte   par défaut plutôt que le contexte courant
      </td>
    </tr>
  </tbody>
  </table>


.. _PopupPalette:

PopupPalette
------------

.. raw:: html

  Ouvre une fen&ecirc;tre de r&eacute;glage de palette pour les
  objets      donn&eacute;s<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
  </tbody>
  </table>


.. _ReloadObject:

ReloadObject
------------

.. raw:: html

  Recharge des objets d&eacute;j&agrave; en m&eacute;moire &agrave;
  partir     de leurs fichiers disque (s'ils ont chang&eacute;). Attention,
  &ccedil;a     ne marche pas pour tous les objets.<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td><br>
      </td>
    </tr>
  </tbody>
  </table>


.. _RemoveObject:

RemoveObject
------------

.. raw:: html

  Removes objects from windows

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of objects to be removed from specified windows</td>
    </tr>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of windows</td>
    </tr>
    <tr>
      <td><tt>remove_children</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        <b>New in Anatomist 3.2.1</b><br>
        Also remove the given objects children (useful for graphs for instance)
        <br/>
        <b>Changed in Anatomist 4.2:</b><br/>
        The value is now a tristate (-1, 0, 1), -1 being the default and meaning that the behaviour is object dependent: true for graphs, and false for other objects types, mainly. This makes the use of this parameter unneeded most of the time.
      </td>
    </tr>
  </tbody>
  </table>


.. _SaveObject:

SaveObject
----------

.. raw:: html

  Sauvegarde un objet sur disque<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>object</tt></td>
      <td><tt>int</tt></td>
      <td><br>
      </td>
    </tr>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>s'il n'est pas donn&eacute;, l'ancien nom de fichier de  l'objet     est   utilis&eacute;</td>
    </tr>
  </tbody>
  </table>


.. _SaveTransformation:

SaveTransformation
-----------------

.. raw:: html

  Ecrit un fichier de transformation<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>filename</tt><br>
      </td>
      <td valign="top"><tt>string</tt><br>
      </td>
      <td valign="top">nom du fichier &agrave; &eacute;crire<br>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>transformation</tt><br>
      </td>
      <td valign="top"><tt>int</tt><br>
      </td>
      <td valign="top">ID de la transformation<br>
      </td>
    </tr>
  </tbody>
  </table>


.. _Select:

Select
------

.. raw:: html

  S&eacute;lectionne et/ou d&eacute;s&eacute;lectionne des objets<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt> (optionnel<tt>)</tt></td>
      <td>objets &agrave; s&eacute;lectionner</td>
    </tr>
    <tr>
      <td><tt>unselect_objects</tt></td>
      <td><tt>int_vector</tt> (optionnel)</td>
      <td>objets &agrave; d&eacute;-s&eacute;lectionner</td>
    </tr>
    <tr>
      <td><tt>group</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>groupe de s&eacute;lection concern&eacute; - par défaut, 0</td>
    </tr>
    <tr>
      <td><tt>modifiers</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>attributs de la s&eacute;lection: <tt>set</tt>, <tt>add</tt> ou                    <tt>toggle</tt>.   Par d&eacute;faut le mode est <tt>set</tt>    si <tt>unselect_objects</tt> est vide, et <tt>add</tt> sinon.</td>
    </tr>

  </tbody>
  </table>


.. _SelectByHierarchy:

SelectByHierarchy
-----------------

.. raw:: html

  <b>Déprécié à partir d'Anatomist 3.0</b>: cette commande est renommée
  <a href="#selectbynomenclature"><tt>SelectByNomenclature</tt></a>.


.. _SelectByNomenclature:

SelectByNomenclature
--------------------

.. raw:: html

  <b>Nouveau dans Anatomist 3.0</b> et remplace <tt>SelectByHierarchy</tt>.<br>
  Sélectionne par une nomenclature (action correspondant à un click sur une nomenclature
  dans un browser)

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>nomenclature</tt></td>
      <td><tt>int</tt></td>
      <td><b>Nouveau dans Anatomist 3.0</b>. nomenclature à utiliser pour la sélection</td>
    </tr>
    <tr>
      <td><tt><em>hierarchy</em></tt></td>
      <td><tt>int</tt></td>
      <td><b>déprécié à partir d'Anatomist 3.0</b>: remplacé par <tt>nomenclature</tt>
      </td>
    </tr>
    <tr>
      <td><tt>names</tt></td>
      <td><tt>string</tt></td>
      <td>liste des éléments de la nomenclature à sélectionner. Pour le moment on utilise
        l'espace pour séparer les noms (ce qui signifie qu'on ne peur pas mettre d'espaces
        dans les noms)
      </td>
    </tr>
    <tr>
      <td><tt>group</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>groupe de fenêtres concerné</td>
    </tr>
    <tr>
      <td><tt>modifiers</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>comme pour la commande <tt>Select</tt><br>
        <b>Nouveau dans la version 1.30b (corrigée):</b> nouveau mode
        <tt>"remove"</tt>, qui permet d'enlever les noeuds concernés des
        fenêtres concernées pour les faire disparaître.
      </td>
    </tr>
  </tbody>
  </table>


.. _Server:

Server
------

.. raw:: html

  Passe Anatomist en mode serveur. Dans ce mode, Anatomist &eacute;coute
  les  connexions r&eacute;seau. Chaque connexion est une source de commandes
  ind&eacute;pendante.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>port</tt><br>
      </td>
      <td valign="top"><tt>int</tt><br>
      </td>
      <td valign="top">port TCP &agrave; &eacute;couter<br>
      </td>
    </tr>
  </tbody>
  </table>


.. _SetControl:

SetControl
----------

.. raw:: html

  Fixe le contr&ocirc;le actif sur les fen&ecirc;tres donn&eacute;es<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>windows</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
      <tr>
    <td><tt>control</tt></td>
      <td><tt>string</tt></td>
      <td></td>
    </tr>
  </tbody>
  </table>


.. _SetMaterial:

SetMaterial
-----------

.. raw:: html

  Sets some or all of the object material properties.<br>
  Since Anatomist 3.0, these properties also include 3D rendering modes that are specifically
  set on the object

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of objects to set material on</td>
    </tr>
    <tr>
      <td><tt>ambient</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">RGBA values of the ambiant component of the material (4 values).
        Negative values are left unchanged (old values are kept)
      </td>
    </tr>
    <tr>
      <td><tt>diffuse</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">RGBA</td>
    </tr>
    <tr>
      <td><tt>emission</tt></td>
      <td><tt>float_vector</tt> (optionnel)</td>
      <td id="txt">RGBA</td>
    </tr>
    <tr>
      <td><tt>specular</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">RGBA</td>
    </tr>
    <tr>
      <td><tt>shininess</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">range 0-124, a negative value doesn't produce any change
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>refresh</tt></td>
      <td valign="top"><tt>int</tt> (optional)
      </td>
      <td id="txt">forces refresh the windows showing the appropriate objects
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>lighting</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        enables (1) or disables (0) objects lighting/shading. Setting this value to -1
        goes back to the default mode (globally set at the view/scene level)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>smooth_shading</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        (0/1/-1) smooth or flat polygons mode
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>polygon_filtering</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        (0/1/-1) filtering (antialiasing) of lines/polygons
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>depth_buffer</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        (0/1/-1) enables/disables writing in the Z-buffer. You can disable it if you
        want to click "through" an object (but it may have strange effects on the rendering)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>face_culling</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        (0/1/-1) don't draw polygons seen from the back side. The best is to enable it for
        transparent objects, and to disable it for "open" (on which both sides may be seen) and
        opaque meshes. For objects both open and transparent, there is no perfoect setting...
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>front_face</tt></td>
      <td valign="top"><tt>string</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 4.3</b><br>
        Set the external face of polygons (as in OpenGL): <tt>"clockwise"</tt>, <tt>"counterclockwise"</tt> or <tt>"neutral"</tt>(default). Normally in Aims/Anatomist indirect referentials are used, so polygons are in clockwise orientation.
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>line_width</tt></td>
      <td valign="top"><tt>float</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.1.4</b><br>
        Lines thickness (meshes, segments, wireframe rendering modes).
        A null or negative value fallsback to default (1 in principle).
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>polygon_mode</tt></td>
      <td valign="top"><tt>string</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.0</b><br>
        polygons rendering mode: "<tt>normal</tt>",
        "<tt>wireframe</tt>",
        "<tt>outline</tt>" (normal + wireframe),
        "<tt>hiddenface_wireframe</tt>" (wireframe with hidden faces), or
        "<tt>default</tt>" (use the global view settings)<br/>
        <b>New in Anatomist 3.1.5</b>:<br>
        "<tt>ext_outlined</tt>" (thickened external boundaries + normal rendering).

      </td>
    </tr>
    <tr>
      <td valign="top"><tt>unlit_color</tt></td>
      <td valign="top"><tt>float_vector</tt> (optional)</td>
      <td id="txt"><b>New in Anatomist 3.1.4</b><br>
        color used for lines when lighting is off. For now it only affects polygons boundaries in "<tt>outlined</tt>" or "<tt>ext_outlined</tt>" polygon modes.
      </td>
    </tr>

  </tbody>
  </table>


.. _SetObjectPalette:

SetObjectPalette
----------------

.. raw:: html

  Affecte une palette &agrave; des objets. Les param&egrave;tres
  (tous    optionnels)  permettent de r&eacute;gler le mode d'utilisation
  de la  palette    par les objets.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td></td>
    </tr>
    <tr>
      <td><tt>palette</tt></td>
      <td><tt>string</tt> (optionnel depuis la version 3.2)</td>
      <td>nom de la palette principale &agrave; appliquer</td>
    </tr>
    <tr>
      <td><tt>palette2</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>palette secondaire (utile uniquement dans le cas de textures
        2D)</td>
    </tr>
    <tr>
      <td><tt>min</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>proportion min de la palette correspondant au min des
        valeurs de la texture de l'objet associ&eacute;es</td>
    </tr>
    <tr>
      <td><tt>max</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>pareil pour le max. <tt>min</tt> <tt>max</tt> peuvent
        &ecirc;tre      n&eacute;gatifs  (utilisation d'une sous-partie de lapalette)
      et <tt>max</tt>         peut &ecirc;tre  sup&eacute;rieur &agrave; <tt>min</tt>
      (-&gt;palette    invers&eacute;e)</td>
    </tr>
    <tr>
      <td><tt>min2</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>pareil pour l'affectation de la 2&egrave;me palette sur
        la  2&egrave;me     composante de texture (objets avec texture 2D)</td>
    </tr>
    <tr>
      <td><tt>max2</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>idem</td>
    </tr>
    <tr>
      <td><tt>mixMethod</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>Mode de m&eacute;lange des 2 palettes pour former une
        palette 2D: "<tt>LINEAR</tt>" ou "<tt>GEOMETRIC</tt>" pour le moment</td>
    </tr>
    <tr>
      <td><tt>linMixFactor</tt></td>
      <td><tt>float</tt> (optionnel)</td>
      <td>facteur de m&eacute;lange entre les 2 palettes en mode
        lin&eacute;aire</td>
    </tr>
    <tr>
      <td><tt>palette1Dmapping</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>Mode de parcours d'une palette 2D (image) pour une texture 1D:
        <tt>FirstLine</tt> (1ère ligne de l'image) ou <tt>Diagonal</tt>
      </td>
    </tr>
    <tr>
      <td><tt>absoluteMode</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td><b>Nouveau dans Anatomist 3.1.4</b><br>
        Si ce flag est non-nul, les valeurs <tt>min</tt>, <tt>max</tt>, <tt>min2</tt> et <tt>max2</tt> sont des valeurs absolues dans la texture des objets concernés. Sinon (par défaut) on est en mode proportionnel.
      </td>
    </tr>
    <tr>
      <td><tt>sizex</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td><b>Nouveau dans Anatomist 3.2</b><br>
        Taille X de la palette/texture interne utilisée par OpenGL.<br/>
        Ce paramètre peut être utile pour avoir une bonne précision de texture sur certains objets.
      </td>
    </tr>
    <tr>
      <td><tt>sizey</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td><b>Nouveau dans Anatomist 3.2</b><br>
        Taille Y de la palette/texture interne utilisée par OpenGL.<br/>
        Ce paramètre peut être utile pour avoir une bonne précision de texture sur certains objets.
      </td>
    </tr>

  </tbody>
  </table>


.. _ShowObject:

ShowObject
----------

.. raw:: html

  <b>New in Anatomist 3.1</b>.<br>
  Shows an object in Anatomist main window if it was hidden previously. This command is merely useful at user level and is generally only used within scripts.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>object</tt></td>
      <td><tt>int</tt></td>
      <td>object ID</td>
    </tr>
  </tbody>
  </table>


.. _SliceParams:

SliceParams
-----------

.. raw:: html

  <b>New in Anatomist 3.0.1</b><br>
  Sets slice parameters on "self-sliceable" objects: Anatomist objects which have a slice
  information within themselves, like Slice objects, or CutPlane objects.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>objects</tt></td>
      <td><tt>int_vector</tt></td>
      <td id="txt">IDs of the target objects</td>
    </tr>
    <tr>
      <td><tt>position</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Any point of the slice plane</td>
    </tr>
    <tr>
      <td><tt>quaternion</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Quaternion specifying the orientation of the slice plane</td>
    </tr>
    <tr>
      <td><tt>plane</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Alternative to position + quaternion: the plane orientation can
        be specified as a plane equation.
      </td>
    </tr>
  </tbody>
  </table>


.. _TexturingParams:

TexturingParams
---------------

.. raw:: html

  <b>New in Anatomist 3.0</b><br>
  Sets texture mapping parameters<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td>objects</td>
      <td><tt>int_vector</tt></td>
      <td>target objects to change parameters on</td>
    </tr>
    <tr>
      <td><tt>texture_index</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td id="txt">texture number (for objects with several textures),
        default: <tt>0</tt>
      </td>
    </tr>
    <tr>
      <td><tt>mode</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">textures color mixing mode: <tt>geometric</tt>,
        <tt>linear</tt>, <tt>replace</tt>, <tt>decal</tt>, <tt>blend</tt>,
        <tt>add</tt>, <tt>combine</tt>, or <tt>linear_on_defined</tt>. These values correspond both to OpenGL texture mapping functions, and also to fusion modes (for 2D fusions objects) (see also <tt>rate</tt>).<br/>
        <b>New in Anatomist 4.3:</b> A bunch of new fusions mixing modes have appeared, allowing to perform various masking modes for instance: <tt>linear_A_if_A_white</tt>, <tt>linear_A_if_B_white</tt> (synonim to <tt>linear_on_defined</tt>), <tt>linear_A_if_A_black</tt>, <tt>linear_A_if_B_black</tt>, <tt>linear_A_if_A_opaque</tt>, <tt>linear_A_if_B_transparent</tt>, <tt>linear_B_if_B_opaque</tt>, <tt>linear_B_if_A_transparent</tt>, <tt>max_channel</tt>, <tt>min_channel</tt>, <tt>max_opacity</tt>, <tt>min_opacity</tt>.<br/>
        <b>New in Anatomist 4.4:</b> New mixing modes: <tt>geometric_lighten</tt> (geometric mixing of (1 - colors)), <tt>geometric_sqrt</tt>: formerly geometric mode, but the geometric mode now does not "normalize" the multiplication by a square root.
      </td>
    </tr>
    <tr>
      <td><tt>filtering</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">texture filtering: <tt>nearest</tt> (default) or <tt>linear</tt></td>
    </tr>
    <tr>
      <td><tt>generation</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">texture generation mode (textures automatically generated by OpenGL completely replace the current texture: thus this option has no real interest but looks nice): <tt>none</tt> (default),
        <tt>object_linear</tt>, <tt>eye_linear</tt>, <tt>sphere_map</tt>,
        <tt>reflection_map</tt>, <tt>normal_map</tt>. (these values directly correspond to OpenGL functions)
      </td>
    </tr>
    <tr>
      <td><tt>generation_params_1</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Parameters associated to texture generation for the first texture coordinate.<br>
        Such parameters are only useful when in <tt>object_linear</tt> or
        <tt>eye_linear</tt> mode. It is a 4 float vector defining the direction of the projection of the first texture coordinate according to real 3D coordinates in the mesh.<br>
        Parameters specified here only affect the current generation mode (the one given by the <tt>generation</tt> parameter, or the mode currently in use if <tt>generation</tt> is not specified).
      </td>
    </tr>
    <tr>
      <td><tt>generation_params_2</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Parameters associated to texture generation for the second texture coordinate.
      </td>
    </tr>
    <tr>
      <td><tt>generation_params_3</tt></td>
      <td><tt>float_vector</tt> (optional)</td>
      <td id="txt">Parameters associated to texture generation for the third texture coordinate.<br>
        For now 3D textures are not supported, so this parameter is useless.
      </td>
    </tr>
    <tr>
      <td><tt>rate</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">mixing rate for textures, in modes supporting it (linear-based modes for instance) (object - texture weighting).
        This parameter has no effect for some objects.
      </td>
    </tr>
    <tr>
      <td><tt>interpolation</tt></td>
      <td><tt>string</tt> (optional)</td>
      <td id="txt">colors interpolation space on textures: <tt>palette</tt> (default) or <tt>rgb</tt>.
      </td>
    </tr>
  </tbody>
  </table>


.. _WindowBlock:

WindowBlock
-----------

.. raw:: html

  <b>New in Anatomist 4.2.</b><br/>
  Opens or configures a windows block, a widget that contains a grid of anatomist windows.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>block</tt></td>
      <td><tt>int</tt></td>
      <td>ID of the (existing or future) window block.<br/>
        If <tt>block</tt> doesn't exist, a new block will be created, otherwise the existing one will be modified.<br>
        <b>Note:</b> The block number is an ID just like those of objects,
        windows etc.: a number already allocated must not be reused.
      </td>
    </tr>
    <tr>
      <td><tt>geometry</tt></td>
      <td><tt>int_vector</tt> (optional)</td>
      <td>position and size of the window: x, y, w, h</td>
    </tr>
    <tr>
      <td><tt>block_columns</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        Set the number of columns. The default is 2, but if specified, it will force an existing block to resize at 2 columns, whereas if unspecified, the block will be left unchanged.
      </td>
    </tr>
    <tr>
      <td><tt>block_rows</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        Set the number of rows. As for <tt>block_columns</tt>, the default is 2, but if specified, it will force an existing block to resize at 2 rows, whereas if unspecified, the block will be left unchanged.<br>
        This option is incompatible with <tt>block_columns</tt>. If both are used, <tt>block_columns</tt> will override <tt>block_rows</tt>.
      </td>
    </tr>
    <tr>
      <td><tt>make_rectangle</tt></td>
      <td><tt>int</tt> (optional)</td>
      <td>
        if non-null, the block will be reorganized to fit a rectangular grid with the specified width / height ratio with all its current anatomist windows.
      </td>
    </tr>
    <tr>
      <td><tt>rectangle_ratio</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td>
        if <tt>make_rectangle</tt> is set, this parameter specifies the width / height ratio of the rectangle. Default: 1.
      </td>
    </tr>
  </tbody>
  </table>


.. _WindowConfig:

WindowConfig
------------

.. raw:: html

  Settings for windows (includes various settings)<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td valign="top"><tt>windows</tt></td>
      <td valign="top"><tt>int_vector (optional)</tt></td>
      <td valign="top">selected windows</td>
    </tr>
    <tr>
      <td valign="top"><tt>clipping</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">number of clipping planes: 0, 1 or 2</td>
    </tr>
    <tr>
      <td valign="top"><tt>clip_distance</tt></td>
      <td valign="top"><tt>float</tt> (optional)</td>
      <td valign="top">distance between the slice plane and the clipping planes
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>cursor_visibility</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.1</b>.
        makes visible (1) or invisible (0) the linked cursor in the chosen
        windows. The value -1 sets back the global setting (of the
        preferences)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>face_culling</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) the elimination
        of polygons seen from the bottom face
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>flat_shading</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) rendering in
        "flat shading" mode (without color smoothing)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>fog</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) fog</td>
    </tr>
    <tr>
      <td valign="top"><tt>geometry</tt></td>
      <td valign="top"><tt>int_vector</tt> (optional)</td>
      <td valign="top">position and size of the window (external size).
        If sizes are zero or not specified, the current window size is
        not changed
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>iconify</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">iconifies (or hides) windows</td>
    </tr>
    <tr>
      <td valign="top"><tt>light</tt></td>
      <td valign="top"><tt>object</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.2.1</b>.
        Windows lighting settings. This dictionary may include the following parameters:
        <li><tt>ambient</tt>: ambiant lighting settings (list of float, 4 elements)
        </li>
        <li><tt>diffuse</tt>: diffuse lighting settings (list of float, 4 elements)
        </li>
        <li><tt>specular</tt>: specular lighting settings (list of float, 4 elements)
        </li>
        <li><tt>background</tt>: background color (list of float, 4 elements)
        </li>
        <li><tt>position</tt>: light position (list of float, 4 elements)
        </li>
        <li><tt>spot_direction</tt>: spot light direction (list of float, 3 elements)
        </li>
        <li><tt>spot_exponent</tt>: spot light intensity exponent (float)
        </li>
        <li><tt>spot_cutoff</tt>: spot light cutoff angle (float)
        </li>
        <li><tt>attenuation_offset</tt>: light attenuation, offset part (float)
        </li>
        <li><tt>attenuation_linear</tt>: light attenuation, linear coefficient (float)
        </li>
        <li><tt>attenuation_quadratic</tt>: light attenuation, quadratic coefficient (float)
        </li>
        <li><tt>model_ambient</tt>: don't really know... (list of float, 4 elements)
        </li>
        <li><tt>model_local_viewer</tt>: don't really know... (float)
        </li>
        <li><tt>model_two_side</tt>: don't really know (float)
        </li>
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>linkedcursor_on_slider_change</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.0</b>.
        enables or disables the mode when slice/time sliders act as linked
        cursor actions (with propagation to other views)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>perspective</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) the perspective
        rendering mode
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>polygon_filtering</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) polygons and lines
        smoothing (anti-aliasing)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>polygon_mode</tt></td>
      <td valign="top"><tt>string</tt> (optional)</td>
      <td valign="top">polygons rendering mode: "<tt>normal</tt>",
        "<tt>wireframe</tt>", "<tt>outline</tt>" (normal + wireframe),
        "<tt>hiddenface_wireframe</tt>" (wireframe with hidden faces)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>raise</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">unicognifies windows and make them move to the
        top of the desktop
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>record_basename</tt></td>
      <td valign="top"><tt>string</tt> (optional)</td>
      <td valign="top">base filename of images written using the
        film recording mode (ex: <tt>/tmp/toto.jpg</tt>). Images
        will actually have numbers appended before the extension
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>record_mode</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) the images recording mode
        (film) of 3D windows. To enable it, <tt>record_basename</tt> must also be
        specified
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>snapshot</tt></td>
      <td valign="top"><tt>string</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.0</b>.
        Saves the image of the view in the specified file. If <tt>windows</tt>
        contains several values, then several images have to be saved: in this
        case, <tt>snapshot</tt> is a list of filenames separated by space characters:
        so the file name/path must not contain any space character (this restriction
        doesn't apply if a single window is used). Node: escape character ("\ ") are
        not supported yet.
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>transparent_depth_buffer</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top">enables (1) or disables (0) writing of transparent objects
        in the depth buffer. Useful if you want to click across transparents objects
        (but the rendering can be wrong)
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>view_size</tt></td>
      <td valign="top"><tt>int_vector</tt> (optional)</td>
      <td valign="top">size of the rendering zone (3D rendering widget).
        This parameter has a higher priority than sizes given using
        <tt>geometry</tt> if both are specified
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>fullscreen</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.1</b>.
        enables or disables the fullscreen mode
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>show_cursor_position</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.1</b>.
        shows or hides the status bar at the bottom of the window, showing the cursor position and a current object value at this position.
      </td>
    </tr>
    <tr>
      <td valign="top"><tt>show_toolbars</tt></td>
      <td valign="top"><tt>int</tt> (optional)</td>
      <td valign="top"><b>New in Anatomist 3.1</b>.
        shows or hides everything around the 3D view (menus, buttons bars, status bar, referential...)
      </td>
    </tr>

  </tbody>
  </table>


Commands defined in plugins
===========================

* ROI module


.. _AddNode:

AddNode
-------

.. raw:: html

  Ajoute un noeud dans un graphe, avec &eacute;ventuellement  un
  bucket    vide  &agrave; l'int&eacute;rieur<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>graph</tt></td>
      <td><tt>int</tt></td>
      <td>graphe auquel on ajoute un noeud</td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID du nouveau noeud cr&eacute;&eacute;</td>
    </tr>
    <tr>
      <td><tt>name</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>nom de l'objet noeud&nbsp; <br>
    d&eacute;faut: <tt>RoiArg</tt></td>
    </tr>
    <tr>
      <td><tt>syntax</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>attribut syntaxique du noeud&nbsp; <br>
    d&eacute;faut: <tt>roi</tt></td>
    </tr>
    <tr>
      <td><tt>with_bucket</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>pr&eacute;cise s'il faut cr&eacute;er un bucket dans le  noeud&nbsp;                   <br>
      d&eacute;faut: oui</td>
    </tr>
    <tr>
      <td><tt>res_bucket</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>ID du bucket cr&eacute;e (s'il est cr&eacute;e)</td>
    </tr>
    <tr>
      <td><tt>no_duplicate</tt></td>
      <td><tt>int</tt> (optionnel)</td>
      <td>si mis &agrave; 1, emp&ecirc;che de recr&eacute;er des noeuds qui ont  le m&ecirc;me attribut "<tt>name</tt>". D&eacute;faut: 0</td>
    </tr>
  </tbody>
  </table>


.. _CreateGraph:

CreateGraph
-----------

.. raw:: html

  Crée un graphe associé à un objet 2D (par ex. un volume). L'objet de départ donne ses dimensions (taille de voxel, extrêma) au nouveau graphe.<br>

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th><b>Attribute:</b></th>
      <th><b>Type:</b></th>
      <th><b>Description:</b></th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><tt>object</tt></td>
      <td><tt>int</tt></td>
      <td>objet 2D servant de "modèle" au graphe</td>
    </tr>
    <tr>
      <td><tt>res_pointer</tt></td>
      <td><tt>int</tt></td>
      <td>ID du nouveau graphe créé</td>
    </tr>
    <tr>
      <td><tt>name</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>nom de l'objet graphe. Défaut: <tt>RoiArg</tt>.
      </td>
    </tr>
    <tr>
      <td><tt>filename</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td><b>Nouveau dans Anatomist 3.1.4</b><br>
        nom de fichier associé au graphe. Défaut: aucun.
      </td>
    </tr>
    <tr>
      <td><tt>syntax</tt></td>
      <td><tt>string</tt> (optionnel)</td>
      <td>attribut syntaxique du graphe. Défaut: <tt>RoiArg</tt>.
      </td>
    </tr>

  </tbody>
  </table>


.. _PaintParams:

PaintParams
-----------

.. raw:: html

  <b>New in Anatomist 4.2.0</b><br>
  Sets various ROI painting parameters.

  <table width="100%" class="docutils">
  <thead>
    <tr>
      <th>Attribute:</th>
      <th>Type:</th>
      <th>Description:</th>
    </tt>
  </thead>
  <tbody>
    <tr>
      <td><tt>brush_size</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt">Radius of the paint brush, in millimeters or in voxels, depending on the millimeter mode</td>
    </tr>
    <tr>
      <td><tt>brush_type</tt></td>
      <td><tt>float</tt> (optional)</td>
      <td id="txt"><tt>"point"</tt>, <tt>"square"</tt>, <tt>"disk"</tt>, or <tt>"sphere"</tt>. <tt>"ball"</tt> is an alias for sphere.</td>
    </tr>
    <tr>
      <td><tt>follow_linked_cursor</tt></td>
      <td><tt>int</tt> (bool) (optional)</td>
      <td id="txt">Linked cursor moving with brush</td>
    </tr>
    <tr>
      <td><tt>line_mode</tt></td>
      <td><tt>int</tt> (bool) (optional)</td>
      <td id="txt">line interpolation mode between brush strokes</td>
    </tr>
    <tr>
      <td><tt>millimeter_mode</tt></td>
      <td><tt>int</tt> (bool) (optional)</td>
      <td id="txt">brush size can be either in mm or in voxels. In voxels mode, the brush may be anisotropic.
      </td>
    </tr>
    <tr>
      <td><tt>replace_mode</tt></td>
      <td><tt>int</tt> (bool) (optional)</td>
      <td id="txt">region replacing mode (when drawing on a different region)
      </td>
    </tr>
  </tbody>
  </table>




