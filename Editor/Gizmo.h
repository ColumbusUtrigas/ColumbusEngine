#pragma once

#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Scene/GameObject.h>
#include <memory>

namespace Columbus
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
	public:
		Gizmo();

		void SetCamera(const Camera& C) { _Camera = C; }
		void Draw();
	};

}


