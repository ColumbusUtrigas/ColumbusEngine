varying vec3 texCoord;

uniform samplerCube uSkybox;

void main()
{
	FragColor = textureCube(uSkybox, texCoord);
}


