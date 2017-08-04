#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;

uniform vec4 uPosition;
uniform vec2 uSize;

uniform int uRenderMode;

void main(void)
{
	vec3 CameraRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
	vec3 CameraUp = vec3(uView[0][1], uView[1][1], uView[2][1]);

	gl_Position = uProjection * uView * vec4(uPosition.xyz + aPos * vec3(uSize, 1.0), 1.0);

	varTexCoord = aUV;
}
