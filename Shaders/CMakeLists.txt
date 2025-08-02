file(GLOB_RECURSE SHADER_SOURCES *.hlsl *.glsl *.hlsli)
group_files_by_directory(SRCS ${SHADER_SOURCES})

# disable visual studio's default shader compilation that fails my thing
set_source_files_properties(SOURCE ${SHADER_SOURCES} PROPERTIES VS_SETTINGS "ExcludedFromBuild=true")

compile_shader_hlsl(Tonemap.hlsl     main:vs,main:ps SHADER_BINARIES)
compile_shader_hlsl(ComputeColourGradingLUT.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(DDGI/ProbeUpdate.hlsl RGS:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeBlendingRadiance.hlsl DDGIProbeBlendingCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeBlendingDistance.hlsl DDGIProbeBlendingCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeRelocation.hlsl DDGIProbeRelocationCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeRelocationReset.hlsl DDGIProbeRelocationResetCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeClassification.hlsl DDGIProbeClassificationCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeClassificationReset.hlsl DDGIProbeClassificationResetCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeVariabilityReduction.hlsl DDGIReductionCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/ProbeVariabilityExtraReduction.hlsl DDGIExtraReductionCS:cs SHADER_BINARIES)
compile_shader_hlsl(DDGI/Apply.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(RadianceCache/RadianceCacheTrace.hlsl RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)
compile_shader_hlsl(RadianceCache/RadianceCacheVisualise.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)

compile_shader_hlsl(RayTracedShadows/DenoiserShadowsPrepare.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedShadows/DenoiserShadowsTileClassification.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedShadows/DenoiserShadowsFilter.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedShadows/RayTraceShadows.hlsl RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)

compile_shader_hlsl(DenoiserReflection/Reproject.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(DenoiserReflection/Prefilter.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(DenoiserReflection/ResolveTemporal.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(RayTracedReflections/RayTraceReflections.hlsl RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)
compile_shader_hlsl(RayTracedReflections/ReflectionsResolve.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(RayTracedGlobalIllumination/RayTracedGI.hlsl RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)
compile_shader_hlsl(RayTracedGlobalIllumination/Downsample.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedGlobalIllumination/Upsample.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedGlobalIllumination/Spatial.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedGlobalIllumination/Temporal.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(TiledLightsPrepare.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(MemsetTexture.hlsl main:cs SHADER_BINARIES)

compile_shader_glsl(FSR1/FSR1CS.glsl main:cs SHADER_BINARIES)
compile_shader_glsl(FSR1/FSR1PS.glsl main:vs,main:ps SHADER_BINARIES)

compile_shader_glsl(LightmapRasterisation.glsl main:vs,main:ps SHADER_BINARIES)
compile_shader_hlsl(LightmapPathTracing.hlsl   RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)

compile_shader_hlsl(IrradianceVolume/IrradianceVolumeTrace.hlsl RGS:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)
compile_shader_hlsl(IrradianceVolume/IrradianceVolumeApply.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(GBufferPass.hlsl VSMain:vs,PSMain:ps SHADER_BINARIES)
compile_shader_hlsl(Decals.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(Particles.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(Sky.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(SkyLut.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(TAA.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(Debug.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(UI.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(Visualisation.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(GBufferLightingPass.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(PathTrace.hlsl   RayGen:rgen,Miss:rmiss,ClosestHit:rchit SHADER_BINARIES)

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES} SOURCES ${SHADER_SOURCES})
set_target_properties(Shaders PROPERTIES FOLDER "Engine")

add_custom_command(TARGET Shaders POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E echo "Copying compiled shaders into target directory"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${BINARIES_DIR}/$<CONFIG>/PrecompiledShaders/"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${BINARIES_DIR}/PrecompiledShaders/" "${BINARIES_DIR}/$<CONFIG>/PrecompiledShaders/"
)