#pragma once

#include <vector>

#include <Graphics/Light.h>
#include <Common/Model/Model.h>
#include <Math/Box.h>

namespace Columbus
{

	class Mesh
	{
	protected:
		Vector3 Position;
		Box BoundingBox;

		uint32 VerticesCount;
	public:
		std::vector<Light*> Lights;
	public:
		Mesh() : VerticesCount(0) { }
		Mesh(const std::vector<Vertex>& Vertices) : VerticesCount(0) { SetVertices(Vertices); }

		virtual void SetVertices(const std::vector<Vertex>& InVertices) {}
		virtual void Load(const Model& InModel) {}
		virtual void Bind() {}
		virtual uint32 Render() { return 0; }
		virtual void Unbind() {}
		virtual uint64 GetMemoryUsage() const { return 0;  }

		void SetLights(const std::vector<Light*>& InLights) { Lights = InLights; }

		Box GetBoundingBox() const { return BoundingBox; }

		void Clear() {}

		virtual ~Mesh() {}
	};

}
