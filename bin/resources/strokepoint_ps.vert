#version 330

uniform mat4 mvMatrix;
uniform mat4 projectionMatrix;
uniform float screenWidth;

in vec3 position;
in float pointSize;
in float opacity;

void main(void)
{
	vec4 eyePos = mvMatrix * vec4(position, 1.0);
	vec4 projCorner = projectionMatrix * vec4(pointSize * 0.5, pointSize * 0.5, eyePos.z, eyePos.w);
	
	// in the NDC, screen width is in [-1, 1]
	// so actual pixel size is screenWidth * (2 * projCorner.x in NDC) / 2
	gl_PointSize = screenWidth * projCorner.x / projCorner.w;
	gl_Position = projectionMatrix * eyePos;
}
