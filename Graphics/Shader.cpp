#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	static const char* gScreenSpaceShader =
		#include <Graphics/Shaders/ScreenSpace.csl>
	;

	static const char* gAutoExposureShader =
		#include <Graphics/Shaders/AutoExposure.csl>
	;

	static const char* gTonemapShader =
		#include <Graphics/Shaders/Tonemap.csl>
	;

	static const char* gResolveMSAAShader =
		#include <Graphics/Shaders/ResolveMSAA.csl>
	;

	static const char* gGaussBlurShader =
		#include <Graphics/Shaders/GaussBlur.csl>
	;

	static const char* gBloomBrightShader =
		#include <Graphics/Shaders/BloomBright.csl>
	;

	static const char* gBloomShader =
		#include <Graphics/Shaders/Bloom.csl>
	;

	static const char* gVignetteShader =
		#include <Graphics/Shaders/Vignette.csl>
	;

	static const char* gFXAAShader =
		#include <Graphics/Shaders/FXAA.csl>
	;

	static const char* gIconShader =
		#include <Graphics/Shaders/Icon.csl>
	;

	static const char* gEditorToolsShader =
		#include <Graphics/Shaders/EditorTools.csl>
	;

	static const char* gSkyboxShader =
		#include <Graphics/Shaders/Skybox.csl>
	;

	static const char* gSkyboxCubemapGenerationShader =
		#include <Graphics/Shaders/SkyboxCubemapGeneration.csl>
	;

	static const char* gIrradianceGenerationShader =
		#include <Graphics/Shaders/IrradianceGeneration.csl>
	;

	static const char* gPrefilterGenerationShader =
		#include <Graphics/Shaders/PrefilterGeneration.csl>
	;

	static const char* gIntegrationGenerationShader =
		#include <Graphics/Shaders/IntegrationGeneration.csl>
	;

	DefaultShaders::DefaultShaders()
	{
#define LOAD_SHADER(name) \
name = std::unique_ptr<ShaderProgram>(gDevice->CreateShaderProgram()); \
name->LoadFromMemory(g##name##Shader, #name); \
name->Compile();

		printf("\n");
		Log::Initialization("Default shaders loading\n");

		LOAD_SHADER(ScreenSpace);
		LOAD_SHADER(AutoExposure);
		LOAD_SHADER(Tonemap);
		LOAD_SHADER(ResolveMSAA);
		LOAD_SHADER(GaussBlur);
		LOAD_SHADER(BloomBright);
		LOAD_SHADER(Bloom);
		LOAD_SHADER(Vignette);
		LOAD_SHADER(FXAA);
		LOAD_SHADER(Icon);
		LOAD_SHADER(EditorTools);
		LOAD_SHADER(Skybox);
		LOAD_SHADER(SkyboxCubemapGeneration);
		LOAD_SHADER(IrradianceGeneration);
		LOAD_SHADER(PrefilterGeneration);
		LOAD_SHADER(IntegrationGeneration);
	}

	DefaultShaders::~DefaultShaders()
	{
	}

}


