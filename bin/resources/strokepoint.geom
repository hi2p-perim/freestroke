#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VertexAttribute
{
	vec4 color;
	int id;
	float size;
} vertex[];

out vec4 color;
out vec3 texcoord;

uniform mat4 mvMatrix;
uniform mat4 projectionMatrix;

void main()
{
	vec4 eyePos = mvMatrix * gl_in[0].gl_Position;
	float halfSize = vertex[0].size * 0.5;
	vec4 tmp;
	int i;

	tmp = eyePos;
	tmp.x += halfSize;
	tmp.y += halfSize;
	gl_Position = projectionMatrix * tmp;
	color = vertex[0].color;
	texcoord = vec3(1.0, 1.0, vertex[0].id);
	EmitVertex();

	tmp = eyePos;
	tmp.x -= halfSize;
	tmp.y += halfSize;
	gl_Position = projectionMatrix * tmp;
	color = vertex[0].color;
	texcoord = vec3(0.0, 1.0, vertex[0].id);
	EmitVertex();

	tmp = eyePos;
	tmp.x += halfSize;
	tmp.y -= halfSize;
	gl_Position = projectionMatrix * tmp;
	color = vertex[0].color;
	texcoord = vec3(1.0, 0.0, vertex[0].id);
	EmitVertex();

	tmp = eyePos;
	tmp.x -= halfSize;
	tmp.y -= halfSize;
	gl_Position = projectionMatrix * tmp;
	color = vertex[0].color;
	texcoord = vec3(0.0, 0.0, vertex[0].id);
	EmitVertex();

	EndPrimitive();
}
