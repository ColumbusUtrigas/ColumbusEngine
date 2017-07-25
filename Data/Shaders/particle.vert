#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPos;

uniform int uCount;

uniform vec3 uDirection;
uniform float uVel;
uniform float uAcc;
uniform float uTTL;
uniform float uTime;

uniform int uRenderMode;

void main(void)
{
	vec3 pos = uTime * uVel * uDirection;

	gl_Position = uProjection * uView * vec4(pos + aPos + uPos, 1.0);

	varTexCoord = aUV;
}
