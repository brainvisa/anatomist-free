varying vec4 v_color;
varying vec3 v_normal;
varying vec3 v_texcoord;
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
    v_color = gl_Color; // jordan gl_Color=Ambient(light+material)​+Diffuse(light+material)​+Emission+Specular but we want only ambient
    v_texcoord = gl_MultiTexCoord0.xyz;
} 
