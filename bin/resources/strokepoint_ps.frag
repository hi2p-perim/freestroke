#version 330

uniform sampler2D colorMap;
uniform vec3 color;

out vec4 fragColor;

void main(void)
{
	vec4 texcolor = texture(colorMap, gl_PointCoord);
	fragColor = vec4(color * (1.0 - texcolor.xyz), texcolor.a);
}