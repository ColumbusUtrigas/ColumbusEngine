#pragma once

#include "Types.h"

namespace Columbus
{

	static const char* ShaderTypeToString(ShaderType type)
	{
		switch (type)
		{
		case ShaderType::Vertex: return "Vertex";
		case ShaderType::Pixel: return "Pixel";
		case ShaderType::Hull: return "Hull";
		case ShaderType::Domain: return "Domain";
		case ShaderType::Geometry: return "Geometry";

		case ShaderType::Compute: return "Compute";

		case ShaderType::Raygen: return "Raygen";
		case ShaderType::Miss: return "Miss";
		case ShaderType::Anyhit: return "Anyhit";
		case ShaderType::ClosestHit: return "ClosestHit";
		case ShaderType::Intersection: return "Intersection";

		case ShaderType::AllGraphics:
		case ShaderType::AllRayTracing:
		default: return "Invalid shader type";
		}
	}

}
