varying vec3 transformedNormal;

void main(void)
{
	// texture
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// normal
	transformedNormal = gl_NormalMatrix * gl_Normal;	
	transformedNormal = normalize(transformedNormal);

	//color
	gl_FrontColor = gl_Color;
}

