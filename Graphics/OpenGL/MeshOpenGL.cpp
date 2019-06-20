#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>
#include <algorithm>

namespace Columbus
{

	struct
	{
		uint32 Slots[16];

		void SetBuffer(uint32 Slot, uint32 ID)
		{
			Slots[Slot] = ID;
		}

		void SetIndexBuffer(uint32 ID)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		}

		void SetLayout(const InputLayout& Layout, uint32* Offsets)
		{
			uint32 offset = 0;
			for (const auto& Element : Layout.Elements)
			{
				size_t index = 0;
				size_t components = 0;

				switch (Element.Semantic)
				{
				case InputLayoutSemantic::Position: index = 0; break;
				case InputLayoutSemantic::UV: index = 1; break;
				case InputLayoutSemantic::Normal: index = 2; break;
				case InputLayoutSemantic::Tangent: index = 3; break;
				}

				switch (Element.Type)
				{
				case InputLayoutFormat::Float2: components = 2; break;
				case InputLayoutFormat::Float3: components = 3; break;
				}

				glVertexAttribPointer(index, components, GL_FLOAT, GL_FALSE, 0, (void*)Offsets[offset++]);
				glEnableVertexAttribArray(index);
			}
		}

		void DrawElements(uint32 Count, uint32 Type)
		{
			glDrawElements(GL_TRIANGLES, Count, Type, nullptr);
		}
	} Dev;

	InputLayout layout =
	{
			{ InputLayoutSemantic::Position, InputLayoutFormat::Float3, 0, 0, InputLayoutClassification::PerVertex },
			{ InputLayoutSemantic::UV, InputLayoutFormat::Float2, 0, 0, InputLayoutClassification::PerVertex },
			{ InputLayoutSemantic::Normal, InputLayoutFormat::Float3, 0, 0, InputLayoutClassification::PerVertex },
			{ InputLayoutSemantic::Tangent, InputLayoutFormat::Float3, 0, 0, InputLayoutClassification::PerVertex },
	};

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);
		glGenVertexArrays(1, &VAO);
	}

	bool MeshOpenGL::Load(const Model& InModel)
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

		for (const auto& Element : layout.Elements)
		{
			size_t size = 0;
			size_t offset = 0;
			void* data = nullptr;

			switch (Element.Type)
			{
			case InputLayoutFormat::Float2: size = sizeof(Vector2); break;
			case InputLayoutFormat::Float3: size = sizeof(Vector3); break;
			}

			switch (Element.Semantic)
			{
			case InputLayoutSemantic::Position: data = (void*)InModel.GetPositions(); offset = VOffset; break;
			case InputLayoutSemantic::UV: data = (void*)InModel.GetUVs(); offset = UOffset; break;
			case InputLayoutSemantic::Normal: data = (void*)InModel.GetNormals(); offset = NOffset; break;
			case InputLayoutSemantic::Tangent: data = (void*)InModel.GetTangents(); offset = TOffset; break;
			}

			glBufferSubData(GL_ARRAY_BUFFER, offset, InModel.GetVerticesCount() * size, data);
		}

		//glBufferSubData(GL_ARRAY_BUFFER, VOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetPositions());
		//glBufferSubData(GL_ARRAY_BUFFER, UOffset, InModel.GetVerticesCount() * sizeof(Vector2), InModel.GetUVs());
		//glBufferSubData(GL_ARRAY_BUFFER, NOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetNormals());
		//glBufferSubData(GL_ARRAY_BUFFER, TOffset, InModel.GetVerticesCount() * sizeof(Vector3), InModel.GetTangents());

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

		return true;
	}

	void MeshOpenGL::Bind()
	{
		//glBindVertexArray(VAO);
		Dev.SetBuffer(0, VBO);
	}
	
	uint32 MeshOpenGL::Render()
	{
		if (Indexed)
		{
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			//glDrawElements(GL_TRIANGLES, IndicesCount, IndicesType, nullptr);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			uint32 offsets[] = { VOffset, UOffset, NOffset, TOffset };

			Dev.SetIndexBuffer(IBO);
			Dev.SetLayout(layout, offsets);
			Dev.DrawElements(IndicesCount, IndicesType);
			Dev.SetIndexBuffer(0);

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
		//glBindVertexArray(0);
	}
	
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &IBO);
		glDeleteVertexArrays(1, &VAO);
	}

}


