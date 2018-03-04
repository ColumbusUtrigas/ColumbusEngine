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

	//STANDART MESh VERTEX SHADER
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
	C_Shader::C_Shader(std::string aVert, std::string aFrag) :
		mLoaded(false),
		mCompiled(false)
	{
		load(aVert, aFrag);
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Shader::C_Shader() :
		mLoaded(false),
		mCompiled(false)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Load shader from two files
	bool C_Shader::load(std::string aVert, std::string aFrag)
	{
		return false;
		/*std::string vertSource;
		std::string fragSource;

		//Vertex shader loading
		if (aVert == "STANDART_MESH_VERTEX")
		{
			vertSource = gMeshVertexShader;
		} else if (aVert == "STANDART_SKY_VERTEX")
		{
			vertSource = gSkyVertexShader;
		}
		else
		{
			C_File vert(aVert, "rt");
			if (!vert.isOpened())
			{ C_Log::error("Shader not loaded: " + aVert); return false; }
			vert.close();

			mBuilder.build(C_ReadFile(aVert.c_str()), E_SHADER_TYPE_VERTEX);
			vertSource = mBuilder.getShader();
		}
		//Fragment shader loading
		if (aFrag == "STANDART_SKY_FRAGMENT")
		{
			fragSource = gSkyFragmentShader;
		} else
		{
			C_File frag(aFrag, "rt");

			if (!frag.isOpened())
			{ C_Log::error("Shader not loaded: " + aFrag); return false; }
			frag.close();

			mBuilder.build(C_ReadFile(aFrag.c_str()), E_SHADER_TYPE_FRAGMENT);
			fragSource = mBuilder.getShader();
		}

		if (vertSource.empty())
		{ C_Log::error("Shader not loaded: " + aVert); return false; }

		if (fragSource.empty())
		{ C_Log::error("Shader not loaded: " + aFrag); return false; }

		mVertShaderPath = aVert;		
		mFragShaderPath = aFrag;

		mVertShaderSource = vertSource;
		mFragShaderSource = fragSource;
		mLoaded = true;

		C_Log::success("Shader loaded: " + aVert);
		C_Log::success("Shader loaded: " + aFrag);

		return true;*/
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Shader::compile()
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_Shader::isCompiled() const
	{
		return mCompiled;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::bind() const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::unbind() const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::addAttribute(std::string aName, const int aValue)
	{
		mAttributes.emplace_back(aName, aValue);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniform1i(std::string aName, const int aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniform1f(std::string aName, const float aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniform2f(std::string aName, const C_Vector2 aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniform3f(std::string aName, const C_Vector3 aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniform4f(std::string aName, const C_Vector4 aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniformMatrix(std::string aName, const float* aValue) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Shader::setUniformArrayf(std::string aName, const float aArray[], const size_t aSize) const
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Shader::~C_Shader()
	{

	}

}
