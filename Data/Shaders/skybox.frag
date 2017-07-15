#version 130

varying vec3 texCoord;

uniform samplerCube uSkybox;

void main()
{
	gl_FragColor = textureCube(uSkybox, texCoord);
	//gl_FragColor = vec4(texCoord, 1.0);
}


