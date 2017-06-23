#pragma once
#include <cstdio>
#include <FreeImage.h>
#include <GL/glew.h>
#include <SDL.h>
#include <cstring>
#include <System.h>

namespace C
{

	class C_Texture
	{
	private:
		FIBITMAP* mBuffer = NULL;
		GLuint mID;
	public:
		C_Texture();
		C_Texture(const char* aPath, bool aSmooth = true);
		C_Texture(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		void load(const char* aPath, bool aSmooth = true);
		
		void loadNoMessage(const char* aPath, bool aSmooth = true);
		
		void loadFromMemory(const char* aData, size_t aSize, bool aSmooth = true);
		
		void load(const char* aData, const int aW, const int aH, bool aSmooth = true);
		
		void save(const char* aFile);
		
		void bind();
		
		static void unbind();
		
		inline int getID() {return mID;}
		
		void sampler2D(int a);
		
		~C_Texture();
	};

}











