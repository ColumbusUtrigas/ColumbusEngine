#pragma once

#include <vector>
#include <Math/Matrix.h>
#include <Math/Vector4.h>
#include <Graphics/Core/Buffer.h>

namespace Columbus
{

	enum class DebugRenderObjectType
	{
		Box,
		Tri,
		Sphere,
		Cone,
		Cylinder,
		Capsule,
		Mesh,
	};

	struct DebugRenderObject
	{
		DebugRenderObjectType Type;
		Matrix Transform;

		Vector4 Colour = Vector4(1, 1, 1, 0.5f);
		Vector4 OutlineColour = Vector4(1, 1, 1, 1);

		bool DrawOutline = false;
		bool UseZTest = true;
		bool Wireframe = false;
		bool UseUnjitteredCamera = true;

		Vector3 Vertices[3]; // for triangle

		// for mesh
		Buffer* VertexBuffer = nullptr;
		Buffer* IndexBuffer = nullptr;
		u32 MeshNumIndices = 0;
	};

	struct DebugRender
	{
	public:
		std::vector<DebugRenderObject> Objects;

	public:
		void AddBox(const Matrix& Transform, Vector4 Colour, bool Wireframe = false, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Box,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
			};
			Objects.push_back(Object);
		}

		void AddBox(const Vector3& Position, const Vector3& Size, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Matrix Transform;
			Transform.Translate(Position);
			Transform.Scale(Size);
			AddBox(Transform, Colour, ZTest);
		}

		void AddBoxWithOutline(const Matrix& Transform, Vector4 Colour, Vector4 OutlineColour, bool Wireframe = false, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Box,
				.Transform = Transform,
				.Colour = Colour,
				.OutlineColour = OutlineColour,
				.DrawOutline = true,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
			};
			Objects.push_back(Object);
		}

		// a box between From and To, Width is in world units
		void AddLineFromTo(const Vector3& From, const Vector3& To, float Width, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
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

		void AddTri(const Vector3& A, const Vector3& B, const Vector3& C, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			DebugRenderObject Object {
				.Type = DebugRenderObjectType::Tri,
				.Transform = Matrix(1),
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
				.Vertices = { A, B, C },
			};
			Objects.push_back(Object);
		}

		void AddSphere(const Vector3& Pos, float Radius, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Matrix Mat;
			Mat.Scale(Vector3(Radius));
			Mat.Translate(Pos);

			AddSphere(Mat, Colour, Wireframe, ZTest);
		}

		void AddSphere(const Matrix& Transform, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			DebugRenderObject Object{
				.Type = DebugRenderObjectType::Sphere,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
			};
			Objects.push_back(Object);
		}

		void AddCone(const Vector3& Pos, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Matrix Mat;
			Mat.Translate(Pos);

			AddCone(Mat, Radius, Height, Colour, Wireframe, ZTest);
		}

		void AddCone(const Matrix& Transform, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Vector3 Packed(Radius, Height, 0);

			DebugRenderObject Object{
				.Type = DebugRenderObjectType::Cone,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
				.Vertices = { Packed, {}, {} }
			};
			Objects.push_back(Object);
		}

		void AddCylinder(const Vector3& Pos, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Matrix Mat;
			Mat.Translate(Pos);

			AddCylinder(Mat, Radius, Height, Colour, Wireframe, ZTest);
		}

		void AddCylinder(const Matrix& Transform, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Vector3 Packed(Radius, Height, 0);

			DebugRenderObject Object{
				.Type = DebugRenderObjectType::Cylinder,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
				.Vertices = { Packed, {}, {} }
			};
			Objects.push_back(Object);
		}

		void AddCapsule(const Vector3& Pos, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Matrix Mat;
			Mat.Translate(Pos);

			AddCapsule(Mat, Radius, Height, Colour, Wireframe, ZTest);
		}

		void AddCapsule(const Matrix& Transform, float Radius, float Height, const Vector4& Colour, bool Wireframe = false, bool ZTest = true)
		{
			Vector3 Packed(Radius, Height, 0);

			DebugRenderObject Object{
				.Type = DebugRenderObjectType::Capsule,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
				.Vertices = { Packed, {}, {} }
			};
			Objects.push_back(Object);
		}

		void AddMesh(const Matrix& Transform, Buffer* VB, Buffer* IB, u32 NumIndices, Vector4 Colour, bool Wireframe = false, bool ZTest = true, bool UseUnjitteredCamera = true)
		{
			DebugRenderObject Object{
				.Type = DebugRenderObjectType::Mesh,
				.Transform = Transform,
				.Colour = Colour,
				.DrawOutline = false,
				.UseZTest = ZTest,
				.Wireframe = Wireframe,
				.UseUnjitteredCamera = UseUnjitteredCamera,
				.Vertices = { {}, {}, {}},
				.VertexBuffer = VB,
				.IndexBuffer = IB,
				.MeshNumIndices = NumIndices,
			};
			Objects.push_back(Object);
		}

		void Clear()
		{
			Objects.clear();
		}
	};

}
