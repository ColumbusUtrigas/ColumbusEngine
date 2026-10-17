# NVIDIA SHaRC 1.8.3

Upstream: https://github.com/NVIDIA-RTX/SHARC
Pinned revision: 4e21b585c33c83d723ca9a1e11bbb1090d145793

The upstream shader headers are unmodified. See License.md for NVIDIA's license.
Engine integration lives in Shaders/RadianceCache and Graphics/RenderPasses/RadianceCache.*.
Directional encoding uses 32-byte accumulation and 24-byte resolved entries.
