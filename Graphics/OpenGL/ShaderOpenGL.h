#pragma once

#include <Graphics/Shader.h>

namespace Columbus
{

	class C_ShaderOpenGL : public C_Shader
	{
	private:
		unsigned int mID = 0;
	public:
		C_ShaderOpenGL();
		C_ShaderOpenGL(std::string aVert, std::string aFrag);

		bool load(std::string aVert, std::string aFrag) override;
		bool compile() override;

		void bind() const override;
		void unbind() const override;

		void setUniform1i(std::string aName, const int aValue) const override; 
		void setUniform1f(std::string aName, const float aValue) const override;
		void setUniform2f(std::string aName, const C_Vector2 aValue) const override;
		void setUniform3f(std::string aName, const C_Vector3 aValue) const override;
		void setUniform4f(std::string aName, const C_Vector4 aValue) const override;
		void setUniformMatrix(std::string aName, const float* aValue) const override;
		void setUniformArrayf(std::string aName, const float aArray[], const size_t aSize) const override;

		~C_ShaderOpenGL();
	};

}


