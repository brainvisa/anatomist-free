#ifndef ANATOMIST_SURFACE_SHADERPROGRAMKEY_H
#define ANATOMIST_SURFACE_SHADERPROGRAMKEY_H

#include <anatomist/surface/shaderTypes.h>
#include <set>
#include <string>
#include <tuple>

namespace anatomist
{
  /**
  * \struct ShaderProgramKey
  * \brief Everything that distinguishes two shader programs.
  *
  * \details
  * Two objects with equal keys share the same program. The key is built by
  * RenderContext::programKey() for each object, and used:
  * - by RenderContext to group objects drawn with the same program,
  * - by ShaderProgramCache to compile each program only once.
  *
  * Any information that changes the generated GLSL code must be part of
  * the key, otherwise two different objects could get the same program.
  */
  struct ShaderProgramKey
  {
    /// Rendering family: surfaces and volumes never share a program.
    RenderPipeline pipeline = RenderPipeline::Surface;

    /// Shader modules of the program. Sorted, so insertion order is irrelevant.
    std::set<ShaderModuleId> modules;

    /// Vertex shader template file name (in shaders/templates).
    std::string vertexTemplate;

    /// Fragment shader template file name (in shaders/templates).
    std::string fragmentTemplate;

    /// Geometry shader template file name; empty if no geometry shader.
    std::string geometryTemplate;

    /**
    * \brief Strict ordering, so that keys can be used in \c std::map.
    *
    * Compares all fields in declaration order.
    */
    bool operator<( const ShaderProgramKey & o ) const
    {
      return std::tie( pipeline, modules, vertexTemplate, fragmentTemplate,
                       geometryTemplate )
           < std::tie( o.pipeline, o.modules, o.vertexTemplate,
                       o.fragmentTemplate, o.geometryTemplate );
    }
  };
}

#endif