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
uniform vec2 uStartSize;
uniform vec2 uFinalSize;

//0..2 - position
//3 - time
//4 - ttl
//5 - isScaleOverLifetime
//6 - isBillboard
//7 - isGradient

uniform float Unif[8];

uniform int uRenderMode;

void main(void)
{
	mat4 ModelView = uView * mat4(1);

	ModelView[0][0] = 1;
	ModelView[0][1] = 0;
	ModelView[0][2] = 0;

	ModelView[1][0] = 0;
	ModelView[1][1] = 1;
	ModelView[1][2] = 0;

	ModelView[2][0] = 0;
	ModelView[2][1] = 0;
	ModelView[2][2] = 1;

	vec3 pos;
	pos.x = Unif[0];
	pos.y = Unif[1];
	pos.z = Unif[2];

	//gl_Position = uProjection * uView * vec4(uPosition.xyz + aPos * vec3(uSize, 1.0), 1.0);
	if (Unif[6] != 0.0)
	{
		if (Unif[5] != 0.0)
		{
			vec2 SizeOverLifetime = mix(uStartSize, uFinalSize, Unif[3] / Unif[4]);
			gl_Position = uProjection * (uView * vec4(pos, 1.0) + vec4(aPos * vec3(SizeOverLifetime, 1.0), 0.0));
		} else
		{
			gl_Position = uProjection * (uView * vec4(pos, 1.0) + vec4(aPos * vec3(uSize, 1.0), 0.0));
		}
	}
	else
	{
		if (Unif[5] != 0.0)
		{
			vec2 SizeOverLifetime = mix(uStartSize, uFinalSize, Unif[3] / Unif[4]);
			gl_Position = uProjection * uView * vec4(pos + aPos * vec3(SizeOverLifetime, 1.0), 1.0);
		} else
		{
			gl_Position = uProjection * uView * vec4(pos + aPos * vec3(uSize, 1.0), 1.0);
		}
	}

	varTexCoord = aUV;
	varTime = Unif[3];
	varTTL = Unif[4];
	varIsGradient = Unif[7];
}
