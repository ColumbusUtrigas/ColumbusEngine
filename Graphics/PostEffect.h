#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <typeinfo>

#include <RenderAPI/APIOpenGL.h>
#include <RenderAPI/Buffer.h>
#include <Graphics/Shader.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
#include <Graphics/Renderbuffer.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Core/Containers/Array.h>

namespace Columbus
{

	struct PostEffectAttributeInt
	{
		std::string name;
		int value;
		PostEffectAttributeInt(std::string aName, int aValue) : name(aName), value(aValue) {}
	};
	
	struct PostEffectAttributeFloat
	{
		std::string name;
		float value;
		PostEffectAttributeFloat(std::string aName, float aValue) : name(aName), value(aValue) {}
	};
	
	struct PostEffectAttributeVector2
	{
		std::string name;
		Vector2 value;
		PostEffectAttributeVector2(std::string aName, Vector2 aValue) : name(aName), value(aValue) {}
	};
	
	struct PostEffectAttributeVector3
	{
		std::string name;
		Vector3 value;
		PostEffectAttributeVector3(std::string aName, Vector3 aValue) : name(aName), value(aValue) {}
	};
	
	struct PostEffectAttributeVector4
	{
		std::string name;
		Vector4 value;
		PostEffectAttributeVector4(std::string aName, Vector4 aValue) : name(aName), value(aValue) {}
	};
	
	class PostEffect
	{
	private:
		ShaderProgram* Shader = nullptr;
		Framebuffer* mFB = nullptr;
		Texture* mTB = nullptr;
		Texture* NormalMap = nullptr;
		Texture* mDepth = nullptr;

		Vector2 PreviousSize;

		uint32 VBO;
		uint32 IBO;
		uint32 VAO;

		std::vector<std::string> AttributeNames;
		std::vector<PostEffectAttributeInt> mAttribsInt;
		std::vector<PostEffectAttributeFloat> mAttribsFloat;
		std::vector<PostEffectAttributeVector2> mAttribsVector2;
		std::vector<PostEffectAttributeVector3> mAttribsVector3;
		std::vector<PostEffectAttributeVector4> mAttribsVector4;
	public:
		PostEffect();

		void SetShader(ShaderProgram* InShader);
		void AddAttributeName(std::string Name);
		void addAttrib(PostEffectAttributeInt aAttrib);
		void addAttrib(PostEffectAttributeFloat aAttrib);
		void addAttrib(PostEffectAttributeVector2 aAttrib);
		void addAttrib(PostEffectAttributeVector3 aAttrib);
		void addAttrib(PostEffectAttributeVector4 aAttrib);
		void clearAttribs();

		void Bind(Vector4 ClearColor, Vector2 ContextSize);
		void Render();
		void Unbind();

		~PostEffect();
	};

}



