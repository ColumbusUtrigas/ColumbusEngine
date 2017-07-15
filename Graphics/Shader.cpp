#include "Shader.h"

namespace C
{

	C_Shader::C_Shader(const char* aVert, const char* aFrag)
	{
		load(aVert, aFrag);
	}
	
	void C_Shader::load(const char* aVert, const char* aFrag)
	{
		GLuint program = glCreateProgram();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		const char* vertSource = C_ReadFile(aVert);
		const char* fragSource = C_ReadFile(aFrag);
		
		if(vertSource == nullptr)
		{
			printf("Error: Shader not loaded: %s\n", aVert);
			return;
		}
			
		if(fragSource == nullptr)
		{
			printf("Error: Shader not loaded: %s\n", aFrag);
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
		
		glValidateProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		mID = program;
		
		printf("Shader successfuly loaded: %s\n", aVert);
		printf("Shader successfuly loaded: %s\n", aFrag);
	}
	
	C_Shader::C_Shader(const char* aFile)
	{		
		/*const char* vertSource;
		const char* fragSource;
		
		C_ZIPFile zip(aFile);
		
		std::string asdsa = aFile;
		std::string vs = asdsa.substr(asdsa.find_last_of('/', asdsa.size()) + 1, asdsa.size() - 4);
			
		for(int i = 0; i < 4; i++)
			vs.pop_back();
		
		vs += ".vert";
		
		std::string fs = asdsa.substr(asdsa.find_last_of('/', asdsa.size()) + 1, asdsa.size() - 4);
		
		for(int i = 0; i < 4; i++)
			fs.pop_back();
		
		fs += ".frag";
		
		
		vertSource = zip.read(vs.c_str());
		fragSource = zip.read(fs.c_str());
		
		FILE* vsfile = fopen("vertex_shader_shd_vert_tmp", "wt");
		fprintf(vsfile, "%s", vertSource);
		fclose(vsfile);
		
		FILE* fsfile = fopen("fragment_shader_shd_frag_tmp", "wt");
		fprintf(fsfile, "%s", fragSource);
		fclose(fsfile);
		
		load("vertex_shader_shd_vert_tmp", "fragment_shader_shd_frag_tmp");
		C_DeleteFile("vertex_shader_shd_vert_tmp");
		C_DeleteFile("fragment_shader_shd_frag_tmp");
		
		printf("\x1b[32;1mShader successfuly loaded: \x1b[0;1m%s\x1b[0m\n", aFile);*/
	}
	
	void C_Shader::setUniform1i(const char* aName, const int aValue)
	{
		if(mID != 0)
			glUniform1i(glGetUniformLocation(mID, aName), aValue);
	}
	
	void C_Shader::setUniform1f(const char* aName, const float aValue)
	{
		if(mID != 0)
			glUniform1f(glGetUniformLocation(mID, aName), aValue);
	}
	
	void C_Shader::setUniform2f(const char* aName, const C_Vector2 aValue)
	{
		if(mID != 0)
			glUniform2f(glGetUniformLocation(mID, aName), aValue.x, aValue.y);
	}
	
	void C_Shader::setUniform3f(const char* aName, const C_Vector3 aValue)
	{
		if(mID != 0)
			glUniform3f(glGetUniformLocation(mID, aName), aValue.x, aValue.y, aValue.z);
	}
	
	void C_Shader::setUniform4f(const char* aName, const C_Vector4 aValue)
	{
		if(mID != 0)
			glUniform4f(glGetUniformLocation(mID, aName), aValue.x, aValue.y, aValue.z, aValue.w);
	}
	
	void C_Shader::setUniformMatrix(const char* aName, const float* aValue)
	{
		if(mID != 0)
			glUniformMatrix4fv(glGetUniformLocation(mID, aName), 1, GL_FALSE, aValue);
	}
	
	void C_Shader::bind()
	{
		glUseProgram(mID);
	}
	
	void C_Shader::unbind()
	{
		glUseProgram(0);
	}
	
	C_Shader::~C_Shader()
	{
	
	}

}










