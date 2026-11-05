varying vec2 v_texCoord;

uniform sampler2D u_layerTexture[16];
uniform int u_numLayers;

out vec4 fragColor;

vec4 fetchLayer(int i)
{
  switch(i)
  {
    case 0:  return texture(u_layerTexture[0],  v_texCoord);
    case 1:  return texture(u_layerTexture[1],  v_texCoord);
    case 2:  return texture(u_layerTexture[2],  v_texCoord);
    case 3:  return texture(u_layerTexture[3],  v_texCoord);
    case 4:  return texture(u_layerTexture[4],  v_texCoord);
    case 5:  return texture(u_layerTexture[5],  v_texCoord);
    case 6:  return texture(u_layerTexture[6],  v_texCoord);
    case 7:  return texture(u_layerTexture[7],  v_texCoord);
    case 8:  return texture(u_layerTexture[8],  v_texCoord);
    case 9:  return texture(u_layerTexture[9],  v_texCoord);
    case 10: return texture(u_layerTexture[10], v_texCoord);
    case 11: return texture(u_layerTexture[11], v_texCoord);
    case 12: return texture(u_layerTexture[12], v_texCoord);
    case 13: return texture(u_layerTexture[13], v_texCoord);
    case 14: return texture(u_layerTexture[14], v_texCoord);
    case 15: return texture(u_layerTexture[15], v_texCoord);
  }
  return vec4(0.0);
}

void main()
{
  // Front-to-back compositing. Layers hold straight (non-premultiplied)
  // colours; the accumulator is premultiplied.
  vec4 acc = vec4(0.0);
  int n = min(u_numLayers, 16);
  for(int i = 0; i < n; ++i)
  {
    vec4 layer = fetchLayer(i);
    acc.rgb += (1.0 - acc.a) * layer.a * layer.rgb;
    acc.a   += (1.0 - acc.a) * layer.a;
    if(acc.a >= 0.999)
      break;
  }
  fragColor = acc; // premultiplied
}