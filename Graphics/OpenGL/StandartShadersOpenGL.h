#pragma once

namespace Columbus
{

	const char* gScreenSpaceShader =
		#include <Graphics/Shaders/ScreenSpace.csl>
	;

	const char* gAutoExposureShader =
		#include <Graphics/Shaders/AutoExposure.csl>
	;

	const char* gTonemapShader =
		#include <Graphics/Shaders/Tonemap.csl>
	;

	const char* gResolveMSAAShader =
		#include <Graphics/Shaders/ResolveMSAA.csl>
	;

	const char* gGaussBlurShader =
		#include <Graphics/Shaders/GaussBlur.csl>
	;

	const char* gBloomBrightShader =
		#include <Graphics/Shaders/BloomBright.csl>
	;

	const char* gBloomShader =
		#include <Graphics/Shaders/Bloom.csl>
	;

	const char* gVignetteShader =
		#include <Graphics/Shaders/Vignette.csl>
	;

	const char* gFXAAShader =
		#include <Graphics/Shaders/FXAA.csl>
	;

	const char* gIconShader =
		#include <Graphics/Shaders/Icon.csl>
	;

	const char* gEditorToolsShader =
		#include <Graphics/Shaders/EditorTools.csl>
	;

	const char* gSkyboxShader =
		#include <Graphics/Shaders/Skybox.csl>
	;

	const char* gSkyboxCubemapGenerationShader =
		#include <Graphics/Shaders/SkyboxCubemapGeneration.csl>
	;

	const char* gIrradianceGenerationShader =
		#include <Graphics/Shaders/IrradianceGeneration.csl>
	;

	const char* gPrefilterGenerationShader =
		#include <Graphics/Shaders/PrefilterGeneration.csl>
	;

	const char* gIntegrationGenerationShader =
		#include <Graphics/Shaders/IntegrationGeneration.csl>
	;

}


