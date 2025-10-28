#ifndef ANA_WINDOW3D_RENDERCONTEXT_H
#define ANA_WINDOW3D_RENDERCONTEXT_H

#include <anatomist/object/Object.h>
#include <anatomist/window/viewstate.h>

class QOpenGLShaderProgram;

class AWindow3D;
namespace anatomist
{
  class GLWidgetManager;
  class GLComponent; 
  class IShaderModule;


  class RenderContext
  {
    public:
      RenderContext(AWindow3D* win, anatomist::GLWidgetManager* widgetManager);
      ~RenderContext();
      std::shared_ptr<anatomist::PrimList> renderObjects( const std::list<carto::shared_ptr<anatomist::AObject>> & objs);

    private:
      void updateObject(carto::shared_ptr<anatomist::AObject> obj, anatomist::PrimList* pl=0, anatomist::ViewState::glSelectRenderMode selectmode
                          = anatomist::ViewState::glSELECTRENDER_NONE);
      void renderObject(bool isTransparent);
      void retrieveShaders(const std::list<carto::shared_ptr<anatomist::AObject>> & objs);
      void shaderBuilding();
      void switchShaderProgram( std::shared_ptr<QOpenGLShaderProgram> program );
      void setupTransparentObjects();
      void postTransparentRenderingSetup();
      std::vector<std::shared_ptr<anatomist::IShaderModule>> getEffectiveShaderModules(const std::string& shaderID);

      struct Private;
      Private * d;
  };
}
#endif

// - une classe qui fait le lien entre la vue et les objets, je l'appelle ici RenderContext (à voir si on trouve un meilleur nom):
// class RenderContext
// public:
//     // should be more or less the only needed public method
//     renderObjects(list<AObjectOrGLComponent>)
// protected/private:
//     currentBoundShaderProgram()
//     renderObject() (moved/modified from AWindow3D::updateObject() + renderOpaqueObjects() etc; I would merge renderOpaqueObjects() and renderTransparentObjects() in a way or another, at least with a flag and call it twice with a different flag value)
//     retrieveShaders() (moved from AWindow3D)
//     shaderBuilding() (moved from AWindow3D)
//     switchShaderProgram()
//     setupTransparentObjects() (moved from AWindow3D)
//     postTransparentRenderingSetup() (moved from AWindow3D)
//     getEffectiveShaderModules() (moved from AWindow3D)

//     // internal variables
//     GLWidgetManager *
//     AWindow * (3D?)
//     map<id, ShaderProgram> (AWidow3D::d->programs)
//     list<ObjectModifier *>
//     ViewState * 2D + 3D

// // need for a placeholer AObjectOrGLComponent:
// struct AObjectOrGLComponent
//     AObject: nullptr
//     GLComponent: nullptr

// AWindow3D::refreshNow() would:
// - instantiate / retreive a clean RenderContext
// - set it up (ViewState, GLWidgetManager, scene shader modules and uniforms)
// - call renderObjects() on it
// - call finalizeRendering() etc as before

// AObject::render() would get a RenderContext instead of ViewState
// MObject::render() (and sub-classes) would overload render() to call RenderContext::renderObjects() for them and their visible children