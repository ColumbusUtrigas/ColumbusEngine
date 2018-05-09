#pragma once

#include <Common/Model/Model.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Camera.h>
#include <Graphics/Material.h>
#include <Graphics/Light.h>
#include <Scene/Transform.h>

namespace Columbus
{

	class MeshInstanced
	{
	protected:
		std::vector<Vertex> Vertices;
		Camera ObjectCamera;
	public:
		Material Mat;
	public:
		MeshInstanced() {}

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }

		virtual void SetVertices(std::vector<Vertex> InVertices) {}
		virtual void Render() {}

		virtual ~MeshInstanced() {}
	};

}



