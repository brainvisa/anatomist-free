#define MAX_OBJECT_CLIP_PLANES 6 //jordan to change with the one in renderContext and globjectuniforms

uniform sampler1D u_texture1D[8]; // transfer function
uniform sampler3D u_texture3D[8]; //[0] volume, [1] gradient
uniform sampler1D u_transferFunction;

uniform vec3 u_bmin;
uniform vec3 u_bmax;
uniform float u_volumeMax;
uniform float u_volumeMin;
uniform vec3 u_texDim;
uniform float u_paletteMin;
uniform float u_paletteMax;


uniform int   u_activeClipPlanes;
uniform vec4  u_clipPlane0;
uniform vec4  u_clipPlane1;
uniform int   u_nbObjectClipPlanes;
uniform vec4  u_objectClipPlanes[MAX_OBJECT_CLIP_PLANES];

in vec3 v_objectPosRaw;
in vec3 v_directionLight;

{Illumination Model Uniforms}
{Effect Uniforms}

out vec4 fragColor;



// NOUVEAU : paramètres partagés par les deux phases
const float depthAlphaThreshold = 0.5;   // opacité accumulée = "surface" pour le Z-buffer
const float earlyTerminationAlpha = 0.95;


{Illumination Model Functions}

float eyeToWindowDepth( vec4 eyePos )
{
    vec4 clipPos = gl_ProjectionMatrix * eyePos;
    float ndcZ = clipPos.z / clipPos.w;
    return 0.5 * ( gl_DepthRange.diff * ndcZ + gl_DepthRange.near + gl_DepthRange.far );
}

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

bool isClipped(vec4 eyePos)
{
    if( u_activeClipPlanes >= 1 && dot(u_clipPlane0, eyePos) < 0.0 )
        return true;
    if( u_activeClipPlanes >= 2 && dot(u_clipPlane1, eyePos) < 0.0 )
        return true;


    for( int i = 0; i < MAX_OBJECT_CLIP_PLANES; ++i )
    {
        if( i >= u_nbObjectClipPlanes )
            break;
        if( dot(u_objectClipPlanes[i], eyePos) < 0.0 )
            return true;
    }

    return false;
}

float sampleAlpha( vec3 texCoord, vec4 eyePos, out vec4 tf )
{
    tf = vec4(0.0);
    if( isClipped(eyePos) )
        return 0.0;

    float rawDensity = texture(u_texture3D[0], texCoord).r;
    float density = (rawDensity - u_volumeMin) / max(u_volumeMax - u_volumeMin , 1e-6);
    float paletteT = clamp((density - u_paletteMin) / (u_paletteMax - u_paletteMin), 0.0, 1.0);
    tf = texture(u_texture1D[0], paletteT);

    return ( tf.a > 0.01 ) ? tf.a : 0.0;
}

bool findSurface( vec3 texCoord, vec3 stepTex, vec4 eyePos, vec4 eyeStep,
                  int numSteps, out vec4 hitEye )
{
    hitEye = vec4(0.0);
    float alpha = 0.0;

    for( int i = 0; i < numSteps; ++i )
    {
        if( any(lessThan(texCoord, vec3(0.0))) || any(greaterThan(texCoord, vec3(1.0))) )
            break;

        vec4 tf;
        float a = sampleAlpha( texCoord, eyePos, tf );
        if( a > 0.0 )
        {
            alpha += (1.0 - alpha) * a;
            if( alpha >= depthAlphaThreshold )
            {
                hitEye = eyePos;
                return true;
            }
        }

        texCoord += stepTex;
        eyePos += eyeStep;
    }
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
    // ---------------------------------

    // texture coordinates
    vec3 texCoordStart = (entryPointObj - u_bmin) / (u_bmax - u_bmin);
    vec3 rayDirTex = rayDir / (u_bmax - u_bmin);
    vec3 stepTex = rayDirTex * stepSizeMM;

    // eye space coordinates
    vec4 eyePosStart = gl_ModelViewMatrix * vec4(entryPointObj, 1.0);
    vec4 eyeStep = gl_ModelViewMatrix * vec4(rayDir * stepSizeMM, 0.0);

    bool depthHit = false;
    vec4 depthHitEye = vec4(0.0);
    float fragDepth = gl_FragCoord.z;

    bool surfaceKnown = false;
#ifdef DEPTH_PEELING
    if( u_layer > 0 )
    {
        depthHit = findSurface( texCoordStart, stepTex, eyePosStart, eyeStep,
                                numSteps, depthHitEye );
        fragDepth = depthHit ? eyeToWindowDepth( depthHitEye ) : gl_FragCoord.z;

        vec2 depthTexCoord = gl_FragCoord.xy / vec2( textureSize( u_previousDepthTexture, 0 ) );
        float previousDepth = texture( u_previousDepthTexture, depthTexCoord ).r;
        if( fragDepth <= previousDepth + 1e-3 )
            discard;

        surfaceKnown = true;
    }
#endif

    vec3 texCoord = texCoordStart;
    vec4 currentPosEye = eyePosStart;
    vec4 accum = vec4(0.0);

    for( int i = 0; i < numSteps; ++i )
    {
        if( any(lessThan(texCoord, vec3(0.0))) || any(greaterThan(texCoord, vec3(1.0))) )
            break;

        vec4 tf;
        float a = sampleAlpha( texCoord, currentPosEye, tf );
        if( a > 0.0 )
        {
             vec3 densityGrad = texture( u_texture3D[1], texCoord ).xyz / (u_bmax - u_bmin);
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

            accum.rgb += (1.0 - accum.a) * a * shadedColor;
            accum.a   += (1.0 - accum.a) * a;

            if( !surfaceKnown && !depthHit && accum.a >= depthAlphaThreshold )
            {
                depthHit = true;
                depthHitEye = currentPosEye;
            }
        }

        if( accum.a >= earlyTerminationAlpha )
            break;

        texCoord += stepTex;
        currentPosEye += eyeStep;
    }

    if( !surfaceKnown )
        fragDepth = depthHit ? eyeToWindowDepth( depthHitEye ) : gl_FragCoord.z;

    gl_FragDepth = fragDepth;
    fragColor = vec4(accum.rgb/max(accum.a,1e-4), accum.a);
}