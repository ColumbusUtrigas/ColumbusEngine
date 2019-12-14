#include <Common/Model/Model.h>
#include <Common/Model/CMF/ModelCMF.h>
#include <Common/Model/OBJ/ModelOBJ.h>

namespace Columbus
{

	Model::Model() {}

	bool Model::Load(const char* File)
	{
		FreeData();

		ModelLoader* Loader = nullptr;

		if (ModelLoaderCMF::IsCMF(File)) Loader = new ModelLoaderCMF();
		if (ModelLoaderOBJ::IsOBJ(File)) Loader = new ModelLoaderOBJ();

		if (Loader != nullptr)
		{
			if (Loader->Load(File) == false)
			{
				delete Loader;
				return false;
			}

			SubModels = Loader->SubModels;
			SubModelsCount = Loader->SubModelsCount;
			RecalculateBounds();
			RecalculateTangents();

			delete Loader;

			for (int a = 0; a < SubModelsCount; a++)
			{
				if (SubModels[a].UVs != nullptr)
				{
					for (uint32 i = 0; i < SubModels[a].VerticesCount; i++)
					{
						SubModels[a].UVs[i].Y = 1.0f - SubModels[a].UVs[i].Y;
					}
				}
			}

			return true;
		}

		return false;
	}

	void Model::FreeData()
	{
		for (int i = 0; i < GetSubModelsCount(); i++)
		{
			SubModels[i].FreeData();
		}

		delete[] SubModels;
		SubModelsCount = 0;
	}

	void Model::RecalculateBounds()
	{
		for (int a = 0; a < GetSubModelsCount(); a++)
		{
			if (SubModels[a].Positions != nullptr)
			{
				for (uint32 i = 0; i < SubModels[a].VerticesCount; i++)
				{
					if (SubModels[a].Positions[i].X < SubModels[a].BoundingBox.Min.X) SubModels[a].BoundingBox.Min.X = SubModels[a].Positions[i].X;
					if (SubModels[a].Positions[i].X > SubModels[a].BoundingBox.Max.X) SubModels[a].BoundingBox.Max.X = SubModels[a].Positions[i].X;
					if (SubModels[a].Positions[i].Y < SubModels[a].BoundingBox.Min.Y) SubModels[a].BoundingBox.Min.Y = SubModels[a].Positions[i].Y;
					if (SubModels[a].Positions[i].Y > SubModels[a].BoundingBox.Max.Y) SubModels[a].BoundingBox.Max.Y = SubModels[a].Positions[i].Y;
					if (SubModels[a].Positions[i].Z < SubModels[a].BoundingBox.Min.Z) SubModels[a].BoundingBox.Min.Z = SubModels[a].Positions[i].Z;
					if (SubModels[a].Positions[i].Z > SubModels[a].BoundingBox.Max.Z) SubModels[a].BoundingBox.Max.Z = SubModels[a].Positions[i].Z;
				}
			}
		}
	}

	void Model::RecalculateTangents()
	{
		Vector3 DeltaPos[2];
		Vector2 DeltaUV[2];
		Vector3 Tangent;
		float R;

		for (int a = 0; a < SubModelsCount; a++)
		{
			if (SubModels[a].Positions != nullptr && SubModels[a].UVs != nullptr && SubModels[a].Normals != nullptr)
			{
				delete[] SubModels[a].Tangents;
				SubModels[a].Tangents = new Vector3[SubModels[a].VerticesCount];

				for (uint32 i = 0; i < SubModels[a].VerticesCount; i += 3)
				{
					DeltaPos[0] = SubModels[a].Positions[i + 1] - SubModels[a].Positions[i];
					DeltaPos[1] = SubModels[a].Positions[i + 2] - SubModels[a].Positions[i];

					DeltaUV[0] = SubModels[a].UVs[i + 1] - SubModels[a].UVs[i];
					DeltaUV[1] = SubModels[a].UVs[i + 2] - SubModels[a].UVs[i];

					R = 1.0f / (DeltaUV[0].X * DeltaUV[1].Y - DeltaUV[0].Y * DeltaUV[1].X);
					Tangent = R * (DeltaPos[0] * DeltaUV[1].Y - DeltaPos[1] * DeltaUV[0].Y);

					for (int j = 0; j < 3; j++) SubModels[a].Tangents[i + j] = Tangent;
				}
			}
		}
	}

	Model::~Model() { FreeData(); }

}


