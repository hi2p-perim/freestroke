#version 330 core

in vec4 color;
in vec3 texcoord;
out vec4 fragColor;

uniform sampler2DArray brushMap;

void main()
{
	vec4 texcolor = texture(brushMap, texcoord);
	fragColor = color * vec4(1.0 - texcolor.xyz, texcolor.a);
	//fragColor = color;
}