#extension GL_EXT_draw_instanced : enable

attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aNorm;
attribute vec3 aTang;

uniform mat4 uModels[5];
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

varying vec2 varUV;

void main()
{
	Position = uProjection * uView * uModels[gl_InstanceID] * vec4(aPos, 1.0);
	varUV = aUV;
}







