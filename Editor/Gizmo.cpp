#include <Editor/Gizmo.h>
#include <Graphics/Device.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Math/Frustum.h>
#include <algorithm>
#include <iterator>
#include <tuple>

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Columbus::Editor
{

namespace
{
	template <typename T>
	void RenderGizmo(const T& transforms, const std::unique_ptr<Mesh>& mesh, ShaderProgramOpenGL* shader)
	{
		mesh->Bind();
		for (const auto& trans : transforms)
		{
			auto model = Matrix(1.0f).Scale(trans.scale).Translate(trans.pos);

			shader->SetUniform("Color", trans.color);
			shader->SetUniform("Model", false, model);
			mesh->Render();
		}
		mesh->Unbind();
	}

	Vector3 MouseRayDir(const Matrix& View, const Matrix& Projection, const Vector2& MousePos)
	{
		using namespace glm;
		auto invertedView = inverse(make_mat4(&View.M[0][0]));
		auto invertedProjection = inverse(make_mat4(&Projection.M[0][0]));
		auto clipCoords = vec4(MousePos.X, -MousePos.Y, -1.0f, 0.0f);
		auto eyeCoords = invertedProjection * clipCoords;
		eyeCoords = {eyeCoords.x, eyeCoords.y, -1, 0};
		auto rayWorld = invertedView * eyeCoords;
		return Vector3(rayWorld.x, rayWorld.y, rayWorld.z).Normalized();
	}

	Vector3 RayPlaneIntersect(const Vector3& RayOrigin, const Vector3& RayDir,
		const Vector3& PlaneOrigin, const Vector3& Normal)
	{
		auto dist = Normal.Dot(PlaneOrigin - RayOrigin) / Normal.Dot(RayDir);
		return RayOrigin + (RayDir * dist);
	}
}

	Gizmo::Gizmo()
	{
		_Box = std::unique_ptr<Mesh>(gDevice->CreateMesh());
		_Box->Load("Data/Meshes/Box.cmf");
	}

	struct GizmoTransform
	{
		int index;
		Vector3 pos;
		Vector3 scale;
		Vector4 color;
		Vector4 color2;
		Vector3 directions;
		Vector3 normal;
	};

	Vector3 PickedPosition;
	GizmoTransform Picked;
	bool WasPressed = false;

	void Gizmo::Draw(Transform& transform, const Vector4& rect)
	{
		auto mat = transform.GetMatrix().GetTransposed();
		auto view = _Camera.GetViewMatrix();
		auto proj = _Camera.GetProjectionMatrix();
		auto mode = (_Operation == Operation::Scale) ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;
		//mode = ImGuizmo::MODE::LOCAL;

		float t[3], r[3], s[3];
		float dt[3], dr[3], ds[3];
		float delta[16];

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(rect.X, rect.Y, rect.Z, rect.W);

		ImGuizmo::Manipulate(&view.M[0][0], &proj.M[0][0], (ImGuizmo::OPERATION)_Operation, mode, &mat.M[0][0], delta);
		ImGuizmo::DecomposeMatrixToComponents(&mat.M[0][0], t, r, s);
		ImGuizmo::DecomposeMatrixToComponents(delta, dt, dr, ds);

		transform.Position = { t[0], t[1], t[2] };
		transform.Rotation /= Quaternion({ dr[0], dr[1], dr[2] });
		transform.Scale = { s[0], s[1], s[2] };
		transform.Update();

		/*if (!ImGui::IsMouseDown(0)) WasPressed = false;
		if (PickedObject == nullptr) return;

		auto shader = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->EditorTools);
		Frustum ViewFrustum(_Camera.GetViewProjection());
		auto origin = _Camera.Pos;
		auto dir = MouseRayDir(_Camera.GetViewMatrix(), _Camera.GetProjectionMatrix(), MousePickingPosition);

		GizmoTransform transforms[] = {
			{0, {1.5f,0,0}, {1,0.2f,0.2f}, {0.5f,0,0,1}, {1,0,0,1}, {1,0,0}, {0,0,1}}, // X
			{1, {0,1.5f,0}, {0.2f,1,0.2f}, {0,0.5f,0,1}, {0,1,0,1}, {0,1,0}, {1,0,0}}, // Y
			{2, {0,0,1.5f}, {0.2f,0.2f,1}, {0,0,0.5f,1}, {0,0,1,1}, {0,0,1}, {0,1,0}}, // Z

			{3, {1,1,0}, {0.4f, 0.4f, 0.05f}, {0,0.5f,0,1}, {0,1,0,1}, {1,1,0}, {0,0,1}}, // XY
			{4, {0,1,1}, {0.05f, 0.4f, 0.4f}, {0.5f,0,0,1}, {1,0,0,1}, {0,1,1}, {1,0,0}}, // YZ
			{5, {1,0,1}, {0.4f, 0.05f, 0.4f}, {0,0,0.5f,1}, {0,0,1,1}, {1,0,1}, {0,1,0}}, // XZ
		};

		// translate gizmo to object position and scale it
		for (auto& trans : transforms)
		{
			float factor = PickedObject->transform.Position.Length(_Camera.Pos) / 15;
			trans.scale *= factor;
			trans.pos *= factor;
			trans.pos += PickedObject->transform.Position;
		}

		if (WasPressed)
		{
			auto pos = RayPlaneIntersect(origin, dir, PickedPosition, Picked.normal);
			PickedObject->transform.Position += (pos - PickedPosition) * Picked.directions;
			PickedPosition = pos;

			auto trans = std::find_if(
				std::begin(transforms), std::end(transforms),
				[&](const auto& a){ return a.index == Picked.index; }
			);
			trans->color = trans->color2;
		}

		if (EnableMousePicking)
		{
			// sort from near to far because we want to get only the first bounding box intersection.
			std::sort(std::begin(transforms), std::end(transforms), [&](const auto& a, const auto& b){
				return _Camera.Pos.LengthSquare(a.pos) < _Camera.Pos.LengthSquare(b.pos);
			});

			// get the first intersection and highlight that mesh.
			for (auto& trans : transforms)
			{
				if ((_Box->GetBoundingBox() * trans.scale + trans.pos).Intersects(origin, dir))
				{
					if (!WasPressed)
					{
						trans.color = trans.color2;
					}

					if (ImGui::IsMouseClicked(0))
					{
						if (!WasPressed)
						{
							Picked = trans;
							PickedPosition = RayPlaneIntersect(origin, dir, trans.pos, trans.normal);
						}

						WasPressed = true;
					}

					break;
				}
			}
		}

		// sort from far to near to render it the right way.
		std::sort(std::begin(transforms), std::end(transforms), [&](const auto& a, const auto& b){
			return _Camera.Pos.LengthSquare(a.pos) > _Camera.Pos.LengthSquare(b.pos);
		});

		shader->Bind();
		shader->SetUniform("ViewProjection", false, _Camera.GetViewProjection());
		shader->SetUniform("CameraPos", _Camera.Pos);
		shader->SetUniform("UseDistanceFade", 0);
		RenderGizmo(transforms, _Box, shader);
		shader->Unbind();*/
	}

}


