#include <Graphics/OpenGL/MeshOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	/*
	*
	* Shader uniforms functions
	*
	*/
	static void ShaderSetMatrices(ShaderProgram* InShader, Transform InTransform, Camera InCamera)
	{
		static float sModelMatrix[16];
		static float sViewMatrix[16];
		static float sProjectionMatrix[16];

		if (InShader != nullptr)
		{
			if (InShader->IsCompiled())
			{
				InTransform.GetMatrix().ElementsTransposed(sModelMatrix);
				InCamera.getViewMatrix().Elements(sViewMatrix);
				InCamera.getProjectionMatrix().ElementsTransposed(sProjectionMatrix);

				InShader->SetUniformMatrix("uModel", sModelMatrix);
				InShader->SetUniformMatrix("uView", sViewMatrix);
				InShader->SetUniformMatrix("uProjection", sProjectionMatrix);
			}
		}
	}

	static void ShaderSetMaterial(Material InMaterial)
	{
		auto tShader = InMaterial.GetShader();

		for (uint32 i = 0; i < 3; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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
					tShader->SetUniform1i("uMaterial.ReflectionMap", 3);
					Reflection->samplerCube(3);
				}

				tShader->SetUniform4f("uMaterial.Color", InMaterial.getColor());
				tShader->SetUniform3f("uMaterial.AmbientColor", InMaterial.getAmbient());
				tShader->SetUniform3f("uMaterial.DiffuseColor", InMaterial.getDiffuse());
				tShader->SetUniform3f("uMaterial.SpecularColor", InMaterial.getSpecular());
				tShader->SetUniform1f("uMaterial.ReflectionPower", InMaterial.getReflectionPower());
				tShader->SetUniform1i("uMaterial.Lighting", InMaterial.getLighting());
			}
		}
	}

	static void ShaderSetLight(ShaderProgram* InShader, Light InLight, uint32 Number)
	{
		if (InShader != nullptr)
		{
			if (InShader->IsCompiled())
			{
				std::string Element = "uLights[" + std::to_string(Number) + "].";
				InShader->SetUniform3f(Element + "Color", InLight.getColor());
				InShader->SetUniform3f(Element + "Position", InLight.getPos());
				InShader->SetUniform3f(Element + "Direction", InLight.getDir());
				InShader->SetUniform1i(Element + "Type", InLight.getType());
				InShader->SetUniform1f(Element + "Constant", InLight.getConstant());
				InShader->SetUniform1f(Element + "Linear", InLight.getLinear());
				InShader->SetUniform1f(Element + "Quadratic", InLight.getQuadratic());
				InShader->SetUniform1f(Element + "InnerCutoff", InLight.getInnerCutoff());
				InShader->SetUniform1f(Element + "OuterCutoff", InLight.getOuterCutoff());
			}
		}
	}

	static void ShaderSetLights(ShaderProgram* InShader, std::vector<Light*> InLights)
	{
		uint32 Counter = 0;

		for (auto& Light : InLights)
		{
			if (Light != nullptr)
			{
				ShaderSetLight(InShader, *Light, Counter++);
			}
		}
	}

	static void ShaderSetLightsAndCamera(ShaderProgram* InShader, std::vector<Light*> InLights, Camera InCamera)
	{
		if (InShader != nullptr)
		{
			if (InShader->IsCompiled())
			{
				ShaderSetLights(InShader, InLights);
				InShader->SetUniform3f("uCamera.Position", InCamera.getPos());
			}
		}
	}

	static void ShaderSetAll(Material InMaterial, std::vector<Light*> InLights, Camera InCamera, Transform InTransform)
	{
		auto tShader = InMaterial.GetShader();

		if (tShader != nullptr)
		{
			if (tShader->IsCompiled())
			{
				ShaderSetMatrices(tShader, InTransform, InCamera);
				ShaderSetMaterial(InMaterial);
				ShaderSetLightsAndCamera(tShader, InLights, InCamera);
			}
		}
	}
	/*
	*
	* End of shader uniforms functions
	*
	*/

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
	}
	
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> InVertices)
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
		SetVertices(InVertices);
	}
	
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> InVertices, Material InMaterial)
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
		mMat = InMaterial;
		SetVertices(InVertices);
	}
	
	void MeshOpenGL::SetVertices(std::vector<Vertex> InVertices)
	{
		Vertices.clear();
		Vertices = InVertices;

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
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;
		uint64_t tcounter = 0;

		for (auto Vertex : Vertices)
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

		glBindBuffer(GL_ARRAY_BUFFER, VBuf);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, VOffset, Vertices.size() * 3 * sizeof(float), v);
		glBufferSubData(GL_ARRAY_BUFFER, UOffset, Vertices.size() * 2 * sizeof(float), u);
		glBufferSubData(GL_ARRAY_BUFFER, NOffset, Vertices.size() * 3 * sizeof(float), n);
		glBufferSubData(GL_ARRAY_BUFFER, TOffset, Vertices.size() * 3 * sizeof(float), t);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		uint64 const offsets[4] = { VOffset, UOffset, NOffset, TOffset };
		uint32 const strides[4] = { 3, 2, 3, 3 };

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBuf);

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
	
	void MeshOpenGL::Render(Transform InTransform)
	{
		if (mMat.GetShader() == nullptr) return;
		if (!mMat.GetShader()->IsCompiled())
		{
			auto tShader = mMat.GetShader();

			tShader->AddAttribute("aPos", 0);
			tShader->AddAttribute("aUV", 1);
			tShader->AddAttribute("aNorm", 2);
			tShader->AddAttribute("aTang", 3);
			tShader->Compile();

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

			for (uint32 i = 0; i < 8; i++)
			{
				std::string Element = "uLights[" + std::to_string(i) + "].";
				tShader->AddUniform(Element + "Color");
				tShader->AddUniform(Element + "Position");
				tShader->AddUniform(Element + "Direction");
				tShader->AddUniform(Element + "Type");
				tShader->AddUniform(Element + "Constant");
				tShader->AddUniform(Element + "Linear");
				tShader->AddUniform(Element + "Quadratic");
				tShader->AddUniform(Element + "InnerCutoff");
				tShader->AddUniform(Element + "OuterCutoff");
			}

			tShader->AddUniform("uModel");
			tShader->AddUniform("uView");
			tShader->AddUniform("uProjection");

			tShader->AddUniform("uCamera.Position");
		}

		mMat.GetShader()->Bind();

		SortLights();

		ShaderSetAll(mMat, Lights, ObjectCamera, InTransform);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, Vertices.size());
		glBindVertexArray(0);
	}
	
	void MeshOpenGL::SortLights()
	{
		Vector3 pos = Position;

		Lights.erase(std::remove(Lights.begin(), Lights.end(), nullptr), Lights.end());

		auto func = [pos](const Light* a, const Light* b) mutable -> bool
		{
			Vector3 q = a->getPos();
			Vector3 w = b->getPos();

			return q.Length(pos) < w.Length(pos);
		};

		std::sort(Lights.begin(), Lights.end(), func);
	}
	
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &VBuf);
		glDeleteVertexArrays(1, &VAO);
	}

}













