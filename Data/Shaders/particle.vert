attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aPoses;
attribute vec4 aTimes;
attribute vec4 aColors;
attribute vec3 aSizes;

varying vec3 varPos;
varying vec2 varTexCoord;
varying vec4 varColor;

uniform mat4 uView;
uniform mat4 uProjection;

uniform vec2 uSize;
uniform vec2 uStartSize;
uniform vec2 uFinalSize;
uniform vec2 uSubUV;
uniform float uScaleOL;
uniform float uBillboard;

#define ROWS 6
#define COLUMNS 8

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
	mat4 Rotation = rotationMatrix(vec3(0, 0, 1), aTimes.z / 90);
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

	vec3 pos = aPoses;

	float lifePercent = aTimes.x / aTimes.y;
	int frameNumber = int(aTimes.w);

	int frameHorizontal = frameNumber % int(uSubUV.x);
	int frameVertical = int(uSubUV.y) - int(frameNumber / uSubUV.x) - 1;

	float frame_X = 1.0 / uSubUV.x;
	float frame_Y = 1.0 / uSubUV.y;

	vec2 frame = vec2(aUV.x * frame_X + frame_X * frameHorizontal,
	                  aUV.y * frame_Y + frame_Y * frameVertical);

	if (uBillboard != 0.0)
	{
		Position = uProjection * (uView * vec4(pos, 1.0) + vec4(aPos, 0.0) * vec4(aSizes, 0.0) * Rotation);
	} else
	{
		Position = uProjection * uView * (vec4(pos, 1.0) + vec4(aPos * aSizes, 0.0) * Rotation);
	}

	varPos = pos + aPos;
	varTexCoord = frame;
	varColor = aColors;
}



