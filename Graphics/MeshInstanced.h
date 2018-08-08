#pragma once

#include <Common/Model/Model.h>
#include <Graphics/Camera.h>
#include <Graphics/Material.h>
#include <Graphics/Light.h>
#include <Scene/Transform.h>

namespace Columbus
{

	class MeshInstanced
	{
	protected:
		Camera ObjectCamera;
		uint32 VerticesCount;
	public:
		Material Mat;
	public:
		MeshInstanced() : VerticesCount(0) {}

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }

		virtual void SetVertices(std::vector<Vertex> Vertices) {}
		virtual void Render() {}

		virtual ~MeshInstanced() {}
	};

}



