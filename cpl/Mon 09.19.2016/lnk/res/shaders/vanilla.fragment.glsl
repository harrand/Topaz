// Fragment Shader version 4.30
#version 430

in vec3 positionShared;
in vec2 texcoordShared;
in vec3 normalShared;
in mat4 viewShared;
uniform sampler2D diffuse;

layout(location = 0) out vec4 fragColor;

void main()
{
	
	vec3 lightDirection = vec3(0, 0, 0);
	
	vec4 lightColour = vec4(1, 1, 1, 1);
	float lightWattage = 60;//600.0;
	vec4 textureColour = texture2D(diffuse, texcoordShared);
	
	float diffuseComponent = clamp(dot(-lightDirection, normalShared), 0.1, 1.0); // this is costheta (clamped)
	
	fragColor = vec4(0, 0, 0, 0);
	vec3 cameraLightSource = (inverse(viewShared) * vec4(0, 1, 0, 1)).xyz;
	float distCam = length(positionShared - cameraLightSource);
	fragColor += textureColour * lightColour * lightWattage * diffuseComponent / (distCam);//) * distCam);
}