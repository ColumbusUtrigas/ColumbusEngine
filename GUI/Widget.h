#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/InterpolationCurve.h>

#include <Graphics/Shader.h>
#include <Graphics/Texture.h>

namespace Columbus
{

	class Widget
	{
	public:
		enum class Event
		{
			None,
			Down,
			Hover
		};
	protected:
		Event State = Event::None;
	public:
		Vector2 Position;
		Vector2 Size;

		Vector4 NoneColor = Vector4(1);
		Vector4 DownColor = Vector4(1);
		Vector4 HoverColor = Vector4(1);

		Vector4 Color;

		ShaderProgram* Shader = nullptr;
	public:
		Widget() {}
		Widget(const Vector2& InPosition, const Vector2& InSize) : Position(InPosition), Size(InSize) {}

		void Update(const VirtualInput& VI, float TimeTick = 0.016f)
		{
			State = Event::None;

			if (IsHover(VI))
			{
				State = Event::Hover;

				if (VI.Mouse.Left)
				{
					State = Event::Down;
				}
			}

			switch (State)
			{
				case Event::None:  Color = NoneColor;  break;
				case Event::Hover: Color = HoverColor; break;
				case Event::Down:  Color = DownColor;  break;
			}
		}

		virtual void Prepare() = 0;
		virtual void Render() = 0;

		virtual bool IsHover(const VirtualInput& VI) const
		{
			Vector2 ULCorner = Position - Size * 0.5f;
			Vector2 BRCorner = Position + Size * 0.5f;
			Vector2 MousePosition = (VI.Mouse.Coords * (1.0f / VI.Screen.Size) - Vector2(0.5f)) * 2.0f;

			return MousePosition.X >= ULCorner.X && MousePosition.X <= BRCorner.X &&
			       MousePosition.Y >= ULCorner.Y && MousePosition.Y <= BRCorner.Y && VI.Mouse.Enabled;
		}

		Event GetState() const { return State; }

		~Widget() {}
	};

}
