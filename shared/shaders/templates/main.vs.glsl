#define MAX_TEXTURE_UNITS 8

uniform int   u_activeClipPlanes;
uniform vec4  u_clipPlane0;
uniform vec4  u_clipPlane1;
uniform vec4  u_clipPlane2;

out VertexData {
    vec4 v_color;
    vec3 v_normal;
    vec3 v_texcoord[MAX_TEXTURE_UNITS];
    vec4 v_eyeVertexPosition;
    vec3 v_directionLight;
};

out float gl_ClipDistance[]; 

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
    v_directionLight = normalize(gl_LightSource[0].position.xyz);
    vec4 normal = vec4(0., 0., 0., 0.);
    normal.xyz = gl_Normal;
    v_normal = normalize((gl_ModelViewMatrix * normal).xyz);
    v_color = gl_Color;
    v_texcoord[0] = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xyz;
    v_texcoord[1] = (gl_TextureMatrix[1] * gl_MultiTexCoord1).xyz;
    v_texcoord[2] = (gl_TextureMatrix[2] * gl_MultiTexCoord2).xyz;
    v_texcoord[3] = (gl_TextureMatrix[3] * gl_MultiTexCoord3).xyz;
    v_texcoord[4] = (gl_TextureMatrix[4] * gl_MultiTexCoord4).xyz;
    v_texcoord[5] = (gl_TextureMatrix[5] * gl_MultiTexCoord5).xyz;
    v_texcoord[6] = (gl_TextureMatrix[6] * gl_MultiTexCoord6).xyz;
    v_texcoord[7] = (gl_TextureMatrix[7] * gl_MultiTexCoord7).xyz;

    gl_ClipDistance[0] = (u_activeClipPlanes >= 1) ? dot(u_clipPlane0, v_eyeVertexPosition) : 1.0;
    gl_ClipDistance[1] = (u_activeClipPlanes >= 2) ? dot(u_clipPlane1, v_eyeVertexPosition) : 1.0;
    gl_ClipDistance[2] = (u_activeClipPlanes >= 3) ? dot(u_clipPlane1, v_eyeVertexPosition) : 1.0; //jordan to change
   
}
