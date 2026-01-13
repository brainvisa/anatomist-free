#include <anatomist/window3D/renderContext.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/shaderMapping.h>
#include <anatomist/surface/IShaderModule.h>
#include <anatomist/surface/dynamicShaderBuilder.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/primitive/primitive.h>
#include <anatomist/misc/error.h>
#include <anatomist/color/Light.h>
#include <anatomist/window3D/cursor.h>
#include <anatomist/reference/Referential.h>

#include <aims/resampling/quaternion.h>




#include <QOpenGLShaderProgram>
#include <QOpenGLContext>

using namespace anatomist;

struct RenderContext::Private
{
  Private(AWindow3D* win, GLWidgetManager* widgetManager);
  ~Private();

  AWindow3D* window;
  GLWidgetManager * glwman;
  //PrimList& primitives;
  PrimList& permanentPrimitives;
  PrimList& temporaryPrimitives;
  PrimList* currentPrimitives;
  dynamicShaderBuilder shaderBuilder;
  std::map<std::string, carto::rc_ptr<QOpenGLShaderProgram>> programs;
  carto::rc_ptr<QOpenGLShaderProgram> currentProgram;
  carto::rc_ptr<ViewState> vs;
};

RenderContext::Private::Private(AWindow3D* win, GLWidgetManager* widgetManager) : 
window(win), glwman(widgetManager),permanentPrimitives(glwman->permanentPrimitivesRef()), temporaryPrimitives(glwman->tempPrimitivesRef()) ,currentProgram(nullptr), currentPrimitives(nullptr)
{
}

RenderContext::Private::~Private()
{
  glwman = nullptr;
  programs.clear();
  currentProgram.reset();
}

RenderContext::RenderContext(AWindow3D* window, GLWidgetManager* widgetManager)
{
  d = new Private(window, widgetManager);
  d->glwman->clearLists();
  shaderMapping::initShaderMapping();
  GLuint localGLL = glGenLists(2);
  d->currentPrimitives = &d->permanentPrimitives;
  setupClippingPlanes(localGLL);
}

RenderContext::~RenderContext()
{
  d= nullptr;
  delete d;
}

bool RenderContext::renderScene( const std::list<carto::shared_ptr<AObject>> & objs, RenderMode mode )
{
  bool success = false;
  d->glwman->qglWidget()->makeCurrent();

  if(mode == RenderMode::Full)
    d->permanentPrimitives.clear();
  
  d->temporaryPrimitives.clear();

  setupOpenGLState();
  if(mode == RenderMode::Full)
  {
    d->currentPrimitives = &d->permanentPrimitives;
    success |= renderObjects(objs, mode);
  }

  d->currentPrimitives = &d->temporaryPrimitives;
  success |= renderObjects(objs, mode);
  finalizeRendering();
  return success;

}


bool RenderContext::renderObjects( const std::list<carto::shared_ptr<AObject>> & objs, RenderMode mode)
{
  bool success = false;
  std::vector<float> bbmin, bbmax;
  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> opaqueDrawables;
  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> transparentDrawables;
  std::vector<carto::shared_ptr<AObject>> nonDrawables;

  retrieveShaders(objs, opaqueDrawables, transparentDrawables ,nonDrawables);
  shaderBuilding( opaqueDrawables, transparentDrawables);

  if(!opaqueDrawables.empty())
    success |= renderObject(opaqueDrawables, mode);

  if(!transparentDrawables.empty())
  {
    if(!d->glwman->useDepthPeeling())
    {
      setupTransparentObjects();
      success |= renderObject(transparentDrawables, mode);
      postTransparentRenderingSetup();
    }
    else
    {
      success |= renderObject(transparentDrawables, mode);
    }
  }

  if(!nonDrawables.empty())
  {
    for(const auto & obj : nonDrawables)
    {
      success |= updateObject(obj);
    }
  }
  return success;
}

bool RenderContext::updateObject(carto::shared_ptr<AObject> obj, PrimList* pl,ViewState::glSelectRenderMode selectmode)
{
  bool success = false;
  unsigned l1=0, l2;
  if(pl)
    l1 = pl->size();
  else
    l1 = d->currentPrimitives->size();

  GLPrimitives gp;
  if(!pl) pl = d->currentPrimitives;
  if(!obj->Is2DObject() || (d->window->viewType() == AWindow3D::ThreeD && obj->Is3DObject()))
  {
    d->vs.reset(new ViewState(d->window->getTimes(), d->window, selectmode));
  }
  else
  {
    d->vs.reset(new SliceViewState(d->window->getTimes(), true, d->window->getPosition(), &d->window->sliceQuaternion(),
                      d->window->getReferential(), d->window->windowGeometry(), &d->glwman->quaternion(), d->window, selectmode));
  }

  success |= obj->render(*pl, *this);

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
    d->currentPrimitives->insert(d->currentPrimitives->end(), gp.begin(), gp.end());
    l2 = d->currentPrimitives->size();
  }

  //if(l2 > l1)
    //Jordan : tmpprims utile ?
  
  return success;
}

bool RenderContext::renderObject(std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & drawables, RenderMode mode)
{
  bool success = false;

  for(const auto & [shader, objects] : drawables)
  { 
    switchShaderProgram(d->programs[shader]);
    auto shaderModules = getEffectiveShaderModules(objects.front()->glAPI()->getShaderModuleIDs());
    for(size_t i=0; i< objects.size(); ++i)
    {
      auto obj = objects[i];

      if(mode == RenderMode::TemporaryOnly && !d->window->isTemporary(obj.get()))
        continue;

      auto glObj = objects[i]->glAPI();

      for(const auto & module : shaderModules)
      {
        if(i==0)
          d->currentPrimitives->push_back(carto::rc_ptr<GLItem>(new GLSceneUniforms(module, d->currentProgram, d->glwman)));
        d->currentPrimitives->push_back(carto::rc_ptr<GLItem>(new GLObjectUniforms(module, d->currentProgram, glObj)));
      }

      success |= updateObject(obj);
    }
  }
  d->currentProgram = carto::rc_ptr<QOpenGLShaderProgram>();
  return success;
}

void RenderContext::retrieveShaders(const std::list<carto::shared_ptr<AObject>> & objs,
                                  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & opaqueDrawables,
                                  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & transparentDrawables, 
                                   std::vector<carto::shared_ptr<AObject>> & nonDrawables)
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
        transparentDrawables[shaderID].push_back(obj);
      else
        opaqueDrawables[shaderID].push_back(obj);
    }
    else
    {
      nonDrawables.push_back(obj);
    }
  }
}

void RenderContext::shaderBuilding(std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>> & opaqueDrawables,
                                  std::unordered_map<std::string, std::vector<carto::shared_ptr<AObject>>>  & transparentDrawables)
{
  for(const auto & [shader, _] : opaqueDrawables)
  {
    if(d->programs[shader].isNull())
    {
      d->programs[shader] = d->shaderBuilder.initShader(shader);
    }
  }

  for(const auto & [shader, _] : transparentDrawables)
  {
    if(d->programs[shader].isNull())
    {
      d->programs[shader] = d->shaderBuilder.initShader(shader);
    }
  }
}

void RenderContext::switchShaderProgram( carto::rc_ptr<QOpenGLShaderProgram> program )
{
  if(program.isNull())
  {
    AWarning("RenderContext::switchShaderProgram: null shader program");
    return;
  }

  if(d->currentProgram == program)
    return;

  d->currentProgram = program;
  d->currentPrimitives->push_back(carto::rc_ptr<GLItem>(new GLBindShader(program)));
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
  d->currentPrimitives->push_back(RefGLItem(renderpr));
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
  d->currentPrimitives->push_back(RefGLItem(renderpr));
}

std::vector<carto::rc_ptr<IShaderModule>> RenderContext::getEffectiveShaderModules(const std::string& shaderID)
{
  auto modules = shaderMapping::getModules(shaderID);
  if(d->glwman->useDepthPeeling())
  {
    auto dpmodules = shaderMapping::getModules("1"); //jordan : id for depth peeling, may change
    modules.insert(modules.end(), dpmodules.begin(), dpmodules.end());
  }
  return modules;
}

void RenderContext::setupClippingPlanes(GLuint localGLL)
{
  Primitive *pr = new Primitive;
  if (!localGLL) AWarning("renderContext::setupClippingPlanes: OpenGL error.");

  glNewList(localGLL, GL_COMPILE);
  glDisable( GL_BLEND);
  GLdouble plane[4];
  Point3df dir = d->window->sliceQuaternion().transformInverse( Point3df(0, 0, -1) );
  plane[0] = dir[0];
  plane[1] = dir[1];
  plane[2] = dir[2];
  plane[3] = -dir.dot(d->window->getPosition()) + d->window->clipDistance();

  switch (d->window->clipMode())
  {
    case AWindow3D::Single:
      glEnable( GL_CLIP_PLANE0);
      glDisable( GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      break;
    case AWindow3D::Double:
      glEnable(GL_CLIP_PLANE0);
      glEnable(GL_CLIP_PLANE1);
      glClipPlane(GL_CLIP_PLANE0, plane);
      plane[0] *= -1;
      plane[1] *= -1;
      plane[2] *= -1;
      plane[3] = dir.dot(d->window->getPosition()) + d->window->clipDistance();
      glClipPlane(GL_CLIP_PLANE1, plane);
      break;
    default:
      glDisable(GL_CLIP_PLANE0);
      glDisable(GL_CLIP_PLANE1);
      break;
  }

  glEndList();

  pr->insertList(localGLL);
  d->currentPrimitives->push_back(RefGLItem(pr));
}

void RenderContext::setupOpenGLState()
{
  GLList *renderpr = new GLList;
  renderpr->generate();
  GLuint renderGLL = renderpr->item();
  if (!renderGLL) AWarning("AWindow3D::Refresh: OpenGL error.");

  glNewList(renderGLL, GL_COMPILE);
  glLineWidth(1);
  d->window->flatShading() ? glShadeModel(GL_FLAT) : glShadeModel(GL_SMOOTH); //jordan : might be deleted ?
  d->window->cullingEnabled() ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  if(d->window->smoothing())
  {
    glEnable( GL_LINE_SMOOTH);
    glEnable( GL_POLYGON_SMOOTH);
  }
  else
  {
    glDisable( GL_LINE_SMOOTH);
    glDisable( GL_POLYGON_SMOOTH);
  }

  glEnable( GL_LIGHTING);
  glPolygonOffset(0, 0);
  glDisable( GL_POLYGON_OFFSET_FILL);
  if (d->window->fog())
  {
    glEnable( GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogf(GL_FOG_DENSITY, 0.01);
    glFogfv(GL_FOG_COLOR, d->window->light()->Background());
  }
  else
  {
    glDisable( GL_FOG);
  }

  switch (d->window->renderingMode())
  {
    case AWindow3D::Wireframe:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      break;
    case AWindow3D::HiddenWireframe:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glPolygonOffset(1.05, 1);
      glEnable(GL_POLYGON_OFFSET_FILL);
      break;
    case Material::Outlined:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glPolygonOffset(1.05, 1);
      glEnable(GL_POLYGON_OFFSET_FILL);
    default:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  }

  glEndList();
  d->currentPrimitives->push_back(RefGLItem(renderpr));
}

const ViewState& RenderContext::getViewState() const
{
  return *d->vs;
}

void RenderContext::setViewState(carto::rc_ptr<ViewState> vs)
{
  d->vs = vs;
}

void RenderContext::finalizeRendering()
{
  Primitive* renderoffpr = 0;
  bool rendertwice = false;

  switch (d->window->renderingMode())
  {
    case AWindow3D::HiddenWireframe:
      renderoffpr = setupHiddenWireframeMode();
      rendertwice = true;
      break;
    case Material::Outlined:
      renderoffpr = setupOutlinedMode();
      rendertwice = true;
      break;
    default:
      break;
  }

  if (renderoffpr)
  {
    d->currentPrimitives->push_back(RefGLItem(renderoffpr)); // jordan : to check which primitive list we'll be using
  }

  if (rendertwice)
  {
    duplicateRenderPrimitives();
  }

  if(d->window->light())
  {
    d->glwman->setLightGLList(d->window->light()->getGLList());
    d->glwman->setBackgroundAlpha(d->window->light()->Background()[3]);
  }
  else
  {
    d->glwman->setBackgroundAlpha(1.0);
  }

  d->glwman->updateGL();
}

Primitive* RenderContext::setupHiddenWireframeMode()
{
  Primitive *pr = new Primitive;
  GLuint hwfGLL = glGenLists(1);
  if (!hwfGLL)
  {
      std::cerr << "AWindow3D: not enough OGL memory.\n";
      delete pr;
      return nullptr;
  }

  glNewList(hwfGLL, GL_COMPILE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEndList();

  pr->insertList(hwfGLL);
  return pr;
}

Primitive* RenderContext::setupOutlinedMode()
{
  Primitive *pr = new Primitive;
  GLuint hwfGLL = glGenLists(1);
  if (!hwfGLL)
  {
      std::cerr << "AWindow3D: not enough OGL memory.\n";
      delete pr;
      return nullptr;
  }

  glNewList(hwfGLL, GL_COMPILE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDisable(GL_LIGHTING);
  glEndList();

  pr->insertList(hwfGLL);
  return pr;
}

void RenderContext::duplicateRenderPrimitives()
{
  unsigned i, n = d->currentPrimitives->size() - 2;
  PrimList::iterator ip = d->currentPrimitives->begin();
  for (++ip, i = 0; i < n; ++i, ++ip)
      d->currentPrimitives->push_back(*ip);
}


