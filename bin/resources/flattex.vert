#version 330

uniform mat4 mvpMatrix;
in vec3 position;
in vec2 texcoord;
out vec2 vTexCoord;

void main(void)
{
	vTexCoord = texcoord;
	gl_Position = mvpMatrix * vec4(position, 1.0);
}

/*
in vec3 position;
out vec2 vTexCoord;

void main(void)
{
	vec2 Pos = sign(position.xy);
	gl_Position = vec4(Pos.xy, 0.0, 1.0);
	vTexCoord.x = 0.5 * (1.0 + Pos.x);
	vTexCoord.y = 0.5 * (1.0 + Pos.y);
}
*/