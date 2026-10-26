#ifndef ANATOMIST_SURFACE_SHADERTYPES_H
#define ANATOMIST_SURFACE_SHADERTYPES_H

/**
* \file shaderTypes.h
* \brief Identifiers shared by the shader pipeline.
*
* This header has no dependency, so that it can be included anywhere
* (GLComponent, shader modules, RenderContext) at no cost.
*/

namespace anatomist
{
  /**
  * \enum ShaderModuleId
  * \brief Identifies a shader module (IShaderModule).
  *
  * \details
  * Each module class returns its own value from IShaderModule::id().
  * To add a module:
  * - add a value here,
  * - implement \c id() in the module class,
  * - register the module in ShaderModuleRegistry.
  */
  enum class ShaderModuleId
  {
    BlinnPhong,   ///< Blinn-Phong illumination model
    DepthPeeling  ///< Depth peeling effect, added by RenderContext when enabled in the window
  };

  /**
  * \enum RenderPipeline
  * \brief Rendering family of an object.
  *
  * Objects of different pipelines never share a shader program, even when
  * they use the same shader modules: their shaders work differently (e.g.
  * surface rasterization vs volume raymarching, which handles clip planes
  * in the fragment shader).
  */
  enum class RenderPipeline
  {
    Surface, ///< Rasterized geometry (meshes, lines, points...)
    Volume   ///< Volume raymarching (VObject)
  };
}

#endif