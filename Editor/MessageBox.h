#pragma once

#include <Math/Vector2.h>
#include <string>
#include <vector>

namespace Columbus::Editor
{

	class MessageBox
	{
	private:
		bool Opened = false;
		// 1 - Cancel, 2 - No, 3 - Yes
		int Selected = 0;

		std::string _name;
		std::string _text;

		Vector2 Size;
	public:
		MessageBox(std::string Name, std::string Text, const Vector2& Size) :
			_name(std::move(Name)), _text(std::move(Text)), Size(Size) {}

		void Open() { Opened = true; Selected = 0; }
		void Close() { Opened = false; }
		void Reset() { Opened = false; Selected = 0;}
		int GetSelected() { return Selected; };
		int Draw();
	};

}
