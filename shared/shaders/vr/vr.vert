varying vec3 frag;

void main( void )
{

  frag = gl_Vertex.xyz;

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  gl_TexCoord[ 0 ] = gl_MultiTexCoord1;
  gl_TexCoord[ 1 ] = gl_Color;
  gl_TexCoord[ 2 ] = gl_Position;

  gl_FrontColor = gl_Color;

}
