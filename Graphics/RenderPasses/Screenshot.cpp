#include "RenderPasses.h"
#include <Common/Float/Float.h>

#include <Lib/STB/stb_image_write.h>
#include <Lib/tinyexr/tinyexr.h>

namespace Columbus
{

	void ScreenshotPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef Texture)
	{
		if (View.ScreenshotPath != nullptr)
		{
			RenderPassParameters Parameters;
			Parameters.SubmitBeforeExecution = true;

			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Texture, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			Graph.AddPass("ScreenshotPass", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture, View](RenderGraphContext& Context)
			{
				SPtr<Texture2> TextureToUpload = Context.GetRenderGraphTexture(Texture);
				SPtr<CommandBufferVulkan> CmdBuf = Context.Device->CreateCommandBufferShared();
				SPtr<FenceVulkan> Fence = Context.Device->CreateFence(false);

				iVector2 Size = { (int)TextureToUpload->GetDesc().Width, (int)TextureToUpload->GetDesc().Height };

				BufferDesc Desc;
				Desc.HostVisible = true;
				Desc.Size = TextureToUpload->GetSize();
				Buffer* ScreenshotBuffer = Context.Device->CreateBuffer(Desc, nullptr);

				CmdBuf->Begin();
				CmdBuf->TransitionImageLayout(TextureToUpload.get(), VK_IMAGE_LAYOUT_GENERAL);
				CmdBuf->CopyImageToBuffer(TextureToUpload.get(), ScreenshotBuffer, { 0,0,0 }, { Size.X, Size.Y, 1 }, 0);
				CmdBuf->End();
				Context.Device->Submit(CmdBuf.get(), Fence, 0, nullptr, 0, nullptr);

				Context.Device->WaitForFence(Fence, UINT64_MAX);

				void* Data = Context.Device->MapBuffer(ScreenshotBuffer);

				if (View.ScreenshotHDR)
				{
					// original format is RGBA16F
					float* DataFloat = new float[Size.X * Size.Y * 4];
					short* DataF16 = (short*)Data;

					// RGBA16F -> RGBA32F
					for (int i = 0; i < Size.X * Size.Y * 4; i++)
					{
						Float16 fp16;
						fp16.u = DataF16[i];
						DataFloat[i] = Float16to32(fp16).f;
					}

					SaveEXR(DataFloat, Size.X, Size.Y, 4, 1, View.ScreenshotPath, nullptr);

					delete[] DataFloat;
				}
				else
				{
					// swapchain format
					ImageUtils::ImageBGRA2RGBA((u8*)Data, ScreenshotBuffer->GetSize());
					stbi_write_png(View.ScreenshotPath, Size.X, Size.Y, 4, Data, Size.X * 4);
				}

				Context.Device->UnmapBuffer(ScreenshotBuffer);
				Context.Device->DestroyBuffer(ScreenshotBuffer);
			});
		}
	}

}