// struct gl_LightSourceParameters
// {
//    vec4 ambient;              // Aclarri
//    vec4 diffuse;              // Dcli
//    vec4 specular;             // Scli
//    vec4 position;             // Ppli
//    vec4 halfVector;           // Derived: Hi
//    vec3 spotDirection;        // Sdli
//    float spotExponent;        // Srli
//    float spotCutoff;          // Crli
//                               // (range: [0.0,90.0], 180.0)
//    float spotCosCutoff;       // Derived: cos(Crli)
//                               // (range: [1.0,0.0],-1.0)
//    float constantAttenuation; // K0
//    float linearAttenuation;   // K1
//    float quadraticAttenuation;// K2
// };
//
// struct gl_MaterialParameters
// {
//    vec4 emission;    // Ecm
//    vec4 ambient;     // Acm
//    vec4 diffuse;     // Dcm
//    vec4 specular;    // Scm
//    float shininess;  // Srm
// };
//
// uniform gl_MaterialParameters gl_FrontMaterial;
// uniform gl_MaterialParameters gl_BackMaterial;
// uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];
uniform sampler1D sampler1d;
uniform sampler2D sampler2d;
uniform bool hasTexture;
uniform int is2dtexture;
uniform int coloringModel;

varying vec3 transformedNormal;
varying vec4 eyeVertexPosition;
varying vec3 modelNormal;

vec3 eyeDirection;
vec4 diffuseMaterial;

void main()
{
  // ------------------------------------- normal -------------------------------------
  vec3 normal = normalize(transformedNormal);

  // ------------------------------------- Ambient -------------------------------------
  vec4 ambientColor = (gl_LightSource[0].ambient + gl_LightModel.ambient) * gl_FrontMaterial.ambient;

  // ------------------------------------- Diffuse -------------------------------------
  vec3 directionLight = normalize(gl_LightSource[0].position.xyz);
  float cos_theta = max(dot(normal, directionLight), 0.0);
  if (!hasTexture)
  {
    if (coloringModel == 0)
    {
      diffuseMaterial = gl_FrontMaterial.diffuse;
    }
    else if (coloringModel == 1)
    {
      diffuseMaterial = abs(vec4(modelNormal, 1));
    }
    else     // should not happend
    {
      diffuseMaterial = vec4(1, 0, 1, 1);
    }
  }
  else
  {
    if (is2dtexture == 1)
    {
      diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
    }
    else
    {
      diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
    }
  }
  vec4 diffuseColor = diffuseMaterial * gl_LightSource[0].diffuse * cos_theta;

  //------------------------------------- Specular (Blinn-Phong Model) -------------------------------------
  //if (local_viewer)
  //{
    // anatomist local viewer behaviour
  //  eyeDirection = normalize(-eyeVertexPosition.xyz);
  //} else {
  // anatomist non local viewer behaviour (default)
  eyeDirection = vec3(0, 0, 1);
  //}
  //XXX : add this mode
  // my suggestion (use this trick because the light is inside the object)
  //eyeDirection = normalize(eyeVertexPosition.xyz);
  vec3 half_vector = directionLight + eyeDirection;
  half_vector = normalize(half_vector);
  float cos_alpha = max(dot(half_vector, normal), 0.0);
  float specularFactor = pow(cos_alpha, gl_FrontMaterial.shininess);
  vec4 specularColor = gl_LightSource[0].specular * gl_FrontMaterial.specular * specularFactor;

  // ------------------------------------- Final color -------------------------------------

  gl_FragColor = vec4(ambientColor.rgb + diffuseColor.rgb + specularColor.rgb, gl_FrontMaterial.diffuse.a);
}
