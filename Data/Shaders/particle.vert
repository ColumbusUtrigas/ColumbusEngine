#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPos;
uniform vec3 uRandom;
uniform float uVel;
uniform float uAcc;
uniform float uTTL;
uniform float uTime;
uniform float uSize;
uniform int uRenderMode;

float rand(vec2 co)
{
	return fract(sin(dot(normalize(co.xy), vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void)
{
	float t = uTime;
	vec3 a = uPos + uRandom * uVel * t + uPos * uAcc * t * t / 2.0;
	a = aPos * uSize + a;

	if (uRenderMode == 0)
		gl_Position = uProjection * (uView * vec4(a.x,  a.y, a.z, 1.0));

	if (uRenderMode == 1)
		gl_Position = uProjection * uView * vec4(a, 1.0);

	varTexCoord = aUV;
}
