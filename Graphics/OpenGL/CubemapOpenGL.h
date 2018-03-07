#pragma once

#include <Graphics/Cubemap.h>

namespace Columbus
{

	class CubemapOpenGL : public Cubemap
	{
	private:
		unsigned int mID = 0;
	public:
		CubemapOpenGL();
		CubemapOpenGL(std::array<std::string, 6> aPath);
		CubemapOpenGL(std::string aPath);

		void bind() const override;
		void samplerCube(int i) const override;
		void unbind() const override;
		
		bool load(std::array<std::string, 6> aPath) override;
		bool load(std::string aPath) override;

		unsigned int getID() const; //Get OpenGL cubemap texture ID

		~CubemapOpenGL();
	};

}











