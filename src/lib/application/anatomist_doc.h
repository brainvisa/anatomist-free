/* This software and supporting documentation are distributed by
 *     Institut Federatif de Recherche 49
 *     CEA/NeuroSpin, Batiment 145,
 *     91191 Gif-sur-Yvette cedex
 *     France
 *
 * This software is governed by the CeCILL-B license under
 * French law and abiding by the rules of distribution of free software.
 * You can  use, modify and/or redistribute the software under the
 * terms of the CeCILL-B license as circulated by CEA, CNRS
 * and INRIA at the following URL "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 */


// This file is only here for Doxygen (doc generation tool)
// Nothing to do with the code, in fact...


/*! \mainpage Anatomist programming documentation

\image html anatomist.png

No real doc up to now...

- <a href="../html/en/changelog.html">Change Log</a>
- <a href="https://bioproj.extra.cea.fr/redmine/projects/brainvisa-aims/issues">Issues in Bioproj</a>, our Redmine based projects platform. To know more about Bioproj, see <a href="http://brainvisa.info/repository.html">this page on BrainVISA website</a>.

\ref objects_opengl

\section links Links to other doc locations

Anatomist/BrainVISA official web site:

- <a href="http://anatomist.info">http://anatomist.info</a>

French helps from Anatomist help menu:

- <a href="../html/en/index.html">
  Main Anatomist help pages</a> (what you get when you go to the Help menu in 
  Anatomist)
- <a href="../html/fr/programmation/index.html">
  Programming help</a>

*/


/*! \page objects_opengl Anatomist Objects and OpenGL rendering

Objects handled by Anatomist always inherit the anatomist::AObject class.
But AObject is still a general class, holding information about the object, its location in space and bounding box, its referential, colors etc, but does not provide a complete access to geometric primitives and OpenGL lists. This is the job of another class: anatomist::GLComponent. Some anatomist::AObject may not have any 3D rendering at all (nomenclatures, for instance).

anatomist::GLComponent holds all the 3D rendering API, which is obviously based on OpenGL. anatomist::GLComponent and anatomist::AObject do not necessarily represent distinct physical objects, the anatomist::GLComponent part may be the rendering functions added to the same objects (a given object may inherit both anatomist::AObject and anatomist::GLComponent). But in some cases, especially for compound objects, the anatomist::GLComponent API may be delegated to another object, different from the anatomist::AObject. anatomist::GLComponent methods are almost all virtual.

The anatomist::GLComponent part may be accessed from an anatomist::AObject via its anatomist::AObject::glAPI() method. This method may return 0 if no 3D rendering is available.

The main function of anatomist::GLComponent is to provide OpenGL display lists: 3D windows will call the anatomist::GLComponent::glMainGLL() method for that, which returns a list of reference-counted OpenGL lists.

An object may have different representations in 3D, depending on viewing parameters, such as 2D/3D mode, slicing orientation, or other rendering parameters. This is why a rendering is always asked to an anatomist::GLComponent using a paremeter which describes which kind of rendering is needed: the anatomist::ViewState.


\section multiple_gllists GLComponent generally contains multipls OpenGL display lists

Anatomist tries to be minimalist when building 3D renderings: this means that whenever possible, display lists are shared between several views, or even between several objects when they have "something" in common. Moreover, when an object changes, anatomist tries to re-build only the part of the display lists that has actually changed. On an OpenGL point of view, this means that an object will generally hold several OpenGL display lists, each being responsible for a given aspect of the object rendering, and a main OpenGL list will call all the sub-lists. Typically, there will be an OpenGL list for color material, one for texture environment properties, one to build and bind the texture image(s), one for the geometric body (vertices and polygons)... This ends up with many lists, but allows to share efficiently things between views and objects, and to allow fast updates when the object changes.

The different anatomist::GLComponent gl*GLL() methods are responsible for all those sub-lists. But remember that only anatomist::GLComponent::glMainGLL() is mandatory and will be explicitly called from rendering views. All the other lists are called by the default implementation of anatomist::GLComponent::glMainGLL(), or by compound objects building their own main GL list.

This is why a GLComponent may be separated into its geometrical primitives, its texture coordinates, its texture image, its texture environment, etc. These components may be recombined and re-used by compound objects: for instance a anatomist::ATexSurface object (a textured mesh) will combine the GL lists of its child mesh and its child texture to build its own rendering main list.

For each of these parts, state flags keep track of components which have changed, which will allow to re-calculate the modified lists when needed.


\section glcomponent_refcounted_gllists OpenGL display lists are reference-counted

Now it becomes obvious that a given OpenGL display list may be used in several objects and several views, and should be able to update when their underlying data have changed. This is why we use reference-counted GL lists.


\section glcomponent_gllists_cache Display lists cache in GLComponent

A given display list can be built once, then reused for another view or object, and other different display lists may also be claimed for the same object, using different rendering parameters (anatomist::ViewState), then the previous list may be used again, etc. To be able to return a same shared display list that has already be built, a GLComponent object must keep pointers to its rendering lists, associated with corresponding anatomist::ViewState fingerprints.

So GLComponent maintains a map of built display lists, indexed by a ViewState
identifier, a string which describes characteristic information for a given anatomist::ViewState which are meaningful to the given object. This identifier is built by the anatomist::GLComponent::viewStateID() method. This GL lists map is a cache which allows lists to be reused. But its size should be limited, otherwise an object could grow and store thousands of display lists that may not be useful later. The cache maintains a given number of lists, and deletes the oldes which have not been used. This number of lists remembered in a given object can be queried and set by the anatomist::GLComponent::glStateMemory() and anatomist::GLComponent::glSetStateMemory() methods. anatomist::GLComponent::glGarbageCollector() may be used to manually free some lists, but normally is is done automatically.


\section gllists_in_views OpenGL display lists in 3D views

Anatomist 3D windows are two-layer:

AWindow3D is the high-level object, contains the GUI, and manages AObject objects registration, removal, and updates after changes, and controls the whole scene. It has methods to completely redraw the scene, or perform partial recalculations. It makes use of OpenGL display lists of objects, and stores them in the lower-level view object: anatomist::GLWidgetManager.

anatomist::GLWidgetManager is the low-level object. On the GUI point of view, it represents the 3D rendering zone, and stores all display lists to be rendered. It has no knowledge of which object a display list is related to, so cannot recalculate anything on its own after an object change. However it manages scene parameters, like camera position and orientation, perspective modes, scene snapshots etc.

However if an object OpenGL display list(s) change, or if non-principal lists in an object are replaced with others, anatomist::GLWidgetManager state does not necessarily need to change, and a fast redraw can be triggered, the existing lists will just be called, and this is generally sufficient. More complete redraws implying objects bounding-box recalculations, adding/removing objects, or objects reordering, should be handled at the anatomist::Window3D level, because they need to change the structure of the principal lists.


\section glcomponent_changing_contents Changing an object contents and updating its views

Objects following the "standard" anatomist::GLComponent multiple display lists organization allow a quite general access to their geometric primitives, OpenGL lists, and "has-changed flags". This means that the vertice, normals, polygons and texture coordinates arrays can be accessed. This is done using the anatomist::GLComponent::glNumVertex(), anatomist::GLComponent::glVertexArray(), anatomist::GLComponent::glNormalArray(), anatomist::GLComponent::glPolygonSize(), anatomist::GLComponent::glNumPolygon(), anatomist::GLComponent::glPolygonArray(), anatomist::GLComponent::glNumTextures(), anatomist::GLComponent::glDimTex(), and anatomist::GLComponent::glTexCoordArray() methods, all using a anatomist::ViewState parameter to specify the desired rendering properties. For textrues, the min/max values are important, and texture images are normally associated with anatomist palettes.

When an object changes, for instance if a vertex position has moved, or a texture coordinates value has changed, the corresponding anatomist::GLComponent::glSetChanged() method must be called, specifying which part of the object needs to be updated. Then the display lists will be recalculated if needed when needed, in a lazy way, and observers (views or parent objects) will also be updated consequently when the anatomist::AObject::notifyObservers() method is called.

So basically, changing an object contents and updating anatomist views and objects does not require any OpenGL programming.

*/

