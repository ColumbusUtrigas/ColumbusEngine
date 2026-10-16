#pragma once

#include "Core/Types.h"
#include <cassert>
#include <optional>
#include <vector>

namespace Columbus
{
	// Persistent square buddy allocator. Metadata is bounded by the atlas/minimum block sizes;
	// freeing four siblings coalesces them without moving any live allocation.
	class ShadowAtlasAllocator
	{
	public:
		struct Allocation
		{
			u32 X    = 0;
			u32 Y    = 0;
			u32 Size = 0;

			bool operator==(const Allocation&) const = default;
		};

		explicit ShadowAtlasAllocator(u32 AtlasSize, u32 MinimumBlockSize = 32)
		    : Size(AtlasSize), MinimumSize(MinimumBlockSize)
		{
			assert(AtlasSize >= MinimumBlockSize && (AtlasSize & (AtlasSize - 1)) == 0);
			assert(MinimumBlockSize > 0 && (MinimumBlockSize & (MinimumBlockSize - 1)) == 0);

			size_t Count = 1;
			size_t Level = 1;

			for (u32 Side = AtlasSize; Side > MinimumBlockSize; Side /= 2)
			{
				Level *= 4;
				Count += Level;
			}

			Nodes.resize(Count, FreeNode);
		}

		std::optional<Allocation> Allocate(u32 RequestedSize)
		{
			if (RequestedSize < MinimumSize || RequestedSize > Size || (RequestedSize & (RequestedSize - 1)) != 0)
			{
				return {};
			}

			return AllocateNode(0, 0, 0, Size, RequestedSize);
		}

		bool Free(const Allocation& Block)
		{
			if (Block.Size < MinimumSize || Block.Size > Size || (Block.Size & (Block.Size - 1)) != 0 ||
			    Block.X % Block.Size != 0 || Block.Y % Block.Size != 0 || Block.X > Size - Block.Size ||
			    Block.Y > Size - Block.Size)
			{
				return false;
			}

			return FreeNodeAt(0, 0, 0, Size, Block);
		}

	private:
		enum : u8
		{
			FreeNode,
			SplitNode,
			UsedNode
		};

		u32 Size;
		u32 MinimumSize;

		std::vector<u8> Nodes;

		std::optional<Allocation> AllocateNode(size_t Node, u32 X, u32 Y, u32 NodeSize, u32 RequestedSize)
		{
			if (Nodes[Node] == UsedNode)
			{
				return {};
			}

			if (NodeSize == RequestedSize)
			{
				if (Nodes[Node] != FreeNode)
				{
					return {};
				}

				Nodes[Node] = UsedNode;
				return Allocation{ X, Y, NodeSize };
			}

			const u32 Half = NodeSize / 2;

			for (u32 Child = 0; Child < 4; ++Child)
			{
				if (auto Result = AllocateNode(
				        Node * 4 + 1 + Child, X + (Child % 2) * Half, Y + (Child / 2) * Half, Half, RequestedSize))
				{
					Nodes[Node] = SplitNode;
					return Result;
				}
			}

			return {};
		}

		bool FreeNodeAt(size_t Node, u32 X, u32 Y, u32 NodeSize, const Allocation& Block)
		{
			if (NodeSize == Block.Size)
			{
				if (Nodes[Node] != UsedNode)
				{
					return false;
				}

				Nodes[Node] = FreeNode;
				return true;
			}

			if (Nodes[Node] != SplitNode)
			{
				return false;
			}

			const u32 Half   = NodeSize / 2;
			const u32 Column = Block.X >= X + Half ? 1 : 0;
			const u32 Row    = Block.Y >= Y + Half ? 1 : 0;

			if (!FreeNodeAt(Node * 4 + 1 + Row * 2 + Column, X + Column * Half, Y + Row * Half, Half, Block))
			{
				return false;
			}

			bool AllFree = true;

			for (size_t Child = 0; Child < 4; ++Child)
			{
				AllFree &= Nodes[Node * 4 + 1 + Child] == FreeNode;
			}

			if (AllFree)
			{
				Nodes[Node] = FreeNode;
			}

			return true;
		}
	};
}
