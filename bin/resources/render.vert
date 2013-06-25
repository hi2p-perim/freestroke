#version 330

uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;

in vec3 position;
in vec3 normal;
out vec3 vNormal;

void main(void)
{
	vNormal = normalMatrix * normal;
	gl_Position = mvpMatrix * vec4(position, 1.0);
}