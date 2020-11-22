#pragma once

#include <Graphics/Device.h>
#include <Math/Matrix.h>
#include <Math/Vector4.h>
#include <vector>

namespace Columbus
{
	class Renderer;

	namespace Editor
	{
		class Editor;
	}
}

namespace Columbus::Graphics
{

	class DebugRenderer
	{
	private:
		friend class Columbus::Renderer;
		friend class Columbus::Editor::Editor;

		bool _enabled = true;

		RasterizerState *RS_solid, *RS_wireframe;
		Mesh* M_cube, * M_sphere;

		struct DRObject
		{
			Matrix transform;
			Vector4 color;
			RasterizerState* rs;
			Mesh* mesh;

			DRObject(const Matrix& transform, const Vector4& color, RasterizerState* rs, Mesh* mesh) :
				transform(transform), color(color), rs(rs), mesh(mesh) {}
		};

		std::vector<DRObject> objects;
	public:
		void Initialize();
		void NewFrame();
		void Shutdown();

		void RenderBox(const Matrix& transform, const Vector4& color, bool wireframe);
		void RenderSphere(const Vector3& pos, float radius, const Vector4& color, bool wireframe);
	};

	extern DebugRenderer gDebugRender;

}
