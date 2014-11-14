uniform int coloringModel;

varying vec4 interpolatedDiffuseMaterial;
uniform bool normalIsDirection;
varying float gl_ClipDistance[gl_MaxClipPlanes];

void main(void)
{
  // texture
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  // vertex
  vec4 eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  // normal
  vec3 transformedNormal = gl_NormalMatrix * gl_Normal;
  transformedNormal = normalize(transformedNormal);
  if( normalIsDirection )
  {
    // get a normal in the (light direction, direction) plane
    vec3 directionLight = normalize(gl_LightSource[0].position.xyz);
    vec3 realNormal = normalize(cross(transformedNormal, directionLight));
    transformedNormal = normalize(cross(realNormal, transformedNormal));
  }

  // diffuse
  if (coloringModel == 0)
    interpolatedDiffuseMaterial = gl_FrontMaterial.diffuse;
  else if (coloringModel == 1)
                interpolatedDiffuseMaterial = abs(vec4(gl_Normal, 1));
  else  interpolatedDiffuseMaterial = vec4(1, 0, 1, 1); // should not happend

  //color
  gl_FrontColor = gl_Color;

  int i;
  for( i=0; i<gl_MaxClipPlanes; ++i )
    gl_ClipDistance[i] = dot( gl_ClipPlane[i], eyeVertexPosition );
}

