#pragma once

#include <vector>
#include <Math/Matrix.h>
#include <Math/Vector4.h>

namespace Columbus
{

	enum class DebugRenderObjectType
	{
		Box,
		Tri,
	};

	struct DebugRenderObject
	{
		DebugRenderObjectType Type;
		Matrix Transform;

		Vector4 Colour = Vector4(1, 1, 1, 0.5f);
		Vector4 OutlineColour = Vector4(1, 1, 1, 1);

		bool DrawOutline = false;
		bool UseZTest = true;

		Vector3 Vertices[3]; // for triangle
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

		void AddBox(const Vector3& Position, const Vector3& Size, const Vector4& Colour, bool ZTest = true)
		{
			Matrix Transform;
			Transform.Translate(Position);
			Transform.Scale(Size);
			AddBox(Transform, Colour, ZTest);
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

		// a box between From and To, Width is in world units
		void AddLineFromTo(const Vector3& From, const Vector3& To, float Width, const Vector4& Colour, bool ZTest = true)
		{
			float Dist = From.Distance(To);
			Vector3 Center = (From + To) / 2;

			Vector3 Direction = (To - From).Normalized();

			Matrix Rot;
			Rot.OrthoNormalBasisFromVector(Direction);

			Matrix Transform;
			Transform.Scale(Vector3(Width, Dist, Width));
			Transform = Rot * Transform;
			Transform.Translate(Center);

			AddBox(Transform, Colour, ZTest);
		}

		void AddTri(const Vector3& A, const Vector3& B, const Vector3& C, const Vector4& Colour, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Tri,
				.Transform = Matrix(1),
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Vertices = { A, B, C },
			};
			Objects.push_back(Object);
		}

		void Clear()
		{
			Objects.clear();
		}
	};

}
