#include <anatomist/surface/blinnPhongIlluminationModel.h>
#include <anatomist/surface/glcomponent.h>
#include <anatomist/window3D/window3D.h>

using namespace anatomist;

BlinnPhongIlluminationModel::BlinnPhongIlluminationModel()
{
  _isIlluminationModel=true;
  _name="BlinnPhong";
}

std::string BlinnPhongIlluminationModel::getUniformDeclarations() const 
{
  return R"(
// uniform vec3 u_lightDirection;
// uniform vec3 u_lightAmbient;
// uniform vec3 u_lightDiffuse;
// uniform vec3 u_lightSpecular;
// uniform float u_lightIntensity;
uniform vec3 u_viewPosition; 
uniform vec4 u_materialAmbient;
uniform vec4 u_materialDiffuse;
uniform vec4 u_materialSpecular;
uniform float u_materialShininess;
uniform bool u_normalIsDirection;
  )";
}

std::string BlinnPhongIlluminationModel::getFunctionImplementation() const
{
  return R"(
  struct BlinnPhongMaterial {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
  };

BlinnPhongMaterial BlinnPhong(vec3 normal)
{
  vec3 lightDirection = normalize(v_directionLight);

  //Local viewer
  //vec3 viewVector = normalize(-v_eyeVertexPosition.xyz);
  //vec3 halfVector = normalize(lightDirection + viewVector);

  // Non local Viewer
  vec3 eyeDirection = vec3(0,0,1);
  vec3 halfVector = normalize(lightDirection + eyeDirection);

  float cos_theta;
  float cos_alpha;

  if(u_normalIsDirection)
  {
    // "normal" actually holds the line direction T: light the line as a
    // thin tube, using the tube normal that maximizes each term.
    vec3 T = normalize(normal);
    float LT = dot(lightDirection, T);
    float HT = dot(halfVector, T);
    cos_theta = sqrt(max(1.0 - LT * LT, 0.0));
    cos_alpha = pow(sqrt(max(1.0 - HT * HT, 0.0)), u_materialShininess);
  }
  else
  {
    cos_theta = abs(dot(normal, lightDirection));
    cos_alpha = pow(max(dot(normal, halfVector), 0.0), u_materialShininess);
  }

  BlinnPhongMaterial material;
  material.ambient = (gl_LightSource[0].ambient + gl_LightModel.ambient) * u_materialAmbient;
  material.diffuse = gl_LightSource[0].diffuse * cos_theta;
  material.specular = gl_LightSource[0].specular * cos_alpha * u_materialSpecular;

  return material; 
}
)";

}

std::string BlinnPhongIlluminationModel::getFunctionCall() const
{
  return R"(
    BlinnPhongMaterial blinnPhong = BlinnPhong(normal);
    vec3 ambientTerm = blinnPhong.ambient.rgb;
    vec3 diffuseTerm = color.rgb * blinnPhong.diffuse.rgb;
    vec3 specularTerm = blinnPhong.specular.rgb;
    color.rgb = ambientTerm + diffuseTerm + specularTerm;
  )";
}

void BlinnPhongIlluminationModel::setupObjectUniforms(QOpenGLShaderProgram& program, GLComponent& obj) const
{
  auto material = obj.glMaterial();


  bool lighting = true;
  bool normalIsDirection = false;

  if( !material )
  {
    int materialAmbientLocation = program.uniformLocation("u_materialAmbient");
    program.setUniformValue(materialAmbientLocation, 1., 1., 1., 1.);

    int materialDiffuseLocation = program.uniformLocation("u_materialDiffuse");
    program.setUniformValue(materialDiffuseLocation, 1.,1., 1., 1.);

    int materialSpecularLocation = program.uniformLocation("u_materialSpecular");
    program.setUniformValue(materialSpecularLocation, 0., 0., 0., 1.);

    int materialShininessLocation = program.uniformLocation("u_materialShininess");
    program.setUniformValue(materialShininessLocation, 20.f);
  }
  else
  {
    int materialAmbientLocation = program.uniformLocation("u_materialAmbient");
    program.setUniformValue(materialAmbientLocation, material->Ambient(0), material->Ambient(1), material->Ambient(2), material->Ambient(3));

    int materialDiffuseLocation = program.uniformLocation("u_materialDiffuse");
    program.setUniformValue(materialDiffuseLocation, material->Diffuse(0), material->Diffuse(1), material->Diffuse(2), material->Diffuse(3));

    int materialSpecularLocation = program.uniformLocation("u_materialSpecular");
    program.setUniformValue(materialSpecularLocation, material->Specular(0), material->Specular(1), material->Specular(2), material->Specular(3));

    int materialShininessLocation = program.uniformLocation("u_materialShininess");
    program.setUniformValue(materialShininessLocation, material->Shininess());

    normalIsDirection = material->renderProperty( Material::NormalIsDirection ) > 0;
  }
  program.setUniformValue(program.uniformLocation("u_normalIsDirection"), normalIsDirection);
}


void BlinnPhongIlluminationModel::setupSceneUniforms(QOpenGLShaderProgram& program, GLWidgetManager& scene) const
{
    // int lightDirectionLocation = program.uniformLocation("u_lightDirection");
    // program.setUniformValue(lightDirectionLocation, 0 /* value*/);

    // int lightAmbientLocation = program.uniformLocation("u_lightAmbient");
    // program.setUniformValue(lightAmbientLocation, 0 /* value*/);

    // int lightDiffuseLocation = program.uniformLocation("u_lightDiffuse");
    // program.setUniformValue(lightDiffuseLocation, 0 /* value*/);

    // int lightSpecularLocation = program.uniformLocation("u_lightSpecular");
    // program.setUniformValue(lightSpecularLocation, 0 /* value*/);

    // int viewPositionLocation = program.uniformLocation("u_viewPosition");
    // program.setUniformValue(viewPositionLocation, 0 /* value*/);
}

