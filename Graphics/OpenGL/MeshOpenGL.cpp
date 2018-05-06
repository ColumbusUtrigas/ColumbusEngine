#include <Graphics/OpenGL/MeshOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
	}
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> InVertices)
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
		SetVertices(InVertices);
	}
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> InVertices, Material InMaterial)
	{
		glGenBuffers(1, &VBuf);
		glGenVertexArrays(1, &VAO);
		mMat = InMaterial;
		SetVertices(InVertices);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
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

		if (mMat.GetShader() == nullptr) return;

		/*if (!mMat.getShader()->IsCompiled())
		{
			mMat.getShader()->AddAttribute("aPos", 0);
			mMat.getShader()->AddAttribute("aUV", 1);
			mMat.getShader()->AddAttribute("aNorm", 2);
			mMat.getShader()->AddAttribute("aTang", 3);
			mMat.getShader()->Compile();

			mMat.getShader()->AddUniform("uMaterial.diffuseMap");
			mMat.getShader()->AddUniform("uMaterial.specularMap");
			mMat.getShader()->AddUniform("uMaterial.normalMap");
			mMat.getShader()->AddUniform("uReflectionMap");

			mMat.getShader()->AddUniform("uModel");
			mMat.getShader()->AddUniform("uView");
			mMat.getShader()->AddUniform("uProjection");
		}*/
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

			tShader->AddUniform("uMaterial.diffuseMap");
			tShader->AddUniform("uMaterial.specularMap");
			tShader->AddUniform("uMaterial.normalMap");
			tShader->AddUniform("uReflectionMap");

			tShader->AddUniform("uModel");
			tShader->AddUniform("uView");
			tShader->AddUniform("uProjection");

			tShader->AddUniform("MaterialUnif");
			tShader->AddUniform("LightUnif");
			tShader->AddUniform("uCamera.pos");
		}

		mMat.GetShader()->Bind();

		SetShaderMatrices(InTransform);
		SetShaderMaterial();
		SetShaderLightAndCamera();
		SetShaderTextures();

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, Vertices.size());
		glBindVertexArray(0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::SetShaderTextures()
	{
		Texture* textures[3] = { mMat.getTexture(), mMat.getSpecMap(), mMat.getNormMap() };
		Cubemap* cubemap = mMat.getReflection();
		std::string unifs[3] = {"uMaterial.diffuseMap", "uMaterial.specularMap", "uMaterial.normalMap"};
		unsigned int indices[3] = {0, 1, 3};

		for (int i = 0; i < 3; i++)
		{
			if (textures[i] != nullptr)
			{
				mMat.GetShader()->SetUniform1i(unifs[i].c_str(), indices[i]);
				textures[i]->sampler2D(indices[i]);
			}
			else
			{
				mMat.GetShader()->SetUniform1i(unifs[i].c_str(), indices[i]);
				if (textures[0] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE0);
				if (textures[1] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE1);
				if (textures[2] == nullptr)
					C_DeactiveTextureOpenGL(C_OGL_TEXTURE3);
			}
		}

		if (cubemap != nullptr)
		{
			mMat.GetShader()->SetUniform1i("uReflectionMap", 2);
			cubemap->samplerCube(2);
		}
		else
		{
			mMat.GetShader()->SetUniform1i("uReflectionMap", 2);
			C_DeactiveCubemapOpenGL(C_OGL_TEXTURE2);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::SetShaderMatrices(Transform InTransform)
	{
		InTransform.GetMatrix().ElementsTransposed(UniformModelMatrix);
		ObjectCamera.getViewMatrix().Elements(UniformViewMatrix);
		ObjectCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

		Position = InTransform.GetPos();
		mMat.GetShader()->SetUniformMatrix("uModel", UniformModelMatrix);
		mMat.GetShader()->SetUniformMatrix("uView", UniformViewMatrix);
		mMat.GetShader()->SetUniformMatrix("uProjection", UniformProjectionMatrix);
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::SetShaderMaterial()
	{
		Vector4 matcol = mMat.getColor();
		Vector3 matamb = mMat.getAmbient();
		Vector3 matdif = mMat.getDiffuse();
		Vector3 matspc = mMat.getSpecular();

		mMaterialUnif[0] = matcol.x;
		mMaterialUnif[1] = matcol.y;
		mMaterialUnif[2] = matcol.z;
		mMaterialUnif[3] = matcol.w;
		mMaterialUnif[4] = matamb.X;
		mMaterialUnif[5] = matamb.Y;
		mMaterialUnif[6] = matamb.Z;
		mMaterialUnif[7] = matdif.X;
		mMaterialUnif[8] = matdif.Y;
		mMaterialUnif[9] = matdif.Z;
		mMaterialUnif[10] = matspc.X;
		mMaterialUnif[11] = matspc.Y;
		mMaterialUnif[12] = matspc.Z;
		mMaterialUnif[13] = mMat.getReflectionPower();
		mMaterialUnif[14] = mMat.getLighting() ? 1.0f : 0.0f;

		mMat.GetShader()->SetUniformArrayf("MaterialUnif", mMaterialUnif, 15);
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::SetShaderLightAndCamera()
	{
		CalculateLights();

		mMat.GetShader()->SetUniformArrayf("LightUnif", mLightUniform, 120);
		mMat.GetShader()->SetUniform3f("uCamera.pos", ObjectCamera.getPos());
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::CalculateLights()
	{
		SortLights();

		//8 - max count of lights, processing in shader
		for (uint32 i = 0; i < 8; i++)
		{
			uint32 offset = i * 15;

			if (i < Lights.size() && mMat.getLighting() == true)
			{
				//Color
				mLightUniform[0 + offset] = Lights[i]->getColor().X;
				mLightUniform[1 + offset] = Lights[i]->getColor().Y;
				mLightUniform[2 + offset] = Lights[i]->getColor().Z;
				//Position
				mLightUniform[3 + offset] = Lights[i]->getPos().X;
				mLightUniform[4 + offset] = Lights[i]->getPos().Y;
				mLightUniform[5 + offset] = Lights[i]->getPos().Z;
				//Direction
				mLightUniform[6 + offset] = Lights[i]->getDir().X;
				mLightUniform[7 + offset] = Lights[i]->getDir().Y;
				mLightUniform[8 + offset] = Lights[i]->getDir().Z;
				//Type
				mLightUniform[9 + offset] = static_cast<float>(Lights[i]->getType());
				//Constant attenuation
				mLightUniform[10 + offset] = Lights[i]->getConstant();
				//Linear attenuation
				mLightUniform[11 + offset] = Lights[i]->getLinear();
				//Quadratic attenuation
				mLightUniform[12 + offset] = Lights[i]->getQuadratic();
				//Inner cutoff
				mLightUniform[13 + offset] = Lights[i]->getInnerCutoff();
				//Outer cutoff
				mLightUniform[14 + offset] = Lights[i]->getOuterCutoff();
			} else
			{
				for (uint32 j = 0; j < 15; j++)
				{
					mLightUniform[j + offset] = -1;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////
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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &VBuf);
		glDeleteVertexArrays(1, &VAO);
	}

}













