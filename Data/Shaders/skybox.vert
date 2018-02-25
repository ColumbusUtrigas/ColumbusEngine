attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;

varying vec3 texCoord;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	Position = uProjection * uView * vec4(aPos, 1.0);
	texCoord = aPos;
}




