#ifndef ANA_WINDOW3D_RENDERCONTEXT_H
#define ANA_WINDOW3D_RENDERCONTEXT_H

#include <anatomist/object/Object.h>
#include <anatomist/window/viewstate.h>
#include <anatomist/surface/shaderProgramKey.h>
#include <cartobase/smart/rcptr.h>
#include <list>
#include <map>
#include <vector>

class QOpenGLShaderProgram;

class AWindow3D;
namespace anatomist
{
  class GLWidgetManager;
  class GLComponent;
  class IShaderModule;


  /**
  * \enum RenderMode
  * \brief Selects which objects a rendering pass handles, and which
  *        primitive list it fills.
  */
  enum class RenderMode
  {
    Full,          ///< permanent objects, then temporary objects, each into its own list
    TemporaryOnly, ///< temporary objects only; the permanent list is kept as is
    PermanentOnly, ///< permanent objects only (used internally by Full)
    Selection      ///< selection pass, into the selection primitive list
  };


  /**
  * \class RenderContext
  * \brief Builds the OpenGL primitive lists of an AWindow3D.
  *
  * \details
  * This class centralizes the rendering logic of a 3D window and decouples
  * it from AWindow3D. For each rendering pass, it:
  * - groups objects by ShaderProgramKey (rendering pipeline, shader
  *   modules, shader templates) and by transparency,
  * - requests the matching GLSL programs from ShaderProgramCache, which
  *   compiles each program only once for the whole application,
  * - binds each program once per group and sets scene and object uniforms,
  * - sets up the OpenGL state (rendering mode, clipping, transparency),
  * - collects all rendering commands into the window primitive lists.
  *
  * The resulting \c PrimList contains:
  * - shader binding commands (\c GLBindShader),
  * - uniform setup commands (\c GLSceneUniforms, \c GLObjectUniforms),
  * - OpenGL state lists,
  * - object rendering primitives (\c AObject::render()).
  *
  * RenderContext owns no GL program: programs belong to ShaderProgramCache.
  * It owns its private data, so it can be moved but not copied.
  */
  class RenderContext
  {
    public:
      friend class ::AWindow3D;

      /**
      * \brief Objects grouped by the shader program they need.
      *
      * All objects of a group share the same program: it is bound once,
      * then all of them are drawn.
      */
      typedef std::map<ShaderProgramKey,
                       std::vector<carto::shared_ptr<AObject> > > DrawableGroups;

      /**
      * \brief Constructs an empty RenderContext.
      *
      * An empty context cannot render. A context built with the window
      * constructor must be moved into it before use.
      */
      RenderContext(){};

      /**
      * \brief Constructs a RenderContext for a given 3D window and GL manager.
      * \param win The parent 3D window.
      * \param widgetManager The OpenGL manager providing context and state.
      */
      RenderContext(AWindow3D* win, anatomist::GLWidgetManager* widgetManager);

      /**
      * \brief Releases the private data. Makes no OpenGL call.
      */
      ~RenderContext();

      /// Copy is forbidden: both copies would own, and delete, the same private data.
      RenderContext( const RenderContext & ) = delete;
      /// Copy is forbidden: both copies would own, and delete, the same private data.
      RenderContext & operator=( const RenderContext & ) = delete;

      /**
      * \brief Takes over the private data of \p other, which is left empty.
      */
      RenderContext( RenderContext && other ) noexcept;

      /**
      * \brief Releases the current private data, then takes over the
      *        private data of \p other, which is left empty.
      */
      RenderContext & operator=( RenderContext && other ) noexcept;

      /**
      * \brief Rebuilds the window primitive lists for a rendering pass,
      *        then requests a repaint.
      *
      * \details
      * Makes the window GL context current, then depending on \p mode:
      * - \c Full: clears the permanent and temporary lists, sets up the
      *   OpenGL state and clipping planes, renders permanent objects
      *   (twice in hidden wireframe and outlined modes, see doubleRender()),
      *   then temporary objects into the temporary list.
      * - \c TemporaryOnly: rebuilds only the temporary list, if there are
      *   temporary objects.
      * - \c Selection: rebuilds the selection list and passes it to the
      *   GLWidgetManager.
      *
      * finalizeRendering() is called in every mode.
      *
      * \param objs Objects of the window.
      * \param mode Rendering pass to perform.
      * \param selectmode Selection render mode (normal rendering by default).
      * \return True if at least one object was rendered.
      */
      bool renderScene( const std::list<carto::shared_ptr<anatomist::AObject>> & objs, RenderMode mode = RenderMode::Full,
                        anatomist::ViewState::glSelectRenderMode selectmode
                          = anatomist::ViewState::glSELECTRENDER_NONE);

      /**
      * \brief Renders a list of objects into the current primitive list.
      *
      * \details
      * Objects are grouped by shader program and transparency
      * (see retrieveShaders()), then rendered in this order:
      * - opaque groups,
      * - transparent groups, with either alpha blending or depth peeling,
      *   depending on the GLWidgetManager settings,
      * - objects without GL API, rendered without shader program.
      *
      * Shader programs are obtained from ShaderProgramCache: a program
      * already compiled for another object or another window is reused.
      *
      * \param objs List of objects to render.
      * \param mode Filter on permanent / temporary objects.
      * \param selectmode Selection render mode (normal rendering by default).
      * \return True if at least one object was rendered.
      */
      bool renderObjects( const std::list<carto::shared_ptr<anatomist::AObject>> & objs, RenderMode mode = RenderMode::Full,
                          anatomist::ViewState::glSelectRenderMode selectmode
                          = anatomist::ViewState::glSELECTRENDER_NONE);

      /**
      * \brief Pushes a per-object clip plane.
      *
      * The plane applies to every object rendered until the matching
      * popObjectClipPlane() (e.g. children of a ClippedObject). Planes can
      * be nested, up to 6; extra planes are ignored with a warning.
      *
      * \param plane Plane equation (a, b, c, d): points with
      *              a*x + b*y + c*z + d < 0 are clipped.
      */
      void pushObjectClipPlane( const Point4df & plane);

      /**
      * \brief Removes the last pushed per-object clip plane.
      */
      void popObjectClipPlane();

      /**
      * \brief Per-object clip planes currently active, in push order.
      */
      const std::vector<Point4df> & objectClipPlanes() const;

      /**
      * \brief ViewState of the object being rendered.
      *
      * Set by updateObject() for each object (2D or 3D view state).
      * Only valid during rendering.
      */
      const anatomist::ViewState& getViewState() const ;

      /**
      * \brief Replaces the current ViewState.
      * \param vs The new view state.
      */
      void setViewState(carto::rc_ptr<anatomist::ViewState> vs);

      /**
      * \brief Sets up window clip planes and appends the matching OpenGL state.
      *
      * \details
      * Computes the window clip planes from the slice orientation, the
      * cursor position and the clip distance, according to the window clip
      * mode (none, single or double), and stores them in the GLWidgetManager
      * clip state. Then appends a GL list enabling the matching
      * \c GL_CLIP_DISTANCEi:
      * - distances 0 and 1: window clip planes,
      * - distances 2 to 7: per-object clip planes, always enabled; the
      *   shader only uses those the object actually has.
      */
      void setupClippingPlanes();

      /**
      * \brief Passes lighting and background alpha to the GLWidgetManager,
      *        then requests a repaint.
      */
      void finalizeRendering();

    private:
      /**
      * \brief Updates the rendering of a single object.
      *
      * Builds the appropriate \c ViewState (2D or 3D) and calls
      * the object's \c render() method.
      * Window-level \c ObjectModifiers are then applied.
      *
      * \param obj The object to update.
      * \param pl Optional primitive list to append to. If null, uses the internal list.
      * \param selectmode The OpenGL render mode (normal, selection, etc.).
      * \return True if the object was rendered.
      */
      bool updateObject(carto::shared_ptr<anatomist::AObject> obj,
                        anatomist::PrimList* pl=0,
                        anatomist::ViewState::glSelectRenderMode selectmode
                          = anatomist::ViewState::glSELECTRENDER_NONE);

      /**
      * \brief Renders groups of objects, one shader program per group.
      *
      * \details
      * For each group:
      * - gets the program matching the group key from ShaderProgramCache
      *   (the group is skipped if the program failed to compile),
      * - binds it once (see switchShaderProgram()),
      * - resolves the shader modules of the key through ShaderModuleRegistry
      *   (e.g. BlinnPhong + DepthPeeling),
      * - adds \c GLSceneUniforms once per module, before the first rendered
      *   object, and \c GLObjectUniforms for each object and module,
      * - calls updateObject() to render each object.
      *
      * Objects not matching \p mode (permanent / temporary) are skipped.
      *
      * \param drawables Groups of objects to render (opaque or transparent).
      * \param mode Filter on permanent / temporary objects.
      * \param selectmode Selection render mode.
      * \return True if at least one object was rendered.
      */
      bool renderObject( DrawableGroups & drawables, RenderMode mode,
                       ViewState::glSelectRenderMode selectmode );

      /**
      * \brief Builds the shader program key of an object.
      *
      * \details
      * The key contains:
      * - the rendering pipeline of the object (surface or volume),
      * - the shader modules declared by the object, plus
      *   ShaderModuleId::DepthPeeling when depth peeling is enabled in the
      *   window (a window decision, not an object one),
      * - the vertex, fragment and geometry shader templates of the object.
      *
      * Objects with equal keys share the same program.
      *
      * \param glObj GL API of the object.
      * \return The key of the program needed by the object.
      */
      ShaderProgramKey programKey( GLComponent * glObj ) const;

      /**
      * \brief Sorts objects into shader program groups.
      *
      * \details
      * For each object:
      * - if it has a GL API, it is stored, under its programKey(), in the
      *   transparent groups if \c isTransparent() is true, in the opaque
      *   groups otherwise,
      * - otherwise it is stored in \p nonDrawables, rendered later without
      *   shader program.
      *
      * \param objs List of objects to sort.
      * \param opaqueDrawables Output: opaque objects grouped by program.
      * \param transparentDrawables Output: transparent objects grouped by program.
      * \param nonDrawables Output: objects without GL API.
      */
      void retrieveShaders( const std::list<carto::shared_ptr<AObject> > & objs,
                          DrawableGroups & opaqueDrawables,
                          DrawableGroups & transparentDrawables,
                          std::vector<carto::shared_ptr<AObject> > & nonDrawables );

      /**
      * \brief Switches to a new shader program if different from the current one.
      *
      * Adds a \c GLBindShader primitive if the program changes.
      * A null program is ignored with a warning.
      *
      * \param program The GLSL program to bind.
      */
      void switchShaderProgram( carto::rc_ptr<QOpenGLShaderProgram> program );

      /**
      * \brief Sets up OpenGL state for rendering transparent objects.
      *
      * Appends a GL list enabling blending with standard alpha blending
      * parameters (\c GL_SRC_ALPHA, \c GL_ONE_MINUS_SRC_ALPHA) and depth
      * writing. Only used when depth peeling is disabled.
      */
      void setupTransparentObjects();

      /**
      * \brief Restores OpenGL state after rendering transparent objects.
      *
      * Appends a GL list re-enabling depth mask writing.
      */
      void postTransparentRenderingSetup();

      /**
      * \brief Appends a GL list setting the window-wide OpenGL state.
      *
      * \details
      * Sets line width, face culling, line and polygon smoothing, and the
      * polygon mode / offset matching the window rendering mode (normal,
      * wireframe, hidden wireframe, outlined). Also updates flat shading
      * and fog parameters in the GLWidgetManager.
      */
      void setupOpenGLState();

      /**
      * \brief Prepares the second rendering pass of two-pass modes.
      *
      * In hidden wireframe and outlined modes, appends the state of the
      * second pass and duplicates the primitive list, so that objects are
      * drawn twice. Does nothing in other modes.
      */
      void doubleRender();

      /**
      * \brief Builds the state primitive of the hidden wireframe second pass
      *        (line polygon mode).
      * \return The primitive, or null if no GL list could be allocated.
      */
      anatomist::Primitive* setupHiddenWireframeMode();

      /**
      * \brief Builds the state primitive of the outlined second pass
      *        (line polygon mode, lighting disabled).
      * \return The primitive, or null if no GL list could be allocated.
      */
      anatomist::Primitive* setupOutlinedMode();

      /**
      * \brief Duplicates the current primitive list, so that the scene is
      *        drawn a second time with the state set by doubleRender().
      */
      void duplicateRenderPrimitives();


      struct Private;
      /// Private data (owned). Null for an empty or moved-from context.
      Private * d = nullptr;
  };
}
#endif