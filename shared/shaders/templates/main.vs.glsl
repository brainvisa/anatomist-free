#define MAX_TEXTURE_UNITS 8

varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_texcoord[MAX_TEXTURE_UNITS];
varying vec4 v_eyeVertexPosition;
varying vec3 v_directionLight;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
    v_directionLight = normalize(gl_LightSource[0].position.xyz);
    vec4 normal = vec4(0., 0., 0., 0.);
    normal.xyz = gl_Normal;
    v_normal = normalize((gl_ModelViewMatrix * normal).xyz);
    v_color = gl_Color;
    v_texcoord[0] = gl_MultiTexCoord0.xyz;
    v_texcoord[1] = gl_MultiTexCoord1.xyz;
    v_texcoord[2] = gl_MultiTexCoord2.xyz;
    v_texcoord[3] = gl_MultiTexCoord3.xyz;
    v_texcoord[4] = gl_MultiTexCoord4.xyz;
    v_texcoord[5] = gl_MultiTexCoord5.xyz;
    v_texcoord[6] = gl_MultiTexCoord6.xyz;
    v_texcoord[7] = gl_MultiTexCoord7.xyz;
}
