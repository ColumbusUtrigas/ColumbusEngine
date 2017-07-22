#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPos;
uniform float uRandom;
uniform float uVel;
uniform float uAcc;
uniform float uTTL;
uniform float uTime;
uniform float uSize;
uniform int uRenderMode;

void main(void)
{
	float t = uTime;
	vec3 a = uPos + 1 * uVel * t + uPos * uAcc * t * t / 2.0;
	a = aPos * uSize + a;

	if (uRenderMode == 0)
		gl_Position = uProjection * (uView * vec4(a.x,  a.y, a.z, 1.0));

	if (uRenderMode == 1)
		gl_Position = uProjection * uView * vec4(a, 1.0);

	gl_Position = uProjection * uView * vec4(vec3(0.0, uTime, 0.0) + aPos, 1.0);

	//gl_Position = uProjection * uView * vec4(a, 1.0);

	varTexCoord = aUV;
}
