file(GLOB_RECURSE SHADER_SOURCES *.glsl)

compile_shader_glsl(Tonemap.glsl     vs,ps SHADER_BINARIES)
compile_shader_glsl(GBufferPass.glsl vs,ps SHADER_BINARIES)
compile_shader_glsl(GBufferLightingPass.glsl cs SHADER_BINARIES)
compile_shader_glsl(PathTrace.glsl   rgen,rmiss,rchit SHADER_BINARIES)
compile_shader_glsl(RayTracedShadowsPass.glsl rgen,rmiss,rchit SHADER_BINARIES)

add_custom_target(Shaders DEPENDS ${SHADER_BINARIES} SOURCES ${SHADER_SOURCES})