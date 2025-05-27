varying vec2 v_texCoord;

void main()
{
    gl_Position = gl_Vertex;
    v_texCoord = gl_Position.xy * 0.5 + 0.5; 
}