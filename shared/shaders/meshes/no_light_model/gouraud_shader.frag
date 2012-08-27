uniform sampler1D sampler1d;
uniform sampler2D sampler2d;
uniform int is2dtexture;

varying vec4 interpolatedDiffuseMaterial;

vec4 diffuseMaterial;

void main()
{
	// diffuse
	if (gl_TexCoord[0].s != 0. || gl_TexCoord[0].t != 0.)
	{
		if (is2dtexture == 1)
			diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
		else	diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
	}
	else diffuseMaterial = interpolatedDiffuseMaterial;

	vec4 diffuseColor = diffuseMaterial;

	//final color
	gl_FragColor = vec4(diffuseColor.rgb, gl_FrontMaterial.diffuse.a);
}
