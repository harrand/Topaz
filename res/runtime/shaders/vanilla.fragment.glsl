// Fragment Shader version 4.30
#version 430

in vec3 positionShared;
in vec2 texcoordShared;
in vec3 normalShared;
in mat4 viewShared;
in vec3 eyeDirectionCamSpace;
uniform sampler2D diffuse;

layout(location = 0) out vec4 fragColor;

const vec4 lightColour = vec4(1, 1, 1, 1);
const float lightWattage = 150;
const vec4 textureColour = texture2D(diffuse, texcoordShared);
const vec3 cameraLightSource = (inverse(viewShared) * vec4(0, 0, 0, 1)).xyz;
const vec3 ld = positionShared - cameraLightSource;
const float distance = length(ld);
const vec3 lightDirection = normalize(ld);

vec4 getDiffuseComponent()
{
	float cosTheta = clamp(dot(normalShared, -lightDirection), 0, 1);
	return textureColour*lightColour* lightWattage *cosTheta / (distance * distance);
}

vec4 getAmbientComponent()
{
	return textureColour * vec4(0.1, 0.1, 0.1, 1);
}

vec4 getSpecularComponent()
{
	vec3 E = normalize(eyeDirectionCamSpace);
	vec3 R = reflect(-lightDirection, normalShared);
	float cosAlpha = clamp(dot(E, R), 0, 1);
	return textureColour * lightColour * lightWattage * pow(cosAlpha, 5) / (distance * distance);
}

void main()
{
	fragColor = getAmbientComponent() + getDiffuseComponent() + getSpecularComponent();
}