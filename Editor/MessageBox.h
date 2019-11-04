#pragma once

#include <Math/Vector2.h>
#include <Core/String.h>

namespace Columbus
{

	class MessageBox
	{
	private:
		bool Opened = false;
		// 1 - Cancel, 2 - No, 3 - Yes
		int Selected = 0;

		String Name;
		String Text;

		Vector2 Size;
	public:
		MessageBox(const Vector2& Size = Vector2()) : Name(" "), Text(""), Size(Size) {}
		MessageBox(const String& Name, const String& Text, const Vector2& Size = Vector2()) :
			Name(Name), Text(Text), Size(Size) {}

		void Open() { Opened = true; Selected = 0; }
		void Close() { Opened = false; }
		void Reset() { Opened = false; Selected = 0;}
		int GetSelected() { return Selected; };
		int Draw();
	};

}


