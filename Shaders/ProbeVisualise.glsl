#version 460 core

layout(push_constant) uniform params
{
	mat4 View, Projection;
} Params;

#define IRRADIANCE_PROBES_SET 0
#include <IrradianceProbeCommon.glsl>
// IRRADIANCE_PROBES_BUFFER(0) // set 0

#ifdef VERTEX_SHADER
	const vec2 Positions[6] = vec2[6](vec2(-1.0, -1.0),
                                vec2(1.0, -1.0),
                                vec2(-1.0, 1.0),

                                vec2(1.0, -1.0),
                                vec2(1.0, 1.0),
                                vec2(-1.0, 1.0));


	layout(location=0) out vec2 OutVertex;
	layout(location=1) out flat int OutProbeId;

	void main()
	{
		OutProbeId = gl_VertexIndex / 6;
		int vertex_id = gl_VertexIndex % 6;
		vec3 probe_pos = IrradianceProbes.Probes[OutProbeId].Position;

		OutVertex = Positions[vertex_id];
		// vec3 screen_pos = ViewMatrixInverse[0].xyz * OutVertex.x + ViewMatrixInverse[1].xyz * OutVertex.y;
		// probe_pos += screen_pos * sphere_size;

		mat4 ViewMatrixInverse = inverse(Params.View);
		vec3 screen_pos = ViewMatrixInverse[0].xyz * OutVertex.x + ViewMatrixInverse[1].xyz * OutVertex.y;

		probe_pos += screen_pos * 10;

		// mat4 V = Params.View;
		gl_Position = Params.Projection * Params.View * vec4(probe_pos, 1.0) * vec4(1, -1, 1, 1);

		// gl_Position = ProjectionMatrix * (ViewMatrix * vec4(ws_cell_location, 1.0));
		// gl_Position.z += 0.0001; /* Small bias to let the icon draw without zfighting */
	}
#endif

#ifdef PIXEL_SHADER
	layout(location=0) in vec2 InVertex;
	layout(location=1) in flat int InProbeId;

	layout(location=0) out vec4 RT0;

	void main()
	{
		float dist_sqr = dot(InVertex, InVertex);

		if (dist_sqr > 1.0) {
			discard;
			return;
		}

		vec3 view_nor = vec3(InVertex, sqrt(max(0.0, 1.0 - dist_sqr)));
		vec3 world_nor = mat3(inverse(Params.View)) * view_nor;

		vec3 color = vec3(0);
		for (int i = 0; i < 64; i++)
		{
			// color += IrradianceProbes.Probes[InProbeId].Directions[i] * max(dot(IrradianceProbes.Probes[InProbeId].Directions[i], world_nor), 0);
			color += IrradianceProbes.Probes[InProbeId].FullIrradiance[i] * max(dot(IrradianceProbes.Probes[InProbeId].Directions[i], world_nor), 0);
		}
		color /= 64.0;

		for (int i = 0; i < 6; i++)
		{
			// color += IrradianceProbes.Probes[InProbeId].Irradiance[i].rgb * (max(dot(IRRADIANCE_BASIS[i], world_nor), 0));
		}
		// color /= 6.0;
		// color = IrradianceProbes.Probes[InProbeId].test.rgb * (1-max(dot(IRRADIANCE_BASIS[0], world_nor), 0));

		// IrradianceData ir_data = load_irradiance_cell(cellOffset, world_nor);
		// FragColor = vec4(compute_irradiance(world_nor, ir_data), 1.0);

		// RT0 = vec4(1,0,0,1);
		// RT0 = vec4(world_nor, 1);
		RT0 = vec4(color, 1);
	}
#endif
