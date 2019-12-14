#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>
#include <Graphics/Device.h>

namespace Columbus
{

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);
	}

	bool MeshOpenGL::Load(const Model& InModel)
	{
		for (int i = 0; i < InModel.GetSubModelsCount(); i++)
		{
			SubMeshes.emplace_back(gDevice->CreateMesh());
			auto mesh = SubMeshes.back();
			static_cast<MeshOpenGL*>(mesh)->Load(InModel.GetSubModel(i));
		}

		return true;
	}

	bool MeshOpenGL::Load(const SubModel& InModel)
	{
		VerticesCount = InModel.VerticesCount;
		IndicesCount = InModel.IndicesCount;
		BoundingBox = InModel.BoundingBox;

		VOffset = 0;
		UOffset = VOffset + (InModel.VerticesCount * sizeof(Vector3));
		NOffset = UOffset + (InModel.VerticesCount * sizeof(Vector2));
		TOffset = NOffset + (InModel.VerticesCount * sizeof(Vector3));

		uint64 Size = InModel.VerticesCount * (sizeof(Vector3) * 3 + sizeof(Vector2));

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Size, nullptr, GL_STATIC_DRAW);

		if (InModel.Positions != nullptr) glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)VOffset, InModel.VerticesCount * sizeof(Vector3), InModel.Positions);
		if (InModel.UVs != nullptr)       glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)UOffset, InModel.VerticesCount * sizeof(Vector2), InModel.UVs);
		if (InModel.Normals != nullptr)   glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)NOffset, InModel.VerticesCount * sizeof(Vector3), InModel.Normals);
		if (InModel.Tangents != nullptr)  glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)TOffset, InModel.VerticesCount * sizeof(Vector3), InModel.Tangents);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		uint64 const Offsets[4] = { VOffset, UOffset, NOffset, TOffset };
		uint32 const Strides[4] = { 3, 2, 3, 3 };

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		for (uint32 i = 0; i < 4; i++)
		{
			glVertexAttribPointer(i, Strides[i], GL_FLOAT, GL_FALSE, 0, (void*)Offsets[i]);
			glEnableVertexAttribArray(i);
		}

		glBindVertexArray(0);

		if (InModel.Indexed)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, InModel.IndicesCount * InModel.IndexSize, InModel.Indices, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			Indexed = true;

			switch (InModel.IndexSize)
			{
			case 1: IndicesType = GL_UNSIGNED_BYTE;  break;
			case 2: IndicesType = GL_UNSIGNED_SHORT; break;
			case 4: IndicesType = GL_UNSIGNED_INT;   break;
			}
		}

		return true;
	}

	void MeshOpenGL::Bind()
	{
		glBindVertexArray(VAO);
	}
	
	uint32 MeshOpenGL::Render()
	{
		if (Indexed)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glDrawElements(GL_TRIANGLES, IndicesCount, IndicesType, nullptr);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			return IndicesCount / 3;
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, VerticesCount);
			return VerticesCount / 3;
		}

		return 0;
	}

	void MeshOpenGL::Unbind()
	{
		glBindVertexArray(0);
	}
	
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
		glDeleteVertexArrays(1, &VAO);
	}

}


