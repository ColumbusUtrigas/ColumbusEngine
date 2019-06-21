#include <Common/Model/Model.h>
#include <Common/Model/CMF/ModelCMF.h>

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

namespace Columbus
{

	Model::Model() {}

	bool Model::Load(const char* File)
	{
		FreeData();

		ModelLoader* Loader = nullptr;

		if (ModelLoaderCMF::IsCMF(File)) Loader = new ModelLoaderCMF();

		if (Loader != nullptr)
		{
			if (Loader->Load(File) == false)
			{
				delete Loader;
				return false;
			}

			Exist         = true;
			Indexed       = Loader->Indexed;
			VerticesCount = Loader->VerticesCount;
			IndicesCount  = Loader->IndicesCount;
			IndexSize     = Loader->IndexSize;

			Positions = Loader->Positions;
			UVs       = Loader->UVs;
			Normals   = Loader->Normals;
			Tangents  = Loader->Tangents;
			Indices   = Loader->Indices;

			BoundingBox = Loader->BoundingBox;

			if (Vertices != nullptr && Indices != nullptr && Indexed)
			{
				// On the moment of writing this code I have not discrete GPU, only laptop with Intel HD4000.
				// So, on integrated GPU cache optimizers do not work.
				// In the future, I will test cache optimizers (AMD Tootle, NvTriStrip) and maybe use it.
				// This block of code needs AMD Tootle library
				/*
				TootleInit();

				uint32* OutIndices = new uint32[IndicesCount];
				for (uint32 i = 0; i < IndicesCount; i++)
				{
					switch (IndexSize)
					{
					case 1: OutIndices[i] = ((uint8*) Indices)[i]; break;
					case 2: OutIndices[i] = ((uint16*)Indices)[i]; break;
					case 4: OutIndices[i] = ((uint32*)Indices)[i]; break;
					}
				}

				TootleOptimizeVCache(OutIndices, IndicesCount / 3, VerticesCount, 8, OutIndices, nullptr);

				for (uint32 i = 0; i < IndicesCount; i++)
				{
					switch (IndexSize)
					{
					case 1: ((uint8*)Indices)[i]  = OutIndices[i]; break;
					case 2: ((uint16*)Indices)[i] = OutIndices[i]; break;
					case 4: ((uint32*)Indices)[i] = OutIndices[i]; break;
					}
				}

				delete[] OutIndices;
				*/
			}

			// Calculate tangent space
			if (Positions != nullptr && UVs != nullptr && Normals != nullptr && Tangents == nullptr)
			{
				Tangents = new Vector3[VerticesCount];

				Vector3 DeltaPos[2];
				Vector2 DeltaUV[2];
				Vector3 Tangent;
				float R;

				if (Indexed)
				{
					uint32 CurrentIndices[3];

					for (uint32 i = 0; i < IndicesCount; i += 3)
					{
						for (int j = 0; j < 3; j++)
						{
							switch (IndexSize)
							{
							case 1: CurrentIndices[j] = ((uint8*) Indices)[i + j]; break;
							case 2: CurrentIndices[j] = ((uint16*)Indices)[i + j]; break;
							case 4: CurrentIndices[j] = ((uint32*)Indices)[i + j]; break;
							}
						}

						DeltaPos[0] = Positions[CurrentIndices[1]] - Positions[CurrentIndices[0]];
						DeltaPos[1] = Positions[CurrentIndices[2]] - Positions[CurrentIndices[0]];

						DeltaUV[0] = UVs[CurrentIndices[1]] - UVs[CurrentIndices[0]];
						DeltaUV[1] = UVs[CurrentIndices[2]] - UVs[CurrentIndices[0]];

						R = 1.0f / (DeltaUV[0].X * DeltaUV[1].Y - DeltaUV[0].Y * DeltaUV[1].X);
						Tangent = R * (DeltaPos[0] * DeltaUV[1].Y - DeltaPos[1] * DeltaUV[0].Y);

						for (int j = 0; j < 3; j++) Tangents[CurrentIndices[j]] = Tangent;
					}
				}
				else
				{
					for (uint32 i = 0; i < VerticesCount; i += 3)
					{
						DeltaPos[0] = Positions[i + 1] - Positions[i];
						DeltaPos[1] = Positions[i + 2] - Positions[i];

						DeltaUV[0] = UVs[i + 1] - UVs[i];
						DeltaUV[1] = UVs[i + 2] - UVs[i];

						R = 1.0f / (DeltaUV[0].X * DeltaUV[1].Y - DeltaUV[0].Y * DeltaUV[1].X);
						Tangent = R * (DeltaPos[0] * DeltaUV[1].Y - DeltaPos[1] * DeltaUV[0].Y);

						for (int j = 0; j < 3; j++) Tangents[i + j] = Tangent;
					}
				}
			}

			delete Loader;
			return true;
		}

		/*Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(File,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate);

		if (scene != nullptr)
		{
			if (scene->HasMeshes())
			{
				Exist         = true;
				Indexed       = false;
				VerticesCount = scene->mMeshes[0]->mNumVertices;
				IndicesCount  = 0;
				IndexSize     = 0;

				Positions = new Vector3[VerticesCount];
				UVs       = new Vector2[VerticesCount];
				Normals   = new Vector3[VerticesCount];
				Tangents  = new Vector3[VerticesCount];

				memcpy(Positions, scene->mMeshes[0]->mVertices, VerticesCount * sizeof(Vector3));
				memcpy(Normals, scene->mMeshes[0]->mNormals, VerticesCount * sizeof(Vector3));
				memcpy(Tangents, scene->mMeshes[0]->mTangents, VerticesCount * sizeof(Vector3));

				for (uint32 i = 0; i < VerticesCount; i++)
					UVs[i] = Vector2(scene->mMeshes[0]->mTextureCoords[0][i].x, scene->mMeshes[0]->mTextureCoords[0][i].y);

				BoundingBox = Box({-1000}, {1000});

				return true;
			}
		}*/

		return false;
	}

	void Model::FreeData()
	{
		Indexed = false;
		Exist = false;
		VerticesCount = 0;
		IndicesCount = 0;
		delete[] Positions;
		delete[] UVs;
		delete[] Normals;
		delete[] Tangents;
		delete[] Vertices;

		switch (IndexSize)
		{
		case 1: delete[] static_cast<uint8*> (Indices); break;
		case 2: delete[] static_cast<uint16*>(Indices); break;
		case 4: delete[] static_cast<uint32*>(Indices); break;
		}
	}

	Model::~Model() { FreeData(); }

}


