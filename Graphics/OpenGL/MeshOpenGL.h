#pragma once

#include <Graphics/Mesh.h>

namespace Columbus
{

	class MeshOpenGL : public Mesh
	{
	private:
		uint32 VBO = 0;
		uint32 IBO = 0;
		uint32 VAO = 0;

		uint64 VOffset = 0;
		uint64 UOffset = 0;
		uint64 NOffset = 0;
		uint64 TOffset = 0;

		bool Indexed = false;
		uint32 IndicesCount = 0;
		uint32 IndicesType = 0;

		void SortLights();
	public:
		MeshOpenGL();
		MeshOpenGL(const std::vector<Vertex>& Vertices);

		virtual void SetVertices(const std::vector<Vertex>& Vertices) final override;
		virtual bool Load(const Model& InModel) final override;

		virtual void Bind()     final override;
		virtual uint32 Render() final override;
		virtual void Unbind()   final override;

		virtual uint64 GetMemoryUsage() const final override;

		~MeshOpenGL() override;
	};

}













