/************************************************
*                  MeshOpenGL.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   16.01.2018                  *
*************************************************/
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	MeshOpenGL::MeshOpenGL()
	{
		glGenBuffers(1, &mVBuf);
	}
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> aVert)
	{
		glGenBuffers(1, &mVBuf);
		setVertices(aVert);
	}
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::MeshOpenGL(std::vector<Vertex> aVert, Material aMaterial)
	{
		glGenBuffers(1, &mVBuf);
		mMat = aMaterial;
		setVertices(aVert);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::setVertices(std::vector<Vertex> aVert)
	{
		mVert.clear();
		mVert = aVert;

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

		float* v = new float[mVert.size() * 3]; //Vertex buffer
		float* u = new float[mVert.size() * 2]; //UV buffer
		float* n = new float[mVert.size() * 3]; //Normal buffer
		float* t = new float[mVert.size() * 3]; //Tangent buffer
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;
		uint64_t tcounter = 0;

		for (auto Vertex : mVert)
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

		uint64 size = (sizeof(float) * mVert.size() * 3)
		            + (sizeof(float) * mVert.size() * 2)
		            + (sizeof(float) * mVert.size() * 3)
		            + (sizeof(float) * mVert.size() * 3);

		VOffset = 0;
		UOffset = VOffset + (sizeof(float) * mVert.size() * 3);
		NOffset = UOffset + (sizeof(float) * mVert.size() * 2);
		TOffset = NOffset + (sizeof(float) * mVert.size() * 3);

		glBindBuffer(GL_ARRAY_BUFFER, mVBuf);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);


		glBufferSubData(GL_ARRAY_BUFFER, VOffset, mVert.size() * 3 * sizeof(float), v);
		glBufferSubData(GL_ARRAY_BUFFER, UOffset, mVert.size() * 2 * sizeof(float), u);
		glBufferSubData(GL_ARRAY_BUFFER, NOffset, mVert.size() * 3 * sizeof(float), n);
		glBufferSubData(GL_ARRAY_BUFFER, TOffset, mVert.size() * 3 * sizeof(float), t);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		delete[] v;
		delete[] u;
		delete[] n;
		delete[] t;

		mOBB.Min = Vector3(OBBData.minX, OBBData.minY, OBBData.minZ);
		mOBB.Max = Vector3(OBBData.maxX, OBBData.maxY, OBBData.maxZ);

		if (mMat.getShader() == nullptr) return;

		if (!mMat.getShader()->IsCompiled())
		{
			mMat.getShader()->AddAttribute("aPos", 0);
			mMat.getShader()->AddAttribute("aUV", 1);
			mMat.getShader()->AddAttribute("aNorm", 2);
			mMat.getShader()->AddAttribute("aTang", 3);
			mMat.getShader()->Compile();
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::render(Transform InTransform)
	{
		if (mMat.getShader() == nullptr) return;
		if (!mMat.getShader()->IsCompiled())
		{
			mMat.getShader()->AddAttribute("aPos", 0);
			mMat.getShader()->AddAttribute("aUV", 1);
			mMat.getShader()->AddAttribute("aNorm", 2);
			mMat.getShader()->AddAttribute("aTang", 3);
			mMat.getShader()->Compile();
		}

		uint64 const offsets[4] = { VOffset, UOffset, NOffset, TOffset };
		uint32 const strides[4] = { 3, 2, 3, 3 };

		glBindBuffer(GL_ARRAY_BUFFER, mVBuf);

		for (uint32 i = 0; i < 4; i++)
		{
			glVertexAttribPointer(i, strides[i], GL_FLOAT, GL_FALSE, 0, (void*)offsets[i]);
			glEnableVertexAttribArray(i);
		}

		mMat.getShader()->Bind();

		setShaderMatrices(InTransform);
		setShaderMaterial();
		setShaderLightAndCamera();
		setShaderTextures();

		glDrawArrays(GL_TRIANGLES, 0, mVert.size());
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::setShaderTextures()
	{
		Texture* textures[3] = { mMat.getTexture(), mMat.getSpecMap(), mMat.getNormMap() };
		Cubemap* cubemap = mMat.getReflection();
		std::string unifs[3] = {"uMaterial.diffuseMap", "uMaterial.specularMap", "uMaterial.normalMap"};
		unsigned int indices[3] = {0, 1, 3};

		for (int i = 0; i < 3; i++)
		{
			if (textures[i] != nullptr)
			{
				mMat.getShader()->SetUniform1i(unifs[i].c_str(), indices[i]);
				textures[i]->sampler2D(indices[i]);
			}
			else
			{
				mMat.getShader()->SetUniform1i(unifs[i].c_str(), indices[i]);
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
			mMat.getShader()->SetUniform1i("uReflectionMap", 2);
			cubemap->samplerCube(2);
		}
		else
		{
			mMat.getShader()->SetUniform1i("uReflectionMap", 2);
			C_DeactiveCubemapOpenGL(C_OGL_TEXTURE2);
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::setShaderMatrices(Transform InTransform)
	{
		InTransform.GetMatrix().ElementsTransposed(UniformModelMatrix);
		mCamera.getViewMatrix().Elements(UniformViewMatrix);
		mCamera.getProjectionMatrix().ElementsTransposed(UniformProjectionMatrix);

		mPos = InTransform.GetPos();
		mMat.getShader()->SetUniformMatrix("uModel", UniformModelMatrix);
		mMat.getShader()->SetUniformMatrix("uView", UniformViewMatrix);
		mMat.getShader()->SetUniformMatrix("uProjection", UniformProjectionMatrix);
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::setShaderMaterial()
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

		mMat.getShader()->SetUniformArrayf("MaterialUnif", mMaterialUnif, 15);
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::setShaderLightAndCamera()
	{
		calculateLights();

		mMat.getShader()->SetUniformArrayf("LightUnif", mLightUniform, 120);
		mMat.getShader()->SetUniform3f("uCamera.pos", mCamera.getPos());
	}
	//////////////////////////////////////////////////////////////////////////////
	void MeshOpenGL::calculateLights()
	{
		sortLights();

		//8 - max count of lights, processing in shader
		for (uint32 i = 0; i < 8; i++)
		{
			uint32 offset = i * 15;

			if (i < mLights.size() && mMat.getLighting() == true)
			{
				//Color
				mLightUniform[0 + offset] = mLights[i]->getColor().X;
				mLightUniform[1 + offset] = mLights[i]->getColor().Y;
				mLightUniform[2 + offset] = mLights[i]->getColor().Z;
				//Position
				mLightUniform[3 + offset] = mLights[i]->getPos().X;
				mLightUniform[4 + offset] = mLights[i]->getPos().Y;
				mLightUniform[5 + offset] = mLights[i]->getPos().Z;
				//Direction
				mLightUniform[6 + offset] = mLights[i]->getDir().X;
				mLightUniform[7 + offset] = mLights[i]->getDir().Y;
				mLightUniform[8 + offset] = mLights[i]->getDir().Z;
				//Type
				mLightUniform[9 + offset] = static_cast<float>(mLights[i]->getType());
				//Constant attenuation
				mLightUniform[10 + offset] = mLights[i]->getConstant();
				//Linear attenuation
				mLightUniform[11 + offset] = mLights[i]->getLinear();
				//Quadratic attenuation
				mLightUniform[12 + offset] = mLights[i]->getQuadratic();
				//Inner cutoff
				mLightUniform[13 + offset] = mLights[i]->getInnerCutoff();
				//Outer cutoff
				mLightUniform[14 + offset] = mLights[i]->getOuterCutoff();
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
	void MeshOpenGL::sortLights()
	{
		Vector3 pos = mPos;

		mLights.erase(std::remove(mLights.begin(), mLights.end(), nullptr), mLights.end());

		auto func = [pos](const Light* a, const Light* b) mutable -> bool
		{
			Vector3 q = a->getPos();
			Vector3 w = b->getPos();

			return q.Length(pos) < w.Length(pos);
		};

		std::sort(mLights.begin(), mLights.end(), func);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	MeshOpenGL::~MeshOpenGL()
	{
		glDeleteBuffers(1, &mVBuf);
	}

}













