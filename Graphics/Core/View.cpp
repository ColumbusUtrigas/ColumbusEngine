#include "View.h"

namespace Columbus
{

#define BLEND_SCALAR(field) field = Math::Mix(field, Other.field, Weight)
#define BLEND_VECTOR(field)   field = Math::Mix(field, Other.field, Weight)
#define BLEND_IF_ENABLED(flag, field) if (Other.flag || flag) { flag = true; BLEND_SCALAR(field); }
#define BLEND_IF_ENABLED_VECTOR(flag, field) if (Other.flag || flag) { flag = true; BLEND_VECTOR(field); }

	void HEffectsSettings::BlendWith(const HEffectsSettings& Other, float Weight)
	{
		// Sky
		BLEND_VECTOR(Sky.BetaRayleigh);
		BLEND_VECTOR(Sky.BetaMie);
		BLEND_VECTOR(Sky.BetaOzone);

		BLEND_SCALAR(Sky.PlanetRadiusKm);
		BLEND_SCALAR(Sky.AtmosphereHeightKm);
		BLEND_SCALAR(Sky.RayleightHeightKm);
		BLEND_SCALAR(Sky.MieHeight);
		BLEND_SCALAR(Sky.OzonePeak);
		BLEND_SCALAR(Sky.OzoneFalloff);
		BLEND_SCALAR(Sky.SunIlluminance);
		BLEND_SCALAR(Sky.MoonIlluminance);
		BLEND_SCALAR(Sky.SpaceIlluminance);
		BLEND_SCALAR(Sky.Exposure);

		BLEND_VECTOR(Sky.SunDiscColour);
		BLEND_SCALAR(Sky.SunAngularRadius);
		BLEND_SCALAR(Sky.SunDiscSoftness);

		BLEND_SCALAR(Sky.StarsBrightness);
		BLEND_SCALAR(Sky.StarsDensity);

		// Note: don't blend sample counts — usually discrete/static
		// Keep higher one?
		Sky.Samples = Math::Max(Sky.Samples, Other.Sky.Samples);
		Sky.LightSamples = Math::Max(Sky.LightSamples, Other.Sky.LightSamples);

		// Colour Correction
		BLEND_VECTOR(ColourCorrection.Lift);
		BLEND_VECTOR(ColourCorrection.Gamma);
		BLEND_VECTOR(ColourCorrection.Gain);
		BLEND_VECTOR(ColourCorrection.Offset);
		BLEND_SCALAR(ColourCorrection.Exposure);
		BLEND_SCALAR(ColourCorrection.Contrast);
		BLEND_SCALAR(ColourCorrection.Saturation);
		BLEND_SCALAR(ColourCorrection.HueShift);
		BLEND_SCALAR(ColourCorrection.Temperature);
		BLEND_SCALAR(ColourCorrection.Tint);

		// Depth of Field
		BLEND_IF_ENABLED(DepthOfField.EnableDoF, DepthOfField.FStop);
		BLEND_IF_ENABLED(DepthOfField.EnableDoF, DepthOfField.FocusDistance);
		BLEND_IF_ENABLED(DepthOfField.EnableDoF, DepthOfField.SensorSize);

		// Film Grain
		BLEND_IF_ENABLED(FilmGrain.EnableGrain, FilmGrain.GrainScale);
		BLEND_IF_ENABLED(FilmGrain.EnableGrain, FilmGrain.GrainAmount);

		// Chromatic Aberration
		BLEND_IF_ENABLED(ChromaticAberration.EnableChromaticAberration, ChromaticAberration.ChromaticAberrationAmount);

		// Vignette
		BLEND_IF_ENABLED(Vignette.EnableVignette, Vignette.Vignette);
	}

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Reflection type definitions

using namespace Columbus;

CREFLECT_STRUCT_BEGIN(HSkySettings)
	CREFLECT_STRUCT_FIELD(Vector4, BetaRayleigh, "")
	CREFLECT_STRUCT_FIELD(Vector4, BetaMie, "")
	CREFLECT_STRUCT_FIELD(Vector4, BetaOzone, "")

	CREFLECT_STRUCT_FIELD(int, Samples, "")
	CREFLECT_STRUCT_FIELD(int, LightSamples, "")

	CREFLECT_STRUCT_FIELD(float, PlanetRadiusKm, "")
	CREFLECT_STRUCT_FIELD(float, AtmosphereHeightKm, "")
	CREFLECT_STRUCT_FIELD(float, RayleightHeightKm, "")
	CREFLECT_STRUCT_FIELD(float, MieHeight, "")
	CREFLECT_STRUCT_FIELD(float, OzonePeak, "")
	CREFLECT_STRUCT_FIELD(float, OzoneFalloff, "")

	CREFLECT_STRUCT_FIELD(float, SunIlluminance, "")
	CREFLECT_STRUCT_FIELD(float, MoonIlluminance, "")
	CREFLECT_STRUCT_FIELD(float, SpaceIlluminance, "")
	CREFLECT_STRUCT_FIELD(float, Exposure, "")

	CREFLECT_STRUCT_FIELD(Vector4, SunDiscColour, "Colour,HDR")
	CREFLECT_STRUCT_FIELD(float, SunAngularRadius, "")
	CREFLECT_STRUCT_FIELD(float, SunDiscSoftness, "")

	CREFLECT_STRUCT_FIELD(float, StarsBrightness, "")
	CREFLECT_STRUCT_FIELD(float, StarsDensity, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HDepthOfFieldSettings)
	CREFLECT_STRUCT_FIELD(bool, EnableDoF, "")
	CREFLECT_STRUCT_FIELD(float, FStop, "")
	CREFLECT_STRUCT_FIELD(float, FocusDistance, "")
	CREFLECT_STRUCT_FIELD(float, SensorSize, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HFilmGrainSettings)
	CREFLECT_STRUCT_FIELD(bool, EnableGrain, "")
	CREFLECT_STRUCT_FIELD(float, GrainScale, "")
	CREFLECT_STRUCT_FIELD(float, GrainAmount, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HChromaticAberrationSettings)
	CREFLECT_STRUCT_FIELD(bool, EnableChromaticAberration, "")
	CREFLECT_STRUCT_FIELD(float, ChromaticAberrationAmount, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HVignetteSettings)
	CREFLECT_STRUCT_FIELD(bool, EnableVignette, "")
	CREFLECT_STRUCT_FIELD(float, Vignette, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HColourCorrectionSettings)
	CREFLECT_STRUCT_FIELD(Vector4, Lift, "Colour,HDR")
	CREFLECT_STRUCT_FIELD(Vector4, Gamma, "Colour,HDR")
	CREFLECT_STRUCT_FIELD(Vector4, Gain, "Colour,HDR")
	CREFLECT_STRUCT_FIELD(Vector4, Offset, "Colour,HDR")

	CREFLECT_STRUCT_FIELD(float, Exposure, "")
	CREFLECT_STRUCT_FIELD(float, Contrast, "")
	CREFLECT_STRUCT_FIELD(float, Saturation, "")
	CREFLECT_STRUCT_FIELD(float, HueShift, "")
	CREFLECT_STRUCT_FIELD(float, Temperature, "")
	CREFLECT_STRUCT_FIELD(float, Tint, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HEffectsSettings)
	CREFLECT_STRUCT_FIELD(HSkySettings, Sky, "")
	CREFLECT_STRUCT_FIELD(HColourCorrectionSettings, ColourCorrection, "")
	CREFLECT_STRUCT_FIELD(HDepthOfFieldSettings, DepthOfField, "")
	CREFLECT_STRUCT_FIELD(HFilmGrainSettings, FilmGrain, "")
	CREFLECT_STRUCT_FIELD(HChromaticAberrationSettings, ChromaticAberration, "")
	CREFLECT_STRUCT_FIELD(HVignetteSettings, Vignette, "")
CREFLECT_STRUCT_END()
