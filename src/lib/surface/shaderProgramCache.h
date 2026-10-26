#ifndef ANATOMIST_SURFACE_SHADERPROGRAMCACHE_H
#define ANATOMIST_SURFACE_SHADERPROGRAMCACHE_H

#include <anatomist/surface/shaderProgramKey.h>
#include <anatomist/surface/dynamicShaderBuilder.h>
#include <cartobase/smart/rcptr.h>
#include <map>

class QOpenGLShaderProgram;
class QOpenGLContextGroup;

namespace anatomist
{
  /**
  * \class ShaderProgramCache
  * \brief Application-wide cache of compiled GLSL programs.
  *
  * \details
  * Each program is compiled once, the first time its ShaderProgramKey is
  * requested, then reused by every object and every window that needs it.
  *
  * Programs are stored per OpenGL share group
  * (\c QOpenGLContext::shareGroup()). All Anatomist windows share the
  * global context created by GLWidgetManager::sharedWidget(), so in
  * practice there is a single group. A window that would not share its
  * context still gets its own valid programs, instead of programs created
  * in another context.
  *
  * Ownership:
  * - the cache owns all programs and the dynamicShaderBuilder used to
  *   compile them,
  * - RenderContext and GLWidgetManager only hold programs temporarily,
  *   during rendering.
  *
  * \warning All methods must be called with an OpenGL context current.
  * \warning The uniform state of a program is shared between all windows:
  *          every draw must set all the uniforms it uses before drawing.
  */
  class ShaderProgramCache
  {
  public:
    /**
    * \brief Returns the unique instance.
    *
    * The instance is never destroyed: an exit-time destructor would delete
    * GL programs after Qt and the OpenGL contexts are gone. Use clear() to
    * release the programs explicitly.
    */
    static ShaderProgramCache & instance();

    /**
    * \brief Returns the program matching a key, compiled on first request.
    *
    * Failures are remembered: a program that failed to compile or link is
    * not rebuilt on later calls, so the error is reported only once.
    *
    * \param key Description of the requested program.
    * \return The program, or null if compilation or link failed, or if no
    *         OpenGL context is current.
    */
    carto::rc_ptr<QOpenGLShaderProgram> program( const ShaderProgramKey & key );

    /**
    * \brief Returns the depth peeling blending program, compiled on first
    *        request.
    *
    * \return The program, or null if compilation or link failed, or if no
    *         OpenGL context is current.
    */
    carto::rc_ptr<QOpenGLShaderProgram> blendProgram();

    /**
    * \brief Releases all programs of all share groups.
    *
    * \warning Must be called with a context of the share group current,
    *          e.g. after GLWidgetManager::sharedWidget()->makeCurrent(),
    *          typically at application shutdown.
    */
    void clear();

  private:
    ShaderProgramCache() = default;

    /**
    * \struct GroupPrograms
    * \brief Programs of one OpenGL share group.
    */
    struct GroupPrograms
    {
      /// Compiled programs by key; a null program records a build failure.
      std::map<ShaderProgramKey, carto::rc_ptr<QOpenGLShaderProgram> > programs;
      /// Depth peeling blending program.
      carto::rc_ptr<QOpenGLShaderProgram> blend;
      /// True once the blending program has been built (even if it failed).
      bool blendBuilt = false;
    };

    /**
    * \brief Returns the programs of the current context share group,
    *        created on first use.
    *
    * The group entry is removed automatically when Qt destroys the group.
    *
    * \return The group programs, or null if no OpenGL context is current.
    */
    GroupPrograms * currentGroup();

    /// Programs of each share group.
    std::map<QOpenGLContextGroup *, GroupPrograms> _groups;
    /// Builder used to compile programs on cache misses.
    dynamicShaderBuilder _builder;
  };
}

#endif