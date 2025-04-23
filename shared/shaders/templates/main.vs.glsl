uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_model;

varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;

void main()
{
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * gl_ModelViewMatrix*gl_Vertex;
    v_normal = gl_Normal;
    v_color = gl_Color;
    v_texcoord = gl_MultiTexCoord0.xy;
}