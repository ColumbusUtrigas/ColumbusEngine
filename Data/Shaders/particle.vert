#version 130

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec3 varPos;
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
//8 - rotation

uniform float Unif[9];

uniform int uRenderMode;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main(void)
{
	mat4 Rotation = rotationMatrix(vec3(0, 0, 1), Unif[8] / 90);
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

	float lifePercent = Unif[3] / Unif[4];

	vec3 Pos = aPos;
	if (Unif[6] != 0.0)
	{
		if (Unif[5] != 0.0)
		{
			vec2 SizeOverLifetime = mix(uStartSize, uFinalSize, lifePercent);
			gl_Position = uProjection * (uView * vec4(pos, 1.0) + vec4(aPos, 0.0) * vec4(SizeOverLifetime, 1.0, 0.0) * Rotation);
		} else
		{
			gl_Position = Rotation * uProjection * (uView * vec4(pos, 1.0) + vec4(aPos * vec3(uSize, 1.0), 0.0));
		}
	}
	else
	{
		if (Unif[5] != 0.0)
		{
			vec2 SizeOverLifetime = mix(uStartSize, uFinalSize, lifePercent);
			gl_Position = uProjection * uView * (vec4(pos, 1.0) + vec4(aPos * vec3(SizeOverLifetime, 1.0), 0.0) * Rotation);
		} else
		{
			gl_Position = uProjection * uView * (vec4(pos, 1.0) + vec4(aPos * vec3(uSize, 1.0), 0.0) * Rotation);
		}
	}

	varPos = pos + aPos;
	varTexCoord = aUV;
	varTime = Unif[3];
	varTTL = Unif[4];
	varIsGradient = Unif[7];
}