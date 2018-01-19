#version 130

varying vec3 texCoord;

uniform samplerCube uSkybox;

void main()
{
	gl_FragColor = textureCube(uSkybox, texCoord);
}


