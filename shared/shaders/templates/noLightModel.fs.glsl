#define MAX_TEXTURE_UNITS 8

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_texcoord[MAX_TEXTURE_UNITS];
varying vec4 v_eyeVertexPosition;
varying vec3 v_directionLight;


uniform bool u_hasTexture;

uniform sampler1D u_texture1D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture1D;

uniform sampler2D u_texture2D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture2D;

uniform sampler3D u_texture3D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture3D;

uniform int u_textureType;

out vec4 fragColor;

vec4 basicColor()
{
  vec4 color = v_color;
  if(u_hasTexture)
  {
    switch(u_textureType)
    {
      case 1:
        color *= texture(u_texture1D[0], v_texcoord[0].x);
        break;
      case 2:
        color *= texture(u_texture2D[0], v_texcoord[0].xy);
        break;
      case 3:
        color *= texture(u_texture3D[0], v_texcoord[0].xyz);
        break;
      default:
        break;
    }
  }
  return color;
}

void main()
{
 vec4 color = basicColor();
 fragColor = color;
}
