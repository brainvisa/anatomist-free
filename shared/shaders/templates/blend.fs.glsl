varying vec2 v_texCoord;

uniform sampler2D u_layerTexture[16];
uniform int u_numLayers;

out vec4 fragColor;

vec4 blendOver(vec4 front, vec4 back) {
    vec4 result;
    // if front is completely opaque, return it
    if (front.a >= 1.0) {
        return front;
    }
    // if front is completely transparent, return back
    if (front.a <= 0.0) {
        return back;
    }
    
    result.rgb = front.rgb * front.a + back.rgb * back.a * (1.0 - front.a); 
    result.a = front.a + back.a * (1.0 - front.a);
    return result;
}

void main()
{
  vec4 finalColor = vec4(0.0);
  for(int i=0; i<u_numLayers; i++)
  {
    vec4 layerColor;
    // const int a =i;  // does not work because 'i' is not a constant expression
    // layerColor = texture(u_layerTexture[a], v_texCoord); // 'a' needs to be a constant expression

    switch(i) {
            case 0: layerColor = texture(u_layerTexture[0], v_texCoord); break;
            case 1: layerColor = texture(u_layerTexture[1], v_texCoord); break;
            case 2: layerColor = texture(u_layerTexture[2], v_texCoord); break;
            case 3: layerColor = texture(u_layerTexture[3], v_texCoord); break;
            case 4: layerColor = texture(u_layerTexture[4], v_texCoord); break;
            case 5: layerColor = texture(u_layerTexture[5], v_texCoord); break;
            case 6: layerColor = texture(u_layerTexture[6], v_texCoord); break;
            case 7: layerColor = texture(u_layerTexture[7], v_texCoord); break;
            case 8: layerColor = texture(u_layerTexture[8], v_texCoord); break;
            case 9: layerColor = texture(u_layerTexture[9], v_texCoord); break;
            case 10: layerColor = texture(u_layerTexture[10], v_texCoord); break;
            case 11: layerColor = texture(u_layerTexture[11], v_texCoord); break;
            case 12: layerColor = texture(u_layerTexture[12], v_texCoord); break;
            case 13: layerColor = texture(u_layerTexture[13], v_texCoord); break;
            case 14: layerColor = texture(u_layerTexture[14], v_texCoord); break;
            case 15: layerColor = texture(u_layerTexture[15], v_texCoord); break;
        }

    finalColor = blendOver(finalColor, layerColor);

    //if(finalColor.a >= 0.99) break;

  }

  finalColor = vec4(1.0,0.0,0.0,1.0);
  fragColor = finalColor;
}