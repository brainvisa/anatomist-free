uniform sampler3D u_texture3D[8];
uniform vec3 u_bmin;
uniform vec3 u_bmax;
uniform sampler3D u_debugTex;
uniform float u_volumeMax;

in vec3 v_objectPosRaw;
in vec3 v_directionLight;

out vec4 fragColor;


bool intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 bmin, vec3 bmax,
                    out float tNear, out float tFar)
{
    vec3 invDir = 1.0 / rayDir;
    vec3 t0 = (bmin - rayOrigin) * invDir;
    vec3 t1 = (bmax - rayOrigin) * invDir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    tNear = max(max(tmin.x, tmin.y), tmin.z);
    tFar  = min(min(tmax.x, tmax.y), tmax.z);
    return tNear <= tFar && tFar > 0.0;
}

vec3 computeGradient(vec3 pos)
{
    float eps = 0.003;
    float dx = texture(u_texture3D[0], pos + vec3(eps,0,0)).r
             - texture(u_texture3D[0], pos - vec3(eps,0,0)).r;
    float dy = texture(u_texture3D[0], pos + vec3(0,eps,0)).r
             - texture(u_texture3D[0], pos - vec3(0,eps,0)).r;
    float dz = texture(u_texture3D[0], pos + vec3(0,0,eps)).r
             - texture(u_texture3D[0], pos - vec3(0,0,eps)).r;
    return vec3(dx, dy, dz);
}

void main()
{
    vec3 rayDir = normalize( (inverse(gl_ModelViewMatrix) * vec4(0.0, 0.0, -1.0, 0.0)).xyz );
    vec3 rayOrigin = v_objectPosRaw;

    float tNear, tFar;
    if( !intersectAABB(rayOrigin, rayDir, u_bmin, u_bmax, tNear, tFar) )
        discard;
    tNear = max(tNear, 0.0);

    vec3 entryPointObj = rayOrigin + rayDir * tNear;
    vec3 pos = (entryPointObj - u_bmin) / (u_bmax - u_bmin);
    vec3 rayDirTex = rayDir / (u_bmax - u_bmin);
    float stepSizeMM = 0.3;
    vec3 stepTex = rayDirTex * stepSizeMM;

    vec4 accum = vec4(0.0);
    for( int i = 0; i < 1200; ++i )
    {
        if( any(lessThan(pos, vec3(0.0))) || any(greaterThan(pos, vec3(1.0))) )
            break;

        float density = texture(u_texture3D[0], pos).r / u_volumeMax;
        float lowCut  = smoothstep(0.08, 0.18, density);
        float highCut = 1.0 - smoothstep(0.28, 0.35, density);
        float d = lowCut * highCut;

        if( d > 0.01 )
        {
            vec3 gradObj = computeGradient(pos) / (u_bmax - u_bmin);
            vec3 normal = normalize(-gradObj);
            float diffuse = max(dot(normal, v_directionLight), 0.0);
            float shade = 0.25 + 0.75 * diffuse;   // ambient + diffuse

            vec4 c = vec4(vec3(shade), d * 0.15);
            accum.rgb += (1.0 - accum.a) * c.a * c.rgb;
            accum.a   += (1.0 - accum.a) * c.a;
        }

        if( accum.a >= 0.98 )
            break;

        pos += stepTex;
    }

    fragColor = vec4(accum.rgb, accum.a);
}