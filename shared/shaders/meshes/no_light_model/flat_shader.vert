varying vec4 vertexPosition;


void main(void)
{
  // texture
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  // vertex
  vertexPosition = gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  //color
  gl_FrontColor = gl_Color;
}

