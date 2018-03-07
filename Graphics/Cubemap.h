/************************************************
*                   Cubemap.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <array>
#include <vector>
#include <string>
#include <GL/glew.h>

#include <RenderAPI/APIOpenGL.h>
#include <Graphics/Texture.h>
#include <System/Log.h>
#include <System/Serializer.h>

namespace Columbus
{

	class Cubemap
	{
	protected:
		C_Image mBitmaps[6];
		bool mInited = false;
	public:
		Cubemap();
		Cubemap(std::array<std::string, 6> aPath);
		Cubemap(std::string aPath);
		
		virtual void bind() const;
		virtual void samplerCube(int i) const;
		virtual void unbind() const;
		
		virtual bool load(std::array<std::string, 6> aPath);
		virtual bool load(std::string aFile);
		bool save(std::array<std::string, 6> aPath) const;
		
		~Cubemap();
	};

}
