#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec2 varTexCoord;
varying float varTime;
varying float varTTL;
varying float varIsGradient;

uniform mat4 uView;
uniform mat4 uProjection;

uniform vec4 uPosition;
uniform vec2 uSize;

uniform float Unif[6];

uniform int uRenderMode;

void main(void)
{
	vec3 CameraRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
	vec3 CameraUp = vec3(uView[0][1], uView[1][1], uView[2][1]);

	vec3 pos;
	pos.x = Unif[0];
	pos.y = Unif[1];
	pos.z = Unif[2];

	//gl_Position = uProjection * uView * vec4(uPosition.xyz + aPos * vec3(uSize, 1.0), 1.0);
	gl_Position = uProjection * uView * vec4(pos + aPos * vec3(uSize, 1.0), 1.0);

	varTexCoord = aUV;
	varTime = Unif[3];
	varTTL = Unif[4];
	varIsGradient = Unif[5];
}
