varying vec4 eyeVertexPosition;
varying vec4 vertexPosition;
varying float gl_ClipDistance[gl_MaxClipPlanes];

void main(void)
{
  // texture
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  // vertex
  eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
  vertexPosition = gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  //color
  gl_FrontColor = gl_Color;

  int i;
  for( i=0; i<gl_MaxClipPlanes; ++i )
    gl_ClipDistance[i] = dot( gl_ClipPlane[i], eyeVertexPosition );
}

