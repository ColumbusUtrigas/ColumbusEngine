file(GLOB_RECURSE SHADER_SOURCES *.glsl)
file(GLOB_RECURSE SHADER_SOURCES *.hlsl)

compile_shader_hlsl(Tonemap.hlsl     vs,ps SHADER_BINARIES)
compile_shader_hlsl(ComputeColourGradingLUT.hlsl cs SHADER_BINARIES)

compile_shader_glsl(GBufferPass.glsl vs,ps SHADER_BINARIES)
compile_shader_glsl(GBufferLightingPass.glsl cs SHADER_BINARIES)
compile_shader_glsl(PathTrace.glsl   rgen,rmiss,rchit SHADER_BINARIES)
compile_shader_glsl(RayTracedShadowsPass.glsl rgen,rmiss,rchit SHADER_BINARIES)

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES} SOURCES ${SHADER_SOURCES})