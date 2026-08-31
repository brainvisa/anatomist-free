uniform sampler3D u_texture3D[8];
uniform sampler1D u_transferFunction;

uniform vec3 u_bmin;
uniform vec3 u_bmax;
uniform float u_volumeMax;
uniform vec3 u_texDim;

uniform int   u_activeClipPlanes;
uniform int   u_clippedObjectActive;
uniform vec4  u_clipPlane0;
uniform vec4  u_clipPlane1;
uniform vec4  u_clipPlane2;

in vec3 v_objectPosRaw;
in vec3 v_directionLight;

{Illumination Model Uniforms}

out vec4 fragColor;


{Illumination Model Functions}

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
    float nbSamples = 3.0;
    vec3 eps = vec3(nbSamples/u_texDim.x, nbSamples/u_texDim.y, nbSamples/u_texDim.z);
    float dx = texture(u_texture3D[0], pos + vec3(eps.x,0,0)).r
             - texture(u_texture3D[0], pos - vec3(eps.x,0,0)).r;
    float dy = texture(u_texture3D[0], pos + vec3(0,eps.y,0)).r
             - texture(u_texture3D[0], pos - vec3(0,eps.y,0)).r;
    float dz = texture(u_texture3D[0], pos + vec3(0,0,eps.z)).r
             - texture(u_texture3D[0], pos - vec3(0,0,eps.z)).r;
    return vec3(dx, dy, dz);
}

bool isClipped(vec4 eyePos)
{
    if( u_activeClipPlanes >= 1 && dot(u_clipPlane0, eyePos) < 0.0 )
        return true;
    if( u_activeClipPlanes >= 2 && dot(u_clipPlane1, eyePos) < 0.0 )
        return true;
    if( u_clippedObjectActive == 1 && dot(u_clipPlane2, eyePos) < 0.0 )
        return true;

    return false;
}

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(gl_ModelViewMatrix)));
    vec3 rayDir = normalize( (inverse(gl_ModelViewMatrix) * vec4(0.0, 0.0, -1.0, 0.0)).xyz );
    vec3 rayOrigin = v_objectPosRaw;

    float tNear, tFar;
    if( !intersectAABB(rayOrigin, rayDir, u_bmin, u_bmax, tNear, tFar) )
        discard;
    tNear = max(tNear, 0.0);
    vec3 entryPointObj = rayOrigin + rayDir * tNear;

    // -------- values to tweak --------
    float stepSizeMM = 0.3;
    int numSteps = int((tFar - tNear) / stepSizeMM);
    float accumFactor = 0.5;
    float densityFloor = 0.1;
    // ---------------------------------

    // texture coordinates
    vec3 texCoord = (entryPointObj - u_bmin) / (u_bmax - u_bmin);
    vec3 rayDirTex = rayDir / (u_bmax - u_bmin);
    vec3 stepTex = rayDirTex * stepSizeMM;

    // eye space coordinates
    vec4 currentPosEye = gl_ModelViewMatrix * vec4(entryPointObj, 1.0);
    vec4 eyeStep = gl_ModelViewMatrix * vec4(rayDir * stepSizeMM, 0.0);

    vec4 accum = vec4(0.0);

    for( int i = 0; i < numSteps; ++i )
    {
        if( any(lessThan(texCoord, vec3(0.0))) || any(greaterThan(texCoord, vec3(1.0))) )
            break;

        if( !isClipped(currentPosEye) )
        {
            float density = texture(u_texture3D[0], texCoord).r / u_volumeMax;
            if( density > densityFloor) 
            {
                vec4 tf = texture(u_transferFunction, density);
                if( tf.a > 0.01 )
                {
                    vec3 densityGrad = computeGradient(texCoord) / (u_bmax - u_bmin);
                    float gradLen = length(densityGrad);
                    vec3 normalObj = gradLen > 0.0001
                                      ? normalize(densityGrad)
                                      : vec3(0.0, 0.0, 1.0);

                    // object space normal to eye space normal
                    vec3 normalEye = normalize( normalMatrix * normalObj );

                    BlinnPhongMaterial bp = BlinnPhong(normalEye);
                    vec3 ambientTerm  = bp.ambient.rgb;
                    vec3 diffuseTerm  = tf.rgb * bp.diffuse.rgb;
                    vec3 specularTerm = bp.specular.rgb;

                    vec3 shadedColor = ambientTerm + diffuseTerm + specularTerm;

                    vec4 c = vec4(shadedColor, tf.a * accumFactor);
                    accum.rgb += (1.0 - accum.a) * c.a * c.rgb;
                    accum.a   += (1.0 - accum.a) * c.a;
                }
            }
        }

        if( accum.a >= 0.95 )
            break;

        texCoord += stepTex;
        currentPosEye += eyeStep;
    }

    fragColor = vec4(accum.rgb, accum.a);
}