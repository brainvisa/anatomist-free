varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_texcoord;
varying vec4 v_eyeVertexPosition;
varying vec3 v_directionLight;


uniform bool u_hasTexture;
uniform sampler1D u_texture1D;
uniform sampler2D u_texture2D;
uniform sampler3D u_texture3D;
uniform int u_textureType;

out vec4 fragColor;

vec4 basicColor()
{
  vec4 color = vec4(1.0);
  if(u_hasTexture == false)
  {
    if(v_color != vec4(0.0))
    {
      color = v_color;
    }
    else
    {
      color = vec4(0.0, 1.0, 0.0, 1.0);
    }
  }
  else if(u_textureType == 1)
  {
    color = texture(u_texture1D, v_texcoord.x);
  }
  else if(u_textureType == 2)
  {
    color = texture(u_texture2D, v_texcoord.xy);
  }
  else if(u_textureType == 3)
  {
    color = texture(u_texture3D, v_texcoord.xyz);
  }

  return color;
}

void main()
{
 vec4 color = basicColor();
 fragColor = color;
}
