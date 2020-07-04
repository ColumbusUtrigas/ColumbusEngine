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
			RecalculateBounds();
			RecalculateTangents();

			delete Loader;

			for (auto& model : SubModels)
			{
				if (model.UVs != nullptr)
				{
					for (uint32 i = 0; i < model.VerticesCount; i++)
					{
						model.UVs[i].Y = 1.0f - model.UVs[i].Y;
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
	}

	void Model::RecalculateBounds()
	{
		for (auto& model : SubModels)
		{
			if (model.Positions != nullptr)
			{
				for (uint32 i = 0; i < model.VerticesCount; i++)
				{
					if (model.Positions[i].X < model.BoundingBox.Min.X) model.BoundingBox.Min.X = model.Positions[i].X;
					if (model.Positions[i].X > model.BoundingBox.Max.X) model.BoundingBox.Max.X = model.Positions[i].X;
					if (model.Positions[i].Y < model.BoundingBox.Min.Y) model.BoundingBox.Min.Y = model.Positions[i].Y;
					if (model.Positions[i].Y > model.BoundingBox.Max.Y) model.BoundingBox.Max.Y = model.Positions[i].Y;
					if (model.Positions[i].Z < model.BoundingBox.Min.Z) model.BoundingBox.Min.Z = model.Positions[i].Z;
					if (model.Positions[i].Z > model.BoundingBox.Max.Z) model.BoundingBox.Max.Z = model.Positions[i].Z;
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

		for (auto& model : SubModels)
		{
			if (model.Positions != nullptr && model.UVs != nullptr && model.Normals != nullptr)
			{
				delete[] model.Tangents;
				model.Tangents = new Vector3[model.VerticesCount];

				for (uint32 i = 0; i < model.VerticesCount; i += 3)
				{
					DeltaPos[0] = model.Positions[i + 1] - model.Positions[i];
					DeltaPos[1] = model.Positions[i + 2] - model.Positions[i];

					DeltaUV[0] = model.UVs[i + 1] - model.UVs[i];
					DeltaUV[1] = model.UVs[i + 2] - model.UVs[i];

					R = 1.0f / (DeltaUV[0].X * DeltaUV[1].Y - DeltaUV[0].Y * DeltaUV[1].X);
					Tangent = R * (DeltaPos[0] * DeltaUV[1].Y - DeltaPos[1] * DeltaUV[0].Y);

					for (int j = 0; j < 3; j++) model.Tangents[i + j] = Tangent;
				}
			}
		}
	}

	Model::~Model() { FreeData(); }

}


