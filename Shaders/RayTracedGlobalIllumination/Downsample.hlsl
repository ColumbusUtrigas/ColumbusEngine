#include "Geometry.hlsli"

[[vk::binding(0, 0)]] Texture2D<float> Depth;
[[vk::binding(1, 0)]] Texture2D<float2> Normals;
[[vk::binding(2, 0)]] [[vk::image_format("r32f")]] RWTexture2D<float> OutputDepth;
[[vk::binding(3, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutputNormals;
[[vk::binding(4, 0)]] [[vk::image_format("rgba32f")]] RWTexture2D<float4> OutputSurfaceData;

[numthreads(8, 8, 1)]
void main(int2 Tile : SV_DispatchThreadID)
{
    if (!RTGIValidTile(Tile))
    {
        return;
    }

    // Stable stratification avoids choosing the foreground in every tile of a
    // dense fence/leaf pattern. One source pixel still produces exactly one ray;
    // the point does not move between frames or need an adaptive allocation.
    const uint Seed       = Random::Hash(uint(Tile.x) ^ Random::Hash(uint(Tile.y)));
    const int2 TileOrigin = Tile * Geometry.Config.x;
    const int2 TileExtent = min(Geometry.Config.x, Geometry.Sizes.xy - TileOrigin);
    const int2 Offset     = int2(Seed % uint(TileExtent.x), (Seed >> 16) % uint(TileExtent.y));
    int2  SelectedPixel = TileOrigin + Offset;
    float SelectedDepth = Depth[SelectedPixel];

    // A tile straddling the sky still gets a probe if any visible surface exists.
    if (IsSkyDepth(SelectedDepth))
    {
        for (int y = 0; y < TileExtent.y; ++y)
        {
            for (int x = 0; x < TileExtent.x; ++x)
            {
                const int2 Pixel = TileOrigin + int2(x, y);
                const float CandidateDepth = Depth[Pixel];

                if (CandidateDepth > SelectedDepth)
                {
                    SelectedDepth = CandidateDepth;
                    SelectedPixel = Pixel;
                }
            }
        }
    }

    const float2 EncodedNormal = Normals[SelectedPixel];
    const bool Valid = !IsSkyDepth(SelectedDepth);
    const float3 GeometricNormal = Valid ? NormalDecode(EncodedNormal) : float3(0, 1, 0);
    const float3 Position = Valid ? RTGIGetSurface(SelectedPixel, SelectedDepth, EncodedNormal).Position : 0;

    OutputDepth[Tile]       = SelectedDepth;
    OutputNormals[Tile]     = float4(GeometricNormal, Valid ? max(0.0005, distance(Position, Geometry.CameraPositionPixelSize.xyz) * Geometry.CameraPositionPixelSize.w) : 0);
    OutputSurfaceData[Tile] = float4(Position, RTGIPackPixel(SelectedPixel));
}
