#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uNormal;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
	texCoord = aUV;
	varNormal = normalize(vec3(uNormal * vec4(aNorm, 1.0)));
	varFragPos = vec3(vec4(aPos, 1.0));
}








