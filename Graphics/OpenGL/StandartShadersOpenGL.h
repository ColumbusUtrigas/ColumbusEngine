namespace Columbus
{

	const char* gSkyboxVertexShader =
		"#version 130\n"
		"in vec3 aPos;\n"
		"in vec2 aUV;\n"
		"in vec3 aNorm;\n"
		"out vec3 texCoord;\n"
		"uniform mat4 uViewProjection;"
		"void main()\n"
		"{\n"
		"gl_Position = uViewProjection * vec4(aPos, 1.0);\n"
		"texCoord = aPos;\n"
		"}\n";

	const char* gSkyboxFragmentShader = 
		"#version 130\n"
		"out vec4 FragColor;\n"
		"in vec3 texCoord;\n"
		"uniform samplerCube uSkybox;\n"
		"void main()\n"
		"{\n"
		"FragColor = texture(uSkybox, texCoord);\n"
		"}\n";

}






















