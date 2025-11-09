struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

// TODO: move away from big push constants, use ID + cbuffer
[[vk::push_constant]]
struct _Params {
	float4x4 Model;
	float4x4 VP;
	float4 Colour;

	// Vertices[0].x is also radius, // Vertices[0].y is also height
	float4 Vertices[3];
	
    uint64_t VertexBuffer;
    uint64_t IndexBuffer;

	uint Type;
} Params;

#define TYPE_BOX 0
#define TYPE_TRI 1
#define TYPE_SPHERE 2
#define TYPE_CONE 3
#define TYPE_CYLINDER 4
#define TYPE_CAPSULE 5
#define TYPE_MESH 6

static const float3 pos[8] = {
	float3(-1, -1, -1),
	float3(+1, -1, -1),
	float3(+1, -1, +1),
	float3(-1, -1, +1),

	float3(-1, +1, -1),
	float3(+1, +1, -1),
	float3(+1, +1, +1),
	float3(-1, +1, +1),
};

static const int index[36] = {
	0, 1, 2,
	0, 2, 3,

	4, 5, 6,
	4, 6, 7,

	0, 4, 7,
	0, 3, 7,

	1, 5, 6,
	1, 2, 6,

	1, 5, 4,
	1, 0, 4,

	2, 6, 7,
	2, 3, 7,
};

// sphere tessellation (adjust as you like)
static const uint SPHERE_SLICES = 32;  // around Y
static const uint SPHERE_STACKS = 16;  // bottom->top
// total verts to draw on CPU side = SPHERE_SLICES * SPHERE_STACKS * 6

// cone tessellation
static const uint CONE_SLICES = 32;
// total verts: side (slices*3) + cap (slices*3) = slices*6
static const float CONE_HEIGHT = 1.0f; // tip at y=+1, base at y=0

// cylinder tessellation
static const uint CYL_SLICES = 32;
static const float CYL_HALF_HEIGHT = 1.0f;
// side: slices * 6
// caps: 2 * slices * 3
// total: slices * 12

// capsule tessellation
static const uint CAP_SLICES = 32;
static const uint CAP_HEMI_STACKS = 8; // per hemisphere
static const uint CAP_CYL_STACKS = 1; // just a band
// per hemi: CAP_SLICES * CAP_HEMI_STACKS * 6
// cylinder: CAP_SLICES * CAP_CYL_STACKS * 6
// total: 2*hemi + cyl
static const float CAP_HALF_HEIGHT = 1.0f; // cylinder from -1 to +1, hemispheres on ends


VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	float4 vertex;

	if (Params.Type == TYPE_BOX)
	{
		vertex = float4(pos[index[VertexID]] * 0.5f, 1.0f);
	}
	else if (Params.Type == TYPE_TRI)
	{
		// expect Draw(3,0)
		vertex = Params.Vertices[VertexID];
	}
	else if (Params.Type == TYPE_SPHERE)
	{
		// same pattern as before: each cell = 2 tris = 6 verts
		uint vertsPerCell = 6;
		uint cellIndex = VertexID / vertsPerCell;
		uint vertInCell = VertexID % vertsPerCell;

		uint slice = cellIndex % SPHERE_SLICES;
		uint stack = cellIndex / SPHERE_SLICES;

		float u0 = (float) slice / SPHERE_SLICES;
		float u1 = (float)(slice + 1) / SPHERE_SLICES;
		float v0 = (float) stack / SPHERE_STACKS;
		float v1 = (float)(stack + 1) / SPHERE_STACKS;

		float2 uv;
		// triangle 0
		if (vertInCell == 0) uv = float2(u0, v0);
		else if (vertInCell == 1) uv = float2(u1, v0);
		else if (vertInCell == 2) uv = float2(u1, v1);
		// triangle 1
		else if (vertInCell == 3) uv = float2(u0, v0);
		else if (vertInCell == 4) uv = float2(u1, v1);
		else /*5*/                uv = float2(u0, v1);

		// map to sphere (unit)
		float lon = uv.x * 6.2831853;  // 0..2pi
		float lat = uv.y * 3.1415926;  // 0..pi

		float3 p;
		p.x = sin(lat) * cos(lon);
		p.y = cos(lat);
		p.z = sin(lat) * sin(lon);

		vertex = float4(p, 1.0f);
	}
	else if (Params.Type == TYPE_CONE)
	{
		// layout:
		// 0 .. CONE_SLICES*3-1        -> side
		// CONE_SLICES*3 .. *6 - 1     -> base cap
		uint sideVerts = CONE_SLICES * 3;
		if (VertexID < sideVerts)
		{
			uint tri = VertexID / 3;
			uint vInTri = VertexID % 3;

			uint slice = tri; // 0..slices-1
			uint nextSlice = (slice + 1) % CONE_SLICES;

			float angle0 = (float) slice / CONE_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CONE_SLICES * 6.2831853;

			float3 base0 = float3(cos(angle0), 0.0, sin(angle0));
			float3 base1 = float3(cos(angle1), 0.0, sin(angle1));
			float3 tip = float3(0.0, CONE_HEIGHT, 0.0);

			float3 p;
			if (vInTri == 0)
				p = base0;
			else if (vInTri == 1)
				p = base1;
			else
				p = tip;

			vertex = float4(p, 1.0);
		}
		else
		{
			// cap triangles, fan around center at y=0
			uint baseIndex = VertexID - sideVerts;
			uint tri = baseIndex / 3;
			uint vInTri = baseIndex % 3;

			uint slice = tri;
			uint nextSlice = (slice + 1) % CONE_SLICES;

			float angle0 = (float) slice / CONE_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CONE_SLICES * 6.2831853;

			float3 center = float3(0, 0, 0);
			float3 rim0 = float3(cos(angle0), 0.0, sin(angle0));
			float3 rim1 = float3(cos(angle1), 0.0, sin(angle1));

			float3 p;
			if (vInTri == 0)
				p = center;
			else if (vInTri == 1)
				p = rim1;
			else
				p = rim0;

			vertex = float4(p, 1.0);
		}
		
        vertex.xz *= Params.Vertices[0].x; // radius
        vertex.y *= Params.Vertices[0].y; // height
    }
	else if (Params.Type == TYPE_CYLINDER)
	{
		// layout:
		// 1) side: CYL_SLICES * 6
		// 2) top cap: CYL_SLICES * 3
		// 3) bottom cap: CYL_SLICES * 3
		uint sideVerts = CYL_SLICES * 6;
		uint topCapVerts = CYL_SLICES * 3;
		uint botCapVerts = CYL_SLICES * 3;

		if (VertexID < sideVerts)
		{
			// quad -> 2 tris
			uint cell = VertexID / 6;
			uint vInCell = VertexID % 6;

			uint slice = cell; // 0..slices-1
			uint nextSlice = (slice + 1) % CYL_SLICES;

			float angle0 = (float) slice / CYL_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CYL_SLICES * 6.2831853;

			float3 b0 = float3(cos(angle0), -CYL_HALF_HEIGHT, sin(angle0));
			float3 b1 = float3(cos(angle1), -CYL_HALF_HEIGHT, sin(angle1));
			float3 t0 = float3(cos(angle0), +CYL_HALF_HEIGHT, sin(angle0));
			float3 t1 = float3(cos(angle1), +CYL_HALF_HEIGHT, sin(angle1));

			float3 p;
			// tri 0: b0, b1, t1
			// tri 1: b0, t1, t0
			if (vInCell == 0)
				p = b0;
			else if (vInCell == 1)
				p = b1;
			else if (vInCell == 2)
				p = t1;
			else if (vInCell == 3)
				p = b0;
			else if (vInCell == 4)
				p = t1;
			else
				p = t0;

			vertex = float4(p, 1.0);
		}
		else if (VertexID < sideVerts + topCapVerts)
		{
			// top cap at y=+CYL_HALF_HEIGHT
			uint baseIndex = VertexID - sideVerts;
			uint tri = baseIndex / 3;
			uint vInTri = baseIndex % 3;

			uint slice = tri;
			uint nextSlice = (slice + 1) % CYL_SLICES;

			float angle0 = (float) slice / CYL_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CYL_SLICES * 6.2831853;

			float3 center = float3(0, +CYL_HALF_HEIGHT, 0);
			float3 rim0 = float3(cos(angle0), +CYL_HALF_HEIGHT, sin(angle0));
			float3 rim1 = float3(cos(angle1), +CYL_HALF_HEIGHT, sin(angle1));

			float3 p;
			if (vInTri == 0)
				p = center;
			else if (vInTri == 1)
				p = rim0;
			else
				p = rim1;

			vertex = float4(p, 1.0);
		}
		else
		{
			// bottom cap at y=-CYL_HALF_HEIGHT
			uint baseIndex = VertexID - sideVerts - topCapVerts;
			uint tri = baseIndex / 3;
			uint vInTri = baseIndex % 3;

			uint slice = tri;
			uint nextSlice = (slice + 1) % CYL_SLICES;

			float angle0 = (float) slice / CYL_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CYL_SLICES * 6.2831853;

			float3 center = float3(0, -CYL_HALF_HEIGHT, 0);
			float3 rim0 = float3(cos(angle0), -CYL_HALF_HEIGHT, sin(angle0));
			float3 rim1 = float3(cos(angle1), -CYL_HALF_HEIGHT, sin(angle1));

			float3 p;
			// flip winding if needed
			if (vInTri == 0)
				p = center;
			else if (vInTri == 1)
				p = rim1;
			else
				p = rim0;

			vertex = float4(p, 1.0);
		}
		
        vertex.xz *= Params.Vertices[0].x; // radius
        vertex.y *= Params.Vertices[0].y * 0.5f; // height
    }
	else if (Params.Type == TYPE_CAPSULE)
	{
		// order:
		// 1) top hemisphere
		// 2) cylinder band
		// 3) bottom hemisphere
		uint hemiVerts = CAP_SLICES * CAP_HEMI_STACKS * 6;
		uint cylVerts = CAP_SLICES * CAP_CYL_STACKS * 6;

		if (VertexID < hemiVerts)
		{
			// top hemisphere centered at y=+CAP_HALF_HEIGHT
			uint vertsPerCell = 6;
			uint cellIndex = VertexID / vertsPerCell;
			uint vInCell = VertexID % vertsPerCell;

			uint slice = cellIndex % CAP_SLICES;
			uint stack = cellIndex / CAP_SLICES;

			float u0 = (float) slice / CAP_SLICES;
			float u1 = (float) (slice + 1) / CAP_SLICES;
			float v0 = (float) stack / CAP_HEMI_STACKS;
			float v1 = (float) (stack + 1) / CAP_HEMI_STACKS;

			float2 uv;
			if (vInCell == 0)
				uv = float2(u0, v0);
			else if (vInCell == 1)
				uv = float2(u1, v0);
			else if (vInCell == 2)
				uv = float2(u1, v1);
			else if (vInCell == 3)
				uv = float2(u0, v0);
			else if (vInCell == 4)
				uv = float2(u1, v1);
			else
				uv = float2(u0, v1);

			float lon = uv.x * 6.2831853;
			float lat = uv.y * (3.1415926 * 0.5); // 0..pi/2

			float3 p;
			p.x = sin(lat) * cos(lon);
			p.y = cos(lat);
			p.z = sin(lat) * sin(lon);

			p.y += CAP_HALF_HEIGHT; // move to top
			vertex = float4(p, 1.0);
		}
		else if (VertexID < hemiVerts + cylVerts)
		{
			// cylinder between -CAP_HALF_HEIGHT and +CAP_HALF_HEIGHT
			uint baseIndex = VertexID - hemiVerts;
			uint cell = baseIndex / 6;
			uint vInCell = baseIndex % 6;

			uint slice = cell; // only 1 stack
			uint nextSlice = (slice + 1) % CAP_SLICES;

			float angle0 = (float) slice / CAP_SLICES * 6.2831853;
			float angle1 = (float) nextSlice / CAP_SLICES * 6.2831853;

			float3 b0 = float3(cos(angle0), -CAP_HALF_HEIGHT, sin(angle0));
			float3 b1 = float3(cos(angle1), -CAP_HALF_HEIGHT, sin(angle1));
			float3 t0 = float3(cos(angle0), +CAP_HALF_HEIGHT, sin(angle0));
			float3 t1 = float3(cos(angle1), +CAP_HALF_HEIGHT, sin(angle1));

			float3 p;
			if (vInCell == 0)
				p = b0;
			else if (vInCell == 1)
				p = b1;
			else if (vInCell == 2)
				p = t1;
			else if (vInCell == 3)
				p = b0;
			else if (vInCell == 4)
				p = t1;
			else
				p = t0;

			vertex = float4(p, 1.0);
		}
		else
		{
			// bottom hemisphere centered at y=-CAP_HALF_HEIGHT
			uint baseIndex = VertexID - hemiVerts - cylVerts;
			uint vertsPerCell = 6;
			uint cellIndex = baseIndex / vertsPerCell;
			uint vInCell = baseIndex % vertsPerCell;

			uint slice = cellIndex % CAP_SLICES;
			uint stack = cellIndex / CAP_SLICES;

			float u0 = (float) slice / CAP_SLICES;
			float u1 = (float) (slice + 1) / CAP_SLICES;
			float v0 = (float) stack / CAP_HEMI_STACKS;
			float v1 = (float) (stack + 1) / CAP_HEMI_STACKS;

			float2 uv;
			if (vInCell == 0)
				uv = float2(u0, v0);
			else if (vInCell == 1)
				uv = float2(u1, v0);
			else if (vInCell == 2)
				uv = float2(u1, v1);
			else if (vInCell == 3)
				uv = float2(u0, v0);
			else if (vInCell == 4)
				uv = float2(u1, v1);
			else
				uv = float2(u0, v1);

			float lon = uv.x * 6.2831853;
			float lat = uv.y * (3.1415926 * 0.5); // 0..pi/2

			float3 p;
			p.x = sin(lat) * cos(lon);
			p.y = -cos(lat); // flipped
			p.z = sin(lat) * sin(lon);

			p.y -= CAP_HALF_HEIGHT; // move to bottom
			vertex = float4(p, 1.0);
		}
		
        float radius = Params.Vertices[0].x;
        float height = Params.Vertices[0].y;
		
        vertex.xz *= radius;
		
		// the cylinder part runs from -halfHeight to +halfHeight
		// hemispheres extend beyond that by ±radius
        float halfHeight = 0.5f * (height - 2.0f * radius);
        vertex.y *= radius; // scale sphere shape
        vertex.y += sign(vertex.y) * halfHeight; // offset top/bottom hemispheres
    }
    else if (Params.Type == TYPE_MESH)
    {
        uint index = vk::RawBufferLoad(Params.IndexBuffer + VertexID * 4);
        vertex = float4(vk::RawBufferLoad<float3>(Params.VertexBuffer + index * 12), 1);
    }
	else
	{
		// fallback: just origin
		vertex = float4(0,0,0,1);
	}

	VS_TO_PS Out;
	float4 World = mul(vertex, Params.Model);
	Out.Pos = mul(World, Params.VP) * float4(1, -1, 1, 1);
	return Out;
}

/*VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	float4 vertex = float4(pos[index[VertexID]] * 0.5f, 1);

	if (Params.Type == TYPE_TRI)
	{
		vertex = Params.Vertices[VertexID];
	}

	VS_TO_PS Out;
	float4 World = mul(vertex, Params.Model);
	Out.Pos = mul(World, Params.VP) * float4(1, -1, 1, 1);
	return Out;
}*/

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
	return Params.Colour;
}