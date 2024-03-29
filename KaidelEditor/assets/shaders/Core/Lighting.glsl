#include "Core.glsl"
struct LightingCalcResult{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};


LightingCalcResult CalculateFragmentLighting(in SpotLight light,vec3 position,vec3 diffuseCol,float specColor,vec3 normal,vec3 viewDir,float shininess){
	LightingCalcResult res;
	res.Ambient = vec3(0.0);
	res.Diffuse = vec3(0.0);
	res.Specular = vec3(0.0);

	vec3 lightPos = light.Position.xyz;
	vec3 lightDir = normalize(lightPos - position);

	vec3 lightDirection = light.Direction.xyz;
	vec3 lightAmbient = light.Ambient.xyz;


	float distance = length(lightPos - position);
	float attenuation = 1.0/(light.ConstantCoefficient+light.LinearCoefficient*distance+
	light.QuadraticCoefficient*(distance*distance));


	//Ambient
	res.Ambient += lightAmbient * diffuseCol * attenuation;
	
	float theta = dot(lightDir,normalize(-lightDirection));
	if(theta > light.CutOffAngle){
		vec3 lightDiffuse = light.Diffuse.rgb;
		vec3 lightSpecular = light.Specular.rgb;

		//Diffuse
		res.Diffuse += diffuseCol * max(dot(normal, lightDir), 0.0) * lightDiffuse * attenuation;
		//Specular
		vec3 reflectDir = reflect(-lightDir,normal);
		res.Specular += lightSpecular * pow(max(dot(viewDir,reflectDir),0.0),shininess) * specColor * attenuation;
	}


	return res;
}





