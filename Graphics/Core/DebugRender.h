#pragma once

#include <vector>
#include <Math/Matrix.h>
#include <Math/Vector4.h>

namespace Columbus
{

	enum class DebugRenderObjectType
	{
		Box,
	};

	struct DebugRenderObject
	{
		DebugRenderObjectType Type;
		Matrix Transform;

		Vector4 Colour = Vector4(1, 1, 1, 0.5f);
		Vector4 OutlineColour = Vector4(1, 1, 1, 1);

		bool DrawOutline = false;
		bool UseZTest = true;
	};

	struct DebugRender
	{
	public:
		std::vector<DebugRenderObject> Objects;

	public:
		void AddBox(const Matrix& Transform, Vector4 Colour, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Box,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
			};
			Objects.push_back(Object);
		}

		void AddBoxWithOutline(const Matrix& Transform, Vector4 Colour, Vector4 OutlineColour, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Box,
				.Transform = Transform,
				.Colour = Colour,
				.OutlineColour = OutlineColour,
				.DrawOutline = true,
				.UseZTest = ZTest,
			};
			Objects.push_back(Object);
		}

		void Clear()
		{
			Objects.clear();
		}
	};

}
