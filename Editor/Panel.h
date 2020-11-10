#pragma once

#include <Math/Vector2.h>
#include <string>
#include <string_view>

namespace Columbus::Editor
{

	class Panel
	{
	private:
		std::string _name;
		bool _menu = false;
		bool _padding = true;
	protected:
		virtual void DrawInternal() = 0;
	public:
		bool Opened = true;
	public:
		Panel(std::string_view name) : _name(name) {}

		void SetMenuBar(bool menu) { _menu = menu;  }
		void SetPadding(bool padding) { _padding = padding; }

		void Draw();

		virtual ~Panel() {}
	};

}
