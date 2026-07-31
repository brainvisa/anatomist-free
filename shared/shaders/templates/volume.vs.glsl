uniform vec3 u_bmin;
uniform vec3 u_bmax;

out vec3 v_objectPosRaw;   // object space position
out vec3 v_directionLight;

void main()
{
    v_objectPosRaw = gl_Vertex.xyz;
    v_directionLight = normalize( (inverse(gl_ModelViewMatrix) * vec4(gl_LightSource[0].position.xyz, 0.0)).xyz );

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}