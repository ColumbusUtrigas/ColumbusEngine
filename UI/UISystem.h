#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <vector>

namespace Columbus
{

	struct UIImage
	{
		Texture2* Img    = nullptr;
		Vector4 Colour   = Vector4(1,1,1,1);
		Vector2 Position = Vector2(0,0);
		Vector2 Size     = Vector2(1,1);

		bool bPreserveAspectRatio = false;

		Vector2 GetActualRenderSize(float ScreenAspectRatio) const
		{
			if (!bPreserveAspectRatio)
				return Size;

			float ImageAspectRatio = (float)Img->GetDesc().Width / (float)Img->GetDesc().Height;

			// TODO:
			return Size;
		}
	};

	struct UISystem
	{
	public:
		std::vector<UIImage*> Images;
	};


}