#include <anatomist/surface/depthPeelingEffect.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window3D/window3D.h>

using namespace anatomist;


DepthPeelingEffect::DepthPeelingEffect()
{
  _id="1";
  _isIlluminationModel=false;
  _name="depthPeeling";
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


void DepthPeelingEffect::setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const
{
}

void DepthPeelingEffect::setupSceneUniforms(QOpenGLShaderProgram& program, AWindow3D& scene) const
{
  int previousDepthTextureLocation = program.uniformLocation("u_previousDepthTexture");
  program.setUniformValue(previousDepthTextureLocation, 0 /*nbUnitTexture*/);

  int nbLayerLocation = program.uniformLocation("u_layer");
  program.setUniformValue(nbLayerLocation, 8/* scene.nbLayer*/);

}

