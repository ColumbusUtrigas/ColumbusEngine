#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;

uniform mat4 uView;
uniform mat4 uProjection;
uniform vec3 uPos;
uniform vec3 uVel;
uniform vec3 uAcc;
uniform float uTTL;
uniform int uTime;

void main(void)
{
	float t = uTime;
	vec3 a = uPos + uVel * t + uAcc * t * t / 2.0;
	
	gl_Position = uProjection * uView * vec4(aPos + a - vec3(0, 2, 0), 1.0);
	
	varTexCoord = aUV;
}


