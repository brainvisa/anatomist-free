#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window3D/window3D.h>

using namespace anatomist;


BlinnPhongIlluminationModel::BlinnPhongIlluminationModel()
{
  _id="0";
  _isIlluminationModel=true;
  _name="BlinnPhong";
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
uniform vec4 u_materialAmbient;
uniform vec4 u_materialDiffuse;
uniform vec4 u_materialSpecular;
uniform float u_materialShininess;
  )";
}

std::string BlinnPhongIlluminationModel::getFunctionImplementation() const
{
  return R"(
vec4 BlinnPhong(vec4 ambient, vec3 fragPos, vec3 normal) {
  vec3 N = normalize(normal);
  vec3 L = normalize(-v_directionLight);
  vec3 V = normalize(v_eyeVertexPosition.xyz - fragPos);
  vec3 H = normalize(L + V);

  // ambient
  vec4 b_ambient = (gl_LightSource[0].ambient + gl_LightModel.ambient) * ambient;

  // diffuse
  float cos_theta = max(dot(N, L), 0.0);
  vec4 diffuse = u_materialDiffuse * gl_LightSource[0].diffuse * cos_theta;

  // specular
  float cos_alpha = pow(max(dot(N, H), 0.0), u_materialShininess);
  vec4 specular = gl_LightSource[0].specular * cos_alpha * u_materialSpecular;

  // final color
  vec4 finalColor = (b_ambient + diffuse + specular);

  return vec4(finalColor.rgb, b_ambient.a);

}
)";

}

std::string BlinnPhongIlluminationModel::getFunctionCall() const
{
  return "BlinnPhong(color, gl_FragCoord.xyz, v_normal);";
}

void BlinnPhongIlluminationModel::setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const
{
  auto material = obj.glMaterial();

  int materialAmbientLocation = program.uniformLocation("u_materialAmbient");
  program.setUniformValue(materialAmbientLocation, material->Ambient(0), material->Ambient(1), material->Ambient(2), material->Ambient(3));

  int materialDiffuseLocation = program.uniformLocation("u_materialDiffuse");
  program.setUniformValue(materialDiffuseLocation, material->Diffuse(0), material->Diffuse(1), material->Diffuse(2), material->Diffuse(3));

  int materialSpecularLocation = program.uniformLocation("u_materialSpecular");
  program.setUniformValue(materialSpecularLocation, material->Specular(0), material->Specular(1), material->Specular(2), material->Specular(3)); 

  int materialShininessLocation = program.uniformLocation("u_materialShininess");
  program.setUniformValue(materialShininessLocation, material->Shininess());
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

