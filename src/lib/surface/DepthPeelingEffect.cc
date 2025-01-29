#include "DepthPeelingEffect.h"

DepthPeelingEffect::DepthPeelingEffect()
{
}

std::string DepthPeelingEffect::getUniformDeclarations() const
{
  return R"(
uniform sampler2D u_previousDepthTexture;
uniform int u_layer;
  )";
}

std::string DepthPeelingEffect::getFunctionImplementation() const
{
  return R"(
vec2 getTexCoord()
{
    return gl_FragCoord.xy / textureSize(u_previousDepthTexture, 0);
}

void peeling(vec4 color)
{
  if(u_layer != 0)// perform the peeling
  {
    vec2 texCoord = getTexCoord();
    vec4 previousDepth = texture(u_previousDepthTexture, texCoord);
    float epsilon = 0.0000001;
    if(gl_FragCoord.z <= previousDepth.r + epsilon)
    {
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

void DepthPeelingEffect::setupUniforms(QOpenGLShaderProgram& shaderProgram, int layer)
{
  shaderProgram.setUniformValue("u_previousDepthTexture", 0);
  shaderProgram.setUniformValue("u_layer", layer);
}
