varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_texcoord;
varying vec4 v_eyeVertexPosition;
varying vec3 v_directionLight;

#define MAX_TEXTURE_UNITS 8


uniform bool u_hasTexture;

uniform sampler1D u_texture1D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture1D;

uniform sampler2D u_texture2D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture2D;

uniform sampler3D u_texture3D[MAX_TEXTURE_UNITS];
uniform int u_nbTexture3D;

uniform int u_textureType;

{Illumination Model Uniforms}
{Effect Uniforms}

out vec4 fragColor;
 
{Illumination Model Functions}
{Effect Functions}

vec4 basicColor()
{
  vec4 color = v_color;
  if(u_hasTexture)
  {
    switch(u_textureType)
    {
      case 1:
        color.rgb = texture(u_texture1D[0], v_texcoord.x).rgb;
        break;
      case 2:
        color.rgb = texture(u_texture2D[0], v_texcoord.xy).rgb;
        break;
      case 3:
        color.rgb = texture(u_texture3D[0], v_texcoord.xyz).rgb;
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
 {Illumination Model Call}
 {Effect Call}
 fragColor = color;
}
