/************************************************
*                  Shader.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Shader.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Shader::C_Shader(std::string aVert, std::string aFrag)
	{
		load(aVert, aFrag);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Shader::C_Shader(const char* aFile)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 3
	C_Shader::C_Shader()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Load shader from two files
	void C_Shader::load(std::string aVert, std::string aFrag)
	{
		GLuint program = glCreateProgram();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		const char* vertSource = C_ReadFile(aVert.c_str());
		const char* fragSource = C_ReadFile(aFrag.c_str());

		if(vertSource == nullptr)
		{
			C_Error("Shader not loaded %s", aVert);
			return;
		}

		if(fragSource == nullptr)
		{
			C_Error("Shader not loaded %s", aFrag);
			return;
		}

		glShaderSource(vertex, 1, &vertSource, NULL);
		glCompileShader(vertex);

		GLint result;
     	glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
		if(result == GL_FALSE)
		{
		    GLint lenght;
		    glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &lenght);
		    std::vector<char> error(lenght);
		    glGetShaderInfoLog(vertex, lenght, &lenght, &error[0]);
		    printf("%s\n", error.data());
		    glDeleteShader(vertex);
		    mID = 0;
		    return;
		}

		glShaderSource(fragment, 1, &fragSource, NULL);
		glCompileShader(fragment);

		GLint fragResult;
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &fragResult);
		if(fragResult == GL_FALSE)
		{
		    GLint lenght;
		    glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &lenght);
		    std::vector<char> error(lenght);
		    glGetShaderInfoLog(fragment, lenght, &lenght, &error[0]);
		    printf("%s\n", error.data());
		    glDeleteShader(fragment);
		    mID = 0;
		    return;
		}

		glAttachShader(program, vertex);
		glAttachShader(program, fragment);

		glLinkProgram(program);

		glBindAttribLocation(program, 0, "aPos");
		glBindAttribLocation(program, 1, "aUV");
		glBindAttribLocation(program, 2, "aNorm");
		glBindAttribLocation(program, 3, "aTang");
		glBindAttribLocation(program, 4, "aBitang");

		for (auto Attrib : mAttributes)
			glBindAttribLocation(program, Attrib.value, Attrib.name.c_str());

		glValidateProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		mID = program;

		C_Success("Shader loaded %s", aVert);
		C_Success("Shader loaded %s", aFrag);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Bind shader
	void C_Shader::bind() const
	{
		glUseProgram(mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::addAttribute(std::string aName, const int aValue)
	{
		mAttributes.emplace_back(aName, aValue);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set integer uniform
	void C_Shader::setUniform1i(std::string aName, const int aValue) const
	{
		if(mID != 0)
			glUniform1i(glGetUniformLocation(mID, aName.c_str()), aValue);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set float uniform
	void C_Shader::setUniform1f(std::string aName, const float aValue) const
	{
		if(mID != 0)
			glUniform1f(glGetUniformLocation(mID, aName.c_str()), aValue);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set 2-axis vector uniform
	void C_Shader::setUniform2f(std::string aName, const C_Vector2 aValue) const
	{
		if(mID != 0)
			glUniform2f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set 3-axis vector uniform
	void C_Shader::setUniform3f(std::string aName, const C_Vector3 aValue) const
	{
		if(mID != 0)
			glUniform3f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y, aValue.z);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set 3-axis vector uniform
	void C_Shader::setUniform4f(std::string aName, const C_Vector4 aValue) const
	{
		if(mID != 0)
			glUniform4f(glGetUniformLocation(mID, aName.c_str()), aValue.x, aValue.y, aValue.z, aValue.w);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set matrix uniform
	void C_Shader::setUniformMatrix(std::string aName, const float* aValue) const
	{
		if(mID != 0)
			glUniformMatrix4fv(glGetUniformLocation(mID, aName.c_str()), 1, GL_FALSE, aValue);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set uniform array
	void C_Shader::setUniformArrayf(std::string aName, const float aArray[], const size_t aSize) const
	{
		if (mID != 0)
			glUniform1fv(glGetUniformLocation(mID, aName.c_str()), aSize, aArray);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind shader
	void C_Shader::unbind()
	{
		glUseProgram(0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Shader::~C_Shader()
	{
		glDeleteProgram(mID);
	}

}
