#include <Common/Model/Model.h>
#include <Common/Model/CMF/ModelCMF.h>

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

			Positions = Loader->Positions;
			UVs       = Loader->UVs;
			Normals   = Loader->Normals;
			Tangents  = Loader->Tangents;
			Indices   = Loader->Indices;

			BoundingBox = Loader->BoundingBox;

			if (Positions != nullptr && UVs != nullptr && Normals != nullptr && Tangents == nullptr)
			{
				Tangents = new Vector3[VerticesCount];

				Vector3 DeltaPos[2];
				Vector2 DeltaUV[2];
				Vector3 Tangent;
				float R;

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

			delete Loader;
			return true;
		}

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
		delete[] Indices;
	}

	Model::~Model() { FreeData(); }

}




