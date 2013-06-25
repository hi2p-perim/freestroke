#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in int id;
layout (location = 3) in float size;

out VertexAttribute
{
	vec4 color;
	int id;
	float size;
} vertex;

void main()
{
	gl_Position = vec4(position, 1.0);
	vertex.color = color;
	vertex.id = id;
	vertex.size = size;
}
