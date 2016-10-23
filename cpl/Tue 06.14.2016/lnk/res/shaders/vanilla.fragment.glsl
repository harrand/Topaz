#version 450

const int MAX_LIGHTS = 4;

varying vec2 texcoord0;
varying vec3 normal0;
varying vec3 position0;
varying mat4 v0;
uniform sampler2D diffuse;

struct Light
{
	vec3 pos;
	vec3 lightDirection;
	vec3 colour;
	float power;
};

//uniform Light lights[MAX_LIGHTS];

float distanceFromSource(Light source)
{
	return length(position0 - source.pos);
}

vec4 getTextureColour(vec2 texcoord)
{
	return texture2D(diffuse, texcoord);
}

vec4 getDiffuseColour(Light light)
{
	float diffuseComponent = clamp(dot(-(light.lightDirection), normal0), 0.1, 1.0);
	return (getTextureColour(texcoord0) * vec4(light.colour, 1) * light.power * diffuseComponent / pow(distanceFromSource(light), 2));
}

vec4 getSpecularColour(Light light)
{
	vec3 towardsEye = (v0 * vec4(0, 0, -1, 0)).xyz;
	vec3 reflection = reflect(-(light.lightDirection), normalize(v0 * vec4(normal0, 0.0)).xyz);
	float cosAlpha = clamp(dot(towardsEye, reflection), 0, 1);
	return (getTextureColour(texcoord0) * vec4(light.colour, 1) * light.power * pow(cosAlpha, 5) / pow(distanceFromSource(light), 2));
}

void main()
{
	// Hardcoded Light Sources here
	//Light test = Light(vec3(14, 10, 110), vec3(0, -1, 1), vec3(1, 1, 1), 200.0, v0, texcoord0, normal0, position0);
	
	gl_FragColor = vec4(0, 0, 0, 0);
	//gl_FragColor += getDiffuseColour(lights[0]) + getSpecularColour(lights[0]);
	//for(unsigned int i = 0; i < MAX_LIGHTS; i++)
	//{
	//	gl_FragColor += (getDiffuseColour(lights[i]) + getSpecularColour(lights[i]));
	//}
	
	//gl_FragColor = getDiffuseColour(test) + getSpecularColour(test);
}