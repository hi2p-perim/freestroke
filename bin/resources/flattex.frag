#version 330

uniform sampler2D colorMap;
in vec2 vTexCoord;
out vec4 fragColor;

void main(void)
{
	fragColor = texture(colorMap, vTexCoord);
}