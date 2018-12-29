#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <typeinfo>

#include <Graphics/Device.h>
#include <Graphics/Shader.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
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

	#include <GL/glew.h>

	struct BasePostEffect
	{
	private:
		iVector2 PreviousSize;
		bool SizeChanged = false;
	public:
		static constexpr int TexturesCount = 4;
		static constexpr Framebuffer::Attachment Attachments[TexturesCount] =
		{ Framebuffer::Attachment::Color0,
		  Framebuffer::Attachment::Color1,
		  Framebuffer::Attachment::Color2,
		  Framebuffer::Attachment::Color3 };

		Framebuffer* FB = nullptr;
		Texture* DepthTexture = nullptr;

		bool DepthTextureEnablement = false;

		Texture* ColorTextures[TexturesCount];
		bool ColorTexturesEnablement[TexturesCount];
		TextureFormat ColorTexturesFormats[TexturesCount];
	public:
		BasePostEffect()
		{
			FB = gDevice->createFramebuffer();
			DepthTexture = nullptr;
			//DepthTexture = gDevice->CreateTexture();
			/*DepthTexture->Create2D(Texture::Properties(100, 100, 0, TextureFormat::Depth16));
			FB->setTexture2D(Framebuffer::Attachment::Depth, DepthTexture);*/

			for (int i = 0; i < TexturesCount; i++)
			{
				//ColorTextures[i] = gDevice->CreateTexture();
				ColorTextures[i] = nullptr;
				ColorTexturesEnablement[i] = false;
				ColorTexturesFormats[i] = TextureFormat::RGBA8;
				/*ColorTextures[i]->Create2D(Texture::Properties(100, 100, 0, TextureFormat::RGBA8));
				FB->setTexture2D(Attachments[i], ColorTextures[i]);*/
			}
		}

		void Bind(const Vector4& Color, const iVector2& Size)
		{
			if (Size != PreviousSize)
			{
				for (int i = 0; i < TexturesCount; i++)
				{
					if (ColorTexturesEnablement[i])
					{
						if (ColorTextures[i] == nullptr)
						{
							ColorTextures[i] = gDevice->CreateTexture();
						}

						ColorTextures[i]->Create2D(Texture::Properties(Size.X, Size.Y, 0, ColorTexturesFormats[i]));
						FB->setTexture2D(Attachments[i], ColorTextures[i]);
					}
				}

				if (DepthTextureEnablement)
				{
					if (DepthTexture == nullptr)
					{
						DepthTexture = gDevice->CreateTexture();
					}

					DepthTexture->Create2D(Texture::Properties(Size.X, Size.Y, 0, TextureFormat::Depth24));
					FB->setTexture2D(Framebuffer::Attachment::Depth, DepthTexture);
				}

				SizeChanged = true;
			}

			PreviousSize = Size;

			FB->prepare(Color, PreviousSize);

			GLenum DrawBuffers[TexturesCount];
			int DrawBuffersNum = 0;

			for (int i = 0; i < TexturesCount; i++)
			{
				if (ColorTexturesEnablement[i])
				{
					DrawBuffers[DrawBuffersNum] = GL_COLOR_ATTACHMENT0 + i;
					DrawBuffersNum++;
				}
			}

			glDrawBuffers(DrawBuffersNum, DrawBuffers);
		}

		void Unbind()
		{
			if (SizeChanged)
			{
				for (int i = 0; i < TexturesCount; i++)
				{
					if (ColorTexturesEnablement[i])
					{
						ColorTextures[i]->generateMipmap();
					}
				}

				if (DepthTextureEnablement)
				{
					DepthTexture->generateMipmap();
				}
			}

			SizeChanged = false;

			FB->unbind();
		}

		~BasePostEffect()
		{
			delete FB;
			delete DepthTexture;

			for (int i = 0; i < TexturesCount; i++) delete ColorTextures[i];
		}
	};
	
	class PostEffect
	{
	public:
		ShaderProgram* Shader = nullptr;
		Framebuffer* FB = nullptr;
		Texture* ColorTexture = nullptr;
		Texture* NormalTexture = nullptr;
		Texture* DepthTexture = nullptr;

		Vector2 PreviousSize;
		bool SizeChanged = false;

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



