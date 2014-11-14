varying vec3 transformedNormal;
varying vec4 eyeVertexPosition;
varying vec3 modelNormal;
uniform bool normalIsDirection;
//#if __VERSION__ >= 130
// varying float gl_ClipDistance[gl_MaxClipPlanes];
//#endif

void main(void)
{
  // texture
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  // vertex
  eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  // normal
  transformedNormal = gl_NormalMatrix * gl_Normal;
  transformedNormal = normalize(transformedNormal);
  if( normalIsDirection )
  {
    // get a normal in the (light direction, direction) plane
    vec3 directionLight = normalize(gl_LightSource[0].position.xyz);
    vec3 realNormal = normalize(cross(transformedNormal, directionLight));
    transformedNormal = normalize(cross(realNormal, transformedNormal));
  }
  modelNormal = normalize(gl_Normal);

//#if __VERSION__ < 130
  gl_ClipVertex = eyeVertexPosition;
// #else
//   int i;
//   for( i=0; i<gl_MaxClipPlanes; ++i )
//     gl_ClipDistance[i] = dot( gl_ClipPlane[i], eyeVertexPosition );
// #endif

  //color
  gl_FrontColor = gl_Color;
}

