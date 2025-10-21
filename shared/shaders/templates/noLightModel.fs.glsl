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

uniform int u_textureDim[MAX_TEXTURE_UNITS * 3];

uniform int u_texEnvMode[MAX_TEXTURE_UNITS * 3];

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

struct textureCounter
{
  int index1D;
  int index2D;
  int index3D;
};

// dummy function to avoir uniforms optimization by compiler
vec4 dummy()
{
  vec4 dummy = vec4(0.0);
  for(int i = 0; i < MAX_TEXTURE_UNITS; ++i)
  {
    dummy += vec4(u_texEnvMode[i]);
    switch(i)
    {
      case 0:
        dummy += texture(u_texture1D[0], v_texcoord[0].x);
        dummy += texture(u_texture2D[0], v_texcoord[0].xy);
        dummy += texture(u_texture3D[0], v_texcoord[0].xyz);
        break;
      case 1:
        dummy += texture(u_texture1D[1], v_texcoord[1].x);
        dummy += texture(u_texture2D[1], v_texcoord[1].xy);
        dummy += texture(u_texture3D[1], v_texcoord[1].xyz);
        break;
      case 2:
        dummy += texture(u_texture1D[2], v_texcoord[2].x);
        dummy += texture(u_texture2D[2], v_texcoord[2].xy);
        dummy += texture(u_texture3D[2], v_texcoord[2].xyz);
        break;
      case 3:
        dummy += texture(u_texture1D[3], v_texcoord[3].x);
        dummy += texture(u_texture2D[3], v_texcoord[3].xy);
        dummy += texture(u_texture3D[3], v_texcoord[3].xyz);
        break;
      case 4:
        dummy += texture(u_texture1D[4], v_texcoord[4].x);
        dummy += texture(u_texture2D[4], v_texcoord[4].xy);
        dummy += texture(u_texture3D[4], v_texcoord[4].xyz);
        break;
      case 5:
        dummy += texture(u_texture1D[5], v_texcoord[5].x);
        dummy += texture(u_texture2D[5], v_texcoord[5].xy);
        dummy += texture(u_texture3D[5], v_texcoord[5].xyz);
        break;
      case 6:
        dummy += texture(u_texture1D[6], v_texcoord[6].x);
        dummy += texture(u_texture2D[6], v_texcoord[6].xy);
        dummy += texture(u_texture3D[6], v_texcoord[6].xyz);
        break;
      case 7:
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
      return texColor * (1 - color); //return color * (1.0 - texColor) + u_texEnvColor * texColor; BUT WE NEED u_texEnvColor
    case TEXENV_ADD:
      return texColor + color;
    case TEXENV_COMBINED: // depends on the values of GL_COMBINE_RGB and GL_COMBINE_ALPHA. default is same as MODULATE
      return texColor * color;
    default:
      return color;
  }
}

void incrementCounter(int dim, inout textureCounter counter)
{
  if (dim == 1)
  {
    counter.index1D += 1;
  }
  else if (dim == 2)
  {
    counter.index2D += 1;
  }
  else if (dim == 3)
  {
    counter.index3D += 1;
  }
}

vec4 getTextureColor(int i, inout textureCounter counter)
{
  vec4 texColor = vec4(1.0);
  int dim = u_textureDim[i];

  if (dim == 1)
  {
    switch (counter.index1D)
    {
      case 0: texColor = texture(u_texture1D[0], v_texcoord[i].x); break;
      case 1: texColor = texture(u_texture1D[1], v_texcoord[i].x); break;
      case 2: texColor = texture(u_texture1D[2], v_texcoord[i].x); break;
      case 3: texColor = texture(u_texture1D[3], v_texcoord[i].x); break;
      case 4: texColor = texture(u_texture1D[4], v_texcoord[i].x); break;
      case 5: texColor = texture(u_texture1D[5], v_texcoord[i].x); break;
      case 6: texColor = texture(u_texture1D[6], v_texcoord[i].x); break;
      case 7: texColor = texture(u_texture1D[7], v_texcoord[i].x); break;
    }
  }
  else if (dim == 2)
  {
    switch (counter.index2D)
    {
      case 0: texColor = texture(u_texture2D[0], v_texcoord[i].xy); break;
      case 1: texColor = texture(u_texture2D[1], v_texcoord[i].xy); break;
      case 2: texColor = texture(u_texture2D[2], v_texcoord[i].xy); break;
      case 3: texColor = texture(u_texture2D[3], v_texcoord[i].xy); break;
      case 4: texColor = texture(u_texture2D[4], v_texcoord[i].xy); break;
      case 5: texColor = texture(u_texture2D[5], v_texcoord[i].xy); break;
      case 6: texColor = texture(u_texture2D[6], v_texcoord[i].xy); break;
      case 7: texColor = texture(u_texture2D[7], v_texcoord[i].xy); break;
    }
  }
  else if (dim == 3)
  {
    switch (counter.index3D)
    {
      case 0: texColor = texture(u_texture3D[0], v_texcoord[i].xyz); break;
      case 1: texColor = texture(u_texture3D[1], v_texcoord[i].xyz); break;
      case 2: texColor = texture(u_texture3D[2], v_texcoord[i].xyz); break;
      case 3: texColor = texture(u_texture3D[3], v_texcoord[i].xyz); break;
      case 4: texColor = texture(u_texture3D[4], v_texcoord[i].xyz); break;
      case 5: texColor = texture(u_texture3D[5], v_texcoord[i].xyz); break;
      case 6: texColor = texture(u_texture3D[6], v_texcoord[i].xyz); break;
      case 7: texColor = texture(u_texture3D[7], v_texcoord[i].xyz); break;
    }
  }
  incrementCounter(dim, counter);
  return texColor;
}


vec4 basicColor()
{
  vec4 color = v_color;

  if(u_hasTexture)
  {
    vec4 texColor;
    textureCounter counter=textureCounter(0,0,0);
    int currentTexture = 0;
    int nbTexture = u_nbTexture1D + u_nbTexture2D + u_nbTexture3D;

    for(int i=0; i< nbTexture; ++i)
    {
      texColor = getTextureColor(i, counter);
      color = texEnv(texColor, color, u_texEnvMode[i]);
    }
  }
  color.r += dummy().r * 0.0001; // to avoid compiler optimisation on unused vars
  return color;
}

void main()
{
 vec4 color = basicColor();
 fragColor = color;
}
