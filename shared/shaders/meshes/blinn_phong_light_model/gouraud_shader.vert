struct gl_LightSourceParameters 
{   
   vec4 ambient;              // Aclarri   
   vec4 diffuse;              // Dcli   
   vec4 specular;             // Scli   
   vec4 position;             // Ppli   
   vec4 halfVector;           // Derived: Hi   
   vec3 spotDirection;        // Sdli   
   float spotExponent;        // Srli   
   float spotCutoff;          // Crli                              
                              // (range: [0.0,90.0], 180.0)   
   float spotCosCutoff;       // Derived: cos(Crli)                 
                              // (range: [1.0,0.0],-1.0)   
   float constantAttenuation; // K0   
   float linearAttenuation;   // K1   
   float quadraticAttenuation;// K2  
};    

struct gl_MaterialParameters  
{   
   vec4 emission;    // Ecm   
   vec4 ambient;     // Acm   
   vec4 diffuse;     // Dcm   
   vec4 specular;    // Scm   
   float shininess;  // Srm  
};  

uniform gl_MaterialParameters gl_FrontMaterial;  
uniform gl_MaterialParameters gl_BackMaterial;
uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];
uniform int coloringModel;

varying vec4 diffuseFactor;
varying vec4 interpolatedDiffuseMaterial;

vec3 eyeDirection;

void main()
{
	// texture
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

	// vertex
	vec4 eyeVertexPosition = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// normal
	vec3 transformedNormal = gl_NormalMatrix * gl_Normal;	
	transformedNormal = normalize(transformedNormal);

	// ambient
	vec4 ambientColor = (gl_LightSource[0].ambient + gl_LightModel.ambient) * gl_FrontMaterial.ambient;

	// diffuse
	vec3 directionLight = normalize(gl_LightSource[0].position.xyz);
	float cos_theta = max(dot(transformedNormal, directionLight), 0.0);
	diffuseFactor = gl_LightSource[0].diffuse * cos_theta;
	if (coloringModel == 0)
		interpolatedDiffuseMaterial = gl_FrontMaterial.diffuse;
	else if (coloringModel == 1)
		interpolatedDiffuseMaterial = vec4(transformedNormal, 1);
	else	interpolatedDiffuseMaterial = vec4(1, 0, 1, 1); // should not happend

	// specular
	//if (local_viewer)
	//{
		// anatomist local viewer behaviour
	//	eyeDirection = normalize(-eyeVertexPosition.xyz);
	//} else {
	// anatomist non local viewer behaviour (default)
	eyeDirection = vec3(0, 0, 1);
	//}
	//XXX : add this mode
	// my suggestion (use this trick because the light is inside the object)
	//eyeDirection = normalize(eyeVertexPosition.xyz);
	vec3 half_vector = directionLight + eyeDirection;
	half_vector = normalize(half_vector);
	float cos_alpha = max(dot(half_vector, transformedNormal), 0.0);
	float specularFactor = pow(cos_alpha, gl_FrontMaterial.shininess);
	vec4 specularColor = gl_LightSource[0].specular * gl_FrontMaterial.specular * specularFactor;

	gl_FrontColor = ambientColor + specularColor;
}
