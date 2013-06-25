#version 330

uniform vec4 emissionColor;
uniform vec4 diffuseColor;
uniform vec4 lightDir;

in vec3 vNormal;
out vec4 fragColor;

void main(void)
{
	vec3 nvNormal = normalize(vNormal);
	vec3 nLightDir = normalize(lightDir.xyz);
	fragColor = emissionColor + diffuseColor * max(dot(nLightDir, nvNormal), 0.0);
}