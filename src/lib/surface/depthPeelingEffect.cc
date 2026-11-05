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
uniform int u_layer;

#define DEPTH_PEELING
  )";
}

std::string DepthPeelingEffect::getFunctionImplementation() const
{
  return R"(
void peeling(vec4 color)
{
  if(color.a > 0.99) return; // if the color is opaque, no need to peel
  if(u_layer > 0)// perform the peeling
  {
    float previousDepth = texelFetch(u_previousDepthTexture, ivec2(gl_FragCoord.xy), 0).r;
    if(gl_FragCoord.z <= previousDepth)
      discard;
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
}

