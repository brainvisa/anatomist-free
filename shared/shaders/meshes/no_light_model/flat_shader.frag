uniform sampler1D sampler1d;
uniform sampler2D sampler2d;
uniform int is2dtexture;
uniform int coloringModel;

varying vec4 vertexPosition;

vec4 diffuseMaterial;
uniform bool normalIsDirection;

void main()
{
  // diffuse
  if (gl_TexCoord[0].s == 0. && gl_TexCoord[0].t == 0.)
  {
    if (coloringModel == 0)
      diffuseMaterial = gl_FrontMaterial.diffuse;
    else if (coloringModel == 1)
    {
      vec3 modelNormal;
      if( normalIsDirection )
      {
        // get a normal in the (light direction, direction) plane
        modelNormal = normalize(dFdx(vertexPosition.xyz)
          + dFdy(vertexPosition.xyz));
      }
      else
        modelNormal =
          normalize(cross(dFdx(vertexPosition.xyz),
                    dFdy(vertexPosition.xyz)));
      diffuseMaterial = abs(vec4(modelNormal, 1));
    }
    // should not happen
    else diffuseMaterial = vec4(1, 0, 1, 1);
  }
  else
  {
    if (is2dtexture == 1)
      diffuseMaterial = texture2D(sampler2d, gl_TexCoord[0].st);
    else
      diffuseMaterial = texture1D(sampler1d, gl_TexCoord[0].s);
  }
  vec4 diffuseColor = diffuseMaterial;

  //final color
  gl_FragColor = vec4(diffuseColor.rgb, gl_FrontMaterial.diffuse.a);
}
