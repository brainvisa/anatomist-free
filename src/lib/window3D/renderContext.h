#ifndef ANA_WINDOW3D_RENDERCONTEXT_H
#define ANA_WINDOW3D_RENDERCONTEXT_H

#include <anatomist/object/Object.h>
#include <anatomist/window/viewstate.h>
#include <cartobase/smart/rcptr.h>

class QOpenGLShaderProgram;

class AWindow3D;
namespace anatomist
{
  class GLWidgetManager;
  class GLComponent; 
  class IShaderModule;


  /**
  * \class RenderContext
  * \brief Handles OpenGL rendering logic for AWindow3D.
  *
  * This class centralizes all OpenGL rendering operations, decoupling
  * rendering logic from AWindow3D.  
  * It sorts objects by shader type, builds and binds GLSL programs,
  * sets scene and object uniforms, manages OpenGL states, and
  * collects all rendering commands into a single primitive list.
  *
  * The resulting \c PrimList contains:
  * - shader binding/unbinding commands (\c GLBindShader, \c GLReleaseShader),
  * - uniform setup commands (\c GLSceneUniforms, \c GLObjectUniforms),
  * - object rendering calls (\c AObject::render()).
  */
  class RenderContext
  {
    public:
    /**
    * \brief Constructs a RenderContext for a given 3D window and GL manager.
    * \param window The parent 3D window.
    * \param widgetManager The OpenGL manager providing context and state.
    */
      RenderContext(AWindow3D* win, anatomist::GLWidgetManager* widgetManager);
      ~RenderContext();

      bool renderScene( const std::list<carto::shared_ptr<anatomist::AObject>> & objs);
    
    /**
    * \brief Renders a list of objects and builds the corresponding primitive list.
    *
    * \details
    * The method sorts objects into opaque and transparent groups,
    * builds shader programs for each group, and generates
    * a complete sequence of OpenGL primitives for rendering.
    *
    * - Opaque objects are rendered directly.
    * - Transparent objects are rendered with either alpha blending or
    *   depth peeling, depending on the OpenGL settings.
    *
    * \param objs List of objects to render.
    * \return Boolean to know if the rendering succeed or not.
    */
      bool renderObjects( const std::list<carto::shared_ptr<anatomist::AObject>> & objs);


      const anatomist::ViewState& getViewState() const ;
      void setViewState(carto::rc_ptr<anatomist::ViewState> vs);
      void setupClippingPlanes(GLuint localGLL);
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
      */
      bool updateObject(carto::shared_ptr<anatomist::AObject> obj, anatomist::PrimList* pl=0, anatomist::ViewState::glSelectRenderMode selectmode
                          = anatomist::ViewState::glSELECTRENDER_NONE);
      /**
      * \brief Renders all objects of a given group (opaque or transparent).
      *
      * \details
      * - Binds the corresponding shader program for the group.
      * - Retrieves all shader modules associated with the shader ID
      *   (e.g., "01" → BlinnPhong + DepthPeeling).
      * - Adds \c GLSceneUniforms once per shader and
      *   \c GLObjectUniforms for each object and module.
      * - Calls \c updateObject() to render each object.
      *
      * \param isTransparent True if rendering the transparent group.
      */
      bool renderObject(std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & drawables);

      /**
      * \brief Sorts objects by shader type and transparency.
      *
      * \details
      * For each object:
      * - The shader ID is retrieved via \c glAPI()->getShaderModuleIDs().
      * - If depth peeling is active, module "1" is appended to the shader ID.
      * - The object is then stored in either the opaque or transparent map.
      *
      * \param objs List of objects to sort.
      */
      void retrieveShaders(const std::list<carto::shared_ptr<anatomist::AObject>> & objs,
                                  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>>& opaqueDrawables,
                                  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & transparentDrawables, 
                                  std::vector<carto::shared_ptr<AObject>> & nonDrawables);

      /**
      * \brief Builds all required GLSL programs.
      *
      * Initializes each program using \c dynamicShaderBuilder for every
      * unique shader ID collected during sorting.
      */
      void shaderBuilding(std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> &opaqueDrawables,
                          std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & transparentDrawables);

      /**
      * \brief Switches to a new shader program if different from the current one.
      *
      * Adds a \c GLBindShader primitive if the program changes.
      * \param program The GLSL program to bind.
      */
      void switchShaderProgram( carto::rc_ptr<QOpenGLShaderProgram> program );

      /**
      * \brief Sets up OpenGL state for rendering transparent objects.
      *
      * Enables blending and applies standard alpha blending parameters:
      * \c GL_SRC_ALPHA and \c GL_ONE_MINUS_SRC_ALPHA.
      */
      void setupTransparentObjects();

      /**
      * \brief Restores OpenGL state after rendering transparent objects.
      *
      * Disables blending and re-enables depth mask writing.
      */
      void postTransparentRenderingSetup();

      /**
      * \brief Returns all effective shader modules for a given shader ID.
      *
      * \details
      * A shader ID may contain multiple digits (e.g., "01"),
      * where each digit corresponds to a specific shader module
      *
      * All corresponding modules are retrieved and concatenated
      * to form the complete shader used for rendering.
      *
      * \param shaderID The composite shader identifier.
      * \return List of GLSL modules combined for the shader.
      */
      std::vector<carto::rc_ptr<anatomist::IShaderModule>> getEffectiveShaderModules(const std::string& shaderID);

      void setupOpenGLState();
      anatomist::Primitive* setupHiddenWireframeMode();
      anatomist::Primitive* setupOutlinedMode();
      void duplicateRenderPrimitives();
      void cursorGLL() const;
      

      struct Private;
      Private * d;
  };
}
#endif