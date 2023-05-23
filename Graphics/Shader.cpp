#include <Graphics/Shader.h>
#include <Graphics/Device.h>
#include <System/Log.h>

namespace Columbus
{

	static const char* gErrorShader =
		#include <Shaders/Error.csl>
	;

	static const char* gScreenSpaceShader =
		#include <Shaders/ScreenSpace.csl>
	;

	static const char* gAutoExposureShader =
		#include <Shaders/AutoExposure.csl>
	;

	static const char* gTonemapShader =
		#include <Shaders/Tonemap.csl>
	;

	static const char* gResolveMSAAShader =
		#include <Shaders/ResolveMSAA.csl>
	;

	static const char* gGaussBlurShader =
		#include <Shaders/GaussBlur.csl>
	;

	static const char* gBloomBrightShader =
		#include <Shaders/BloomBright.csl>
	;

	static const char* gBloomShader =
		#include <Shaders/Bloom.csl>
	;

	static const char* gVignetteShader =
		#include <Shaders/Vignette.csl>
	;

	static const char* gFXAAShader =
		#include <Shaders/FXAA.csl>
	;

	static const char* gIconShader =
		#include <Shaders/Icon.csl>
	;

	static const char* gEditorToolsShader =
		#include <Shaders/EditorTools.csl>
	;

	static const char* gSkyboxShader =
		#include <Shaders/Skybox.csl>
	;

	static const char* gSkyboxCubemapGenerationShader =
		#include <Shaders/SkyboxCubemapGeneration.csl>
	;

	static const char* gIrradianceGenerationShader =
		#include <Shaders/IrradianceGeneration.csl>
	;

	static const char* gPrefilterGenerationShader =
		#include <Shaders/PrefilterGeneration.csl>
	;

	static const char* gIntegrationGenerationShader =
		#include <Shaders/IntegrationGeneration.csl>
	;

	DefaultShaders::DefaultShaders()
	{
#define LOAD_SHADER(name) \
name = std::unique_ptr<ShaderProgram>(gDevice->CreateShaderProgram()); \
name->LoadFromMemory(g##name##Shader, #name); \
name->Compile();

		printf("\n");
		Log::Initialization("Default shaders loading\n");

		LOAD_SHADER(Error);
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


