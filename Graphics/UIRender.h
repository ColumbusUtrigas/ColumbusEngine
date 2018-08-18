#pragma once

#include <GUI/VirtualInput.h>
#include <GUI/Widget.h>
#include <Core/Containers/Array.h>

#include <GL/glew.h>

namespace Columbus
{

	class UIRender
	{
	public:
		UIRender() {}

		virtual void Render(const Array<Widget*>& Widgets, const VirtualInput& VI)
		{
			glDepthMask(GL_FALSE);

			for (const auto& Element : Widgets)
			{
				if (Element != nullptr)
				{
					if (Element->Shader != nullptr)
					{
						if (!Element->Shader->IsCompiled())
						{
							Element->Shader->Compile();
						}

						Element->Shader->Bind();
						Element->Prepare();
					}

					Element->Render();

					if (Element->Shader != nullptr)
					{
						Element->Shader->Unbind();
					}
				}
			}

			glDepthMask(GL_TRUE);

			for (int i = 0; i < 1; i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		~UIRender() {}
	};

}


















