#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;
attribute vec3 aTang;
attribute vec3 aBitang;

varying vec3 varPos;
varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;
varying mat3 varTBN;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uNormal;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	texCoord = aUV;
	varNormal = normalize(vec3(uNormal * vec4(aNorm, 1.0)));
	varFragPos = vec3(uModel * vec4(aPos, 1.0));
	
	vec3 T = normalize(vec3(uNormal * vec4(aTang, 0.0)));
	vec3 B = normalize(vec3(uNormal * vec4(aBitang, 0.0)));
	vec3 N = normalize(vec3(uNormal * vec4(aNorm, 0.0)));

	varTBN = transpose(mat3(T, B, N));
}








