#include "Graphics/Core/ShadowAtlasAllocator.h"
#include "Graphics/Core/ShaderDebugText.h"
#include "Math/Matrix.h"
#include "Math/Frustum.h"
#include "Math/Geometry.h"
#include <array>
#include <limits>
#include <cstring>
#include <filesystem>
#include "ShaderBytecode/ShaderBytecode.h"
#include "Lib/SPIRV-Reflect/include/spirv/unified1/spirv.h"
#include "Graphics/World.h"
#include "Core/Serialisation.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

static void Require(bool value, const char* message)
{
	if (!value)
	{
		std::fprintf(stderr, "%s\n", message);
		std::abort();
	}
}

static void TestShaderDebugStrings()
{
	using namespace Columbus;
	const std::string_view Source = "Lights: 42\n\tHello, shaders!";
	const auto Text = ShaderDebugText::MakeString(Source);
	Require(Text.Length == Source.size(), "Shader debug string length mismatch");

	for (u32 i = 0; i < Text.Length; ++i)
	{
		const u32 Character = (Text.Characters[i / 4] >> ((i % 4) * 8)) & 255;
		Require(Character == u8(Source[i]), "Shader debug ASCII packing crossed a word boundary incorrectly");
	}

	std::array<char, 64> Full;
	Full.fill('W');
	const auto Maximum = ShaderDebugText::MakeString(std::string_view(Full.data(), Full.size()));
	Require(Maximum.Length == 64 && Maximum.Characters[15] == 0x57575757, "Shader debug strings must preserve all 64 characters");

	const auto Empty = ShaderDebugText::MakeString("");
	Require(Empty.Length == 0, "Empty shader debug strings must have zero length");

	for (u32 Word : Empty.Characters)
	{
		Require(Word == 0, "Empty shader debug strings must initialize packed words");
	}

	std::puts("PASS shader debug ASCII packing, controls, empty strings and maximum capacity");
}

// Check the deployed bytecode: a runtime branch around clip still carries a
// discard instruction and does not provide the opaque fast path we require.
static bool HasDiscard(const CompiledShaderBytecode& shader)
{
	const auto& bytes = shader.Bytecode;
	Require(bytes.size() >= 5 * sizeof(uint32_t) && bytes.size() % sizeof(uint32_t) == 0, "Invalid SPIR-V size");
	auto word = [&](size_t index)
	{
		uint32_t value;
		std::memcpy(&value, bytes.data() + index * sizeof(value), sizeof(value));
		return value;
	};
	Require(word(0) == SpvMagicNumber, "Invalid SPIR-V magic");

	for (size_t offset = 5; offset < bytes.size() / sizeof(uint32_t);)
	{
		const uint32_t instruction = word(offset);
		const uint32_t count       = instruction >> 16;
		Require(count > 0 && offset + count <= bytes.size() / sizeof(uint32_t), "Invalid SPIR-V instruction");
		const auto opcode = static_cast<SpvOp>(instruction & 0xffff);

		if (opcode == SpvOpKill || opcode == SpvOpTerminateInvocation || opcode == SpvOpDemoteToHelperInvocation)
		{
			return true;
		}

		offset += count;
	}

	return false;
}

static void TestRasterCoverageShaders(const std::filesystem::path& shaderDirectory)
{
	using namespace Columbus;
	auto load = [&](const char* name)
	{
		const auto path = shaderDirectory / name;
		Require(std::filesystem::exists(path), "Missing deployed raster coverage shader; build Shaders first");
		return LoadCompiledShaderData(path.string());
	};
	const auto ShadowDepth = load("ShadowDepth.csd");
	const bool HasCoverageAxis = ShadowDepth.PermutationAxes.size() == 1 && ShadowDepth.PermutationAxes[0].Name == "ALPHA_MASKED" &&
	                             ShadowDepth.PermutationAxes[0].MinValue == 0 && ShadowDepth.PermutationAxes[0].MaxValue == 1;
	Require(HasCoverageAxis && ShadowDepth.Permutations.size() == 2, "Shadow depth requires opaque and masked permutations in one CSD");

	bool FoundOpaque = false;
	bool FoundMasked = false;

	for (const auto& Permutation : ShadowDepth.Permutations)
	{
		Require(Permutation.AxisValues.size() == 1, "Missing shadow coverage permutation value");
		const bool Masked = Permutation.AxisValues[0] == 1;
		Require(Permutation.AxisValues[0] == 0 || Masked, "Invalid shadow coverage permutation value");

		bool& FoundPermutation = Masked ? FoundMasked : FoundOpaque;
		Require(!FoundPermutation, "Duplicate shadow coverage permutation value");
		FoundPermutation = true;

		Require(Permutation.Bytecodes.size() == (Masked ? 2 : 1), "Only masked shadows may contain a pixel stage");
		bool FoundVertex = false;
		bool FoundPixel  = false;

		for (const auto& Shader : Permutation.Bytecodes)
		{
			if (Shader.Stage == ShaderType::Vertex)
			{
				FoundVertex = true;
			}
			else if (Shader.Stage == ShaderType::Pixel)
			{
				FoundPixel = true;
				Require(HasDiscard(Shader), "Masked shadow coverage must discard fragments below the cutoff");
			}
		}

		Require(FoundVertex && (FoundPixel == Masked), "Opaque shadows must be vertex-only; masked shadows require vertex and pixel stages");
	}

	Require(FoundOpaque && FoundMasked, "Missing opaque or masked shadow permutation");

	const auto gbuffer = load("GBufferPass.csd");
	Require(gbuffer.PermutationAxes.size() == 1 && gbuffer.PermutationAxes[0].Name == "ALPHA_MASKED" &&
	            gbuffer.PermutationAxes[0].MinValue == 0 && gbuffer.PermutationAxes[0].MaxValue == 1 &&
	            gbuffer.Permutations.size() == 2,
	    "GBuffer requires opaque and masked permutations");

	for (const auto& permutation : gbuffer.Permutations)
	{
		Require(permutation.AxisValues.size() == 1, "Missing GBuffer coverage permutation value");
		bool foundPixel = false;

		for (const auto& shader : permutation.Bytecodes)
		{
			if (shader.Stage == ShaderType::Pixel)
			{
				foundPixel = true;
				Require(HasDiscard(shader) == (permutation.AxisValues[0] != 0),
				    "Only the masked GBuffer shader may discard fragments");
			}
		}

		Require(foundPixel, "GBuffer requires material pixel shading in both variants");
	}

	std::puts("PASS deployed opaque/masked shadow stages and GBuffer discard separation");
}

static bool Overlap(
    const Columbus::ShadowAtlasAllocator::Allocation& a, const Columbus::ShadowAtlasAllocator::Allocation& b)
{
	return a.X < b.X + b.Size && b.X < a.X + a.Size && a.Y < b.Y + b.Size && b.Y < a.Y + a.Size;
}

static bool Contains(const Columbus::Box& box, const Columbus::Vector3& point)
{
	return point.X >= box.Min.X - 1e-4f && point.X <= box.Max.X + 1e-4f && point.Y >= box.Min.Y - 1e-4f &&
	       point.Y <= box.Max.Y + 1e-4f && point.Z >= box.Min.Z - 1e-4f && point.Z <= box.Max.Z + 1e-4f;
}

static void TestShadowCulling()
{
	using namespace Columbus;
	using namespace Columbus::Geometry;
	Matrix projection;
	projection.Perspective(90, 1, 0.1f, 10);
	const Frustum frustum(projection, Frustum::DepthRange::ZeroToOne);
	Require(frustum.Intersects(Box(Vector3(-0.1f, -0.1f, -2), Vector3(0.1f, 0.1f, -1))), "Visible caster rejected");
	Require(!frustum.Intersects(Box(Vector3(-0.1f, -0.1f, 1), Vector3(0.1f, 0.1f, 2))), "Caster behind light accepted");
	Require(!frustum.Intersects(Box(Vector3(3, 0, -2), Vector3(4, 1, -1))), "Caster outside side plane accepted");
	Require(!frustum.Intersects(Box(Vector3(-0.01f, -0.01f, -0.05f), Vector3(0.01f, 0.01f, -0.01f))),
	    "Reverse-Z near-plane rejection failed");
	Require(!frustum.Intersects(Box(Vector3(-1, -1, -12), Vector3(1, 1, -11))), "Reverse-Z far-plane rejection failed");
	Require(frustum.Intersects(Box(Vector3(-1, -1, -1), Vector3(1, 1, 1))), "Caster crossing near plane rejected");
	Require(frustum.Intersects(Sphere{ Vector3(0, 0, -2), 0.5f }), "Visible light sphere rejected");
	Require(!frustum.Intersects(Sphere{ Vector3(0, 0, 2), 0.5f }), "Light sphere behind camera accepted");
	Require(frustum.Intersects(Sphere{ Vector3(0), 2 }), "Camera inside light sphere rejected");
	Require(!frustum.Intersects(Sphere{ Vector3(0, 0, -12), 1 }), "Light sphere beyond far plane accepted");
	Require(frustum.Intersects(Sphere{ Vector3(0), -1 }), "Unknown light sphere must remain visible");

	const std::array<Vector3, 4> Outside = { Vector3(3, -1, -1), Vector3(3, 1, -1), Vector3(3, -1, -2), Vector3(3, 1, -2) };
	Require(!frustum.Intersects(Outside), "Convex bounds outside frustum accepted");
	Require(frustum.Intersects(Outside, 2), "World-space receiver bias margin was ignored");
	const std::array<Vector3, 2> Crossing = { Vector3(-10, 0, -2), Vector3(10, 0, -2) };
	Require(frustum.Intersects(Crossing), "Crossing convex bounds with no inside vertices rejected");

	Matrix guard(1);
	guard.M[0][0] = guard.M[1][1] = 504.0f / 512;
	const Box seam(Vector3(1.01f, 0, -1), Vector3(1.01f, 0, -1));
	Require(!frustum.Intersects(seam) && Frustum(guard * projection, Frustum::DepthRange::ZeroToOne).Intersects(seam),
	    "PCF guard-band caster rejected");
	Matrix ortho;
	ortho.Ortho(-2, 2, -2, 2, 0, 20);
	Require(Frustum(ortho, Frustum::DepthRange::ZeroToOne).Intersects(Box(Vector3(-1, -1, -15), Vector3(1, 1, -14))),
	    "Directional caster extrusion rejected");
	Require(!Frustum(ortho, Frustum::DepthRange::ZeroToOne).Intersects(Box(Vector3(3, 0, -15), Vector3(4, 1, -14))),
	    "Directional side-plane rejection failed");
	Require(BoxSphereIntersection(Box(Vector3(2, -1, -1), Vector3(3, 1, 1)), { Vector3(0), 2 }),
	    "Sphere-tangent caster rejected");
	Require(!BoxSphereIntersection(Box(Vector3(3), Vector3(4)), { Vector3(0), 2 }),
	    "Distant caster accepted by light range");
	Require(BoxSphereIntersection(Box(Vector3(2.0001f, -1, -1), Vector3(3, 1, 1)), { Vector3(0), 2 }, 0.0002f),
	    "Conservative sphere tolerance must retain near-tangent bounds");
	Require(!BoxSphereIntersection(Box(Vector3(2.0001f, -1, -1), Vector3(3, 1, 1)), { Vector3(0), 2 }),
	    "Exact sphere test must not inherit shadow culling tolerance");
	const Box unknown(Vector3(1), Vector3(-1));
	Require(!unknown.IsValid(), "Inverted box must be invalid");
	Require(frustum.Intersects(unknown) && BoxSphereIntersection(unknown, { Vector3(0), 1 }),
	    "Unknown bounds must remain visible");
	const Box invalid(Vector3(std::numeric_limits<float>::quiet_NaN()), Vector3(1));
	Require(!invalid.IsValid(), "Non-finite box must be invalid");
	Require(frustum.Intersects(invalid) && BoxSphereIntersection(invalid, { Vector3(0), 1 }),
	    "Non-finite bounds must remain visible");

	const Box    base(Vector3(-1, -2, -3), Vector3(2, 3, 4));
	const Matrix transform(
	    Vector4(0, -2, 0, 5), Vector4(-3, 0, 0, -4), Vector4(0.2f, 0, 0.5f, -10), Vector4(0, 0, 0, 1));
	const Box transformed = base.CalcTransformedBox(transform);
	Require(base.IsValid() && transformed.IsValid(), "Valid transformed box rejected");
	Require(!unknown.CalcTransformedBox(transform).IsValid(), "Transform must preserve unknown bounds");

	for (int corner = 0; corner < 8; ++corner)
	{
		const Vector3 p(corner & 1 ? base.Min.X : base.Max.X, corner & 2 ? base.Min.Y : base.Max.Y,
		    corner & 4 ? base.Min.Z : base.Max.Z);
		Require(Contains(transformed, (transform * Vector4(p, 1)).XYZ()), "Negative-scale/sheared bound lost a corner");
	}

	const std::array<Box, 2>   morphs              = { Box(Vector3(-2, 0, -1), Vector3(3, 1, 4)),
		Box(Vector3(-1, -2, 0), Vector3(1, 2, 1)) };
	const std::array<float, 2> weights             = { -2, 1.5f };
	std::array<Matrix, 2>      skins               = { Matrix(1), Matrix(1) };
	skins[0].M[0][3]                               = 15;
	skins[1].M[2][3]                               = -20;
	const Box                             animated = base.CalcDeformedBox(morphs, weights, skins);
	std::mt19937                          random(456);
	std::uniform_real_distribution<float> uniform(0, 1);

	for (int i = 0; i < 1000; ++i)
	{
		const auto sample = [&uniform, &random](const Box& box)
		{ return box.Min + box.CalcSize() * Vector3(uniform(random), uniform(random), uniform(random)); };
		Vector3 p = sample(base);

		for (size_t j = 0; j < morphs.size(); ++j)
		{
			p += sample(morphs[j]) * weights[j];
		}

		Require(Contains(animated, p), "Unweighted animated vertex lost");
		const float   blend   = uniform(random);
		const Vector3 skinned = ((skins[0] * Vector4(p, 1)) * blend + (skins[1] * Vector4(p, 1)) * (1 - blend)).XYZ();
		Require(Contains(animated, skinned), "Skinned/morphed animated vertex lost");
	}

	std::puts(
	    "PASS conservative shadow range/frustum/guard-band culling and transformed, skinned, signed-morph bounds");
}

int main(int, char** argv)
{
	using namespace Columbus;
	TestShaderDebugStrings();
	TestRasterCoverageShaders(std::filesystem::absolute(argv[0]).parent_path() / "PrecompiledShaders");
	TestShadowCulling();
	ShadowAtlasAllocator                          allocator(4096);
	std::vector<ShadowAtlasAllocator::Allocation> blocks;

	for (int i = 0; i < 4; ++i)
	{
		auto block = allocator.Allocate(2048);
		Require(block.has_value(), "Four 2K blocks must fit a 4K atlas");

		for (auto other : blocks)
		{
			Require(!Overlap(*block, other), "Allocated blocks overlap");
		}

		blocks.push_back(*block);
	}

	Require(!allocator.Allocate(32), "Full atlas must reject further allocations");
	Require(allocator.Free(blocks[1]), "Free failed");
	Require(!allocator.Free(blocks[1]), "Double free must fail");
	auto replacement = allocator.Allocate(2048);
	Require(replacement && *replacement == blocks[1], "Freed block should be reused without moving other blocks");

	for (auto block : blocks)
	{
		Require(allocator.Free(block), "Free failed");
	}

	auto whole = allocator.Allocate(4096);
	Require(whole.has_value(), "Free siblings must coalesce to the full atlas");
	Require(allocator.Free(*whole), "Full-atlas free failed");
	Require(!allocator.Allocate(33) && !allocator.Allocate(8192), "Invalid request accepted");
	Require(!allocator.Free({ 1, 0, 32 }), "Misaligned free accepted");

	std::mt19937 random(123);
	blocks.clear();

	for (int iteration = 0; iteration < 20000; ++iteration)
	{
		if (!blocks.empty() && random() % 3 == 0)
		{
			auto index = random() % blocks.size();
			Require(allocator.Free(blocks[index]), "Random free failed");
			blocks.erase(blocks.begin() + index);
		}
		else if (auto block = allocator.Allocate(32u << (random() % 7)))
		{
			Require(block->X + block->Size <= 4096 && block->Y + block->Size <= 4096, "Out of bounds allocation");

			for (auto other : blocks)
			{
				Require(!Overlap(*block, other), "Random allocations overlap");
			}

			blocks.push_back(*block);
		}
	}

	for (auto block : blocks)
	{
		Require(allocator.Free(block), "Final free failed");
	}

	Require(allocator.Allocate(4096).has_value(), "Random allocations failed to coalesce");

	Matrix projection;
	projection.Ortho(-3, 5, -2, 6, 0.2f, 200);
	auto nearPoint = projection * Vector4(-3, -2, -0.2f, 1);
	auto farPoint  = projection * Vector4(5, 6, -200, 1);
	Require(fabsf(nearPoint.X + 1) < 1e-5f && fabsf(nearPoint.Y + 1) < 1e-5f && fabsf(nearPoint.Z - 1) < 1e-5f,
	    "Incorrect orthographic near mapping");
	Require(fabsf(farPoint.X - 1) < 1e-5f && fabsf(farPoint.Y - 1) < 1e-5f && fabsf(farPoint.Z) < 1e-5f,
	    "Incorrect reverse-Z far mapping");
	Reflection::Initialise();
	ALight source;
	source.Shadows            = true;
	source.RayTracedShadows   = false;
	source.ShadowResolution   = 2048;
	source.ShadowDepthBias    = 0.003f;
	source.ShadowNormalBias   = 0.04f;
	source.ShadowSlopeBias    = 2;
	source.ShadowFilterRadius = 2;
	source.ShadowNearPlane    = 0.1f;
	source.ShadowDistance     = 80;
	source.ShadowCascadeCount = 3;
	source.ShadowSplitLambda  = 0.8f;
	source.ShadowCascadeBlend = 0.2f;
	source.ShadowConeAngle    = 160;
	nlohmann::json json;
	Reflection_SerialiseStructJson(source, json);
	ALight restored;
	Reflection_DeserialiseStructJson(restored, json);
	nlohmann::json roundtrip;
	Reflection_SerialiseStructJson(restored, roundtrip);
	Require(json == roundtrip, "Reflected light properties did not roundtrip through JSON");
	Require(json["ShadowResolution"] == 2048 && json["RayTracedShadows"] == false, "Missing light shadow properties");
	json.erase("RayTracedShadows");
	ALight legacy;
	Reflection_DeserialiseStructJson(legacy, json);
	Require(legacy.RayTracedShadows, "Legacy lights must default to ray-traced shadows");
	std::puts("PASS reflected light shadow properties roundtrip and legacy defaults");
	std::puts("PASS shadow atlas allocation, reuse, coalescing, randomized non-overlap and reverse-Z orthographic "
	          "projection");
}
