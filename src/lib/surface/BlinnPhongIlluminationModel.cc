#include "BlinnPhongIlluminationModel.h"

BlinnPhongIlluminationModel::BlinnPhongIlluminationModel()
{
}

std::string BlinnPhongIlluminationModel::getUniformDeclarations() const 
{
  return R"(
uniform vec3 u_lightDirection;
uniform vec3 u_lightAmbient;
uniform vec3 u_lightDiffuse;
uniform vec3 u_lightSpecular;
uniform float u_lightIntensity;
uniform vec3 u_viewPosition; 
//uniform vec4 u_materialAmbient; v_Color is used instead
uniform vec3 u_materialDiffuse;
uniform vec3 u_materialSpecular;
uniform float u_materialShininess;
  )";
}

std::string BlinnPhongIlluminationModel::getFunctionImplementation() const
{
  return R"(
vec4 BlinnPhong(vec4 ambiant, vec3 fragPos, vec3 normal) {
  vec3 N = normalize(normal);
  vec3 L = normalize(-u_lightDirection);
  vec3 V = normalize(u_viewPosition - fragPos);
  vec3 H = normalize(L + V);

  // ambiant
  vec3 ambient = u_lightAmbient * ambiant.rgb;

  // diffuse
  float diff = max(dot(N, L), 0.0);
  vec3 diffuse = u_lightDiffuse * (diff * u_materialDiffuse);

  // specular
  float spec = pow(max(dot(N, H), 0.0), u_materialShininess);
  vec3 specular = u_lightSpecular * (spec * u_materialSpecular);

  // final color
  vec3 finalColor = (ambient + diffuse + specular) * u_lightIntensity;

  return vec4(finalColor, ambiant.a);
}
)";

}

std::string BlinnPhongIlluminationModel::getFunctionCall() const
{
  return "BlinnPhong(color, gl_FragCoord.xyz, v_normal);";
}

void BlinnPhongIlluminationModel::setupUniforms(QOpenGLShaderProgram& shaderProgram, Light light, Material material)
{
  shaderProgram.setUniformValue("u_lightDirection", light.direction);
  shaderProgram.setUniformValue("u_lightAmbient", light.ambient);
  shaderProgram.setUniformValue("u_lightDiffuse", light.diffuse);
  shaderProgram.setUniformValue("u_lightSpecular", light.specular);
  shaderProgram.setUniformValue("u_lightIntensity", light.intensity);
  shaderProgram.setUniformValue("u_materialDiffuse", material.diffuse);
  shaderProgram.setUniformValue("u_materialSpecular", material.specular);
  shaderProgram.setUniformValue("u_materialShininess", material.shininess);
}