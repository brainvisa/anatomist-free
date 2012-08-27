varying vec4 eyeVertexPosition;


void main(void)
{
	// texture
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// vertex
	eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	//color
	gl_FrontColor = gl_Color;
}

