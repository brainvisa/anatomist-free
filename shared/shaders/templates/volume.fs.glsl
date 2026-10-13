#define MAX_OBJECT_CLIP_PLANES 6 //jordan to change with the one in renderContext and globjectuniforms

uniform sampler1D u_texture1D[8]; // transfer function
uniform sampler3D u_texture3D[8]; //[0] volume, [1] gradient

uniform vec3 u_bmin;
uniform vec3 u_bmax;
uniform float u_volumeMax;
uniform float u_volumeMin;
uniform float u_paletteMin;
uniform float u_paletteMax;
uniform int   u_colorVolume; // 1 if the volume is RGB/RGBA, 0 if scalar
uniform int  u_alphaComputed; // 1 if the alpha channel of a color volume was recomputed from the color norm, 0 if it was kept as is 


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


const float depthAlphaThreshold = 0.5;   // accumulated alpha threshold for depth hit
const float earlyTerminationAlpha = 0.95; // accumulated alpha threshold for early termination

// ratio between the current step and the reference step (1 voxel),
// used for opacity correction. Set in main().
float g_stepRatio = 1.0;

// gradient length (relative to the value range) under which the region is
// considered homogeneous: no shading
const float gradientReliability = 0.01;

// color volumes: ambient part of the luminance-only lighting
const float colorAmbient = 0.4;


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

// distance to a clip plane (eye space), kept as the nearest one so far
void testClipPlane( vec4 plane, vec4 eyePos, inout float dmin, inout vec3 normal )
{
    float len = length( plane.xyz );
    if( len < 1e-6 )
        return;
    float d = dot( plane, eyePos ) / len;   // >= 0 on the kept side
    if( d < dmin )
    {
        dmin = d;
        normal = -plane.xyz / len;          // outward normal of the cut face
    }
}

// distance to the nearest active clip plane, and its normal (eye space)
float nearestClipPlane( vec4 eyePos, out vec3 normal )
{
    float dmin = 1e20;
    normal = vec3( 0.0, 0.0, 1.0 );
    if( u_activeClipPlanes >= 1 )
        testClipPlane( u_clipPlane0, eyePos, dmin, normal );
    if( u_activeClipPlanes >= 2 )
        testClipPlane( u_clipPlane1, eyePos, dmin, normal );
    for( int i = 0; i < MAX_OBJECT_CLIP_PLANES; ++i )
    {
        if( i >= u_nbObjectClipPlanes )
            break;
        testClipPlane( u_objectClipPlanes[i], eyePos, dmin, normal );
    }
    return dmin;
}

// orients a normal toward the camera (orthographic view: eye looks along -z)
vec3 faceCamera( vec3 n )
{
    return n.z < 0.0 ? -n : n;
}

/* Shading normal (eye space, facing the camera), shared by scalar and
   color volumes:
   - gradient normal, faded out in homogeneous regions (noisy gradient)
   - clip plane normal near a cut, where the visible surface is the plane
*/
vec3 shadingNormal( vec3 grad, vec3 voxelSize, float valueRange,
                    vec4 eyePos, mat3 normalMatrix, float clipShadingDistance )
{
    vec3 normalEye = vec3( 0.0, 0.0, 1.0 );   // facing the camera

    float relLen = length( grad ) / valueRange;
    float wGrad = smoothstep( gradientReliability, 4.0 * gradientReliability, relLen );
    if( wGrad > 0.0 )
        normalEye = normalize( mix( normalEye,
            faceCamera( normalize( normalMatrix * ( grad / voxelSize ) ) ), wGrad ) );

    vec3 clipNormal;
    float dClip = nearestClipPlane( eyePos, clipNormal );
    float wClip = 1.0 - smoothstep( 0.0, clipShadingDistance, dClip );
    return normalize( mix( normalEye, faceCamera( clipNormal ), wClip ) );
}

vec4 transferFunction( float raw )
{
    float density  = (raw - u_volumeMin) / max(u_volumeMax - u_volumeMin, 1e-6);
    float paletteT = clamp((density - u_paletteMin) / max(u_paletteMax - u_paletteMin, 1e-6), 0.0, 1.0);
    return texture( u_texture1D[0], paletteT );
}

float sampleAlpha( vec3 texCoord, vec4 eyePos, out vec4 tf )
{
    tf = vec4(0.0);
    if( isClipped(eyePos) )
        return 0.0;

    vec4 voxel = texture( u_texture3D[0], texCoord );
    if( u_colorVolume != 0 )
    {
        // premultiplied texture: recover the correctly interpolated color
        vec3 color = min( voxel.rgb / max( voxel.a, 1e-4 ), vec3(1.0) );
        color.r = transferFunction( color.r ).r;
        color.g = transferFunction( color.g ).g;
        color.b = transferFunction( color.b ).b;

        float alpha = ( u_alphaComputed != 0 )
            ? transferFunction( voxel.a ).a   // computed alpha: palette = opacity curve
            : voxel.a;            // stored alpha: respected, global scale only
        tf = vec4( color, alpha );
    }
    else
        tf = transferFunction( voxel.r );

    if( tf.a <= 0.01 )
        return 0.0;
    // opacity correction: palette alpha is defined for a 1-voxel step
    return 1.0 - pow( 1.0 - tf.a, g_stepRatio );
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

    // step: half the smallest voxel size, opacity corrected accordingly
    vec3 voxelSize = (u_bmax - u_bmin) / vec3( textureSize( u_texture3D[0], 0 ) );
    float refStep = min( min( voxelSize.x, voxelSize.y ), voxelSize.z );
    float stepSizeMM = 0.5 * refStep;
    g_stepRatio = stepSizeMM / refStep;
    int numSteps = int((tFar - tNear) / stepSizeMM);

    // distance from a clip plane under which its normal is used for shading
    float clipShadingDistance = 2.0 * refStep;

    vec3 entryPointObj = rayOrigin + rayDir * ( tNear * stepSizeMM );

    // texture coordinates
    vec3 texCoordStart = (entryPointObj - u_bmin) / (u_bmax - u_bmin);
    vec3 stepTex = rayDir * stepSizeMM / (u_bmax - u_bmin);

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
    float valueRange = ( u_colorVolume != 0 ) ? 1.0 : max( u_volumeMax - u_volumeMin, 1e-6 );

    for( int i = 0; i < numSteps; ++i )
    {
        if( any(lessThan(texCoord, vec3(0.0))) || any(greaterThan(texCoord, vec3(1.0))) )
            break;

        vec4 tf;
        float a = sampleAlpha( texCoord, currentPosEye, tf );
        if( a > 0.0 )
        {
            vec3 grad = texture( u_texture3D[1], texCoord ).xyz;
            vec3 normalEye = shadingNormal( grad, voxelSize, valueRange,
                                            currentPosEye, normalMatrix,
                                            clipShadingDistance );

            vec3 shadedColor;
            if( u_colorVolume != 0 )
            {
                // luminance-only diffuse: direction colors are kept,
                // no (white) specular that would desaturate them
                vec3 lightEye = normalize( gl_LightSource[0].position.xyz );
                float diffuse = max( dot( normalEye, lightEye ), 0.0 );
                shadedColor = tf.rgb * ( colorAmbient + ( 1.0 - colorAmbient ) * diffuse );
                shadedColor = tf.rgb ;
            }
            else
            {
                BlinnPhongMaterial bp = BlinnPhong( normalEye );
                shadedColor = bp.ambient.rgb + tf.rgb * bp.diffuse.rgb + bp.specular.rgb;
                shadedColor = tf.rgb;
            }

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