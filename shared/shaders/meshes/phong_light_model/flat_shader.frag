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
uniform int is2dtexture;
uniform int coloringModel;

varying vec4 eyeVertexPosition;
varying vec4 vertexPosition;

vec3 eyeDirection;
vec4 diffuseMaterial;
uniform bool normalIsDirection;

void main()
{
  // normal
  vec3 normal = normalize(cross(dFdx(eyeVertexPosition.xyz),
                          dFdy(eyeVertexPosition.xyz)));

  // ambient
  vec4 ambientColor = (gl_LightSource[0].ambient + gl_LightModel.ambient) * gl_FrontMaterial.ambient;

  // diffuse
  vec3 directionLight = normalize(gl_LightSource[0].position.xyz);
  if( normalIsDirection )
  {
    // get a normal in the (light direction, direction) plane
//           normal = normalize(dFdx(eyeVertexPosition.xyz) + dFdy(eyeVertexPosition.xyz));
    // FIXME: I cannot do better for now...
    normal = vec3( 1, 1, 1 );
  }
  float cos_theta = max(dot(normal, directionLight), 0.0);
  if (gl_TexCoord[0].s == 0. && gl_TexCoord[0].t == 0.)
  {
    if (coloringModel == 0)
            diffuseMaterial = gl_FrontMaterial.diffuse;
    else if (coloringModel == 1)
    {
      vec3 modelNormal;
      if( normalIsDirection )
      {
        // get a normal in the (light direction, direction) plane
        modelNormal = normalize(dFdx(vertexPosition.xyz)
          + dFdy(vertexPosition.xyz));
      }
      else
        modelNormal =
          normalize(cross(dFdx(vertexPosition.xyz),
                    dFdy(vertexPosition.xyz)));
      diffuseMaterial = abs(vec4(modelNormal, 1));
    }
    // should not happen
    else diffuseMaterial = vec4(1, 0, 1, 1);
  }
  else
  {
    if (is2dtexture == 1)
      diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
    else
      diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
  }
  vec4 diffuseColor = diffuseMaterial * gl_LightSource[0].diffuse * cos_theta;

  // specular
  //if (local_viewer)
  //{
          // anatomist local viewer behaviour
  //	eyeDirection = normalize(-eyeVertexPosition.xyz);
  //} else {
  // anatomist non local viewer behaviour (default)
  eyeDirection = vec3(0, 0, 1);
  //}
  //XXX : add this mode
  // my suggestion (use this trick because the light is inside the object)
  //eyeDirection = normalize(eyeVertexPosition.xyz);
  vec3 reflectionDirection = reflect(directionLight, normal);
  float cos_alpha = max(dot(reflectionDirection, -eyeDirection), 0.0);

  float specularFactor = pow(cos_alpha, gl_FrontMaterial.shininess);
  vec4 specularColor = gl_LightSource[0].specular * gl_FrontMaterial.specular * specularFactor;

  //final color
  gl_FragColor = vec4(ambientColor.rgb + diffuseColor.rgb + specularColor.rgb, gl_FrontMaterial.diffuse.a);
}
