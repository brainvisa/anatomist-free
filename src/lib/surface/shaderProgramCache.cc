#include <anatomist/surface/shaderProgramCache.h>
#include <anatomist/surface/shaderModuleRegistry.h>
#include <anatomist/misc/error.h>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>

using namespace anatomist;

ShaderProgramCache & ShaderProgramCache::instance()
{
  // deliberately never destroyed: an exit-time destructor would delete
  // GL programs after Qt and the contexts are gone. Use clear() instead.
  static ShaderProgramCache *cache = new ShaderProgramCache;
  return *cache;
}

ShaderProgramCache::GroupPrograms * ShaderProgramCache::currentGroup()
{
  QOpenGLContext *ctx = QOpenGLContext::currentContext();
  if( !ctx )
  {
    AWarning( "ShaderProgramCache: no current OpenGL context" );
    return nullptr;
  }

  QOpenGLContextGroup *group = ctx->shareGroup();
  auto it = _groups.find( group );
  if( it == _groups.end() )
  {
    it = _groups.emplace( group, GroupPrograms() ).first;
    // forget the group's programs when Qt destroys the group
    QObject::connect( group, &QObject::destroyed,
                      [ this, group ]{ _groups.erase( group ); } );
  }
  return &it->second;
}

carto::rc_ptr<QOpenGLShaderProgram>
ShaderProgramCache::program( const ShaderProgramKey & key )
{
  GroupPrograms *g = currentGroup();
  if( !g )
    return carto::rc_ptr<QOpenGLShaderProgram>();

  auto it = g->programs.find( key );
  if( it != g->programs.end() )
    return it->second;  // cache hit, including a previous failure (null)

  carto::rc_ptr<QOpenGLShaderProgram> prog = _builder.initShader(
    ShaderModuleRegistry::resolve( key.modules ),
    key.vertexTemplate, key.fragmentTemplate, key.geometryTemplate );
  g->programs[ key ] = prog;  // stored even if null: no rebuild every frame
  return prog;
}

carto::rc_ptr<QOpenGLShaderProgram> ShaderProgramCache::blendProgram()
{
  GroupPrograms *g = currentGroup();
  if( !g )
    return carto::rc_ptr<QOpenGLShaderProgram>();

  if( !g->blendBuilt )
  {
    g->blend = _builder.initBlendingShader();
    g->blendBuilt = true;
  }
  return g->blend;
}

void ShaderProgramCache::clear()
{
  _groups.clear();
}