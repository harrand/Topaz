// Fragment Shader version 4.30
#version 430

in vec3 position_modelspace;
in vec2 texcoord_modelspace;
in vec3 normal_modelspace;

in mat4 modelMatrix;
in mat4 viewMatrix;
in mat3 tbnMatrix;

uniform sampler2D textureSampler;
uniform sampler2D normalMapSampler;
uniform sampler2D parallaxMapSampler;

uniform float parallaxMultiplier;
uniform float parallaxBias;

layout(location = 0) out vec4 fragColor;

const unsigned int MAX_LIGHTS = 8;

struct BaseLight
{
	vec3 pos;
	float power;
};

uniform BaseLight lights[MAX_LIGHTS];

const vec4 lightColour = vec4(1, 1, 1, 1);
const float lightWattage = 50000;

const vec3 position_worldspace = (modelMatrix * vec4(position_modelspace, 1.0)).xyz;
const vec3 position_cameraspace = (viewMatrix * vec4(position_worldspace, 1.0)).xyz;

const vec3 cameraPosition_cameraspace = vec3(0, 0, 0);
const vec3 eyeDirection_cameraspace = vec3(0, 0, 0) - position_cameraspace;
const vec3 eyeDirection_tangentspace = tbnMatrix * eyeDirection_cameraspace;

const vec3 ld_cameraspace = cameraPosition_cameraspace - position_cameraspace;
const float distance = length(ld_cameraspace);
const vec3 lightDirection_cameraspace = normalize(ld_cameraspace);
const vec3 lightDirection_tangentspace = tbnMatrix * lightDirection_cameraspace;

vec2 getTexcoordOffset()
{
	return texcoord_modelspace + lightDirection_tangentspace.xy * (texture2D(parallaxMapSampler, texcoord_modelspace).r * parallaxMultiplier + parallaxBias);
}

vec4 textureColour = texture2D(textureSampler, getTexcoordOffset());

vec4 getDiffuseComponent(vec3 parsedNormal_tangentspace)
{
	float cosTheta = clamp(dot(parsedNormal_tangentspace, lightDirection_tangentspace), 0.0, 1.0);
	return textureColour * lightColour * lightWattage * cosTheta / (distance * distance);
}

vec4 getDiffuseComponentFromLight(BaseLight l, vec3 parsedNormal_tangentspace)
{
	vec3 lightPos_cameraspace = (viewMatrix * vec4(l.pos, 1.0)).xyz;
	vec3 displacementFromLight = lightPos_cameraspace - position_cameraspace;
	float cosTheta = clamp(dot(parsedNormal_tangentspace, tbnMatrix * displacementFromLight), 0.0, 1.0);
	float distanceFromLight = length(displacementFromLight);
	return textureColour * vec4(1, 1, 1, 1) * l.power * cosTheta / (distanceFromLight * distanceFromLight);
}

vec4 getAmbientComponent()
{
	return textureColour * vec4(0.01, 0.01, 0.01, 1);
}

vec4 getSpecularComponent(vec3 parsedNormal_tangentspace)
{
	vec3 E = normalize(eyeDirection_tangentspace);
	vec3 R = reflect(-normalize(lightDirection_tangentspace), parsedNormal_tangentspace);
	float cosAlpha = clamp(dot(E, R), 0, 1);
	return textureColour * lightColour * lightWattage * pow(cosAlpha, 5) / (distance * distance);
}

vec4 getSpecularComponentFromLight(BaseLight l, vec3 parsedNormal_tangentspace)
{
	vec3 lightPos_cameraspace = (viewMatrix * vec4(l.pos, 1.0)).xyz;
	vec3 displacementFromLight = lightPos_cameraspace - position_cameraspace;
	vec3 E = normalize(eyeDirection_tangentspace);
	vec3 R = reflect(-normalize(tbnMatrix * displacementFromLight), parsedNormal_tangentspace);
	float cosAlpha = clamp(dot(E, R), 0, 1);
	float distanceFromLight = length(displacementFromLight);
	return textureColour * vec4(1, 1, 1, 1) * l.power * pow(cosAlpha, 5) / (distanceFromLight * distanceFromLight);
}

void main()
{
	vec3 normal_tangentspace = normalize(texture2D(normalMapSampler, getTexcoordOffset()).xyz * 255.0/128.0 - 1);
	fragColor = getAmbientComponent() + getDiffuseComponent(normal_tangentspace) + getSpecularComponent(normal_tangentspace);
	for(unsigned int i = 0; i < MAX_LIGHTS; i++)
	{
		fragColor += getDiffuseComponentFromLight(lights[i], normal_tangentspace) + getSpecularComponentFromLight(lights[i], normal_tangentspace);
	}
}