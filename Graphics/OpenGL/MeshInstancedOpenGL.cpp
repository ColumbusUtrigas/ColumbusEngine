#include <Graphics/OpenGL/MeshInstancedOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	/*
	*
	* Shader uniforms functions
	*
	*/
	static void ShaderSetMaterial(Material InMaterial)
	{
		auto tShader = InMaterial.GetShader();

		if (tShader != nullptr)
		{
			if (tShader->IsCompiled())
			{
				Texture* Textures[3] = { InMaterial.getTexture(), InMaterial.getSpecMap(), InMaterial.getNormMap() };
				Cubemap* Reflection = InMaterial.getReflection();
				std::string Names[3] = { "uMaterial.DiffuseMap" , "uMaterial.SpecularMap", "uMaterial.NormalMap" };

				for (int32 i = 0; i < 3; i++)
				{
					if (Textures[i] != nullptr)
					{
						tShader->SetUniform1i(Names[i], i);
						Textures[i]->sampler2D(i);
					}
				}

				if (Reflection != nullptr)
				{
					tShader->SetUniform1i("uMaterial.ReflectionMap", 4);
					Reflection->samplerCube(4);
				}

				tShader->SetUniform4f("uMaterial.Color", InMaterial.getColor());
				tShader->SetUniform3f("uMaterial.AmbientColor", InMaterial.getAmbient());
				tShader->SetUniform3f("uMaterial.DiffuseColor", InMaterial.getDiffuse());
				tShader->SetUniform3f("uMaterial.SpecularColor", InMaterial.getSpecular());
			}
		}
	}
	/*
	*
	* End of shader uniforms functions
	*
	*/

	MeshInstancedOpenGL::MeshInstancedOpenGL()
	{
		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
	}

	void MeshInstancedOpenGL::SetVertices(std::vector<Vertex> InVertices)
	{
		Vertices.clear();
		Vertices = InVertices;

		float* v = new float[Vertices.size() * 3]; //Vertex buffer
		float* u = new float[Vertices.size() * 2]; //UV buffer
		float* n = new float[Vertices.size() * 3]; //Normal buffer
		float* t = new float[Vertices.size() * 3]; //Tangent buffer
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;
		uint64_t tcounter = 0;

		for (auto Vertex : Vertices)
		{
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

		if (Mat.GetShader() == nullptr) return;
	}

	void MeshInstancedOpenGL::Render()
	{
		if (Mat.GetShader() == nullptr) return;
		if (!Mat.GetShader()->IsCompiled())
		{
			auto tShader = Mat.GetShader();

			tShader->AddAttribute("aPos", 0);
			tShader->AddAttribute("aUV", 1);
			tShader->AddAttribute("aNorm", 2);
			tShader->AddAttribute("aTang", 3);
			tShader->Compile();

			for (int i = 0; i < 5; i++)
			{
				tShader->AddUniform("uModels[" + std::to_string(i) + "]");
			}

			tShader->AddUniform("uView");
			tShader->AddUniform("uProjection");

			tShader->AddUniform("uMaterial.DiffuseMap");
			tShader->AddUniform("uMaterial.SpecularMap");
			tShader->AddUniform("uMaterial.NormalMap");
			tShader->AddUniform("uMaterial.ReflectionMap");

			tShader->AddUniform("uMaterial.Color");
			tShader->AddUniform("uMaterial.AmbientColor");
			tShader->AddUniform("uMaterial.DiffuseColor");
			tShader->AddUniform("uMaterial.SpecularColor");
			tShader->AddUniform("uMaterial.ReflectionPower");
			tShader->AddUniform("uMaterial.Lighting");
		}

		Mat.GetShader()->Bind();

		Matrix ModelMat;
		ModelMat.Translate(Vector3(0, 5, 0));

		ModelMat.ElementsTransposed(UniformModelMatrix);
		ObjectCamera.getViewMatrix().Elements(UniformViewMatrix);
		ObjectCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

		Vector3 Offsets[5] = { {3, 0, 0}, {3, 4, 1}, {6, 5, 2}, {-6, 4, 5}, {-2, 8, 3} };
		Vector3 Rotations[5] = { {56, 0, 0}, { 123, 13, 51 },{ 6, 551, 52 },{ -67, 412, 587 },{ -21, 8, 3123 } };
		Matrix Models[5];

		for (int i = 0; i < 5; i++)
		{
			static float Model[16];

			Models[i].SetIdentity();
			Models[i].Scale(Vector3(0.5, 0.5, 0.5));
			Models[i].Rotate(Vector3(1, 0, 0), Rotations[i].X);
			Models[i].Rotate(Vector3(0, 1, 0), Rotations[i].Y);
			Models[i].Rotate(Vector3(0, 0, 1), Rotations[i].Z);
			Models[i].Translate(Offsets[i]);
			Models[i].ElementsTransposed(Model);

			Mat.GetShader()->SetUniformMatrix("uModels[" + std::to_string(i) + "]", Model);
		}

		ShaderSetMaterial(Mat);

		Mat.GetShader()->SetUniformMatrix("uModel", UniformModelMatrix);
		Mat.GetShader()->SetUniformMatrix("uView", UniformViewMatrix);
		Mat.GetShader()->SetUniformMatrix("uProjection", UniformProjectionMatrix);

		glBindVertexArray(VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, Vertices.size(), 5);
		glBindVertexArray(0);
	}

	MeshInstancedOpenGL::~MeshInstancedOpenGL()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

}








