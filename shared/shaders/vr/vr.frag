varying vec3 frag;

uniform sampler2D tex;
uniform sampler3D volume_tex;
uniform sampler1D tf_tex;

uniform float stepsize;

uniform vec4 eyepos;
uniform vec4 lightpos;

uniform float deltag;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform float shininess;

uniform int method;

uniform float tex2tf;

uniform float aFact;


vec3 phong( vec3 N, vec3 V, vec3 L )
{

  vec3 ka = vec3( 1.0f, 1.0f, 1.0f );
  vec3 kd = vec3( 1.0f, 1.0f, 1.0f );
  vec3 ks = vec3( 1.0f, 1.0f, 1.0f );

  float diff = max( dot( L, N ), 0.0 );

  vec3 H = normalize( L + V );

  float spec = pow( max( dot( H, N ), 0.0 ), shininess );

  if ( diff < 0.0 )
  {
    spec = 0.0;
  }

  return ka * ambient.rgb + 
         kd * diffuse.rgb * diff + 
         ks * specular.rgb * spec;

}

vec4 volumeRendering( vec4 rayStart, vec3 rayDir )
{

  float len = length( rayDir.xyz );
  vec3 normDir = normalize( rayDir );
  vec3 deltaDir = normDir * stepsize;
  float deltaDirLen = length( deltaDir );
  vec4 colAcc = vec4( 0, 0, 0, 0 );
  float lenAcc = 0.0;
  vec4 colorSample;
  float aSample;
  vec3 vect = rayStart.xyz;

  for ( int i = 0; i < 2048; i++ )
  {
    float intensity = texture3D( volume_tex, vect );
    float v = tex2tf * intensity;
    colorSample = texture1D( tf_tex, v );
    colorSample.a *= aFact;

    if ( colorSample.a > 0.02 )
    {
      vec3 p1, p2, a1, a2;
      p1.x = texture3D( volume_tex, vect.xyz - vec3( deltag, 0, 0 ) );
      p2.x = texture3D( volume_tex, vect.xyz + vec3( deltag, 0, 0 ) );
      p1.y = texture3D( volume_tex, vect.xyz - vec3( 0, deltag, 0 ) );
      p2.y = texture3D( volume_tex, vect.xyz + vec3( 0, deltag, 0 ) );
      p1.z = texture3D( volume_tex, vect.xyz - vec3( 0, 0, deltag ) );
      p2.z = texture3D( volume_tex, vect.xyz + vec3( 0, 0, deltag ) );
      vec3 N = normalize( p2 - p1 );
      vec3 V = normalize( eyepos.xyz - frag );
      vec3 L = normalize( lightpos.xyz - frag );
      colorSample.rgb *= phong( N, V, L );
    }

    colorSample.rgb *= colorSample.a;
    colAcc += ( 1.0 - colAcc.a ) * colorSample;
    vect += deltaDir;
    lenAcc += deltaDirLen;

    if ( ( lenAcc >= len ) || ( colAcc.a > 1.0 ) )
      break;

  }

  return colAcc;

}

vec4 mipRendering( vec4 rayStart, vec3 rayDir )
{

  float len = length( rayDir.xyz );
  vec3 normDir = normalize( rayDir );
  vec3 deltaDir = normDir * stepsize;
  float deltaDirLen = length( deltaDir );
  vec3 vect = rayStart.xyz;
  float lenAcc = 0.0;
  float maxIntensity = 0.0;

  for ( int i = 0; i < 2048; i++ )
  {
    float intensity = texture3D( volume_tex, vect );

    if ( intensity > maxIntensity )
    {
      maxIntensity = intensity;
    }

    vect += deltaDir;
    lenAcc += deltaDirLen;

    if ( lenAcc >= len )
      break;

  }

  float v = tex2tf * maxIntensity;
  return texture1D( tf_tex, v );

}

void main( void )
{

  vec2 texc = ( ( gl_TexCoord[ 2 ].xy / 
                  gl_TexCoord[ 2 ].w ) + 1.0 ) / 2.0;
  vec4 start = gl_TexCoord[ 0 ];
  vec4 backPos = texture2D( tex, texc );
  vec3 dir = vec3( 0.0, 0.0, 0.0 );
  dir.x = backPos.x - start.x;
  dir.y = backPos.y - start.y;
  dir.z = backPos.z - start.z;

  if ( method == 0 )
  {
    gl_FragColor = volumeRendering( start, dir );
  }

  if ( method == 1 )
  {
    gl_FragColor = mipRendering( start, dir );
  }

}
