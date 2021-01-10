#pragma once

#include <Graphics/Device.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
#include <RenderAPIOpenGL/OpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	struct PostEffect
	{
	public:
		static constexpr int TexturesCount = 4;

		Framebuffer* FB = nullptr;
		Texture* DepthTexture = nullptr;

		bool DepthTextureEnablement = false;
		bool DepthTextureMipmaps = false;
		Texture::Flags DepthTextureFlags;

		Texture* ColorTextures[TexturesCount] = { nullptr };
		bool ColorTexturesEnablement[TexturesCount];
		bool ColorTexturesMipmaps[TexturesCount];
		TextureFormat ColorTexturesFormats[TexturesCount];
		Texture::Flags ColorTextureFlags[TexturesCount];

		uint32 Multisampling = 0;
		bool Clear = true;
	private:
		Texture* PrevColorTextures[TexturesCount];
		Texture* PrevDepthTexture = nullptr;
		bool ColorTexturesAttached[TexturesCount];
		bool DepthTextureAttached = false;
		Texture::Flags PrevColorTextureFlags[TexturesCount];
		Texture::Flags PrevDepthTextureFlags;

		iVector2 _PrevSize;
		int32 _PrevMSAA;
	public:
		PostEffect()
		{
			if (gDevice != nullptr)
			{
				Texture::Flags Flags(Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Clamp);

				FB = gDevice->CreateFramebuffer();
				DepthTexture = nullptr;
				DepthTextureFlags = Flags;

				for (int i = 0; i < TexturesCount; i++)
				{
					ColorTextures[i] = nullptr;
					ColorTexturesEnablement[i] = false;
					ColorTexturesMipmaps[i] = false;
					ColorTexturesFormats[i] = TextureFormat::RGBA8;
					ColorTextureFlags[i] = Flags;

					PrevColorTextures[i] = nullptr;
					ColorTexturesAttached[i] = false;
				}
			}
		}

		void Bind(const Vector4& Color, const iVector2& Origin, const iVector2& Size)
		{
			Framebuffer::Attachment Attachments[TexturesCount] =
			{ Framebuffer::Attachment::Color0,
			  Framebuffer::Attachment::Color1,
			  Framebuffer::Attachment::Color2,
			  Framebuffer::Attachment::Color3 };

			FB->Bind();

			int32 ColorMS = OpenGL::GetMaxColorTextureSamples();
			int32 DepthMS = OpenGL::GetMaxDepthTextureSamples();

			if ((int32)Multisampling < ColorMS) ColorMS = (int32)Multisampling;
			if ((int32)Multisampling < DepthMS) DepthMS = (int32)Multisampling;

			if (Multisampling == 0)
			{
				ColorMS = 0;
				DepthMS = 0;
			}

			for (int i = 0; i < TexturesCount; i++)
			{
				if (ColorTexturesEnablement[i])
				{
					if (ColorTextures[i] == nullptr)
					{
						ColorTextures[i] = gDevice->CreateTexture();
					}

					if (_PrevSize != (Size - Origin) || _PrevMSAA != ColorMS || _PrevMSAA != DepthMS)
					{
						ColorTextures[i]->Create2D(TextureDesc(Size.X, Size.Y, 0, ColorMS, ColorTexturesFormats[i]));
					}

					if (!ColorTexturesAttached[i] || ColorTextureFlags[i] != PrevColorTextureFlags[i] || PrevColorTextures[i] != ColorTextures[i])
					{
						ColorTextures[i]->SetFlags(ColorTextureFlags[i]);
					}

					if (!ColorTexturesAttached[i] || PrevColorTextures[i] != ColorTextures[i])
					{
						FB->SetTexture2D(Attachments[i], ColorTextures[i]);
						ColorTexturesAttached[i] = true;
					}

					PrevColorTextureFlags[i] = ColorTextureFlags[i];
					PrevColorTextures[i] = ColorTextures[i];
				}
			}

			if (DepthTextureEnablement)
			{
				if (DepthTexture == nullptr)
				{
					DepthTexture = gDevice->CreateTexture();
				}

				if (_PrevSize != (Size - Origin) || _PrevMSAA != ColorMS || _PrevMSAA != DepthMS)
				{
					DepthTexture->Create2D(TextureDesc(Size.X, Size.Y, 0, DepthMS, TextureFormat::Depth24));
				}

				if (!DepthTextureAttached || DepthTexture != PrevDepthTexture || PrevDepthTextureFlags != DepthTextureFlags)
				{
					DepthTexture->SetFlags(DepthTextureFlags);
				}

				if (!DepthTextureAttached || PrevDepthTexture != DepthTexture)
				{
					FB->SetTexture2D(Framebuffer::Attachment::Depth, DepthTexture);
					DepthTextureAttached = true;
				}

				PrevDepthTexture = DepthTexture;
				PrevDepthTextureFlags = DepthTextureFlags;
			}
			
			glViewport(Origin.X, Origin.Y, Size.X, Size.Y);
			if (Clear)
			{
				glClearColor(Color.X, Color.Y, Color.Z, Color.W);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}

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

			_PrevSize = (Size - Origin);
			_PrevMSAA = ColorMS;

			glDrawBuffers(DrawBuffersNum, DrawBuffers);
		}

		void Mipmaps()
		{
			for (int i = 0; i < TexturesCount; i++)
			{
				if (ColorTexturesEnablement[i] && ColorTexturesMipmaps[i])
				{
					ColorTextures[i]->GenerateMipmap();
				}
			}

			if (DepthTextureEnablement && DepthTextureMipmaps)
			{
				DepthTexture->GenerateMipmap();
			}
		}

		void Unbind()
		{
			FB->Unbind();
		}

		~PostEffect()
		{
			delete FB;
			delete DepthTexture;

			for (int i = 0; i < TexturesCount; i++) delete ColorTextures[i];
		}
	};

}


