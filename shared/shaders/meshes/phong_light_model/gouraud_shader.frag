uniform sampler2D sampler2d;
uniform sampler1D sampler1d;
uniform int is2dtexture;

varying vec4 diffuseFactor;
varying vec4 interpolatedDiffuseMaterial;

void main(void)
{
  vec4 diffuseMaterial;

  if (gl_TexCoord[0].s != 0. || gl_TexCoord[0].t != 0.)
  {
    if (is2dtexture == 1)
      diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
    else  diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
  }
  else diffuseMaterial = interpolatedDiffuseMaterial;

  vec4 diffuseColor = diffuseFactor * diffuseMaterial;
  gl_FragColor = vec4(gl_Color.rgb + diffuseColor.rgb, gl_FrontMaterial.diffuse.a);
}
