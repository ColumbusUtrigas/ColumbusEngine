#pragma once

#include <Graphics/Device.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
#include <GL/glew.h>

namespace Columbus
{

	struct PostEffect
	{
	public:
		static constexpr int TexturesCount = 4;
		/*static constexpr Framebuffer::Attachment Attachments[TexturesCount] =
		{ Framebuffer::Attachment::Color0,
		  Framebuffer::Attachment::Color1,
		  Framebuffer::Attachment::Color2,
		  Framebuffer::Attachment::Color3 };*/

		Framebuffer* FB = nullptr;
		Texture* DepthTexture = nullptr;

		bool DepthTextureEnablement = false;
		bool DepthTextureMipmaps = false;
		Texture::Flags DepthTextureFlags;

		Texture* ColorTextures[TexturesCount];
		bool ColorTexturesEnablement[TexturesCount];
		bool ColorTexturesMipmaps[TexturesCount];
		uint32 ColorTexturesMultisampling[TexturesCount];
		TextureFormat ColorTexturesFormats[TexturesCount];
		Texture::Flags ColorTextureFlags[TexturesCount];
	public:
		PostEffect()
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
				ColorTexturesMultisampling[i] = 0;
				ColorTexturesFormats[i] = TextureFormat::RGBA8;
				ColorTextureFlags[i] = Flags;
			}
		}

		void Bind(const Vector4& Color, const iVector2& Origin, const iVector2& Size)
		{
			Framebuffer::Attachment Attachments[TexturesCount] =
			{ Framebuffer::Attachment::Color0,
			  Framebuffer::Attachment::Color1,
			  Framebuffer::Attachment::Color2,
			  Framebuffer::Attachment::Color3 };

			for (int i = 0; i < TexturesCount; i++)
			{
				if (ColorTexturesEnablement[i])
				{
					if (ColorTextures[i] == nullptr)
					{
						ColorTextures[i] = gDevice->CreateTexture();
					}

					ColorTextures[i]->Create2D(Texture::Properties(Size.X, Size.Y, 0, ColorTexturesMultisampling[i], ColorTexturesFormats[i]));
					ColorTextures[i]->SetFlags(ColorTextureFlags[i]);
					FB->SetTexture2D(Attachments[i], ColorTextures[i]);
				}
			}

			if (DepthTextureEnablement)
			{
				if (DepthTexture == nullptr)
				{
					DepthTexture = gDevice->CreateTexture();
				}

				DepthTexture->Create2D(Texture::Properties(Size.X, Size.Y, 0, 0, TextureFormat::Depth24));
				DepthTexture->SetFlags(DepthTextureFlags);
				FB->SetTexture2D(Framebuffer::Attachment::Depth, DepthTexture);
			}

			FB->Prepare(Color, Origin, Size);

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



