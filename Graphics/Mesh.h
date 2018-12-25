#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <Graphics/Camera.h>
#include <Graphics/Light.h>
#include <Common/Model/Model.h>
#include <Math/Box.h>

namespace Columbus
{

	class Mesh
	{
	protected:
		Vector3 Position;
		Camera ObjectCamera;
		Box BoundingBox;

		uint32 VerticesCount;
	public:
		std::vector<Light*> Lights;
	public:
		Mesh() : VerticesCount(0) { }
		Mesh(const std::vector<Vertex>& Vertices) : VerticesCount(0) { SetVertices(Vertices); }

		virtual void SetVertices(const std::vector<Vertex>& InVertices) {}
		virtual void Bind() {}
		virtual uint32 Render() { return 0; }
		virtual void Unbind() {}
		virtual uint64 GetMemoryUsage() const { return 0;  }

		void SetCamera(Camera InCamera) { ObjectCamera = InCamera; }

		void SetLights(const std::vector<Light*>& InLights) { Lights = InLights; }

		Box GetBoundingBox() const { return BoundingBox; }
		Camera& GetCamera() { return ObjectCamera; }

		void Clear() {}

		virtual ~Mesh() {}
	};

}
