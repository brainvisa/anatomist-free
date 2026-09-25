#define MAX_OBJECT_CLIP_PLANES 6 //jordan to change with the one in renderContext and globjectuniforms

uniform vec3 u_bmin;
uniform vec3 u_bmax;

out vec3 v_objectPosRaw;   // object space position
out vec3 v_directionLight;

out float gl_ClipDistance[2 + MAX_OBJECT_CLIP_PLANES];

void main()
{
    v_objectPosRaw = gl_Vertex.xyz;
    v_directionLight = normalize( (inverse(gl_ModelViewMatrix) * vec4(gl_LightSource[0].position.xyz, 0.0)).xyz );

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    for (int i = 0; i < MAX_OBJECT_CLIP_PLANES; ++i)
    {
        gl_ClipDistance[i] = 1.0;
    }
}