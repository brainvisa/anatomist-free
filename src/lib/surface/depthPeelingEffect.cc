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

// tie-break of the fragment kept in this layer : (object, triangle)
layout (location = 1) out uvec2 fragKey;

#define DEPTH_PEELING
  )";
}

std::string DepthPeelingEffect::getFunctionImplementation() const
{
  return R"(
// depth snapped to a 2^-17 grid: triangles that coincide up to rounding
// errors get exactly the same depth, so the tie-break orders them the same
// way whatever the camera. Power of 2: the snapping is exact in float.
float peelDepth()
{
  const float q = 131072.0;
  return floor(gl_FragCoord.z * q) / q;
}


void peeling(vec4 color)
{
  // key grows with submission order, which is also the order in which
  // GL_LESS resolves equal depths within a layer
  fragKey = uvec2(uint(u_drawIndex), uint(gl_PrimitiveID));

  if(color.a < 1.0 / 255.0) discard; // invisible: must not consume a layer
  if(color.a > 0.99) return;         // opaque fragments are never peeled

  if(u_layer > 0)
  {
    ivec2 p = ivec2(gl_FragCoord.xy);
    float prevZ = texelFetch(u_previousDepthTexture, p, 0).r;
    float z = peelDepth(); // snap to a grid to avoid peeling coincident triangles

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
}

