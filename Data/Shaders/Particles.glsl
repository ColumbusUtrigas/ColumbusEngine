#include "Data/Shaders/Common.glsl"

#shader vertex

#attribute vec3 Vertices  0
#attribute vec2 Texcoords 1
#attribute vec3 Positions 2
#attribute vec3 Sizes     3
#attribute vec4 Colors    4
#attribute vec2 OtherData 5

#uniform mat4 View
#uniform mat4 Projection
#uniform mat4 Billboard
#uniform vec2 Frame

out vec4 Color;
out vec2 UV;

mat4 RotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return  mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	             oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	             oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	             0.0,                                0.0,                                0.0,                                1.0);
}

void main(void)
{
	mat4 Rotation = RotationMatrix(vec3(0, 0, 1), 0.011111 * OtherData.x);
	vec4 VertexPosition = vec4(Vertices * Sizes, 1);
	vec4 WorldPosition = vec4(Positions, 0);

	SV_Position = Projection * (View * (Billboard * (VertexPosition * Rotation) + WorldPosition));
	UV = ExtractSubUV(int(OtherData.y), Texcoords, Frame);
	Color = Colors;
}

#shader fragment

#uniform Texture2D Texture

in vec4 Color;
in vec2 UV;

void main(void)
{
	RT0 = Sample2D(Texture, UV) * Color;
}


