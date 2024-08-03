#pragma once

#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Scene/GameObject.h>
#include <memory>

namespace Columbus::Editor
{

	class Gizmo
	{
	private:
		std::unique_ptr<Mesh> _Box;
		Camera _Camera;
	public:
		bool EnableMousePicking = false;
		Vector2 MousePickingPosition;
		GameObject* PickedObject = nullptr;

		enum Operation
		{
			Translate,
			Rotate,
			Scale,
			Bounds
		} _Operation = Translate;
	public:
		Gizmo();

		void SetCamera(const Camera& C) { _Camera = C; }
		void Draw(Transform& transform, const Vector4& rect);
	};

}


