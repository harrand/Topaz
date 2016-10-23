// Fragment Shader version 4.30
#version 430

varying vec2 texcoord0;
varying vec3 normal0;
varying vec3 position0;
varying mat4 v0;
uniform sampler2D diffuse;

void main()
{
	// Output pixel colour. every pixel is red.
	// Both lD and normal0 need to be normalised.
	// Light shoots out from the z-axis, hardcoded example
	// need about 6 light sources

	vec3 towardsEye = (v0 * vec4(0, 0, -1, 0)).xyz;
	
	vec3 lightDirection = vec3(0, -1, 1);
	
	vec3 R = reflect(-lightDirection, normalize(v0 * vec4(normal0, 0.0)).xyz);
	float cosAlpha = clamp(dot(towardsEye, R), 0, 1);
	
	vec4 lightColour = vec4(1, 1, 1, 1);
	float lightWattage = 200.0;
	
	vec4 textureColour = texture2D(diffuse, texcoord0);
	float diffuseComponent = clamp(dot(-lightDirection, normal0), 0.1, 1.0);
	
	float distances[9];
	vec3 lightSources[9];
	gl_FragColor = vec4(0, 0, 0, 0);
	for(int i = 0; i < 9; i++)
	{
		lightSources[i] = vec3(14, 3, i * 50);
		distances[i] = length(position0 - lightSources[i]);
		gl_FragColor += (textureColour * lightColour * lightWattage * diffuseComponent / (distances[i] * distances[i]));
		gl_FragColor += (textureColour * lightColour * lightWattage * pow(cosAlpha, 5) / (distances[i] * distances[i]));
	}
	//gl_FragColor = texture2D(diffuse, texcoord0) * vec4(lightColour, 1) * lightWattage /** clamp(dot(-lightDirection, normal0), 0.1, 1.0)*/ / (distance * distance);
}