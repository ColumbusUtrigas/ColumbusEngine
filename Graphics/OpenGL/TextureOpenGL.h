#pragma once

#include <Graphics/Texture.h>

namespace Columbus
{

	class C_TextureOpenGL : public C_Texture
	{
	private:
		unsigned int mID = 0;
	public:
		C_TextureOpenGL();
		C_TextureOpenGL(std::string aPath, bool aSmooth = true);
		C_TextureOpenGL(const char* aData, const int aW, const int aH, bool aSmooth = true);

		void load(std::string aPath, bool aSmooth = true) override;
		void load(const char* aData, const int aW, const int aH, bool aSmooth = true) override;
		void loadDepth(const char* aData, const int aW, const int aH, bool aSmooth = true) override;

		void setConfig(C_TextureConfig aConfig) override;
		void setSmooth(const bool aSmooth) override;
		void setAnisotropy(const unsigned int aAnisotropy) override;

		void bind() override;
		void unbind() override;

		void sampler2D(int a) override;
		void generateMipmap() override;

		std::string getType() override;
		unsigned int getID() const; //Get OpenGL texture ID

		~C_TextureOpenGL();
	};

}






