#include <anatomist/window3D/renderContext.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidgetmanager.h>
#include <anatomist/surface/IShaderModule.h>
#include <anatomist/surface/shaderModuleRegistry.h>
#include <anatomist/surface/shaderProgramCache.h>
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
  PrimList& permanentPrimitives;
  PrimList& temporaryPrimitives;
  PrimList& selectionPrimitives;
  PrimList* currentPrimitives;
  carto::rc_ptr<QOpenGLShaderProgram> currentProgram;
  carto::rc_ptr<ViewState> vs;
  std::list<AObject *> renderobj;
  std::vector<Point4df> objectClipPlanes;
};

RenderContext::Private::Private(AWindow3D* win, GLWidgetManager* widgetManager) : 
window(win), glwman(widgetManager),permanentPrimitives(glwman->permanentPrimitivesRef()), temporaryPrimitives(glwman->tempPrimitivesRef()), selectionPrimitives(glwman->selectionPrimitivesRef()) ,currentProgram(nullptr), currentPrimitives(nullptr)
{
}

RenderContext::Private::~Private()
{
  glwman = nullptr;
  currentProgram.reset();
}

RenderContext::RenderContext(AWindow3D* window, GLWidgetManager* widgetManager)
{
  d = new Private(window, widgetManager);
  d->glwman->clearLists();
  d->currentPrimitives = &d->permanentPrimitives;

}

RenderContext::RenderContext( RenderContext && other ) noexcept
  : d( other.d )
{
  other.d = nullptr;
}

RenderContext & RenderContext::operator=( RenderContext && other ) noexcept
{
  if( this != &other )
  {
    delete d;
    d = other.d;
    other.d = nullptr;
  }
  return *this;
}

RenderContext::~RenderContext()
{
  if( !d )
    return;
  delete d;
  d = nullptr;
}

bool RenderContext::renderScene( const std::list<carto::shared_ptr<AObject>> & objs, RenderMode mode, anatomist::ViewState::glSelectRenderMode selectmode )
{
  bool success = false;
  d->glwman->qglWidget()->makeCurrent();

  bool hasTemporary = false;
  for(const auto & obj : objs)
  {
    if(d->window->isTemporary(obj.get()))
    {
      hasTemporary = true;
      break;
    }
  }

  if(mode == RenderMode::Full)
  {
    d->permanentPrimitives.clear();
    d->temporaryPrimitives.clear();
    
    d->currentPrimitives = &d->permanentPrimitives;
    d->glwman->setOutlinedRendering(false);
    setupOpenGLState();
    setupClippingPlanes();

    success |= renderObjects(objs, RenderMode::PermanentOnly, selectmode);

    doubleRender();

    if(hasTemporary)
    {
      d->currentPrimitives = &d->temporaryPrimitives;
      success |= renderObjects(objs, RenderMode::TemporaryOnly, selectmode);
    }
  }
  else if(mode == RenderMode::TemporaryOnly)
  {
    if(hasTemporary)
    {
      d->temporaryPrimitives.clear();

      d->currentPrimitives = &d->temporaryPrimitives;
      setupOpenGLState();
      setupClippingPlanes();

      success |= renderObjects(objs, RenderMode::TemporaryOnly, selectmode);
    }
  }
  else if(mode == RenderMode::Selection)
  {
    d->selectionPrimitives.clear();

    d->currentPrimitives = &d->selectionPrimitives;
    setupClippingPlanes();

    success |= renderObjects(objs, RenderMode::Selection, selectmode);
    d->glwman->setSelectionPrimitives(d->selectionPrimitives);
  }

  finalizeRendering();
  return success;

}


bool RenderContext::renderObjects( const std::list<carto::shared_ptr<AObject>> & objs, RenderMode mode, anatomist::ViewState::glSelectRenderMode selectmode )
{
  bool success = false;
  std::vector<float> bbmin, bbmax;
  DrawableGroups opaqueDrawables, transparentDrawables;
  std::vector<carto::shared_ptr<AObject> > nonDrawables;

  retrieveShaders(objs, opaqueDrawables, transparentDrawables ,nonDrawables);

  if(!opaqueDrawables.empty())
    success |= renderObject(opaqueDrawables, mode, selectmode);

  if(!transparentDrawables.empty())
  {
    if(!d->glwman->useDepthPeeling())
    {
      setupTransparentObjects();
      success |= renderObject(transparentDrawables, mode, selectmode);
      postTransparentRenderingSetup();
    }
    else
    {
      success |= renderObject(transparentDrawables, mode, selectmode);
    }
  }

  if(!nonDrawables.empty())
  {
    for(const auto & obj : nonDrawables)
    {
      success |= updateObject(obj, 0, selectmode);
    }
  }
  return success;
}

bool RenderContext::updateObject(carto::shared_ptr<AObject> obj, PrimList* pl,
                                 ViewState::glSelectRenderMode selectmode)
{
  bool success = false;
  unsigned l1=0, l2;
  if(pl)
    l1 = pl->size();
  else
    l1 = d->currentPrimitives->size();

  GLPrimitives gp;
  if(!pl) pl = d->currentPrimitives;
  bool slice = obj->Is2DObject()
    && ( d->window->viewType() != AWindow3D::ThreeD || !obj->Is3DObject() );
  d->vs = d->window->viewState( slice, selectmode );

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
  
  return success;
}

bool RenderContext::renderObject( DrawableGroups & drawables, RenderMode mode,
                                  ViewState::glSelectRenderMode selectmode )
{
  bool success = false;

  for( const auto & [ key, objects ] : drawables )
  {
    carto::rc_ptr<QOpenGLShaderProgram> program
      = ShaderProgramCache::instance().program( key );
    if( program.isNull() )
      continue;  // build failure already reported by the builder

    switchShaderProgram( program );
    auto shaderModules = ShaderModuleRegistry::resolve( key.modules );
    bool sceneUniformsPushed = false;

    for( const auto & obj : objects )
    {
      if( mode == RenderMode::PermanentOnly && d->window->isTemporary( obj.get() ) )
        continue;
      if( mode == RenderMode::TemporaryOnly && !d->window->isTemporary( obj.get() ) )
        continue;

      if( !sceneUniformsPushed )
      {
        for( const auto & module : shaderModules )
          d->currentPrimitives->push_back( carto::rc_ptr<GLItem>(
            new GLSceneUniforms( module, d->currentProgram, d->glwman ) ) );
        sceneUniformsPushed = true;
      }
      for( const auto & module : shaderModules )
        d->currentPrimitives->push_back( carto::rc_ptr<GLItem>(
          new GLObjectUniforms( module, d->currentProgram, obj.get(),
                                d->objectClipPlanes ) ) );

      success |= updateObject( obj, 0, selectmode );
    }
  }
  d->currentProgram = carto::rc_ptr<QOpenGLShaderProgram>();
  return success;
}

ShaderProgramKey RenderContext::programKey( GLComponent * glObj ) const
{
  ShaderProgramKey key;
  key.pipeline = glObj->renderPipeline();
  key.modules = glObj->shaderModules();
  if( d->glwman->useDepthPeeling() )
    key.modules.insert( ShaderModuleId::DepthPeeling ); // context decision
  key.vertexTemplate   = glObj->glVertexShaderTemplate();
  key.fragmentTemplate = glObj->glFragmentShaderTemplate();
  key.geometryTemplate = glObj->glGeometryShaderTemplate();
  return key;
}

void RenderContext::retrieveShaders(
  const std::list<carto::shared_ptr<AObject> > & objs,
  DrawableGroups & opaqueDrawables, DrawableGroups & transparentDrawables,
  std::vector<carto::shared_ptr<AObject> > & nonDrawables )
{
  for( const auto & obj : objs )
  {
    GLComponent *glObj = obj->glAPI();
    if( !glObj )
    {
      nonDrawables.push_back( obj );
      continue;
    }
    DrawableGroups & groups
      = obj->isTransparent() ? transparentDrawables : opaqueDrawables;
    groups[ programKey( glObj ) ].push_back( obj );
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
  glDepthMask(GL_TRUE);
  glEndList();
  d->currentPrimitives->push_back(RefGLItem(renderpr));
}

void RenderContext::setupClippingPlanes()
{
  Point3df dir = d->window->sliceQuaternion().transformInverse(Point3df(0, 0, -1));
  d->glwman->clipState().plane0[0] = dir[0];
  d->glwman->clipState().plane0[1] = dir[1];
  d->glwman->clipState().plane0[2] = dir[2];
  d->glwman->clipState().plane0[3] = -dir.dot(d->window->getPosition()) + d->window->clipDistance();

  switch(d->window->clipMode())
  {
    case AWindow3D::Single:
      d->glwman->clipState().activePlanes = 1;
      break;
    case AWindow3D::Double:
      d->glwman->clipState().activePlanes = 2;
      d->glwman->clipState().plane1[0] = -dir[0];
      d->glwman->clipState().plane1[1] = -dir[1];
      d->glwman->clipState().plane1[2] = -dir[2];
      d->glwman->clipState().plane1[3] = dir.dot(d->window->getPosition()) + d->window->clipDistance();
      break;
    default:
      d->glwman->clipState().activePlanes = 0;
      break;
  }

  GLuint localGLL = glGenLists(1);
  Primitive *pr = new Primitive;
  if(!localGLL) AWarning("renderContext::setupClippingPlanes: OpenGL error.");

  glNewList(localGLL, GL_COMPILE);
  glDisable(GL_BLEND);
  if(d->glwman->clipState().activePlanes >= 1)
    glEnable(GL_CLIP_DISTANCE0);
  else
    glDisable(GL_CLIP_DISTANCE0);
  if(d->glwman->clipState().activePlanes >= 2)
    glEnable(GL_CLIP_DISTANCE1);
  else
    glDisable(GL_CLIP_DISTANCE1);

  // clip planes always enabled for object clipping
  // but the shader will use the object clip planes only if the object has them
  for( unsigned i = 0; i < 6; ++i ) // MaxClipPlanes : jordan to change with the one in glObjectUniforms
    glEnable( GL_CLIP_DISTANCE2 + i );

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
  d->glwman->setFlatShading(d->window->flatShading());
  d->window->cullingEnabled() ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  if(d->window->smoothing()) // jordan : is it still working like this ?
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
  if (d->window->fog()) // jordan : is it still working like this ? 
  {
    d->glwman->fogParameters().hasFog = true;
    d->glwman->fogParameters().fogDensity = 0.01; 
    d->glwman->fogParameters().fogColor[0] = d->window->light()->Background()[0];
    d->glwman->fogParameters().fogColor[1] = d->window->light()->Background()[1];
    d->glwman->fogParameters().fogColor[2] = d->window->light()->Background()[2];
    d->glwman->fogParameters().fogColor[3] = d->window->light()->Background()[3];
  }
  else
  {
    d->glwman->fogParameters().hasFog = false;
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


void RenderContext::doubleRender()
{
  Primitive* renderoffpr = nullptr;
  bool rendertwice = false;

  switch (d->window->renderingMode())
  {
    case AWindow3D::HiddenWireframe:
      renderoffpr = setupHiddenWireframeMode();
      rendertwice = renderoffpr != nullptr;
      break;
    case Material::Outlined:
      renderoffpr = setupOutlinedMode();
      rendertwice = renderoffpr != nullptr;
      d->glwman->setOutlinedRendering(true);
      break;
    default:
      break;
  }

  if (renderoffpr)
  {
    d->currentPrimitives->push_back(RefGLItem(renderoffpr));
  }

  if (rendertwice)
  {
    duplicateRenderPrimitives();
  }
}

void RenderContext::finalizeRendering()
{
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
  if(d->currentPrimitives->size() < 1)
    return;
  unsigned i, n = d->currentPrimitives->size();
  PrimList::iterator ip = d->currentPrimitives->begin();
  for (ip++, i = 0; i < n; ++i, ++ip)
      d->currentPrimitives->push_back(*ip);
}

void RenderContext::pushObjectClipPlane( const Point4df & plane)
{
  if(d->objectClipPlanes.size() >= 6)// MaxClipPlanes : jordan to change with the one in glObjectUniforms
  {
    AWarning("RenderContext::pushObjectClipPlane: too many nested clip planes, "
              "extra planes are ignored" );
    return;
  }
   d->objectClipPlanes.push_back(plane);
}

void RenderContext::popObjectClipPlane()
{
  if(d->objectClipPlanes.empty())
    AWarning("RenderContext::popObjectClipPlane: no clip plane to pop" );
  else
    d->objectClipPlanes.pop_back();
}

const std::vector<Point4df> & RenderContext::objectClipPlanes() const
{
  return d->objectClipPlanes;
}
