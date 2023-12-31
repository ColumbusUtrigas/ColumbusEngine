file(GLOB_RECURSE SHADER_SOURCES *.glsl)
file(GLOB_RECURSE SHADER_SOURCES *.hlsl)

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

compile_shader_hlsl(RayTracedShadows/DenoiserShadowsPrepare.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedShadows/DenoiserShadowsTileClassification.hlsl main:cs SHADER_BINARIES)
compile_shader_hlsl(RayTracedShadows/DenoiserShadowsFilter.hlsl main:cs SHADER_BINARIES)

compile_shader_hlsl(TiledLightsPrepare.hlsl main:cs SHADER_BINARIES)

compile_shader_glsl(LightmapRasterisation.glsl main:vs,main:ps SHADER_BINARIES)
compile_shader_glsl(LightmapPathTracing.glsl   main:rgen,main:rmiss,main:rchit SHADER_BINARIES)

compile_shader_glsl(GBufferPass.glsl main:vs,main:ps SHADER_BINARIES)
compile_shader_hlsl(Decals.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_hlsl(Debug.hlsl Vertex:vs,Pixel:ps SHADER_BINARIES)
compile_shader_glsl(GBufferLightingPass.glsl main:cs SHADER_BINARIES)
compile_shader_glsl(PathTrace.glsl   main:rgen,main:rmiss,main:rchit SHADER_BINARIES)
compile_shader_glsl(RayTracedShadowsPass.glsl main:rgen,main:rmiss,main:rchit SHADER_BINARIES)

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES} SOURCES ${SHADER_SOURCES})