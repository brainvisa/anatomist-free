varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;
varying vec4 v_eyeVertexPosition;
varying vec3 v_directionLight;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
    v_directionLight = normalize(gl_LightSource[0].position.xyz);
    v_normal = normalize(gl_Normal);
    v_color = gl_Color;
    v_texcoord = gl_MultiTexCoord0.xy;
} 