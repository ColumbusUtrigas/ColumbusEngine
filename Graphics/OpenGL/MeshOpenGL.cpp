#include <Graphics/OpenGL/MeshOpenGL.h>
#include <GL/glew.h>
#include <algorithm>

namespace Columbus
{

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);
	}
	
	MeshOpenGL::MeshOpenGL(const std::vector<Vertex>& InVertices)
	{
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);
		SetVertices(InVertices);
	}
	
	void MeshOpenGL::SetVertices(const std::vector<Vertex>& Vertices)
	{
		VerticesCount = Vertices.size();

		//Temperary Oriented Bounding Box Data
		struct
		{
			float minX = 0.0f;
			float maxX = 0.0f;
			float minY = 0.0f;
			float maxY = 0.0f;
			float minZ = 0.0f;
			float maxZ = 0.0f;
		} OBBData;

		float* v = new float[Vertices.size() * 3]; //Vertex buffer
		float* u = new float[Vertices.size() * 2]; //UV buffer
		float* n = new float[Vertices.size() * 3]; //Normal buffer
		float* t = new float[Vertices.size() * 3]; //Tangent buffer
		uint64 vcounter = 0;
		uint64 ucounter = 0;
		uint64 ncounter = 0;
		uint64 tcounter = 0;

		for (auto& Vertex : Vertices)
		{
			if (Vertex.pos.X < OBBData.minX) OBBData.minX = Vertex.pos.X;
			if (Vertex.pos.X > OBBData.maxX) OBBData.maxX = Vertex.pos.X;
			if (Vertex.pos.Y < OBBData.minY) OBBData.minY = Vertex.pos.Y;
			if (Vertex.pos.Y > OBBData.maxY) OBBData.maxY = Vertex.pos.Y;
			if (Vertex.pos.Z < OBBData.minZ) OBBData.minZ = Vertex.pos.Z;
			if (Vertex.pos.Z > OBBData.maxZ) OBBData.maxZ = Vertex.pos.Z;

			v[vcounter++] = Vertex.pos.X;
			v[vcounter++] = Vertex.pos.Y;
			v[vcounter++] = Vertex.pos.Z;

			u[ucounter++] = Vertex.UV.X;
			u[ucounter++] = Vertex.UV.Y;

			n[ncounter++] = Vertex.normal.X;
			n[ncounter++] = Vertex.normal.Y;
			n[ncounter++] = Vertex.normal.Z;

			t[tcounter++] = Vertex.tangent.X;
			t[tcounter++] = Vertex.tangent.Y;
			t[tcounter++] = Vertex.tangent.Z;
		}

		uint64 size = (sizeof(float) * Vertices.size() * 3)
		            + (sizeof(float) * Vertices.size() * 2)
		            + (sizeof(float) * Vertices.size() * 3)
		            + (sizeof(float) * Vertices.size() * 3);

		VOffset = 0;
		UOffset = VOffset + (sizeof(float) * Vertices.size() * 3);
		NOffset = UOffset + (sizeof(float) * Vertices.size() * 2);
		TOffset = NOffset + (sizeof(float) * Vertices.size() * 3);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, VOffset, Vertices.size() * 3 * sizeof(float), v);
		glBufferSubData(GL_ARRAY_BUFFER, UOffset, Vertices.size() * 2 * sizeof(float), u);
		glBufferSubData(GL_ARRAY_BUFFER, NOffset, Vertices.size() * 3 * sizeof(float), n);
		glBufferSubData(GL_ARRAY_BUFFER, TOffset, Vertices.size() * 3 * sizeof(float), t);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		uint64 const offsets[4] = { VOffset, UOffset, NOffset, TOffset };
		uint32 const strides[4] = { 3, 2, 3, 3 };

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		for (uint32 i = 0; i < 4; i++)
		{
			glVertexAttribPointer(i, strides[i], GL_FLOAT, GL_FALSE, 0, (void*)offsets[i]);
			glEnableVertexAttribArray(i);
		}

		glBindVertexArray(0);

		delete[] v;
		delete[] u;
		delete[] n;
		delete[] t;

		BoundingBox.Min = Vector3(OBBData.minX, OBBData.minY, OBBData.minZ);
		BoundingBox.Max = Vector3(OBBData.maxX, OBBData.maxY, OBBData.maxZ);
	}

	void MeshOpenGL::Load(const Model& InModel)
	{
		VerticesCount = InModel.GetVerticesCount();
		IndicesCount = InModel.GetIndicesCount();
		BoundingBox = InModel.GetBoundingBox();

		VOffset = 0;
		UOffset = VOffset + (InModel.GetVerticesCount() * sizeof(Vector3));
		NOffset = UOffset + (InModel.GetVerticesCount() * sizeof(Vector2));
		TOffset = NOffset + (InModel.GetVerticesCount() * sizeof(Vector3));

		uint64 Size = InModel.GetVerticesCount() * (sizeof(Vector3) * 3 + sizeof(Vector2));

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Size, nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, VOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetPositions());
		glBufferSubData(GL_ARRAY_BUFFER, UOffset, InModel.GetVerticesCount() * sizeof(Vector2), InModel.GetUVs());
		glBufferSubData(GL_ARRAY_BUFFER, NOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetNormals());
		glBufferSubData(GL_ARRAY_BUFFER, TOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetTangents());

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

		if (InModel.IsIndexed())
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, InModel.GetIndicesCount() * InModel.GetIndexSize(), InModel.GetIndices(), GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			Indexed = true;

			switch (InModel.GetIndexSize())
			{
			case 1: IndicesType = GL_UNSIGNED_BYTE;  break;
			case 2: IndicesType = GL_UNSIGNED_SHORT; break;
			case 4: IndicesType = GL_UNSIGNED_INT;   break;
			}
		}
	}

	void MeshOpenGL::Bind()
	{
		SortLights();
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

	uint64 MeshOpenGL::GetMemoryUsage() const
	{
		uint64 Usage = 0;
		Usage += sizeof(Vertex) * VerticesCount;
		if (Indexed)
		{
			switch (IndicesType)
			{
			case GL_UNSIGNED_BYTE:  Usage += IndicesCount * 1; break;
			case GL_UNSIGNED_SHORT: Usage += IndicesCount * 2; break;
			case GL_UNSIGNED_INT:   Usage += IndicesCount * 4; break;
			}
		}
		return Usage;
	}
	
	void MeshOpenGL::SortLights()
	{
		if (!LightsSorted)
		{
			Lights.erase(std::remove(Lights.begin(), Lights.end(), nullptr), Lights.end());

			static auto func = [&](const Light* A, const Light* B) -> bool
			{
				double ADistance = Math::Sqr(A->Pos.X - Position.X) + Math::Sqr(A->Pos.Y - Position.Y) + Math::Sqr(A->Pos.Z - Position.Z);
				double BDistance = Math::Sqr(B->Pos.X - Position.X) + Math::Sqr(B->Pos.Y - Position.Y) + Math::Sqr(B->Pos.Z - Position.Z);
				return ADistance < BDistance;
			};

			if (Lights.size() >= 4)
			{
				std::sort(Lights.begin(), Lights.end(), func);
			}

			LightsSorted = true;
		}
	}
	
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
		glDeleteVertexArrays(1, &VAO);
	}

}


