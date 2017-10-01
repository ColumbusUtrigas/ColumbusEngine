#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;
attribute vec3 aTang;
attribute vec3 aBitang;

varying vec3 varPos;
varying vec2 varUV;
varying vec3 varNormal;
varying vec3 varTangent;
varying vec3 varBitangent;
varying vec3 varFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uNormal;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);

	varPos = vec3(uModel * vec4(aPos, 1.0));
	varUV = aUV;
	varNormal = normalize(vec3(uNormal * vec4(aNorm, 0.0)));

	varTangent = normalize(vec3(uNormal * vec4(aTang, 0.0)));
	varBitangent = cross(varNormal, varTangent);

	varFragPos = vec3(uModel * vec4(aPos, 1.0));
}




