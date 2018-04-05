#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	//STANDART MESH VERTEX SHADER
	const std::string gMeshVertexShader =
		"#version 130\n"
		"attribute vec3 aPos;\n"
		"attribute vec2 aUV;\n"
		"attribute vec3 aNorm;\n"
		"attribute vec3 aTang;\n"
		"attribute vec3 aBitang;\n"
		"varying vec3 varPos;\n"
		"varying vec2 varUV;\n"
		"varying vec3 varNormal;\n"
		"varying vec3 varTangent;\n"
		"varying vec3 varBitangent;\n"
		"varying vec3 varFragPos;\n"
		"varying mat3 varTBN;\n"
		"uniform mat4 uModel;\n"
		"uniform mat4 uView;\n"
		"uniform mat4 uProjection;\n"
		"uniform mat4 uNormal;\n"
		"void main()\n"
		"{\n"
		"gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
		"varPos = vec3(uModel * vec4(aPos, 1.0));\n"
		"varUV = aUV;\n"
		"varNormal = normalize(vec3(uNormal * vec4(aNorm, 0.0)));\n"
		"varTangent = normalize(vec3(uNormal * vec4(aTang, 0.0)));\n"
		"varBitangent = cross(varNormal, varTangent);\n"
		"varFragPos = vec3(uModel * vec4(aPos, 1.0));\n"
		"varTBN = transpose(mat3(varTangent, varBitangent, varNormal));\n"
		"}\n";

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//STANDART SKYBOX VERTEX SHADER
	const std::string gSkyVertexShader =
		"#version 130\n"
		"attribute vec3 aPos;\n"
		"attribute vec2 aUV;\n"
		"attribute vec3 aNorm;\n"
		"varying vec3 texCoord;\n"
		"uniform mat4 uView;\n"
		"uniform mat4 uProjection;\n"
		"void main()\n"
		"{\n"
		"gl_Position = uProjection * uView * vec4(aPos, 1.0);\n"
		"texCoord = aPos;\n"
		"}\n";
	//////////////////////////////////////////////////////////////////////////////
	//STANDART SKYBOX FRAGMENT SHADER
	const std::string gSkyFragmentShader =
		"#version 130\n"
		"varying vec3 texCoord;\n"
		"uniform samplerCube uSkybox;\n"
		"void main()\n"
		"{\n"
		"gl_FragColor = textureCube(uSkybox, texCoord);\n"
		"}\n";
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ShaderOpenGL::ShaderOpenGL()
	{
		mID = glCreateProgram();
	}
	//////////////////////////////////////////////////////////////////////////////
	ShaderOpenGL::ShaderOpenGL(std::string aVert, std::string aFrag)
	{
		mID = glCreateProgram();
		load(aVert, aFrag);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ShaderOpenGL::load(std::string aVert, std::string aFrag)
	{
		std::string vertSource;
		std::string fragSource;

		//Vertex shader loading
		if (aVert == "STANDART_MESH_VERTEX")
		{
			vertSource = gMeshVertexShader;
		}
		else if (aVert == "STANDART_SKY_VERTEX")
		{
			vertSource = gSkyVertexShader;
		}
		else
		{
			File vert(aVert, "rt");

			if (!vert.isOpened())
			{
				Log::error("Shader not loaded: " + aVert);
				return false;
			}

			char* vertFile = new char[vert.getSize() + 1];
			vertFile[vert.getSize()] = 0;

			vert.readBytes(vertFile, vert.getSize());
			vert.close();

			mBuilder.build(vertFile, E_SHADER_TYPE_VERTEX);
			vertSource = mBuilder.getShader();
			delete[] vertFile;
		}
		//Fragment shader loading
		if (aFrag == "STANDART_SKY_FRAGMENT")
		{
			fragSource = gSkyFragmentShader;
		}
		else
		{
			File frag(aFrag, "rt");

			if (!frag.isOpened())
			{
				Log::error("Shader not loaded: " + aFrag);
				return false;
			}

			char* fragFile = new char[frag.getSize() + 1];
			fragFile[frag.getSize()] = 0;

			frag.readBytes(fragFile, frag.getSize());
			frag.close();

			mBuilder.build(fragFile, E_SHADER_TYPE_FRAGMENT);
			fragSource = mBuilder.getShader();
			delete[] fragFile;
		}

		if (vertSource.empty())
		{
			Log::error("Shader not loaded: " + aVert);
			return false;
		}

		if (fragSource.empty())
		{
			Log::error("Shader not loaded: " + aFrag);
			return false;
		}

		mVertShaderPath = aVert;
		mFragShaderPath = aFrag;

		mVertShaderSource = vertSource;
		mFragShaderSource = fragSource;
		mLoaded = true;

		Log::success("Shader loaded: " + aVert);
		Log::success("Shader loaded: " + aFrag);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ShaderOpenGL::compile()
	{
		if (!mLoaded) return false;

		uint32 vertex = glCreateShader(GL_VERTEX_SHADER);
		uint32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
		int32 length, result;
		char* error;

		auto VertexSource = mVertShaderSource.c_str();
		auto FragmentSource = mFragShaderSource.c_str();

		glShaderSource(vertex, 1, &VertexSource, NULL);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE)
		{
			glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);
			error = new char[length];
			glGetShaderInfoLog(vertex, length, &length, error);
			printf("%s\n", error);
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			mID = 0;

			delete[] error;
			return false;
		}

		glShaderSource(fragment, 1, &FragmentSource, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE)
		{
			glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);
			error = new char[length];
			glGetShaderInfoLog(fragment, length, &length, error);
			printf("%s\n", error);
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			mID = 0;

			delete error;
			return false;
		}

		glAttachShader(mID, vertex);
		glAttachShader(mID, fragment);

		for (auto Attrib : mAttributes)
			glBindAttribLocation(mID, Attrib.value, Attrib.name.c_str());

		glLinkProgram(mID);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		mCompiled = true;

		Log::success("Shader compiled: " + mVertShaderPath);
		Log::success("Shader compiled: " + mFragShaderPath);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void ShaderOpenGL::bind() const
	{
		if (mCompiled) glUseProgram(mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ShaderOpenGL::unbind() const
	{
		glUseProgram(0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void ShaderOpenGL::setUniform1i(std::string aName, const int aValue) const
	{
		if (mID != 0 && mCompiled) glUniform1i(glGetUniformLocation(mID, aName.c_str()), aValue);
	}

	void ShaderOpenGL::setUniform1f(std::string aName, const float aValue) const
	{
		if (mID != 0 && mCompiled) glUniform1f(glGetUniformLocation(mID, aName.c_str()), aValue);
	}

	void ShaderOpenGL::setUniform2f(std::string aName, const Vector2 aValue) const
	{
		if (mID != 0 && mCompiled) glUniform2f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y);
	}

	void ShaderOpenGL::setUniform3f(std::string aName, const Vector3 aValue) const
	{
		if (mID != 0 && mCompiled) glUniform3f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y, aValue.z);
	}

	void ShaderOpenGL::setUniform4f(std::string aName, const Vector4 aValue) const
	{
		if (mID != 0 && mCompiled) glUniform4f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y, aValue.z, aValue.w);
	}
	
	void ShaderOpenGL::setUniformMatrix(std::string aName, const float* aValue) const
	{
		if (mID != 0 && mCompiled) glUniformMatrix4fv(glGetUniformLocation(mID, aName.c_str()), 1, GL_FALSE, aValue);
	}
	
	void ShaderOpenGL::setUniformArrayf(std::string aName, const float aArray[], const size_t aSize) const
	{
		if (mID != 0 && mCompiled) glUniform1fv(glGetUniformLocation(mID, aName.c_str()), aSize, aArray);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ShaderOpenGL::~ShaderOpenGL()
	{
		glDeleteProgram(mID);
	}

}








