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

layout(location = 0) out vec4 fragColor;

const vec4 lightColour = vec4(1, 1, 1, 1);
const float lightWattage = 5000;

const vec4 textureColour = texture2D(textureSampler, texcoord_modelspace);

const vec3 position_worldspace = (modelMatrix * vec4(position_modelspace, 1.0)).xyz;
const vec3 position_cameraspace = (viewMatrix * vec4(position_worldspace, 1.0)).xyz;

const vec3 cameraPosition_worldspace = (inverse(viewMatrix) * vec4(0, 0, 0, 1)).xyz;
const vec3 eyeDirection_cameraspace = vec3(0, 0, 0) - position_cameraspace;

const vec3 ld = cameraPosition_worldspace - position_worldspace;
const float distance = length(ld);
const vec3 lightDirection = normalize(ld);

vec4 getDiffuseComponent(vec3 parsedNormal)
{
	float cosTheta = clamp(abs(dot(parsedNormal, lightDirection)), 0.3, 1.0);
	return textureColour * lightColour * lightWattage * cosTheta / (distance * distance);
}

vec4 getAmbientComponent()
{
	return textureColour * vec4(0.01, 0.01, 0.01, 1);
}

vec4 getSpecularComponent(vec3 parsedNormal)
{
	vec3 E = normalize(eyeDirection_cameraspace);
	vec3 R = reflect(-lightDirection, parsedNormal);
	float cosAlpha = clamp(dot(E, R), 0, 1);
	return textureColour * lightColour * lightWattage * pow(cosAlpha, 5) / (distance * distance);
}

void main()
{
	vec3 normal = normalize(/*tbnMatrix * */(255.0/128.0 * texture2D(normalMapSampler, texcoord_modelspace).xyz - 1));
	fragColor = getAmbientComponent() + getDiffuseComponent(normal) + getSpecularComponent(normal);
}