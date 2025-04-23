varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;

uniform bool u_hasTexture;
uniform sampler1D u_texture1D;
uniform sampler2D u_texture2D;
uniform int u_textureType;
{Illumination Model Uniforms}
{Effect Uniforms}

out vec4 fragColor;

{Illumination Model Functions}
{Effect Functions}

vec4 basicColor()
{
  vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
  if(u_hasTexture == false)
  {
    if(v_color != vec4(0.0, 0.0, 0.0, 0.0))
    {
      color = v_color;
    }
    else
    {
      color = vec4(0.0, 1.0, 0.0, 1.0);
    }
  }
  else if(u_textureType == 0)
  {
    color = texture(u_texture1D, v_texcoord.x);
  }
  else
  {
    color = texture(u_texture2D, v_texcoord);
  }
  
  return color;
}

void main()
{
 vec4 color = v_color; //basicColor();
 color = {Illumination Model Call}
 {Effect Call}
 fragColor = color;
}
