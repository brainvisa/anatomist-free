uniform int coloringModel;

varying vec4 interpolatedDiffuseMaterial;

void main(void)
{
	// texture
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// normal
	vec3 transformedNormal = gl_NormalMatrix * gl_Normal;	
	transformedNormal = normalize(transformedNormal);

	// diffuse
	if (coloringModel == 0)
		interpolatedDiffuseMaterial = gl_FrontMaterial.diffuse;
	else if (coloringModel == 1)
		interpolatedDiffuseMaterial = vec4(transformedNormal, 1);
	else	interpolatedDiffuseMaterial = vec4(1, 0, 1, 1); // should not happend

	//color
	gl_FrontColor = gl_Color;
}

