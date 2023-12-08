#include "GraphicsCore.h"
#include "Core/Hash.h"

namespace Columbus
{

	size_t HashTextureDesc2::operator()(const TextureDesc2& X) const
	{
		size_t Hash = 0;

		hash_combine(Hash, X.Type);
		hash_combine(Hash, X.Usage);
		hash_combine(Hash, X.Width);
		hash_combine(Hash, X.Height);
		hash_combine(Hash, X.Depth);
		hash_combine(Hash, X.Mips);
		hash_combine(Hash, X.ArrayLayers);
		hash_combine(Hash, X.Samples);
		hash_combine(Hash, X.Format);

		hash_combine(Hash, X.Anisotropy);
		hash_combine(Hash, X.MinLOD);
		hash_combine(Hash, X.MaxLOD);
		hash_combine(Hash, X.AddressU);
		hash_combine(Hash, X.AddressV);
		hash_combine(Hash, X.AddressW);
		hash_combine(Hash, X.MinFilter);
		hash_combine(Hash, X.MagFilter);
		hash_combine(Hash, X.MipFilter);

		return Hash;
	}

	size_t HashBufferDesc::operator()(const BufferDesc& X) const
	{
		size_t Hash = 0;

		hash_combine(Hash, X.Size);
		hash_combine(Hash, X.Usage);
		hash_combine(Hash, X.BindFlags);
		hash_combine(Hash, X.CpuAccess);
		hash_combine(Hash, X.MiscFlags);
		hash_combine(Hash, X.StructureByteStride);
		hash_combine(Hash, X.UsedInAccelerationStructure);
		hash_combine(Hash, X.HostVisible);

		return Hash;
	}

}
