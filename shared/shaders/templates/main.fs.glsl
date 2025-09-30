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

uniform int u_textureDim;

uniform int u_texEnvMode[MAX_TEXTURE_UNITS];

#define TEXENV_GEOMETRIC 0
#define TEXENV_LINEAR 1
#define TEXENV_REPLACE 2
#define TEXENV_DECAL 3
#define TEXENV_BLEND 4
#define TEXENV_ADD 5
#define TEXENV_COMBINED 6

{Illumination Model Uniforms}
{Effect Uniforms}

out vec4 fragColor;
 
{Illumination Model Functions}
{Effect Functions}


vec4 texEnv(vec4 texColor, vec4 color, int mode)
{
  return texColor * color; // default
  // switch(mode)
  // { 
  //   case TEXENV_GEOMETRIC:
  //     return texColor * color;
  //   case TEXENV_LINEAR:
  //     return texColor * color.a + color * (1.0 - texColor.a);
  //   case TEXENV_REPLACE:
  //     return texColor;
  //   case TEXENV_DECAL:
  //     return texColor * color.a + color * (1.0 - texColor.a);
  //   case TEXENV_BLEND:
  //     return mix(color, texColor, 0.5); // may the blend factor be a uniform ?
  //   case TEXENV_ADD:
  //     return texColor + color;
  //   case TEXENV_COMBINED: // depends on the values of GL_COMBINE_RGB and GL_COMBINE_ALPHA
  //     return color;
  //   default:
  //     return color;
  // }
}

vec4 basicColor()
{
  vec4 color = v_color;
  if(u_hasTexture)
  {
    vec4 texColor;

    switch(u_textureDim)
    {
      case 1:
        for(int i=0; i<u_nbTexture1D; ++i)
        {
          switch(i)
          {
            case 0:
              texColor= texture(u_texture1D[0], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 1:
              texColor= texture(u_texture1D[1], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 2:
              texColor= texture(u_texture1D[2], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 3:
              texColor= texture(u_texture1D[3], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 4:
              texColor= texture(u_texture1D[4], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 5:
              texColor= texture(u_texture1D[5], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 6:
              texColor= texture(u_texture1D[6], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            case 7:
              texColor= texture(u_texture1D[7], v_texcoord.x);
              color = texEnv(texColor, color, 0); 
              break;
            default:
              break;
          }
        }
        break;
      case 2:
        for(int i=0; i<u_nbTexture2D; ++i)
        {
          switch(i)
          {
            case 0:
              texColor = texture(u_texture2D[0], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 1:
              texColor = texture(u_texture2D[1], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 2:
              texColor = texture(u_texture2D[2], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 3:
              texColor = texture(u_texture2D[3], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 4:
              texColor = texture(u_texture2D[4], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 5:
              texColor = texture(u_texture2D[5], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 6:
              texColor = texture(u_texture2D[6], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            case 7:
              texColor = texture(u_texture2D[7], v_texcoord.xy);
              color = texEnv(texColor, color, 0); 
              break;
            default:
              break;
          }
        }
        break;
      case 3:
        for(int i=0; i<u_nbTexture3D; ++i)
        {
          switch(i)
          {
            case 0:
              texColor = texture(u_texture3D[0], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 1:
              texColor = texture(u_texture3D[1], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 2:
              texColor = texture(u_texture3D[2], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 3:
              texColor = texture(u_texture3D[3], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 4:
              texColor = texture(u_texture3D[4], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 5:
              texColor = texture(u_texture3D[5], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 6:
              texColor = texture(u_texture3D[6], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            case 7:
              texColor = texture(u_texture3D[7], v_texcoord.xyz);
              color = texEnv(texColor, color, 0);
              break;
            default:
              break;
          }
        };
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


