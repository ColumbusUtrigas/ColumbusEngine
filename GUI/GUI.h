#pragma once

#include <Core/Containers/Array.h>
#include <GUI/Widget.h>
#include <Graphics/UIRender.h>

namespace Columbus
{

	class GUI
	{
	private:
		Array<Widget*> Widgets;
		UIRender UI;
	public:
		GUI() {}

		void Add(Widget* Element) { Widgets.Add(Element); }

		void Render(const VirtualInput& VI)
		{
			for (auto& Element : Widgets)
			{
				if (Element != nullptr)
				{
					Element->Update(VI);
				}
			}

			UI.Render(Widgets, VI);
		}

		~GUI() {}
	};

}





















