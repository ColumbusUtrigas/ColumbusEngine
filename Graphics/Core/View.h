#pragma once

#include "Math/Vector2.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Core/Reflection.h"
#include "Graphics/Camera.h"
#include "Graphics/Core/DebugRender.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include "UI/UISystem.h"

namespace Columbus
{

	struct HSkySettings
	{
		// scaterring coefficients: w parameter unused for GPU alignments
		Vector4 BetaRayleigh = Vector4(3.8e-6f, 13.5e-6f, 33.1e-6f, 0);
		Vector4 BetaMie      = Vector4(21e-6f, 21e-6f, 21e-6f, 0);
		Vector4 BetaOzone    = Vector4(2.04e-5f, 4.97e-5f, 1.95e-6f, 0);

		// samples
		int Samples = 8;
		int LightSamples = 1; // Set to more than 1 for a realistic, less vibrant sunset

		// scattering distributions, dimensions
		float PlanetRadiusKm     = 6371;
		float AtmosphereHeightKm = 100;
		float RayleightHeightKm  = 8;
		float MieHeight    = 1.2f;
		float OzonePeak    = 30;
		float OzoneFalloff = 3;

		// illuminance
		float SunIlluminance   = 128000;
		float MoonIlluminance  = 0.32f;
		float SpaceIlluminance = 0.01f;
		float Exposure = 16.0f;

		// sun disc
		Vector4 SunDiscColour = Vector4(1.0f, 0.95f, 0.85f, 0.0f) * 16;
		float SunAngularRadius = 0.267f; // degrees on a CPU, radians on a GPU
		float SunDiscSoftness = 0.5f; // divided by 1000 for a GPU

		// stars
		float StarsBrightness = 0.0f;
		float StarsDensity = 500.0f;

		// 128
	};

	struct HDepthOfFieldSettings
	{
		bool  EnableDoF = false;
		float FStop = 1.4f;
		float FocusDistance = 10.0f;
		float SensorSize = 0.1f;
	};

	struct HFilmGrainSettings
	{
		bool  EnableGrain = false;
		float GrainScale = 0.01f;
		float GrainAmount = 0.0f;
	};

	struct HChromaticAberrationSettings
	{
		bool  EnableChromaticAberration = false;
		float ChromaticAberrationAmount = 0.0f;
	};

	struct HVignetteSettings
	{
		bool  EnableVignette = false;
		float Vignette = 0.5f;
	};

	struct HColourCorrectionSettings
	{
		Vector4 Lift   = Vector4(0, 0, 0, 0);
		Vector4 Gamma  = Vector4(1, 1, 1, 0);
		Vector4 Gain   = Vector4(1, 1, 1, 0);
		Vector4 Offset = Vector4(0, 0, 0, 0);
		float Exposure    = 0.0f;
		float Contrast    = 1.0f;
		float Saturation  = 1.0f;
		float HueShift    = 0.0f;
		float Temperature = 6500.0f;
		float Tint        = 0.0f;
		int _pad[10]; // 128
	};

	struct HEffectsSettings
	{
		HSkySettings                 Sky;
		HColourCorrectionSettings    ColourCorrection;
		HDepthOfFieldSettings        DepthOfField;
		HFilmGrainSettings           FilmGrain;
		HChromaticAberrationSettings ChromaticAberration;
		HVignetteSettings            Vignette;

		void BlendWith(const HEffectsSettings& Other, float Weight);
	};


	struct RenderView
	{
		iVector2 OutputSize;
		iVector2 RenderSize; // internal resolution
		Camera CameraCur;
		Camera CameraPrev;
		DebugRender DebugRender;
		UISystem* UI = nullptr;

		HEffectsSettings EffectsSettings;

		bool ScreenshotHDR = false; // if true, screenshot is done before tonemapping
		char* ScreenshotPath = nullptr; // when is not nullptr, saves a screenshot on disk
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::HSkySettings,                 1, "C8CD8A8A-05D2-4428-B6C5-324C0333964F");
CREFLECT_DECLARE_STRUCT(Columbus::HColourCorrectionSettings,    1, "2AB3A87D-9012-4627-8F00-4634A5614EC0");
CREFLECT_DECLARE_STRUCT(Columbus::HDepthOfFieldSettings,        1, "C222F247-7DF5-4CD9-B515-DA23DC1CD9EB");
CREFLECT_DECLARE_STRUCT(Columbus::HFilmGrainSettings,           1, "9B03BC30-0A0B-4AED-96A8-278DEFDB81F0");
CREFLECT_DECLARE_STRUCT(Columbus::HChromaticAberrationSettings, 1, "98F10CDD-9535-4138-9F9A-5A0414B805F4");
CREFLECT_DECLARE_STRUCT(Columbus::HVignetteSettings,            1, "3DC8377D-265B-40BA-BAF6-CE24A45A3637");
CREFLECT_DECLARE_STRUCT(Columbus::HEffectsSettings,             1, "5DB245D6-0D72-4A63-ACA1-BFC883E64B80");