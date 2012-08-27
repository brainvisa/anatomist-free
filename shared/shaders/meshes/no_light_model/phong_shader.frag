uniform sampler1D sampler1d;
uniform sampler2D sampler2d;
uniform int is2dtexture;
uniform int coloringModel;

varying vec3 transformedNormal;

vec4 diffuseMaterial;

void main()
{
	// normal
	vec3 normal = normalize(transformedNormal);

	// diffuse
	if (gl_TexCoord[0].s == 0. && gl_TexCoord[0].t == 0.)
	{
		if (coloringModel == 0)
			diffuseMaterial = gl_FrontMaterial.diffuse;
		else if (coloringModel == 1)
			diffuseMaterial = vec4(normal, 1);
		// should not happend
		else diffuseMaterial = vec4(1, 0, 1, 1);
	}
	else
	{
		if (is2dtexture == 1)
			diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
		else	diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
	}
	vec4 diffuseColor = diffuseMaterial;

	//final color
	gl_FragColor = vec4(diffuseColor.rgb, gl_FrontMaterial.diffuse.a);
}
