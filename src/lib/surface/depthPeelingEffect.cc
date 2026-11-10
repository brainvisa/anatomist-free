#include <anatomist/surface/depthPeelingEffect.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window3D/window3D.h>
#include <anatomist/window/glwidgetmanager.h>

using namespace anatomist;

DepthPeelingEffect::DepthPeelingEffect()
{
  _isIlluminationModel=false;
  _name="depthPeeling";
}

std::string DepthPeelingEffect::getUniformDeclarations() const
{
  return R"(
uniform sampler2D u_previousDepthTexture;
uniform usampler2D u_previousKeyTexture;
uniform int u_layer;
uniform int u_drawIndex;
uniform int u_opaqueFirst; // if true, opaque fragments are peeled first, then transparent ones

// tie-break of the fragment kept in this layer : (object, triangle)
layout (location = 1) out uvec2 fragKey;

#define DEPTH_PEELING
  )";
}

std::string DepthPeelingEffect::getFunctionImplementation() const
{
  return R"(
// Depth used by depth peeling. Snapped to a 2^-14 grid, so that surfaces
// coinciding up to rounding errors get the same value, then refined by
// opacity below the grid step: the depth test itself orders coincident
// fragments, most opaque first. Exact in float (14 + 8 fractional bits).
float peelDepth(float alpha)
{
  const float q = 16384.0;                           // 2^14
  float zq = floor(gl_FragCoord.z * q) / q;
  float a = floor(clamp(alpha, 0.0, 1.0) * 255.0 + 0.5); // opaque -> 0
  if(u_opaqueFirst == 1)
    a = 255 - a;                                      // opaque -> 255
  return min(zq + a / (q * 256.0), 1.0);             // offset < one grid step

  return zq;
}


void peeling(vec4 color)
{
  float z = peelDepth(color.a);
  gl_FragDepth = z;   // the depth test sorts on (depth, opacity)

  fragKey = uvec2(uint(u_drawIndex), uint(gl_PrimitiveID));

  if(color.a < 1.0 / 255.0) discard; // invisible: must not consume a layer
  if(color.a > 0.99) return;         // opaque fragments are never peeled

  if(u_layer > 0)
  {
    ivec2 p = ivec2(gl_FragCoord.xy);
    float prevZ = texelFetch(u_previousDepthTexture, p, 0).r;

    if(z < prevZ)
      discard;                       // in front of the previous layer: already peeled
    if(z == prevZ)
    {
      // same depth: peeled only if its key comes after the one kept before
      uvec2 prevKey = texelFetch(u_previousKeyTexture, p, 0).rg;
      if(fragKey.x < prevKey.x
         || (fragKey.x == prevKey.x && fragKey.y <= prevKey.y))
        discard;
    }
  }
}
)";
}

std::string DepthPeelingEffect::getFunctionCall() const
{
  return "peeling(color);";
}


void DepthPeelingEffect::setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const
{
}

void DepthPeelingEffect::setupSceneUniforms(QOpenGLShaderProgram& program, GLWidgetManager& scene) const
{
  int nbLayerLocation = program.uniformLocation("u_layer");
  program.setUniformValue(nbLayerLocation, scene.currentLayer());

  int previousDepthTextureLocation = program.uniformLocation("u_previousDepthTexture");
  program.setUniformValue(previousDepthTextureLocation, scene.depthPeelingUnitTexture());

  int previousKeyTextureLocation = program.uniformLocation("u_previousKeyTexture");
  program.setUniformValue(previousKeyTextureLocation, scene.depthPeelingKeyUnitTexture());

  int opaqueFirstLocation = program.uniformLocation("u_opaqueFirst");
  program.setUniformValue(opaqueFirstLocation, scene.depthPeelingOpaqueFirst() ? 1 : 0);
}

