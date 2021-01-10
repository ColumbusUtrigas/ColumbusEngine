#include <Graphics/Types.h>
#include <Core/SmartPointer.h>
#include <Core/Assert.h>
#include <string_view>

#include <d3d12.h>
#include <d3dcompiler.h>

namespace Columbus::Graphics::DX12
{

	SPtr<D3D12_SHADER_BYTECODE> CompileShaderStage_DX12(SPtr<ShaderStage> stage, std::string_view target)
	{
		SPtr<D3D12_SHADER_BYTECODE> result(new D3D12_SHADER_BYTECODE());

		if (!stage)
			return result;

		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

		ID3DBlob* compiled;
		ID3DBlob* error;
		ID3DBlob* disasm;

		const char* strtype = "";

		switch (stage->Type)
		{
		case ShaderType::Vertex:   strtype = "Vertex";   break;
		case ShaderType::Pixel:    strtype = "Pixel";    break;
		case ShaderType::Hull:     strtype = "Hull";     break;
		case ShaderType::Domain:   strtype = "Domain";   break;
		case ShaderType::Geometry: strtype = "Geometry"; break;
		case ShaderType::Compute:  strtype = "Compute";  break;
		}

		const auto& src = stage->Source;
		const auto& entry = stage->EntryPoint;
		auto hr = D3DCompile(src.data(), src.size(), strtype, nullptr, nullptr, entry.data(), target.data(), compileFlags, 0, &compiled, &error);

		if (FAILED(hr))
		{
			COLUMBUS_ASSERT_MESSAGE(false, (char*)error->GetBufferPointer());
		}

		result->BytecodeLength = compiled->GetBufferSize();
		result->pShaderBytecode = compiled->GetBufferPointer();

		return result;
	}

}
