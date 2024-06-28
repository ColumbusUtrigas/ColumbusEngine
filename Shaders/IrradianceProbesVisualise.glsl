#version 460 core
#extension GL_GOOGLE_include_directive : require

layout(push_constant) uniform params
{
	// TODO: use view cbuffer?
	mat4 View, Projection;

	// TODO: move to appropriate place
	vec4 Position;
	vec4 Extent;
	ivec4 ProbesCount;
	ivec4 ProbeIndex;

	vec4 TestPointPosition;

	// how it should be
	// uint ViewId;
	// uint VolumeId;
} Params;

#define IRRADIANCE_PROBES_SET 0
#include "IrradianceProbeCommon.glsl"
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

		vec3 probe_pos = vec3(0);
		float probe_size = 0.2;

		if (OutProbeId >= Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z)
		{
			if (OutProbeId == Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z)
			{
				probe_pos = Params.TestPointPosition.xyz;
				probe_size = 0.3;
			}
			else
			{
				IrradianceVolume Volume;
				Volume.Position = Params.Position.xyz;
				Volume.Extent = Params.Extent.xyz;
				Volume.ProbesCount = Params.ProbesCount.xyz;

				IrradianceProbe Probe = GetClosestIrradianceProbe(Volume, Params.TestPointPosition.xyz);

				vec3 PosRelativeToProbe = Params.TestPointPosition.xyz - Probe.Position;
				vec3 CellSize = Volume.Extent / Volume.ProbesCount;
				vec3 NextProbes = sign(PosRelativeToProbe) * CellSize;
				NextProbes = vec3(1);

				IrradianceProbe Neighbour1 = GetClosestIrradianceProbe(Volume, Params.TestPointPosition.xyz + NextProbes.x * vec3(1, 0, 0));
				IrradianceProbe Neighbour2 = GetClosestIrradianceProbe(Volume, Params.TestPointPosition.xyz + NextProbes.y * vec3(0, 1, 0));
				IrradianceProbe Neighbour3 = GetClosestIrradianceProbe(Volume, Params.TestPointPosition.xyz + NextProbes.z * vec3(0, 0, 1));

				if (OutProbeId - Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z == 1)
				{
					probe_pos = Neighbour1.Position;
				}
				if (OutProbeId - Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z == 2)
				{
					probe_pos = Neighbour2.Position;
				}
				if (OutProbeId - Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z == 3)
				{
					probe_pos = Neighbour3.Position;
				}

				probe_size = 0.4;
			}
		}
		else
		{
			probe_pos = IrradianceProbes.Probes[OutProbeId].Position;
		}

		OutVertex = Positions[vertex_id];
		// vec3 screen_pos = ViewMatrixInverse[0].xyz * OutVertex.x + ViewMatrixInverse[1].xyz * OutVertex.y;
		// probe_pos += screen_pos * sphere_size;

		mat4 ViewMatrixInverse = inverse(transpose(Params.View));
		vec3 screen_pos = ViewMatrixInverse[0].xyz * OutVertex.x + ViewMatrixInverse[1].xyz * OutVertex.y;

		probe_pos += screen_pos * probe_size;

		// mat4 V = Params.View;
		gl_Position = vec4(probe_pos, 1.0) * (Params.View * Params.Projection) * vec4(1, -1, 1, 1);

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
			// color += IrradianceProbes.Probes[InProbeId].FullIrradiance[i] * max(dot(IrradianceProbes.Probes[InProbeId].Directions[i], world_nor), 0);
		}
		// color /= 64.0;

		IrradianceVolume Volume;
		Volume.Position = Params.Position.xyz;
		Volume.Extent = Params.Extent.xyz;
		Volume.ProbesCount = Params.ProbesCount.xyz;

		if (InProbeId >= Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z)
		{
			if (InProbeId == Params.ProbesCount.x * Params.ProbesCount.y * Params.ProbesCount.z)
			{
				color = SampleIrradianceProbes(Volume, Params.TestPointPosition.xyz, world_nor);
			}
			else
			{
				color = vec3(1,0,0);
			}
		}
		else
		{
			color = SampleIrradianceProbe(IrradianceProbes.Probes[InProbeId], world_nor);
		}

		// for (int i = 0; i < 6; i++)
		// {
		// 	color += IrradianceProbes.Probes[InProbeId].Irradiance[i].rgb * (max(dot(IRRADIANCE_BASIS[i], world_nor), 0));
		// }
		// color /= 6.0;
		// color = IrradianceProbes.Probes[InProbeId].test.rgb * (1-max(dot(IRRADIANCE_BASIS[0], world_nor), 0));

		// IrradianceData ir_data = load_irradiance_cell(cellOffset, world_nor);
		// FragColor = vec4(compute_irradiance(world_nor, ir_data), 1.0);

		// RT0 = vec4(1,0,0,1);
		// RT0 = vec4(world_nor, 1);
		RT0 = vec4(color, 1);
	}
#endif
