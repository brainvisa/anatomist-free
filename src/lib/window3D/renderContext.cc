#include <anatomist/window3D/renderContext.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/shaderMapping.h>
#include <anatomist/surface/IShaderModule.h>
#include <anatomist/surface/dynamicShaderBuilder.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/misc/error.h>
#include <cartobase/smart/rcptr.h>



#include <QOpenGLShaderProgram>
#include <QOpenGLContext>

using namespace anatomist;

struct RenderContext::Private
{
  Private(AWindow3D* win, GLWidgetManager* widgetManager);
  ~Private();

  AWindow3D* window;
  GLWidgetManager * glwman;
  std::shared_ptr<PrimList> primitives;
  dynamicShaderBuilder shaderBuilder;
  std::map<std::string, std::shared_ptr<QOpenGLShaderProgram>> programs;
  std::shared_ptr<QOpenGLShaderProgram> currentProgram;
  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> opaqueDrawables;
  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> transparentDrawables;
};

RenderContext::Private::Private(AWindow3D* win, GLWidgetManager* widgetManager) : 
window(win), glwman(widgetManager), primitives(std::make_shared<PrimList>()), currentProgram(nullptr)
{}

RenderContext::Private::~Private()
{
  glwman = nullptr;
  primitives.reset();
  programs.clear();
  currentProgram = nullptr;
  opaqueDrawables.clear();
  transparentDrawables.clear();
}

RenderContext::RenderContext(AWindow3D* window, GLWidgetManager* widgetManager)
{
  d = new Private(window, widgetManager);
}

RenderContext::~RenderContext()
{
  d= nullptr;
  delete d;
}

std::shared_ptr<PrimList> RenderContext::renderObjects( const std::list<carto::shared_ptr<AObject>> & objs)
{
  d->opaqueDrawables.clear();
  d->transparentDrawables.clear();

  retrieveShaders(objs);
  shaderBuilding();

  if(!d->opaqueDrawables.empty())
    renderObject(false);

  if(!d->transparentDrawables.empty())
  {
    if(!d->glwman->useDepthPeeling())
    {
      setupTransparentObjects();
      renderObject(true);
      postTransparentRenderingSetup();
    }
    else
    {
      renderObject(true);
    }
  }
  return d->primitives;
}

void RenderContext::updateObject(carto::shared_ptr<AObject> obj, PrimList* pl,
                                 ViewState::glSelectRenderMode selectmode)
{
  unsigned l1=0, l2;
  if(pl)
    l1 = pl->size();
  else
    l1 = d->primitives->size();

  GLPrimitives gp;
  if(!obj->Is2DObject() || (d->window->viewType() == AWindow3D::ThreeD && obj->Is3DObject())) // replace 1 by viewType
  {
    if(!pl) pl = d->primitives.get();
    obj->render(*pl, ViewState(d->window->getTimes(), d->window, selectmode));
  }
  else
  {
    if(!pl) pl = d->primitives.get();
    SliceViewState st(d->window->getTimes(), true, d->window->getPosition(), &d->window->sliceQuaternion(),
                      d->window->getReferential(), d->window->windowGeometry(), &d->glwman->quaternion(), d->window, selectmode);
    obj->render(*pl, st);
    std::cout << "Rendering 2D Object" << std::endl;
  }

  auto objModifiers = d->window->getModifiers();
  std::list<AWindow3D::ObjectModifier *>::iterator im, em = objModifiers.end();
  for(im = objModifiers.begin(); im != em; ++im)
    (*im)->modify( obj.get(), gp );

  if(pl)
  {
    pl->insert(pl->end(), gp.begin(), gp.end());
    l2 = pl->size();
  }
  else
  {
    d->primitives->insert(d->primitives->end(), gp.begin(), gp.end());
    l2 = d->primitives->size();
  }

  //if(l2 > l1)
    //Jordan : tmpprims utile ?

}

void RenderContext::renderObject(bool isTransparent)
{
  const auto & drawables = isTransparent ? d->transparentDrawables : d->opaqueDrawables;
  for(const auto & [shader, objects] : drawables)
  {
    switchShaderProgram(d->programs[shader]);
    auto shaderModules = getEffectiveShaderModules(objects.front()->glAPI()->getShaderModuleIDs());
    for(size_t i=0; i< objects.size(); ++i)
    {
      auto glObj = objects[i]->glAPI();
      for(const auto & module : shaderModules)
      {
        if(i==0)
          d->primitives->push_back(carto::rc_ptr<GLItem>(new GLSceneUniforms(module, d->currentProgram, d->glwman))); // Jordan : segfaulting why ?
        d->primitives->push_back(carto::rc_ptr<GLItem>(new GLObjectUniforms(module, d->currentProgram, glObj)));
      }

      // update Object
      updateObject(objects[i]);
    }
  }
  d->currentProgram = nullptr;
}

void RenderContext::retrieveShaders(const std::list<carto::shared_ptr<AObject>> & objs)
{
  for(const auto & obj : objs)
  {
    auto glObj = obj->glAPI();
    if(glObj)
    {
      std::string shaderID = glObj->getShaderModuleIDs();
      if(d->glwman->useDepthPeeling())
        shaderID += "1"; //id for depth peeling
      if(obj->isTransparent())
        d->transparentDrawables[shaderID].push_back(obj);
      else
        d->opaqueDrawables[shaderID].push_back(obj);
    }
  }
}

void RenderContext::shaderBuilding()
{
  for(const auto & [shader, _] : d->opaqueDrawables)
  {
    if(d->programs[shader] == nullptr)
    {
      d->programs[shader] = d->shaderBuilder.initShader(shader);
    }
  }

  for(const auto & [shader, _] : d->transparentDrawables)
  {
    if(d->programs[shader] == nullptr)
    {
      d->programs[shader] = d->shaderBuilder.initShader(shader);
    }
  }
}

void RenderContext::switchShaderProgram( std::shared_ptr<QOpenGLShaderProgram> program )
{
  if(!program)
  {
    AWarning("RenderContext::switchShaderProgram: null shader program");
    return;
  }

  if(d->currentProgram == program)
    return;

  d->currentProgram = program;
  d->primitives->push_back(carto::rc_ptr<GLItem>(new GLBindShader(program)));
}

void RenderContext::setupTransparentObjects()
{
  GLList * renderpr = new GLList;
  renderpr->generate();
  GLuint renderGLL = renderpr->item();
  if(!renderGLL)
  {
    AWarning("RenderContext::setupTransparentObjects: could not generate GL list for transparent objects rendering");
    delete renderpr;
    return;
  }
  glNewList( renderGLL, GL_COMPILE );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDepthMask(GL_TRUE);
  glEndList();
  d->primitives->push_back(RefGLItem(renderpr));
}

void RenderContext::postTransparentRenderingSetup()
{
  GLList * renderpr = new GLList;
  renderpr->generate();
  GLuint renderGLL = renderpr->item();
  if(!renderGLL)
  {
    AWarning("RenderContext::postTransparentRenderingSetup: could not generate GL list for post transparent objects rendering");
    delete renderpr;
    return;
  }
  glNewList( renderGLL, GL_COMPILE );
  glDisable( GL_BLEND );
  glDepthMask(GL_TRUE);
  glEndList();
  d->primitives->push_back(RefGLItem(renderpr));
}

std::vector<std::shared_ptr<IShaderModule>> RenderContext::getEffectiveShaderModules(const std::string& shaderID)
{
  auto modules = shaderMapping::getModules(shaderID);
  if(d->glwman->useDepthPeeling())
  {
    auto dpmodules = shaderMapping::getModules("1"); //id for depth peeling
    modules.insert(modules.end(), dpmodules.begin(), dpmodules.end());
  }
  return modules;
}
