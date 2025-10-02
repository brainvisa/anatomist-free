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

// dummy function to avoir uniforms optimization by compiler
vec4 dummy()
{
  vec4 dummy = vec4(0.0);
  for(int i = 0; i < MAX_TEXTURE_UNITS; ++i)
  {
    switch(i)
    {
      case 0:
        dummy += vec4(u_texEnvMode[0]);
        dummy += texture(u_texture1D[0], v_texcoord[0].x);
        dummy += texture(u_texture2D[0], v_texcoord[0].xy);
        dummy += texture(u_texture3D[0], v_texcoord[0].xyz);
        break;
      case 1:
        dummy += vec4(u_texEnvMode[1]);
        dummy += texture(u_texture1D[1], v_texcoord[1].x);
        dummy += texture(u_texture2D[1], v_texcoord[1].xy);
        dummy += texture(u_texture3D[1], v_texcoord[1].xyz);
        break;
      case 2:
        dummy += vec4(u_texEnvMode[2]);
        dummy += texture(u_texture1D[2], v_texcoord[2].x);
        dummy += texture(u_texture2D[2], v_texcoord[2].xy);
        dummy += texture(u_texture3D[2], v_texcoord[2].xyz);
        break;
      case 3:
        dummy += vec4(u_texEnvMode[3]);
        dummy += texture(u_texture1D[3], v_texcoord[3].x);
        dummy += texture(u_texture2D[3], v_texcoord[3].xy);
        dummy += texture(u_texture3D[3], v_texcoord[3].xyz);
        break;
      case 4:
        dummy += vec4(u_texEnvMode[4]);
        dummy += texture(u_texture1D[4], v_texcoord[4].x);
        dummy += texture(u_texture2D[4], v_texcoord[4].xy);
        dummy += texture(u_texture3D[4], v_texcoord[4].xyz);
        break;
      case 5:
        dummy += vec4(u_texEnvMode[5]);
        dummy += texture(u_texture1D[5], v_texcoord[5].x);
        dummy += texture(u_texture2D[5], v_texcoord[5].xy);
        dummy += texture(u_texture3D[5], v_texcoord[5].xyz);
        break;
      case 6:
        dummy += vec4(u_texEnvMode[6]);
        dummy += texture(u_texture1D[6], v_texcoord[6].x);
        dummy += texture(u_texture2D[6], v_texcoord[6].xy);
        dummy += texture(u_texture3D[6], v_texcoord[6].xyz);
        break;
      case 7:
        dummy += vec4(u_texEnvMode[7]);
        dummy += texture(u_texture1D[7], v_texcoord[7].x);
        dummy += texture(u_texture2D[7], v_texcoord[7].xy);
        dummy += texture(u_texture3D[7], v_texcoord[7].xyz);
        break;
      default:
        break;
    }
  }
  return dummy;
}


vec4 texEnv(vec4 texColor, vec4 color, int mode)
{
  switch(mode)
  { 
    case TEXENV_GEOMETRIC:
      return texColor * color;
    case TEXENV_LINEAR:
      return texColor * color.a + color * (1.0 - texColor.a);
    case TEXENV_REPLACE:  
      return texColor;
    case TEXENV_DECAL:
      return texColor * color.a + color * (1.0 - texColor.a);
    case TEXENV_BLEND:
      return mix(color, texColor, 0.5); // may the blend factor be a uniform ?
    case TEXENV_ADD:
      return texColor + color;
    case TEXENV_COMBINED: // depends on the values of GL_COMBINE_RGB and GL_COMBINE_ALPHA
      return color;
    default:
      return color;
  }
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
              texColor= texture(u_texture1D[0], v_texcoord[0].x);
              color = texEnv(texColor, color, u_texEnvMode[0]); 
              break;
            case 1:
              texColor= texture(u_texture1D[1], v_texcoord[1].x);
              color = texEnv(texColor, color, u_texEnvMode[1]); 
              break;
            case 2:
              texColor= texture(u_texture1D[2], v_texcoord[2].x);
              color = texEnv(texColor, color, u_texEnvMode[2]); 
              break;
            case 3:
              texColor= texture(u_texture1D[3], v_texcoord[3].x);
              color = texEnv(texColor, color, u_texEnvMode[3]); 
              break;
            case 4:
              texColor= texture(u_texture1D[4], v_texcoord[4].x);
              color = texEnv(texColor, color, u_texEnvMode[4]); 
              break;
            case 5:
              texColor= texture(u_texture1D[5], v_texcoord[5].x);
              color = texEnv(texColor, color, u_texEnvMode[5]); 
              break;
            case 6:
              texColor= texture(u_texture1D[6], v_texcoord[6].x);
              color = texEnv(texColor, color, u_texEnvMode[6]); 
              break;
            case 7:
              texColor= texture(u_texture1D[7], v_texcoord[7].x);
              color = texEnv(texColor, color, u_texEnvMode[7]); 
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
              texColor = texture(u_texture2D[0], v_texcoord[0].xy);
              color = texEnv(texColor, color, u_texEnvMode[0]); 
              break;
            case 1:
              texColor = texture(u_texture2D[1], v_texcoord[1].xy);
              color = texEnv(texColor, color, u_texEnvMode[1]); 
              break;
            case 2:
              texColor = texture(u_texture2D[2], v_texcoord[2].xy);
              color = texEnv(texColor, color, u_texEnvMode[2]); 
              break;
            case 3:
              texColor = texture(u_texture2D[3], v_texcoord[3].xy);
              color = texEnv(texColor, color, u_texEnvMode[3]); 
              break;
            case 4:
              texColor = texture(u_texture2D[4], v_texcoord[4].xy);
              color = texEnv(texColor, color, u_texEnvMode[4]); 
              break;
            case 5:
              texColor = texture(u_texture2D[5], v_texcoord[5].xy);
              color = texEnv(texColor, color, u_texEnvMode[5]); 
              break;
            case 6:
              texColor = texture(u_texture2D[6], v_texcoord[6].xy);
              color = texEnv(texColor, color, u_texEnvMode[6]); 
              break;
            case 7:
              texColor = texture(u_texture2D[7], v_texcoord[7].xy);
              color = texEnv(texColor, color, u_texEnvMode[7]); 
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
              texColor = texture(u_texture3D[0], v_texcoord[0].xyz);
              color = texEnv(texColor, color, u_texEnvMode[0]);
              break;
            case 1:
              texColor = texture(u_texture3D[1], v_texcoord[1].xyz);
              color = texEnv(texColor, color, u_texEnvMode[1]);
              break;
            case 2:
              texColor = texture(u_texture3D[2], v_texcoord[2].xyz);
              color = texEnv(texColor, color, u_texEnvMode[2]);
              break;
            case 3:
              texColor = texture(u_texture3D[3], v_texcoord[3].xyz);
              color = texEnv(texColor, color, u_texEnvMode[3]);
              break;
            case 4:
              texColor = texture(u_texture3D[4], v_texcoord[4].xyz);
              color = texEnv(texColor, color, u_texEnvMode[4]);
              break;
            case 5:
              texColor = texture(u_texture3D[5], v_texcoord[5].xyz);
              color = texEnv(texColor, color, u_texEnvMode[5]);
              break;
            case 6:
              texColor = texture(u_texture3D[6], v_texcoord[6].xyz);
              color = texEnv(texColor, color, u_texEnvMode[6]);
              break;
            case 7:
              texColor = texture(u_texture3D[7], v_texcoord[7].xyz);
              color = texEnv(texColor, color, u_texEnvMode[7]);
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
  color.r += dummy().r * 0.0001; // to avoid compiler optimisation on unused vars
  return color;
}

void main()
{
 vec4 color = basicColor();
 {Illumination Model Call}
 {Effect Call}
 fragColor = color;
}


