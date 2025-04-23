#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window3D/window3D.h>

using namespace anatomist;


BlinnPhongIlluminationModel::BlinnPhongIlluminationModel()
{
  id="0";
  isIlluminationModel=true;
  name="BlinnPhong";
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

  //return vec4(finalColor, ambiant.a);
  return vec4(1.0, 0.0, 0.0, 1.0);
}
)";

}

std::string BlinnPhongIlluminationModel::getFunctionCall() const
{
  return "BlinnPhong(color, gl_FragCoord.xyz, v_normal);";
}

void BlinnPhongIlluminationModel::setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const
{
  int materialAmbientLocation = program.uniformLocation("u_materialAmbient");
  program.setUniformValue(materialAmbientLocation, 0 /*value*/);

  int materialDiffuseLocation = program.uniformLocation("u_materialDiffuse");
  program.setUniformValue(materialDiffuseLocation, 0/* value*/);

  int materialSpecularLocation = program.uniformLocation("u_materialSpecular");
  program.setUniformValue(materialSpecularLocation, 0 /* value*/);

  int materialShininessLocation = program.uniformLocation("u_materialShininess");
  program.setUniformValue(materialShininessLocation, 0 /*, value*/);
}


void BlinnPhongIlluminationModel::setupSceneUniforms(QOpenGLShaderProgram& program, AWindow3D& scene) const
{
    int lightDirectionLocation = program.uniformLocation("u_lightDirection");
    program.setUniformValue(lightDirectionLocation, 0 /* value*/);

    int lightAmbientLocation = program.uniformLocation("u_lightAmbient");
    program.setUniformValue(lightAmbientLocation, 0 /* value*/);

    int lightDiffuseLocation = program.uniformLocation("u_lightDiffuse");
    program.setUniformValue(lightDiffuseLocation, 0 /* value*/);

    int lightSpecularLocation = program.uniformLocation("u_lightSpecular");
    program.setUniformValue(lightSpecularLocation, 0 /* value*/);

    int lightIntensityLocation = program.uniformLocation("u_lightIntensity");
    program.setUniformValue(lightIntensityLocation, 0 /* value*/);

    int viewPositionLocation = program.uniformLocation("u_viewPosition");
    program.setUniformValue(viewPositionLocation, 0 /* value*/);
}

