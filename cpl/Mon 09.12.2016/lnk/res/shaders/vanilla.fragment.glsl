// Fragment Shader version 4.30
#version 430

varying vec2 texcoordShared;
varying vec3 normalShared;
varying vec3 positionShared;
varying mat4 viewShared;
uniform sampler2D diffuse;

void main()
{
	vec3 towardsEye = (viewShared * vec4(0, 0, -1, 0)).xyz;
	
	vec3 lightDirection = vec3(0, 0, 0);//vec3(0, -1, 1);
	
	vec3 R = reflect(-lightDirection, normalize(viewShared * vec4(normalShared, 0.0)).xyz);
	float cosAlpha = clamp(dot(towardsEye, R), 0, 1);
	
	vec4 lightColour = vec4(1, 1, 1, 1);
	float lightWattage = 5000.0;
	
	vec4 textureColour = texture2D(diffuse, texcoordShared);
	float diffuseComponent = clamp(dot(-lightDirection, normalShared), 0.1, 1.0);
	
	float distances[9];
	vec3 lightSources[9];
	gl_FragColor = vec4(0, 0, 0, 0);

	for(int i = 0; i < 9; i++)
	{
		lightSources[i] = vec3(14, 3, i * 50);
		distances[i] = length(positionShared - lightSources[i]);
		gl_FragColor += (textureColour * lightColour * lightWattage * diffuseComponent / (distances[i] * distances[i]));
		gl_FragColor += (textureColour * lightColour * lightWattage * pow(cosAlpha, 5) / (distances[i] * distances[i]));
	}
	//gl_FragColor = texture2D(diffuse, texcoordShared) * vec4(lightColour, 1) * lightWattage /** clamp(dot(-lightDirection, normalShared), 0.1, 1.0)*/ / (distance * distance);
}